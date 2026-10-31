#pragma once

class ShotRecord
{
public:
	__forceinline ShotRecord() : m_target{}, m_record{}, m_time{}, m_lat{}, m_damage{}, m_pos{}, m_matched{}, m_command_number{} { }

public:
	Player* m_target;
	LagRecord* m_record;
	float      m_time, m_lat, m_damage;
	vec3_t     m_pos;
	bool       m_matched;
	int		   m_hitbox;
	float      m_weapon_dist;
	int        m_command_number;
};

class HitmarkerImpacts
{
public:
	vec3_t m_pos;
	float m_time;
	int m_damage;
	bool m_lethal, m_headshot;
};

class VisualImpactData_t
{
public:
	vec3_t m_impact_pos, m_shoot_pos;
	int    m_tickbase;
	bool   m_ignore;
	Color m_color;
	int m_ent_index;

public:
	__forceinline VisualImpactData_t( const vec3_t& impact_pos, const vec3_t& shoot_pos, int tickbase, Color color, int idx ) :
		m_impact_pos{ impact_pos }, m_shoot_pos{ shoot_pos }, m_tickbase{ tickbase }, m_ignore{ false }, m_color{ color }, m_ent_index{ idx }
	{
	}
};

class ImpactRecord
{
public:
	__forceinline ImpactRecord() : m_shot{}, m_pos{}, m_tick{}, m_hit{}, m_ACTUALLY_HITGROUP_TAPPER{} { }

public:
	ShotRecord* m_shot;
	int         m_tick;
	vec3_t      m_pos;
	bool m_hit;
	int m_ACTUALLY_HITGROUP_TAPPER;
};

class HitRecord
{
public:
	__forceinline HitRecord() : m_impact{}, m_group{ -1 }, m_damage{} { }

public:
	ImpactRecord* m_impact;
	int           m_group;
	float         m_damage;
};

class Shots
{
public:
	void Process();
	void OnShotFire( Player* target, float damage, int bullets, LagRecord* record, int hitbox );
	void OnImpact( IGameEvent* evt );
	void OnHurt( IGameEvent* evt );

public:
	std::array< std::string, 11 > m_groups = {
	XOR( "body" ),
	XOR( "head" ),
	XOR( "chest" ),
	XOR( "stomach" ),
	XOR( "left arm" ),
	XOR( "right arm" ),
	XOR( "left leg" ),
	XOR( "right leg" ),
	XOR( "neck" ),
	XOR( "unknown" ),
	XOR( "gear" )
	};

	std::deque< ShotRecord >          m_unapproved_shots;
	std::deque< ShotRecord >          m_real_shots;

	std::vector< VisualImpactData_t > m_vis_impacts;

	std::deque< HitmarkerImpacts >        m_pre_hitmarker_impacts;
	std::deque< HitmarkerImpacts >         m_post_hitmarker_impacts;

	std::deque< ImpactRecord >        m_impacts;

	std::deque< HitRecord >           m_hits;
};

extern Shots g_shots;