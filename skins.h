#pragma once

constexpr int sequence_default_draw = 0;
constexpr int sequence_default_idle1 = 1;
constexpr int sequence_default_idle2 = 2;
constexpr int sequence_default_heavy_miss1 = 9;
constexpr int sequence_default_heavy_hit1 = 10;
constexpr int sequence_default_heavy_backstab = 11;
constexpr int sequence_default_lookat01 = 12;

constexpr int sequence_butterfly_draw = 0;
constexpr int sequence_butterfly_draw2 = 1;
constexpr int sequence_butterfly_lookat01 = 13;
constexpr int sequence_butterfly_lookat03 = 15;

constexpr int sequence_falchion_idle1 = 1;
constexpr int sequence_falchion_heavy_miss1 = 8;
constexpr int sequence_falchion_heavy_miss1_noflip = 9;
constexpr int sequence_falchion_lookat01 = 12;
constexpr int sequence_falchion_lookat02 = 13;

constexpr int sequence_push_idle1 = 1;
constexpr int sequence_push_heavy_miss2 = 11;

struct KnifeData
{
	int         m_id;
	std::string m_model, m_world_model, m_name;
	int         m_model_index, m_world_model_index;

	__forceinline KnifeData() : m_id{}, m_model{}, m_world_model{}, m_name{}, m_model_index{}, m_world_model_index{} { }

	__forceinline void setup( int id, const std::string& model, const std::string& world_model, const std::string& name )
	{
		m_id = id;
		m_model = model;
		m_name = name;
		m_model_index = g_csgo.m_model_info->GetModelIndex( model.data() );
		m_world_model_index = g_csgo.m_model_info->GetModelIndex( world_model.data() );
	}
};

struct GloveData
{
	int         m_id;
	std::string m_model, m_world_model;
	int         m_model_index, m_world_model_index;

	__forceinline GloveData() : m_id{}, m_model{}, m_model_index{} { }

	__forceinline void setup( int id, const std::string& model, const std::string& world_model )
	{
		m_id = id;
		m_model = model;
		m_world_model = world_model;
		m_model_index = g_csgo.m_model_info->GetModelIndex( model.data() );
		m_world_model_index = g_csgo.m_model_info->GetModelIndex( world_model.data() );
	}
};

struct HudWeapons_t
{
	std::int32_t* GetWeaponCount()
	{
		return reinterpret_cast<std::int32_t*>( std::uintptr_t( this ) + 0x80 );
	}
};

struct weaponting_t {
	Weapons_t definition_index;
	const char* name;
	int set_paintkit;
	//float wear = 0.f;
	weaponting_t() {
		definition_index = Weapons_t::WEAPON_NONE;
		name = nullptr;
	}

	weaponting_t(Weapons_t w, const char* c) {
		definition_index = w;
		name = c;
	}
};

struct Skinwrapper_t {
	char name[19]; // the highest charcount i saw was 19...
	Color custom_col[4];
	Color saved_col[4];

	void fill(Color a, Color b, Color c, Color d) {
		saved_col[0] = custom_col[0] = a;
		saved_col[1] = custom_col[1] = b;
		saved_col[2] = custom_col[2] = c;
		saved_col[3] = custom_col[3] = d;
	}
};

class Skins
{
public:
	enum knives_t
	{
		BAYONET = 1,
		BOWIE,
		BUTTERFLY,
		FALCHION,
		FLIP,
		GUT,
		HUNTSMAN,
		KARAMBIT,
		M9,
		DAGGER,
		KNIFE_MAX
	};

	enum gloves_t
	{
		BLOODHOUND = 1,
		SPORTY,
		DRIVER,
		HANDWRAP,
		MOTO,
		SPECIALIST,
		HYDRA,
		GLOVE_MAX
	};

public:
	KnifeData   m_knife_data[ knives_t::KNIFE_MAX ];
	GloveData   m_glove_data[ gloves_t::GLOVE_MAX ];
	int         m_last_seq;
	CBaseHandle m_glove_handle;
	bool        m_update;
	float       m_update_time;
	uintptr_t glove_render_handle;
	bool first_handle;

	std::array<weaponting_t, 49> m_weapon_name_map = {
		weaponting_t(AK47, "ak-47"),
		weaponting_t(AUG, "aug"),
		weaponting_t(AWP, "awp"),
		weaponting_t(BIZON, "bizon"),
		weaponting_t(CZ75A, "cz-75"),
		weaponting_t(DEAGLE, "desert eagle"),
		weaponting_t(ELITE, "dual berettas"),
		weaponting_t(FAMAS, "famas"),
		weaponting_t(FIVESEVEN, "five-seven"),
		weaponting_t(G3SG1, "g3sg1"),
		weaponting_t(GALIL, "galil"),
		weaponting_t(GLOCK, "glock"),
		weaponting_t(M4A1S, "m4a1-s"),
		weaponting_t(M4A4, "m4a4"),
		weaponting_t(MAG7, "mag7"),
		weaponting_t(MAC10, "mac10"),
		weaponting_t(MP5SD, "mp5-sd"),
		weaponting_t(MP7, "mp7"),
		weaponting_t(MP9, "mp9"),
		weaponting_t(M249, "m249"),
		weaponting_t(NOVA, "nova"),
		weaponting_t(NEGEV, "negev"),
		weaponting_t(P2000, "p2000"),
		weaponting_t(P250, "p250"),
		weaponting_t(P90, "p90"),
		weaponting_t(REVOLVER, "revolver"),
		weaponting_t(SAWEDOFF, "sawedoff"),
		weaponting_t(SCAR20, "scar20"),
		weaponting_t(SSG08, "ssg08"),
		weaponting_t(SG553, "sg-553"),
		weaponting_t(TEC9, "tec-9"),
		weaponting_t(UMP45, "ump45"),
		weaponting_t(USPS, "usp-s"),
		weaponting_t(XM1014, "xm1014"),
		weaponting_t(KNIFE_BAYONET, "bayonet"),
		weaponting_t(KNIFE_BOWIE, "bowie knife"),
		weaponting_t(KNIFE_BUTTERFLY, "butterfly knife"),
		weaponting_t(KNIFE_FALCHION, "falchion knife"),
		weaponting_t(KNIFE_FLIP, "flip knife"),
		weaponting_t(KNIFE_GUT, "gut knife"),
		weaponting_t(KNIFE_GYPSY_JACKKNIFE, "navaja knife"),
		weaponting_t(KNIFE_HUNTSMAN, "huntsman knife"),
		weaponting_t(KNIFE_KARAMBIT, "karambit"),
		weaponting_t(KNIFE_M9_BAYONET, "m9 bayonet"),
		weaponting_t(KNIFE_SHADOW_DAGGERS, "shadow daggers"),
		weaponting_t(KNIFE_STILETTO, "stiletto"),
		weaponting_t(KNIFE_WIDOWMAKER, "talon knife"),
		weaponting_t(KNIFE_URSUS, "ursus knife"),
		weaponting_t(WEAPON_NONE, "gloves" ),
	};

	//std::array<Skinwrapper_t, 706> saved_kits; // 0 is blank but idc
	std::unordered_map<int, Skinwrapper_t> saved_kits; // 0 is blank but idc
	std::unordered_map<int, char[14]> saved_gloves; // 14 is the longest.
	bool m_finished = false;
public:
	void load();
	void OnPostDataUpdate();
	void UpdateItem( Weapon* item );
	void UpdateAnimations( Entity* ent );
	void OnLevelShutdown();
	void Init();
public:
	int ConvertDefinitionIndexToMapID(int definition_index) {
		switch (definition_index) { // thanks chatgpt
			case AK47: return 0;
			case AUG: return 1;
			case AWP: return 2;
			case BIZON: return 3;
			case CZ75A: return 4;
			case DEAGLE: return 5;
			case ELITE: return 6;
			case FAMAS: return 7;
			case FIVESEVEN: return 8;
			case G3SG1: return 9;
			case GALIL: return 10;
			case GLOCK: return 11;
			case M4A1S: return 12;
			case M4A4: return 13;
			case MAG7: return 14;
			case MAC10: return 15;
			case MP5SD: return 16;
			case MP7: return 17;
			case MP9: return 18;
			case M249: return 19;
			case NOVA: return 20;
			case NEGEV: return 21;
			case P2000: return 22;
			case P250: return 23;
			case P90: return 24;
			case REVOLVER: return 25;
			case SAWEDOFF: return 26;
			case SCAR20: return 27;
			case SSG08: return 28;
			case SG553: return 29;
			case TEC9: return 30;
			case UMP45: return 31;
			case USPS: return 32;
			case XM1014: return 33;
			case KNIFE_BAYONET: return 34;
			case KNIFE_BOWIE: return 35;
			case KNIFE_BUTTERFLY: return 36;
			case KNIFE_FALCHION: return 37;
			case KNIFE_FLIP: return 38;
			case KNIFE_GUT: return 39;
			case KNIFE_GYPSY_JACKKNIFE: return 40;
			case KNIFE_HUNTSMAN: return 41;
			case KNIFE_KARAMBIT: return 42;
			case KNIFE_M9_BAYONET: return 43;
			case KNIFE_SHADOW_DAGGERS: return 44;
			case KNIFE_STILETTO: return 45;
			case KNIFE_WIDOWMAKER: return 46;
			case KNIFE_URSUS: return 47;
			case WEAPON_NONE: return 48;
			default: return -1; // Error case
		}
	}

	bool IsWeaponKnife(int definition_index) {
		return definition_index == KNIFE_T || definition_index == KNIFE_CT || (definition_index >= KNIFE_BAYONET && definition_index <= KNIFE_WIDOWMAKER);
	}
};

extern Skins g_skins;