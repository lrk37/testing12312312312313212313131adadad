#pragma once

struct OffScreenDamageData_t
{
	float m_time, m_color_step;
	Color m_color;

	__forceinline OffScreenDamageData_t() : m_time{ 0.f }, m_color{ colors::white } { }
	__forceinline OffScreenDamageData_t( float time, float m_color_step, Color color ) : m_time{ time }, m_color{ color } { }
};

enum HitType_t : char {
	NORMAL,
	LETHAL,
	HEADSHOT
};

class Visuals
{
public:
	std::array< bool, 64 > m_draw;
	std::array< float, 64 > m_opacities;
	std::array< float, 64 > m_time_cache;

	std::array< OffScreenDamageData_t, 64 > m_offscreen_damage;
	vec2_t                                  m_crosshair;
	float					                m_hit_start;
	HitType_t                               m_hit_type;

	// info about planted c4.
	bool                                    planted;
	bool                                    bombexploded;
	bool                                    bombedefused;
	std::string                             m_bombsite;
	bool                                    bombeindefuse;
	float                                   m_planted_c4_defuse_time;
	bool        m_c4_planted;
	Entity* m_planted_c4;
	float       m_planted_c4_explode_time;
	vec3_t      m_planted_c4_explosion_origin;
	float       m_planted_c4_damage;
	float       m_planted_c4_radius;
	float       m_planted_c4_radius_scaled;
	//float       miss;
	//float       miss1;
	//float       miss2;
	std::string m_last_bombsite;

	IMaterial* smoke1;
	IMaterial* smoke2;
	IMaterial* smoke3;
	IMaterial* smoke4;

	std::array<vec3_t, 64> origin_cache;

	std::map<int, char> m_weapon_icon_map = {
		{ DEAGLE, 'A' },
		{ ELITE, 'B' },
		{ FIVESEVEN, 'C' },
		{ GLOCK, 'D' },
		{ P2000, 'E' },
		{ P250, 'F' },
		{ USPS, 'G' },
		{ TEC9, 'H' },
		{ CZ75A, 'I' },
		{ REVOLVER, 'J' },
		{ MAC10, 'K' },
		{ UMP45, 'L' },
		{ BIZON, 'M' },
		{ MP7, 'N' },
		{ MP5SD, 'L' },
		{ MP9, 'O' },
		{ P90, 'P' },
		{ GALIL, 'Q' },
		{ FAMAS, 'R' },
		{ M4A4, 'S' },
		{ M4A1S, 'T' },
		{ AUG, 'U' },
		{ SG553, 'V' },
		{ AK47, 'W' },
		{ G3SG1, 'X' },
		{ SCAR20, 'Y' },
		{ AWP, 'Z' },
		{ SSG08, 'a' },
		{ XM1014, 'b' },
		{ SAWEDOFF, 'c' },
		{ MAG7, 'd' },
		{ NOVA, 'e' },
		{ NEGEV, 'f' },
		{ M249, 'g' },
		{ ZEUS, 'h' },
		{ KNIFE_T, 'i' },
		{ KNIFE_CT, 'j' },
		{ KNIFE_FALCHION, '0' },
		{ KNIFE_BAYONET, '1' },
		{ KNIFE_FLIP, '2' },
		{ KNIFE_GUT, '3' },
		{ KNIFE_KARAMBIT, '4' },
		{ KNIFE_M9_BAYONET, '5' },
		{ KNIFE_HUNTSMAN, '6' },
		{ KNIFE_BOWIE, '7' },
		{ KNIFE_BUTTERFLY, '8' },
		{ KNIFE_URSUS, 'j' },
		{ KNIFE_GYPSY_JACKKNIFE, 'j' },
		{ KNIFE_STILETTO, 'j' },
		{ KNIFE_WIDOWMAKER, 'j' },
		{ FLASHBANG, 'k' },
		{ HEGRENADE, 'l' },
		{ SMOKE, 'm' },
		{ MOLOTOV, 'n' },
		{ DECOY, 'o' },
		{ FIREBOMB, 'p' },
		{ C4, 'q' },
	};

	struct
	{
		int r, g, b, a;
	} custom_game_crosshair;

public:
	static void ModulateWorld();
	std::string GetWeaponIcon( int id );
	void ThirdpersonThink();
	void NoSmoke();
	void think();
	void Spectators();
	void StatusIndicators();
	void SpreadCrosshair();
	void ManualAntiAim();
	void PenetrationCrosshair();
	void DrawPlantedC4();
	void DrawHitmarkers();
	void draw( Entity* ent );
	void DrawProjectile( Weapon* ent );
	void DrawItem( Weapon* item );
	void OffScreen( Player* player, float alpha );
	void DrawPlayer( Player* player );
	bool GetPlayerBoxRect( Player* player, Rect& box );
	void DrawSkeleton( Player* player, Color color, int opacity );
	void RenderGlow();
	void DrawHitboxMatrix( Player* player, BoneArray* bones, Color col, float time );
	void DrawBeams();
	void DebugAimbotPoints( Player* player );
	void ServersideEspBySpectatorsLolThanksNaphack();
	void RaceChanger();
	void PrePaint();
	void HandleLocal(Player* local);
};

extern Visuals g_visuals;