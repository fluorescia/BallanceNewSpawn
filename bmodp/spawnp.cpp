#include "spawnp.h"

IMod* BMLEntry(IBML* bml) {
	return new spawnp(bml);
}

void spawnp::OnLoad()
{
	cmdspawnp_ptr = new cmdspawnp();
	m_bml->RegisterCommand(cmdspawnp_ptr);
	GetConfig()->SetCategoryComment("Main", "Main settings.");
	prop_enabled = GetConfig()->GetProperty("Main", "Enabled");
	prop_enabled->SetDefaultBoolean(true);
	prop_enabled->SetComment("Enabled to use");
	enabled = prop_enabled->GetBoolean();

	reset_prop_enabled = GetConfig()->GetProperty("Main", "Prop_Reset");
	reset_prop_enabled->SetDefaultBoolean(false);
	reset_prop_enabled->SetComment("Enable to reset prop when press shortcut key");
	reset_enabled = reset_prop_enabled->GetBoolean();

	prop_key = GetConfig()->GetProperty("Main", "Shortcut");
	prop_key->SetDefaultKey(CKKEY_C);
	prop_key->SetComment("Set your shortcut key");
	key = prop_key->GetKey();

	input_manager = m_bml->GetInputManager();
	
}

void spawnp::OnModifyConfig(C_CKSTRING category, C_CKSTRING key, IProperty* prop) {
	//m_bml->SendIngameMessage("Config is modified.");
	enabled = prop_enabled->GetBoolean();
	reset_enabled = reset_prop_enabled->GetBoolean();
	if (prop == prop_key) {
		this->key = prop->GetKey();
	}
}

void spawnp::OnLoadScript(C_CKSTRING filename, CKBehavior* script) {
	if (strcmp(script->GetName(), "Gameplay_Ingame") == 0) {
		CKBehavior* ballMgr = ScriptHelper::FindFirstBB(script, "BallManager");
		CKBehavior* newBall = ScriptHelper::FindFirstBB(ballMgr, "New Ball");
		m_dynamicPos = ScriptHelper::FindNextBB(script, ballMgr, "TT Set Dynamic Position");
		m_phyNewBall = ScriptHelper::FindFirstBB(newBall, "physicalize new Ball");
	}
}

void spawnp::OnStartLevel() {
	//spawn sector set 1
	cmdspawnp_ptr->set_curSector(1);
	//spawn position set default
	m_bml->AddTimer(10ul, [this]() {
		VxMatrix matrix = m_bml->Get3dEntityByName("Cam_OrientRef")->GetWorldMatrix();
		for (int i = 0; i < 4; i++) {
			std::swap(matrix[0][i], matrix[2][i]);
			matrix[0][i] = -matrix[0][i];
		}
		cmdspawnp_ptr->set_matrix(matrix);
		//get part 1 in spawndata
		cmdspawnp_ptr->set_ball_type(ball_name_to_id[get_curBall()->GetName()]);
	});
}

void spawnp::OnBallNavActive()
{
	Ball_Active = true;
}

void spawnp::OnBallNavInactive()
{
	Ball_Active = false;
}

void spawnp::OnPostStartMenu()
{
	if (init)
		return;
	if (!m_curLevel) {
		m_curLevel = m_bml->GetArrayByName("CurrentLevel");
		m_checkpoints = m_bml->GetArrayByName("Checkpoints");
		m_resetpoints = m_bml->GetArrayByName("ResetPoints");
		m_ingameParam = m_bml->GetArrayByName("IngameParameter");
		CKBehavior* events = m_bml->GetScriptByName("Gameplay_Events");
		CKBehavior* id = ScriptHelper::FindNextBB(events, events->GetInput(0));
		m_curSector = id->GetOutputParameter(0)->GetDestination(0);
	}
	
	auto* script = m_bml->GetScriptByName("Gameplay_Ingame");
	CKBehavior* trafoMgr = ScriptHelper::FindFirstBB(script, "Trafo Manager");
	m_setNewBall = ScriptHelper::FindFirstBB(trafoMgr, "set new Ball");
	CKBehavior* sop = ScriptHelper::FindFirstBB(m_setNewBall, "Switch On Parameter");
	m_curTrafo = sop->GetInputParameter(0)->GetDirectSource();
	
	init = true;
}

void spawnp::OnProcess() {
	if (!(enabled && m_bml->IsCheatEnabled() && m_bml->IsPlaying() && Ball_Active))
		return;
	if (input_manager->IsKeyPressed(key)) {

		///sector judge
		//get spawn'sector
		// 
		//m_bml->AddTimer(3u, [this]() {
		int set_sector = cmdspawnp_ptr->get_curSector();
		///get current sector
		CKBehavior* events = m_bml->GetScriptByName("Gameplay_Events");
		int cur_sector = ScriptHelper::GetParamValue<int>(ScriptHelper::FindNextBB(events, events->GetInput(0))->GetOutputParameter(0)->GetDestination(0));
		//do sector if yes
		CKContext* ctx = m_bml->GetCKContext();

		if (m_curLevel) {
			if (set_sector != cur_sector) {

				VxMatrix matrix;
				m_resetpoints->GetElementValue(set_sector - 1, 0, &matrix);
				m_curLevel->SetElementValue(0, 3, &matrix);

				m_ingameParam->SetElementValue(0, 1, &set_sector);
				m_ingameParam->SetElementValue(0, 2, &cur_sector);
				ScriptHelper::SetParamValue(m_curSector, set_sector);

				//m_bml->SendIngameMessage(("Changed to Sector " + std::to_string(set_sector)).c_str());

				CKBehavior* sectorMgr = m_bml->GetScriptByName("Gameplay_SectorManager");
				ctx->GetCurrentScene()->Activate(sectorMgr, true);

				m_bml->AddTimerLoop(1ul, [this, set_sector, sectorMgr, ctx]() {
					if (sectorMgr->IsActive())
						return true;

					m_bml->AddTimer(2ul, [this, set_sector, ctx]() {
						CKBOOL active = false;
						m_curLevel->SetElementValue(0, 4, &active);

						CK_ID flameId;
						m_checkpoints->GetElementValue(set_sector % 2, 1, &flameId);
						CK3dEntity* flame = static_cast<CK3dEntity*>(ctx->GetObject(flameId));
						ctx->GetCurrentScene()->Activate(flame->GetScript(0), true);

						m_checkpoints->GetElementValue(set_sector - 1, 1, &flameId);
						flame = static_cast<CK3dEntity*>(ctx->GetObject(flameId));
						ctx->GetCurrentScene()->Activate(flame->GetScript(0), true);

						if (set_sector > m_checkpoints->GetRowCount()) {
							CKMessageManager* mm = m_bml->GetMessageManager();
							CKMessageType msg = mm->AddMessageType("last Checkpoint reached");
							mm->SendMessageSingle(msg, m_bml->GetGroupByName("All_Sound"));
						}
						else {
							m_bml->AddTimer(2ul, [this, set_sector, ctx, flame]() {
								VxMatrix matrix;
								m_checkpoints->GetElementValue(set_sector - 1, 0, &matrix);
								flame->SetWorldMatrix(matrix);
								CKBOOL active = true;
								m_curLevel->SetElementValue(0, 4, &active);
								ctx->GetCurrentScene()->Activate(flame->GetScript(0), true);
								m_bml->Show(flame, CKSHOW, true);
							});
						}
					});

					return false;
				});

			}
		}
		//	});
		//m_bml->AddTimer(3u, [this]() {
		CKDataArray* ph = m_bml->GetArrayByName("PH");
		for (int i = 0; i < ph->GetRowCount(); i++) {
			CKBOOL set = true;
			char name[100];
			ph->GetElementStringValue(i, 1, name);
			if (!strcmp(name, "P_Extra_Point"))
				ph->SetElementValue(i, 4, &set);
		}
		if (reset_enabled) {
			m_ingameParam->SetElementValueFromParameter(0, 1, m_curSector);
			m_ingameParam->SetElementValueFromParameter(0, 2, m_curSector);
			CKBehavior* sectorMgr = m_bml->GetScriptByName("Gameplay_SectorManager");
			ctx->GetCurrentScene()->Activate(sectorMgr, true);
		}
		//}); 
	///ball position tp
		CKMessageManager* mm = m_bml->GetMessageManager();
		CKMessageType ballDeact = mm->AddMessageType("BallNav deactivate");
		mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Gameplay"));
		mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Sound"));
		m_dynamicPos->ActivateInput(1);
		m_dynamicPos->Activate();
		auto current_ball = get_curBall();

		m_bml->AddTimer(2ul, [this, current_ball,mm,ballDeact]() {
			ExecuteBB::Unphysicalize(current_ball);

			auto matrix = cmdspawnp_ptr->get_matrix();

			current_ball->SetWorldMatrix(matrix);
			CK3dEntity* camMF = m_bml->Get3dEntityByName("Cam_MF");
			m_bml->RestoreIC(camMF, true);
			camMF->SetWorldMatrix(matrix);

			m_dynamicPos->ActivateInput(0);
			m_dynamicPos->Activate();
			m_phyNewBall->ActivateInput(0);
			m_phyNewBall->Activate();
			m_phyNewBall->GetParent()->Activate();
			mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Gameplay"));
			mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Sound"));


			//});
			//m_bml->AddTimer(5u, [this]() {

			CK3dEntity* curBall = get_curBall();
			ExecuteBB::Unphysicalize(curBall);
			static char trafoTypes[3][6] = { "paper", "wood", "stone" };
			SetParamString(m_curTrafo, trafoTypes[cmdspawnp_ptr->get_ball_type()]);
			m_setNewBall->ActivateInput(0);
			m_setNewBall->Activate();
			GetLogger()->Info("NewSpawn Reset Ball");
		});

		//if (ball_name_to_id[current_ball->GetName()] != cmdspawnp_ptr->get_ball_type()) {
		//
		//	m_bml->AddTimer(3u, [this, mm, ballDeact]() {
		//		mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Gameplay"));
		//		mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Sound"));
		//	
		//	
		//	//});
		//	//m_bml->AddTimer(5u, [this]() {
		//
		//		CK3dEntity* curBall = get_curBall();
		//		ExecuteBB::Unphysicalize(curBall);
		//		static char trafoTypes[3][6] = { "paper", "wood", "stone" };
		//		SetParamString(m_curTrafo, trafoTypes[cmdspawnp_ptr->get_ball_type()]);
		//		m_setNewBall->ActivateInput(0);
		//		m_setNewBall->Activate();
		//		GetLogger()->Info("NewSpawn Reset Ball");
		//	});
		//}
	}
}

//////////////////cmdspawnp

void cmdspawnp::Execute(IBML* bml, const std::vector<std::string>& args)
{

	if (bml->IsIngame())
	{
		//get spawn position
		CK3dEntity* camRef = bml->Get3dEntityByName("Cam_OrientRef");
		matrix = camRef->GetWorldMatrix();
		for (int i = 0; i < 3/*BML,4*/; i++) {
			std::swap(matrix[0][i], matrix[2][i]);
			matrix[0][i] = -matrix[0][i];
		}
		//get spawn sector
		CKBehavior* events = bml->GetScriptByName("Gameplay_Events");
		set_curSector(ScriptHelper::GetParamValue<int>(ScriptHelper::FindNextBB(events, events->GetInput(0))->GetOutputParameter(0)->GetDestination(0)));
		//get spawn ball type
		ball_type = ball_name_to_id[spawnp::this_instance->get_curBall()->GetName()];

		bml->SendIngameMessage(("Set NSpawn Point to (sector "
			+ std::to_string(get_curSector()) + ", position "
			+ std::to_string(matrix[3][0]) + ", "
			+ std::to_string(matrix[3][1]) + ", "
			+ std::to_string(matrix[3][2]) + ")").c_str());
	}
}

const std::vector<std::string> cmdspawnp::GetTabCompletion(IBML* bml, const std::vector<std::string>& args)
{
	return {};
}
