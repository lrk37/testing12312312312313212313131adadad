#pragma once

struct AnimationBackup_t
{
	vec3_t           m_abs_origin;
	vec3_t           m_velocity, m_abs_velocity;
	int              m_flags, m_eflags;
	float            m_duck;

	__forceinline void store( Player* player )
	{
		m_abs_origin = player->GetAbsOrigin();
		m_velocity = player->m_vecVelocity();
		m_abs_velocity = player->m_vecAbsVelocity();
		m_flags = player->m_fFlags();
		m_eflags = player->m_iEFlags();
		m_duck = player->m_flDuckAmount();
	}

	__forceinline void restore( Player* player )
	{
		player->SetAbsOrigin( m_abs_origin );
		player->m_vecVelocity() = m_velocity;
		player->m_vecAbsVelocity() = m_abs_velocity;
		player->m_fFlags() = m_flags;
		player->m_iEFlags() = m_eflags;
		player->m_flDuckAmount() = m_duck;
	}
};

struct HitscanData_t
{
	int m_hitbox;
	float m_damage;
	vec3_t m_pos;
	float m_fake_amt;

	__forceinline HitscanData_t() : m_hitbox{ -1 }, m_damage{ -1.f }, m_pos{}, m_fake_amt{ -1.f } { }
};

struct AimPoint_t
{
	vec3_t m_pos;
	bool m_edge_point;
	float m_fake_amt;

	float m_damage;
	float m_hitchance;
	int m_hitbox;
};

class AimPlayer
{
public:
	using records_t = std::deque< std::shared_ptr< LagRecord > >;

public:
	// essential data.
	Player* m_player;
	float	  m_spawn;
	float     m_update_increment;
	float     m_last_lby_delta; 
	float     m_lby_changed;
	float     m_last_change_yaw;
	float    m_lby_change_standing;
	records_t m_records;

	bool m_ignore;

	bool m_went_dormant;
	bool m_accurate_velocity;

	bool m_get_death_yaw;

	// aimbot data.
	bool m_breaking_lc;

	float m_held_damage;

	void OnImpact( ImpactRecord* impact );

	enum LogReason_t
	{
		DEATH_CLAMPED = 0,
		DEATH_UNCLAMPED,
		MISS,
		HIT,
		OVERLAP
	};

	enum LbySide_t
	{
		EQUAL = 0,
		LEFT,
		RIGHT
	};

	enum LogDelta_t
	{
		LBY_EYE = 0,
		BACKWARDS_EYE,
		FREESTAND_EYE
	};

	struct ResolverLog_t
	{
		int m_reason;
		bool m_on_ground;

		float m_confidence;

		int m_lby_side;

		std::array<float, 3> m_deltas;
	};

	std::vector<ResolverLog_t> m_shot_log;
	std::vector<float> m_blacklisted_angles;
	std::vector<int> m_blacklisted_timings;

	struct
	{
		float m_times_clamped;
		float m_times_unclamped;
	} m_clamp_log;

	enum FakeColor_t
	{
		DONT_DRAW = 0,
		WHITE,
		GREEN,
		RED,
	};

	enum ResolverConfidence_t
	{
		VERY_LOW = 0,
		LOW,
		MEDIUM,
		HIGH,
		VERY_HIGH,
		MAX
	};
	
	// if they flick to feet yaw every time, it will keep delaying over and over again.
	// otherwise its just a static offset.
	enum LBYDelayType_t
	{
		CONSTANT = 0,
		REPEATING
	};

	// offset, time when logged.
	std::deque<std::pair<int, float>> m_update_offsets;

	int m_old_979_count;
	int m_979_count;

	int m_resolver_confidence;

	float m_override;
	bool m_override_agreed;

	Player* m_facing_player;

	float m_velocity_dir;
	float m_air_angle;
	float m_freestand_angle;

	int m_missed_shots;

	bool m_scary_lby;

	float m_last_update_time;

	int m_logged_timer;

	float m_lby_timer;
	int m_updates_post_lby;
	bool m_cant_predict_lby;

	float m_last_shot_time;

	bool m_detected_desync;

	// data about the LBY proxy.
	float m_body;
	float m_old_body;

	float m_max_feet_yaw;
	float m_min_feet_yaw;

	bool m_exploit;
	float m_highest_simtime;

	// silent updates (defensive).
	struct
	{
		float m_flCycle;
		float m_flPlaybackRate;
	} last_alive_loop_data;

	struct {
		bool m_can_hit;
		bool m_backstab;
		bool can_hit_secondary_swing;
	} knife_info;

	LagRecord m_extrapolated_record;
public:
	void reset()
	{
		//m_player = nullptr;
		//m_ignore = false;
		m_highest_simtime = -1.f;
		m_went_dormant = true;
		m_get_death_yaw = false;

		m_cant_predict_lby = true;
		m_facing_player = nullptr;
		//m_logged_timer = -1;

		reset_knife();

		m_records.clear();
	}

	void reset_knife()
	{
		knife_info.m_can_hit				= false;
		knife_info.m_backstab				= false;
		knife_info.can_hit_secondary_swing	= false;
	}

public:
	void UpdatePlayerAnims( CCSGOPlayerAnimState* state );
	bool EstimateVelocity( Player* m_player, LagRecord* record, LagRecord* previous, LagRecord* pre_previous, float max_speed );
	bool UpdateAnimations( LagRecord* record );
	void OnNetUpdate( Player* player );

	bool SetupHitboxPoints( LagRecord* record, std::vector< AimPoint_t >& points, float min_dmg );
	bool AllowHitbox( int hitbox, float mindmg, int resolver_mode );

	enum ScanResult_t
	{
		HITCHANCE_TOO_LOW = 1,
		PASSED
	};

	// reference dont work properly with our threading? This isnt what was causing issues anyways tho.
	void ScanPoint( AimPoint_t* point, float damage, vec3_t& shootpos, LagRecord* record, int* result ); 

	int AimAtRecord( LagRecord* record, vec3_t& aim_pos, int& hitbox, float& damage, float& chance, float& fake_amount, bool skip_edges = false );
	bool CanEarlyStop( LagRecord*& out_record, float& damage, float& fake_amount );
	bool UpdateExtrapolation( int ticks, bool broke_lc = true );
};

enum AimbotHitbox_t {
	AIM_HEAD = 0,
	AIM_CHEST,
	AIM_BODY,
	AIM_ARMS,
	AIM_LEGS,
	AIM_FEET
};

struct AimbotSettings_t {
	float minimum_damage;
	std::array<bool, 6> hitboxes;
	std::array<bool, 6> multipoint;
	float multipoint_head_scale;
	float multipoint_normal_scale;
	bool multipoint_dynamic_scale;
	float hitchance;

	float prefer_overlap;
	float prefer_hitchance;
	bool conditional_standing;
	bool conditional_unsafe_moving;
	bool conditional_in_air;
	bool conditional_already_missed;
	float conditional_prefer_overlap;
	float conditional_prefer_hitchance;
	int avoid_head_shots;

	bool delay_shot;
	bool delay_shot_disable_vulnerable;
	bool delay_shot_damage;
	bool delay_shot_safety;
	float delay_shot_strictness;

	void Init();
};

class Aimbot
{
public:
	std::array< AimPlayer, 64 > m_players;
	std::vector<AimPlayer*> m_sortable_players;
	BackupRecord m_backup[ 64 ];
	struct BestTarget_t
	{
		AimPlayer* player;
		vec3_t pos;
		LagRecord* record;

		int hitbox;
		float damage;
		float hitchance;
		float fake_amount;

		void Reset()
		{
			player = nullptr;
			pos = {};
			damage = -1.f;
			record = nullptr;
			hitbox = -1;
			hitchance = -1.f;
		};

		__forceinline BestTarget_t()
		{ 
			Reset();
		}
	};

	std::vector<AimPoint_t> m_aim_points;

	// target selection stuff.
	float m_best_dist;
	float m_best_fov;
	float m_best_damage;
	int   m_best_hp;
	float m_best_lag;
	float m_best_height;

	// found target stuff.
	AimPlayer* m_aimplayer;
	ang_t      m_angle;
	vec3_t     m_aim;
	float      m_damage;
	float      m_hitchance;
	float      m_hitbox;
	LagRecord* m_record;
	player_info_t m_player_info;

	bool DelayShot( AimPlayer* data, LagRecord* record, float damage, float safety );
	void UpdateEarlyStop();

	void DoKnifeBot();

	vec3_t m_early_stop_shootpos;

	bool m_allow_auto_stop;

	bool m_stop_next_tick;

	bool m_skip_hold_fakelag;

	AimbotSettings_t m_settings;

	int m_stop_ticks;
	float m_stop_inaccuracy;

	std::vector<std::string> m_debug_overlay;

	std::vector<std::tuple<float, float, float>> precomputed_seeds;

public:
	__forceinline void reset()
	{
		// reset aimbot data.
		init();

		// reset all players data.
		for ( auto& p : m_players )
			p.reset();
	}

	__forceinline bool IsValidTarget( Player* player, bool check_local = true )
	{
		if ( !player )
			return false;

		if ( !player->IsPlayer() )
			return false;

		if ( !player->alive() )
			return false;

		if ( player->m_bIsLocalPlayer() && check_local )
			return false;

		if ( !player->enemy( g_ctx.m_local ) )
			return false;

		return true;
	}

public:
	// aimbot.
	void init();
	bool record_meets_safety( LagRecord* record );
	float GetMinDamage( int health = 100 );
	void StripAttack();

	bool ScanPlayer( AimPlayer* data, BestTarget_t& target_data );

	void think();
	void OnInject();
	bool TryOldCmd();
	bool RepredictAutostop();
	bool RecalcShootpos( ang_t& aim_angle );
};

extern Aimbot g_aimbot;