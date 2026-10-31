#pragma once

// pre-declare.
class LagRecord;

class BackupRecord
{
public:
	BoneArray* m_bones;
	int        m_bone_count;
	vec3_t     m_origin, m_abs_origin;
	vec3_t     m_mins;
	vec3_t     m_maxs;
	ang_t      m_abs_ang;

public:
	__forceinline void store( Player* player )
	{
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache();

		// store bone data.
		m_bones = cache->m_pCachedBones;
		m_bone_count = cache->m_CachedBoneCount;

		m_origin = player->m_vecOrigin();
		m_mins = player->m_vecMins();
		m_maxs = player->m_vecMaxs();
		m_abs_origin = player->GetAbsOrigin();
		m_abs_ang = player->GetAbsAngles();
	}

	__forceinline void restore( Player* player )
	{
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache();

		cache->m_pCachedBones = m_bones;
		cache->m_CachedBoneCount = m_bone_count;

		player->m_vecOrigin() = m_origin;
		player->SetCollisionBounds( m_mins, m_maxs );
		player->SetAbsAngles( m_abs_ang );
		player->SetAbsOrigin( m_origin );
	}
};

class LagRecord
{
public:
	// data.
	Player* m_player;
	float   m_immune;
	int     m_tick;
	float     m_lag_time;
	int     m_lag;

	bool m_left_dormancy;

	// netvars.
	float  m_sim_time;
	float  m_old_sim_time;

	int    m_flags;

	vec3_t m_origin;
	vec3_t m_old_origin;
	vec3_t m_velocity;

	vec3_t m_mins;
	vec3_t m_maxs;

	ang_t  m_eye_angles;
	ang_t  m_fake_angles;

	float m_backwards_angle;
	float m_freestand_angle;

	float  m_body;
	float  m_duck;

	C_AnimationLayer m_layers[ 13 ];
	float            m_poses[ 24 ];
	CCSGOPlayerAnimState m_old_state;

	vec3_t           m_anim_velocity;

	float m_max_feet_yaw;
	float m_min_feet_yaw;

	bool       m_setup;

	BoneArray* m_bones;
	ang_t m_abs_ang;

	bool   m_broke_lc;
	bool   m_extrapolated;

	vec3_t m_interp_org;
	ang_t m_interp_ang;
	float m_interp_time;

	int m_lby_offset;
	size_t m_mode;
	bool   m_fake_walk;
	float  m_away;
	float  m_anim_time;

public:

	// ctor.
	__forceinline LagRecord() :
		m_setup{ false },
		m_broke_lc{ false },
		m_extrapolated{ false },
		m_fake_walk{ false },
		m_lag{},
		m_bones{}
	{
	}

	// dtor.
	__forceinline ~LagRecord()
	{
		// free heap allocated game mem.
		g_csgo.m_mem_alloc->Free( m_bones );
	}

	__forceinline void invalidate()
	{
		// free heap allocated game mem.
		g_csgo.m_mem_alloc->Free( m_bones );

		// mark as not setup.
		m_setup = false;

		// allocate new memory.
		m_bones = (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );
	}

	// function: allocates memory for SetupBones and stores relevant data.
	void store( Player* player, bool allocate = true )
	{
		// allocate game heap.
		if ( allocate )
			m_bones = (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );

		// player data.
		m_player = player;
		m_immune = player->m_fImmuneToGunGameDamageTime();
		m_tick = g_csgo.m_cl->m_server_tick;

		// netvars.
		m_sim_time = player->m_flSimulationTime();
		m_flags = player->m_fFlags();
		m_origin = m_interp_org = player->m_vecOrigin();
		m_eye_angles = m_fake_angles = player->m_angEyeAngles();
		m_abs_ang = m_interp_ang = player->GetAbsAngles();
		m_body = player->m_flLowerBodyYawTarget();
		m_mins = player->m_vecMins();
		m_maxs = player->m_vecMaxs();
		m_duck = player->m_flDuckAmount();
		m_velocity = player->m_vecVelocity();
		m_old_sim_time = player->m_flOldSimulationTime();
		m_old_origin = player->m_vecOldOrigin();

		// save networked animlayers.
		player->GetAnimLayers( m_layers );

		// normalize eye angles.
		m_eye_angles.normalize();
		math::clamp( m_eye_angles.x, -90.f, 90.f );

		// get lag.
		m_lag_time = m_sim_time - m_old_sim_time;
		m_lag = game::TIME_TO_TICKS( m_sim_time - m_old_sim_time );

		// compute animtime.
		m_anim_time = m_old_sim_time + g_csgo.m_globals->m_interval;
	}

	__forceinline void clear()
	{
		m_player = nullptr;
		m_immune = -1.f;
		m_tick = -1;

		// netvars.
		m_sim_time = -1.f;;
		m_flags = -1.f;;
		m_origin = m_interp_org = vec3_t();;
		m_eye_angles = m_fake_angles = ang_t();
		m_abs_ang = m_interp_ang = ang_t();;
		m_body = -1.f;;
		m_mins = vec3_t();;
		m_maxs = vec3_t();;
		m_duck = -1.f;;
		m_velocity = vec3_t();;
		m_old_sim_time = -1.f;;
		m_old_origin = vec3_t();;
		m_lag_time = 0.f;
		m_lag = 0;
		m_anim_time = 0;

		for ( int i = 0; i < 13; i++ )
		{
			m_layers[ i ].m_anim_time = 0;
			m_layers[ i ].m_fade_out_time = 0;
			m_layers[ i ].m_prev_cycle = 0;
			m_layers[ i ].m_weight = 0;
			m_layers[ i ].m_weight_delta_rate = 0;
			m_layers[ i ].m_playback_rate = 0;
			m_layers[ i ].m_cycle = 0;
			m_layers[ i ].m_bits = 0;
			m_layers[ i ].m_owner = nullptr;
		}
	}

	// function: writes current record to bone cache.
	__forceinline void cache()
	{
		// get bone cache ptr.
		CBoneCache* cache = &m_player->m_BoneCache();

		cache->m_pCachedBones = m_bones;
		cache->m_CachedBoneCount = 128;

		m_player->m_vecOrigin() = m_origin;
		m_player->SetCollisionBounds(m_mins, m_maxs);

		m_player->SetAbsAngles( m_abs_ang );
		m_player->SetAbsOrigin( m_origin );
	}

	__forceinline bool immune()
	{
		return m_immune > 0.f;
	}

	// keeping this because its useful for stuff like early autostop
	// we could check it against interp but its an intensive process for very little gain.
	bool valid( float custom_time = 0.f, float custom_timebase = 0.f )
	{
		float shift_time = g_tickbase.m_pre_shift_ticks * g_csgo.m_globals->m_interval;
		float simtime = custom_time == 0.f ? m_sim_time : custom_time;
		float curtime = custom_timebase == 0.f ? g_csgo.m_globals->m_curtime : custom_timebase;

		if ( simtime < floorf( ( curtime - shift_time ) - g_csgo.sv_maxunlag->GetFloat() ) )
			return false;

		float correct = 0;

		correct += g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING );
		correct += g_csgo.m_net->GetLatency( INetChannel::FLOW_INCOMING );
		correct += g_ctx.m_lerp;

		correct = std::clamp( correct, 0.f, g_csgo.sv_maxunlag->GetFloat() );

		return fabsf( correct - ( ( curtime - shift_time ) - simtime ) ) <= 0.2f;
	}
};