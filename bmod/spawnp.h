#pragma once
#include <BML/BMLAll.h>

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

std::unordered_map<std::string, int> ball_name_to_id{{"Ball_Paper", 0}, {"Ball_Wood", 1}, {"Ball_Stone", 2}};

class cmdspawnp : public ICommand
{
public:
	std::string GetName() override { return "newspawn"; }
	std::string GetAlias() override { return "nsp"; }
	std::string GetDescription() override { return "Sets your spawn point to tp without reset prop"; }
	bool IsCheat() override { return true; }
	void Execute(IBML* bml, const std::vector<std::string>& args) override;
	const std::vector<std::string> GetTabCompletion(IBML* bml, const std::vector<std::string>& args) override;
	const VxMatrix& get_matrix() const { return matrix; }
	int get_curSector() const { return curSector; }
	int get_ball_type() const { return ball_type; }
	void set_ball_type(const int type) { ball_type = type; }
	void set_matrix(const VxMatrix& mat) { matrix = mat; }
	void set_curSector(const int sec) { curSector = sec; }
private:
	VxMatrix matrix{};
	int curSector{};
	int ball_type{};
};

class spawnp : public IMod {
public:
	spawnp(IBML* bml) : IMod(bml) { this_instance = this; }

	virtual CKSTRING GetID() override { return "NewSpawn"; }
	virtual CKSTRING GetVersion() override { return "0.0.1"; }
	virtual CKSTRING GetName() override { return "NewSpawn"; }
	virtual CKSTRING GetAuthor() override { return "fluoresce"; }
	virtual CKSTRING GetDescription() override { return "\"newspawn\" or \"nsp\" to set a position. shortcut key to tp to the position but do not reset the prop. reset the prop and tp to sector if the current is not the set sector.Thanks for the help of Bug and Swung "; }
	DECLARE_BML_VERSION;
	//VxMatrix matrix;
	CK3dEntity* get_curBall() { return static_cast<CK3dEntity*>(m_curLevel->GetElementObject(0, 1)); }
	inline static spawnp* this_instance = nullptr;

private: 
	void OnLoad() override;
	void OnProcess() override;
	void OnModifyConfig(CKSTRING category, CKSTRING key, IProperty* prop) override;
	void OnLoadScript(CKSTRING filename, CKBehavior* script) override;
	void OnStartLevel() override;
	void OnBallNavActive() override;
	void OnBallNavInactive() override;
	void OnPostStartMenu() override;


	bool init = false;
	bool Ball_Active = false;
	IProperty *prop_enabled, *reset_prop_enabled, *prop_key;
	InputHook* input_manager;
	CKKEYBOARD key;
	bool enabled = false;
	bool reset_enabled = false;
	cmdspawnp* cmdspawnp_ptr = nullptr;

	CKBehavior* m_dynamicPos = nullptr;
	CKBehavior* m_phyNewBall = nullptr;
	CKDataArray* m_curLevel, * m_checkpoints, * m_resetpoints, * m_ingameParam;
	CKParameter* m_curSector;
	//
	CKParameter* m_curTrafo;
	CKBehavior* m_setNewBall;
	void SetParamString(CKParameter* param, CKSTRING value) {
		param->SetStringValue(value);
	}
};
namespace sectorget {
	template<typename T>
	T GetParamValue(CKParameter* param) {
		T res;
		param->GetValue(&res);
		return res;
	}
}
