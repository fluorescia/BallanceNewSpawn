#pragma once
#include <BML/BMLAll.h>
#include <fstream>
#include <md5.h>

#define TICK_SPEED 8

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

constexpr int READ_SIZE = 1024 * 16;


std::unordered_map<std::string, int> ball_name_to_id{ {"Ball_Paper", 0}, {"Ball_Wood", 1}, {"Ball_Stone", 2} };

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
	void set_matrix(const VxMatrix& mat) { matrix = mat; }
	int get_curSector() const { return curSector; }
	void set_curSector(const int sec) { curSector = sec; }

	int get_ball_type() const { return ball_type; }
	void set_ball_type(const int type) { ball_type = type; }

	bool get_is_spawned() const { return is_spawned; }
	void set_is_spawned(const bool type) { is_spawned = type; }

	std::string get_map_hash_setlevel() const { return map_hash_setlevel; }
	void set_map_hash_setlevel(const std::string type) { map_hash_setlevel = type; }
private:
	VxMatrix matrix{};
	int curSector{};
	int ball_type{};

	bool is_spawned = false;

	std::string map_hash_setlevel;
};

class spawnp : public IMod {
public:
	spawnp(IBML* bml) : IMod(bml) { this_instance = this; }

	virtual CKSTRING GetID() override { return "NewSpawn"; }
	virtual CKSTRING GetVersion() override { return "0.0.2"; }
	virtual CKSTRING GetName() override { return "NewSpawn"; }
	virtual CKSTRING GetAuthor() override { return "fluoresce"; }
	virtual CKSTRING GetDescription() override { return "\"nsp\" to set a position. shortcut key to tp to the position. Thanks for the help of BallanceBug and Swung "; }
	DECLARE_BML_VERSION;
	//VxMatrix matrix;
	CK3dEntity* get_curBall() { return static_cast<CK3dEntity*>(m_curLevel->GetElementObject(0, 1)); }
	inline static spawnp* this_instance = nullptr;
	std::string get_map_hash_curlevel() { return map_hash_curlevel; }
	void set_map_hash_curlevel(const std::string type) { map_hash_curlevel = type; }
private:
	//[transport]
	void OnLoad() override;
	void OnProcess() override;
	void OnModifyConfig(CKSTRING category, CKSTRING key, IProperty* prop) override;
	void OnLoadScript(CKSTRING filename, CKBehavior* script) override;

	void OnBallNavActive() override;
	void OnBallNavInactive() override;
	void OnPostStartMenu() override;
	void OnLoadObject(CKSTRING filename, BOOL isMap, CKSTRING masterName,
		CK_CLASSID filterClass, BOOL addtoscene, BOOL reuseMeshes, BOOL reuseMaterials,
		BOOL dynamic, XObjectArray* objArray, CKObject* masterObj) override;

	//[spirit]
	void OnPreResetLevel() override { StopRecording(); StopPlaying(); record_save_enable = false;};
	void OnPreExitLevel() override { StopRecording(); StopPlaying(); record_save_enable = false;};
	void OnPreEndLevel() override { StopRecording(); StopPlaying(); record_save_enable = false;};

	int GetCurrentBall();
	void SetCurrentBall(int curBall);

	void StopPlaying();
	void StopRecording(bool save_data = false);

	void StartRecording();
	void StartPlaying();

	//[transport]
	bool init = false;
	bool Ball_Active = false;
	IProperty* prop_enabled, * reset_prop_enabled, * prop_key, * reset_spirit_enabled, * prop_key_record;
	InputHook* input_manager;
	CKKEYBOARD key_prop, key_record;
	bool enabled = false;
	bool reset_enabled = false;
	bool spirit_enabled = false;
	cmdspawnp* cmdspawnp_ptr = nullptr;

	CKBehavior* m_dynamicPos = nullptr;
	CKBehavior* m_phyNewBall = nullptr;
	CKDataArray* m_curLevel, * m_checkpoints, * m_resetpoints, * m_ingameParam;
	CKParameter* m_curSector;

	//[transport] Ball type
	CKParameter* m_curTrafo;
	CKBehavior* m_setNewBall;
	void SetParamString(CKParameter* param, CKSTRING value) {
		param->SetStringValue(value);
	}

	//[transport] Map hash
	std::string map_hash_curlevel;

	//[spirit]
	bool m_isRecording = false;
	bool m_isPlaying = false;

	bool record_exist = false;

	bool record_save_enable = false;

	float m_recordTimer;
	float m_playTimer;
	float m_srtimer;

	struct SpiritBall {
		std::string name;
		CK3dObject* obj;
		std::vector<CKMaterial*> materials;
	};
	std::vector<SpiritBall> m_dualBalls;

	struct Record {
		struct State {
			VxVector pos;
			VxQuaternion rot;
		};

		std::vector<State> states;
		std::vector<std::pair<int, int>> trafo;
	};
	Record m_record, m_play;

	size_t m_playBall;
	size_t m_curBall;
	size_t m_playFrame, m_playTrafo;
};






namespace sectorget {
	template<typename T>
	T GetParamValue(CKParameter* param) {
		T res;
		param->GetValue(&res);
		return res;
	}
}
