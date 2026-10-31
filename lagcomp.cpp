#include "includes.h"

LagCompensation g_lagcomp{};;

void AimPlayer::UpdatePlayerAnims( CCSGOPlayerAnimState* state )
{
	if ( state->m_last_update_frame >= g_csgo.m_globals->m_frame )
	{
		state->m_last_update_frame = g_csgo.m_globals->m_frame - 1;
	}

	m_player->m_bClientSideAnimation() = true;
	g_hooks.m_UpdateClientSideAnimation( m_player );
	m_player->m_bClientSideAnimation() = false;
}

bool AimPlayer::EstimateVelocity( Player* m_player, LagRecord* record, LagRecord* previous, LagRecord* pre_previous, float max_speed )
{
	// set a base velocity estimate at animtime.
	auto accel = ( ( record->m_velocity - previous->m_velocity ) / record->m_lag_time ) * g_csgo.m_globals->m_interval;
	record->m_anim_velocity = previous->m_velocity + accel;

	if ( !( record->m_flags & FL_ONGROUND ) )
		return false;

	// both speeds use 2d velocity.  
	const float average_speed = record->m_anim_velocity.length_2d();
	const auto* l6 = &record->m_layers[ 6 ];
	const auto* l11 = &record->m_layers[ 11 ];

	// layer6 is clamped to WalkToTopSpeed (not run), so we only want to use it < 1.f weight.
	// we also already check if its over 0 in the fakewalk check.
	// also if we dont check > prev then ppl go invisible when they deaccelerate LOL
	if ( l6->m_weight < 1.f && l6->m_weight > previous->m_layers[ 6 ].m_weight )
	{
		auto closest_speed_to_minimal = max_speed * 0.34f;
		auto spd_multiplier = std::fmaxf( 0.f, ( max_speed * 0.52f ) - ( max_speed * 0.34f ) );

		auto v208 = 1.f - record->m_duck;

		auto speed_via_6th_layer = ( ( ( v208 * spd_multiplier ) + closest_speed_to_minimal ) * l6->m_weight ) / average_speed;

		record->m_anim_velocity *= speed_via_6th_layer;
		record->m_anim_velocity *= speed_via_6th_layer;
		return true;
	}

	// this logic should (?) work and make sense and be nice and happy
	// also run this after cuz if l6 weight < 1 then we know for sure they aint static velocity.
	if ( !record->m_left_dormancy
		&& !previous->m_left_dormancy
		&& pre_previous && ( record->m_origin - previous->m_origin ) == ( previous->m_origin - pre_previous->m_origin ) )
		return true;

	if ( l11->m_weight > 0.f
		&& l11->m_weight < 1.f
		&& l11->m_playback_rate == previous->m_layers[ 11 ].m_playback_rate )
	{
		const auto speed_as_portion_of_run_top_speed = 0.55f + ( 1.f - l11->m_weight ) * 0.35f;
		const auto velocity_length_xy = speed_as_portion_of_run_top_speed * max_speed;

		record->m_anim_velocity.x *= ( velocity_length_xy / average_speed );
		record->m_anim_velocity.y *= ( velocity_length_xy / average_speed );
		return true;
	}

	return false;
}

bool AimPlayer::UpdateAnimations( LagRecord* record )
{
	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState();
	if ( !state )
		return false;

	if ( m_player->m_flSpawnTime() != m_spawn )
	{
		game::ResetAnimationState( state );
		m_spawn = m_player->m_flSpawnTime();
	}

	const float curtime = g_csgo.m_globals->m_curtime;
	const float frametime = g_csgo.m_globals->m_frametime;

	LagRecord* previous = m_records.size() > 1 ? m_records[ 1 ].get() : nullptr;
	LagRecord* pre_previous = m_records.size() > 2 ? m_records[ 2 ].get() : nullptr;

	record->store( m_player );
	record->m_left_dormancy = m_went_dormant;

	const bool valid_update = !previous || record->m_sim_time > m_highest_simtime;

	g_csgo.m_globals->m_curtime = record->m_anim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	AnimationBackup_t backup;
	backup.store( m_player );

	record->m_fake_walk = false;
	record->m_mode = Resolver::Modes::RESOLVE_NONE;

	// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
	if ( record->m_lag > 0 || m_went_dormant )
	{
		if ( previous && !m_went_dormant )
		{
			record->m_velocity = ( record->m_origin - previous->m_origin ) * ( 1.f / game::TICKS_TO_TIME( record->m_lag ) );

			// restore previous animation data, makes sure our new anim update is accurate.
			m_player->SetAnimLayers( previous->m_layers );

			state->m_primary_cycle = previous->m_layers[ 6 ].m_cycle;
			state->m_move_weight = previous->m_layers[ 6 ].m_weight;
			state->m_strafe_sequence = previous->m_layers[ 7 ].m_sequence;
			state->m_strafe_change_weight = previous->m_layers[ 7 ].m_weight;
			state->m_strafe_change_cycle = previous->m_layers[ 7 ].m_cycle;
			state->m_acceleration_weight = previous->m_layers[ 12 ].m_weight;

			record->m_broke_lc = ( record->m_origin - previous->m_origin ).length_sqr() > 4096.0f;
			m_breaking_lc = record->m_broke_lc;
		}
		// bro was dormant he coulda desync walked.
		else
		{
			m_detected_desync = false;

			// could be lower than 58 here,
			// depending on velocity etc.
			m_max_feet_yaw = record->m_body + 58.f;
			m_min_feet_yaw = record->m_body - 58.f;
		}
	}

	record->m_anim_velocity = record->m_velocity;

	if ( record->m_lag > 1 )
	{
		m_accurate_velocity = false;

		if ( record->m_flags & FL_ONGROUND && record->m_layers[ 6 ].m_weight == 0.f )
		{
			record->m_fake_walk = true;
			record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };
		}

		if ( previous && !m_went_dormant )
		{
			m_player->m_fFlags() = previous->m_flags;
			m_player->m_fFlags() &= ~FL_ONGROUND;

			if ( record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND )
			{
				m_player->m_fFlags() |= FL_ONGROUND;
			}
			else
			{
				if ( record->m_layers[ 4 ].m_weight != 1.f && previous->m_layers[ 4 ].m_weight == 1.f && record->m_layers[ 5 ].m_weight != 0.f )
				{
					m_player->m_fFlags() |= FL_ONGROUND;
				}
				if ( record->m_flags & FL_ONGROUND && !( previous->m_flags & FL_ONGROUND ) )
				{
					m_player->m_fFlags() &= ~FL_ONGROUND;
				}
			}

			float duck = record->m_duck - previous->m_duck;
			float time = record->m_sim_time - previous->m_sim_time;

			float change = ( duck / time ) * g_csgo.m_globals->m_interval;

			m_player->m_flDuckAmount() = previous->m_duck + change;

			if ( !record->m_fake_walk )
			{
				float max_spd = 260.f;

				auto weapon = m_player->GetActiveWeapon();
				if ( weapon && !weapon->IsKnife() )
				{
					auto wpndata = weapon->GetWpnData();
					if ( wpndata )
					{
						max_spd = weapon->m_zoomLevel() == 0 ? wpndata->m_max_player_speed_alt : wpndata->m_max_player_speed;
					}
				}

				m_accurate_velocity = EstimateVelocity( record->m_player, record, previous, pre_previous, max_spd );

				// this happens very occasionally and makes ppl go yabadabadoo
				// also this is better than just setting it to 0 like the old 1 did (obviously). 
				if ( !record->m_anim_velocity.valid() )
				{
					record->m_anim_velocity = record->m_velocity;
					m_accurate_velocity = false;
				}
			}
		}
	}
	else
	{
		m_accurate_velocity = true;
	}

	m_player->SetAbsOrigin( record->m_origin );

	m_player->m_vecVelocity() = m_player->m_vecAbsVelocity() = record->m_anim_velocity;
	m_player->m_iEFlags() &= ~0x1000;

	g_resolver.Resolve( this, record );
	g_resolver.UpdateTransition( this, record, previous, state );

	m_player->m_angEyeAngles() = record->m_eye_angles;

	record->m_old_state = *state;

	UpdatePlayerAnims( state );

	g_server_animations.SimulateFeetYaw( state, record->m_body, record->m_body/*record->m_eye_angles.y*/, m_max_feet_yaw );
	g_server_animations.SimulateFeetYaw( state, record->m_body, record->m_body/*record->m_eye_angles.y*/, m_min_feet_yaw );

	record->m_max_feet_yaw = m_max_feet_yaw;
	record->m_min_feet_yaw = m_min_feet_yaw;

	if ( !m_went_dormant && m_player->enemy( g_ctx.m_local ) && record->m_mode == Resolver::Modes::RESOLVE_MOVE && previous )
	{
		if ( previous->m_mode <= Resolver::Modes::RESOLVE_THRESHOLD )
		{
			record->m_max_feet_yaw = m_max_feet_yaw = record->m_body + 58.f;
			record->m_min_feet_yaw = m_min_feet_yaw = record->m_body - 58.f;
		}

		g_resolver.ResolveTwist( this, record, previous, state );
	}
	else
		m_detected_desync = false;

	m_player->GetPoseParameters( record->m_poses );
	record->m_abs_ang = record->m_interp_ang = m_player->GetAbsAngles();

	m_player->SetAnimLayers( record->m_layers ); // bone setup does NOT do this
	g_bones.setup( m_player, nullptr, record );

	backup.restore( m_player );

	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = frametime;

	return valid_update;
}

void AimPlayer::OnNetUpdate( Player* player )
{
	if ( !g_Variables.m_bools["aimbot_enabled"] || !player->alive())
	{
		player->m_bClientSideAnimation() = true;
		reset();
		return;
	}

	m_get_death_yaw = true;

	if ( !m_player || m_player != player )
	{
		reset();

		m_shot_log.clear();
		m_logged_timer = -1;

		//m_ignore = false; // sus
	}

	m_player = player;
	m_breaking_lc = false;

	if ( player->dormant() )
	{
		m_went_dormant = true;
		return;
	}

	if ( last_alive_loop_data.m_flCycle == m_player->m_AnimOverlay()[ 11 ].m_cycle
		&& last_alive_loop_data.m_flPlaybackRate == m_player->m_AnimOverlay()[ 11 ].m_playback_rate )
	{
		// IF THEY COME OUT OF DORMANCY N HAVNT UPDATED
		// Then we dont want to break tings for no reasonnn.
		m_went_dormant = false;
		return;
	}

	last_alive_loop_data.m_flCycle = m_player->m_AnimOverlay()[ 11 ].m_cycle;
	last_alive_loop_data.m_flPlaybackRate = m_player->m_AnimOverlay()[ 11 ].m_playback_rate;

	LagRecord* current = m_records.emplace_front( std::make_shared<LagRecord>() ).get();
	if ( UpdateAnimations( current ) )
	{
		m_exploit = false;
	}
	else
	{
		current->m_immune = 1.f;
		m_exploit = true;
	}

	m_highest_simtime = std::max( m_highest_simtime, current->m_sim_time );
	m_went_dormant = false;

	while ( m_records.size() > g_ctx.m_tickrate )
		m_records.pop_back();
}

vec3_t GETTHELOCALPOSLOL( int hitbox )
{
	const model_t* model = g_ctx.m_local->GetModel();
	if ( !model )
		return{ 0, 0, 0 };

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return{ 0, 0, 0 };

	mstudiohitboxset_t* set = hdr->GetHitboxSet( g_ctx.m_local->m_nHitboxSet() );
	if ( !set )
		return{ 0, 0, 0 };

	mstudiobbox_t* bbox = set->GetHitbox( hitbox );
	if ( !bbox )
		return{ 0, 0, 0 };

	vec3_t center = (bbox->m_mins + bbox->m_maxs) * 0.5f;

	math::VectorTransform( center, g_ctx.m_local->m_BoneCache().m_pCachedBones[ bbox->m_bone ], center );

	return center;
}

void LagCompensation::SimulatePlayer( Player* player, float time, float speed, float direction, PreviousData_t& old_data )
{
	static CUserCmd cmd;
	static CMoveData moveData;

	memset( &cmd, 0, sizeof( CUserCmd ) );
	memset( &moveData, 0, sizeof( CMoveData ) );

	const float backupAttack = player->m_flNextAttack();

	cmd.m_view_angles = player->m_angEyeAngles();
	cmd.m_tick = game::TIME_TO_TICKS( time );
	cmd.m_command_number = g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands + 1;

	if ( player->m_fFlags() & FL_ONGROUND && !( old_data.m_flags & FL_ONGROUND ) )
	{
		player->m_flNextAttack() = time;
		cmd.m_buttons |= IN_JUMP;
	}
	else
		player->m_flNextAttack() = 0.f;

	if ( player->m_flDuckAmount() > old_data.m_duck || player->m_fFlags() & FL_DUCKING )
		cmd.m_buttons |= IN_DUCK;

	if ( player->m_iMoveState() == 1 )
		cmd.m_buttons |= IN_SPEED;

	auto movedir = cmd.m_view_angles;
	movedir.y = direction;

	cmd.m_forward_move = 450.f;

	g_movement.FixMove( &cmd, movedir, false );

	const auto originalplayercommand = player->m_pCurrentCommand() ? *player->m_pCurrentCommand() : CUserCmd();

	const auto originalrandomseed = *g_csgo.m_nPredictionRandomSeed;
	const auto PredictionPlayer = g_csgo.m_pPredictionPlayer;

	const auto backupInPrediction = g_csgo.m_prediction->m_in_prediction;
	const auto backupFirstTimePredicted = g_csgo.m_prediction->m_first_time_predicted;

	const auto backupCurtime = g_csgo.m_globals->m_curtime;
	const auto backupFrametime = g_csgo.m_globals->m_frametime;

	{
		g_csgo.m_prediction->m_in_prediction = true;
		g_csgo.m_prediction->m_first_time_predicted = false;

		player->m_pCurrentCommand() = &cmd;

		*g_csgo.m_nPredictionRandomSeed = g_csgo.MD5_PseudoRandom( cmd.m_command_number ) & 0x7fffffff;
		g_csgo.m_pPredictionPlayer = player;

		g_csgo.m_globals->m_curtime = time;
		g_csgo.m_globals->m_frametime = g_csgo.m_prediction->m_engine_paused ? 0.f : g_csgo.m_globals->m_interval;

		g_csgo.m_move_helper->SetHost( player );
		g_csgo.m_prediction->SetupMove( player, &cmd, g_csgo.m_move_helper, &moveData );
		g_csgo.m_game_movement->ProcessMovement( player, &moveData );
		g_csgo.m_prediction->FinishMove( player, &cmd, &moveData );
		g_csgo.m_move_helper->SetHost( nullptr );
	}

	*player->m_pCurrentCommand() = originalplayercommand;

	*g_csgo.m_nPredictionRandomSeed = originalrandomseed;
	g_csgo.m_pPredictionPlayer = PredictionPlayer;

	g_csgo.m_prediction->m_in_prediction = backupInPrediction;
	g_csgo.m_prediction->m_first_time_predicted = backupFirstTimePredicted;

	player->m_flNextAttack() = backupAttack;

	g_csgo.m_globals->m_curtime = backupCurtime;
	g_csgo.m_globals->m_frametime = backupFrametime;
}

LagCompensation::LagCompData_t LagCompensation::GetLagcompData( AimPlayer* data )
{
	if ( data->m_records.size() < 2 )
		return { 0, false, false };

	LagRecord* front = data->m_records[ 0 ].get();
	LagRecord* previous = data->m_records[ 1 ].get();

	LagCompData_t ret{ 0, false, false };

	// factor in switch fakelag.
	int expected_lag = std::clamp( ( !previous || previous->m_left_dormancy ) ? front->m_lag : previous->m_lag, 1, g_csgo.sv_maxusrcmdprocessticks->GetInt() + 2 );

	ret.m_extrap_ticks = expected_lag;

	int t = game::TIME_TO_TICKS( g_ctx.m_latency );
	int t_1w = game::TIME_TO_TICKS( g_ctx.m_latency * 0.5f );

	int elapsed = ( g_csgo.m_cl->m_server_tick + t_1w ) - front->m_tick;

	// +1 because we dont send packet until the tick after we shoot.
	if ( t + 1 <= expected_lag )
	{
		ret.m_needs_extrap = false;
		ret.m_can_fire = elapsed <= t_1w + 1;
		return ret;
	}

	ret.m_needs_extrap = true;

	// too unsafe to extrapolate them.
	if ( front->m_left_dormancy )
	{
		ret.m_can_fire = false;
		return ret;
	}

	ret.m_can_fire = ( front->m_tick + ( expected_lag * 2 ) ) - elapsed > t_1w;
	return ret;
}

bool LagCompensation::valid_interp( AimPlayer* data, LagRecord* record, bool allow_interp )
{
	//float shift_time = game::TICKS_TO_TIME( g_tickbase.m_pre_shift_ticks );
	//float curtime = g_csgo.m_globals->m_curtime - shift_time;

	// we are gonna call from visuals so yeh
	float curtime = game::TICKS_TO_TIME( g_ctx.m_local->m_nTickBase() - g_tickbase.m_pre_shift_ticks );

	// deadtime.
	if ( record->m_sim_time < floorf( curtime - g_csgo.sv_maxunlag->GetFloat() ) )
		return false;

	float correct = 0;
	correct += g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING );
	correct += g_csgo.m_net->GetLatency( INetChannel::FLOW_INCOMING );
	correct += g_ctx.m_lerp;

	correct = std::clamp( correct, 0.f, g_csgo.sv_maxunlag->GetFloat() );

	// hi we dont factor lerp into this
	// we add it into our cmd->tickcount because it gets subtracted on the server
	// but the goal is to get targettime to match the record's sim time.
	float flTargetTime = record->m_sim_time;

	// calculate difference between tick sent by player and our latency based tick
	float deltaTime = correct - ( curtime - flTargetTime );

	if ( fabs( deltaTime ) > 0.2f )
	{
		// difference between cmd time and latency is too big > 200ms, use time correction based on latency
		flTargetTime = curtime - correct;
	}

	static ConVar* sv_lagpushticks = g_csgo.m_cvar->FindVar( HASH( "sv_lagpushticks" ) );
	flTargetTime += game::TICKS_TO_TIME( sv_lagpushticks->GetInt() );

	if ( record->m_sim_time > flTargetTime )
		return false;

	if ( !allow_interp )
		return true;

	if ( data->m_records.size() > 1 )
	{
		LagRecord* next = nullptr;

		// find the next record.
		for ( int i = data->m_records.size() - 1; i >= 0; i-- )
		{
			// sim time will break here with tickbase manipulation, use stored server tick instead.
			next = data->m_records[ i ].get();
			if ( next->m_tick > record->m_tick )
				break;

			next = nullptr;
		}

		if ( next
			&& record->m_sim_time < flTargetTime
			&& record->m_sim_time < next->m_sim_time )
		{
			// we didn't find the exact time but have a valid previous record
			// so interpolate between these two records;

			float frac = ( flTargetTime - record->m_sim_time ) / ( next->m_sim_time - record->m_sim_time );

			// this will happen on low ping.
			if ( frac < 0 || frac > 1 )
				return false;

			auto ang = math::Lerp( frac, record->m_abs_ang, next->m_abs_ang );
			auto org = math::Lerp( frac, record->m_origin, next->m_origin );

			if ( record->m_interp_ang != ang || record->m_interp_org != org )
			{
				record->m_interp_ang = ang;
				record->m_interp_org = org;

				record->m_interp_time = next->m_lag * frac;

				g_bones.update_interp( record );
			}
		}
		// in theory this could happen
		// since ping spike delays the backtrack window
		// a new record could be invalid, and then become valid, in which case we want to make sure its not interpolated.
		//else if ( record->m_interp_ang != record->m_abs_ang || record->m_interp_org != record->m_origin )
		//{
		//	record->m_interp_ang = record->m_abs_ang;
		//	record->m_interp_org = record->m_origin;
		//
		//	record->m_interp_time = 0.f;
		//
		//	g_bones.update_interp( record );
		//}
	}

	return true;
}