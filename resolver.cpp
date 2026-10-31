#include "includes.h"

Resolver g_resolver{};;

void Resolver::OnBodyUpdate( Player* player, float value )
{
	AimPlayer* data = &g_aimbot.m_players[ player->index() - 1 ];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;

	if ( data->m_body != data->m_old_body )
	{
		data->m_lby_changed = data->m_lby_change_standing = true;
		data->m_last_lby_delta = math::AngleDiff( data->m_old_body, data->m_body );
	}
	else
		data->m_lby_changed = false;
}

void Resolver::MatchShot( AimPlayer* data, LagRecord* record )
{
	Weapon* weapon = data->m_player->GetActiveWeapon();
	if ( !weapon )
		return;

	WeaponInfo* weaponinfo = weapon->GetWpnData();
	if ( !weaponinfo )
		return;

	int type = weaponinfo->m_weapon_type;

	if ( type == WEAPONTYPE_C4 
		|| type == WEAPONTYPE_GRENADE 
		|| type == WEAPONTYPE_KNIFE )
		return;

	float shot_time = weapon->m_fLastShotTime();

	if ( shot_time != data->m_last_shot_time )
	{
		if ( shot_time - record->m_old_sim_time > g_csgo.m_globals->m_interval )
		{
			for ( int i = 0; i < data->m_records.size(); i++ )
			{
				LagRecord* nigga = data->m_records[ i ].get();
				if ( !nigga )
					continue;

				if ( abs( math::NormalizedAngle( nigga->m_eye_angles.x - record->m_eye_angles.x ) ) > 10.f )
				{
					record->m_eye_angles.x = nigga->m_eye_angles.x;
					break;
				}
			}
		}

		data->m_last_shot_time = shot_time;
	}
}

float Resolver::SnapToNearest( float angle, std::vector<float> angles )
{
	float ret = 0.f;
	float lowest_diff = std::numeric_limits<float>::max();

	for ( int i = 0; i < angles.size(); i++ )
	{
		float delta = abs( math::NormalizedAngle( angle - angles[ i ] ) );

		if ( delta < lowest_diff )
		{
			ret = angles[ i ];
			lowest_diff = delta;
		}
	}

	return ret;
}

void Resolver::SetConfidence( AimPlayer* data, LagRecord* record, bool balance_adjust, int clamp_preference )
{
	float confidence = AimPlayer::ResolverConfidence_t::VERY_LOW;
	float& a = record->m_eye_angles.y;

	confidence += 1.f - ( abs( math::NormalizedAngle( a - record->m_backwards_angle ) ) / 180.f );
	confidence += 1.f - ( abs( math::NormalizedAngle( a - record->m_freestand_angle ) ) / 180.f );

	switch ( clamp_preference )
	{
	case unsure:
		if ( balance_adjust )
		{
			confidence -= 1.f - ( abs( math::NormalizedAngle( a - record->m_body ) ) / 180.f );
		}
		else
		{
			confidence += 1.f - ( abs( math::NormalizedAngle( a - record->m_body ) ) / 180.f );
		}
		break;
	case bias_clamped:
		confidence += 1.f - ( abs( math::NormalizedAngle( a - ( record->m_body + 180.f ) ) ) / 180.f );
		break;
	case bias_unclamped:
		confidence += 1.f - ( abs( math::NormalizedAngle( a - record->m_body ) ) / 180.f );
		break;
	// this shit is unhittable 1337
	case neutral:
		confidence -= 2.f;
		break;
	}

	if ( g_hvh.IsYawSideways( g_ctx.m_shoot_pos, record->m_origin, a ) )
		confidence += 1.f;

	// dont want it going < 0 with balance_adjust
	data->m_resolver_confidence = std::max( 0, static_cast<int>( confidence ) );
}

void Resolver::CheckAnimations( LagRecord* record, AimPlayer* data )
{
	int new_count = 0, old_count = 0;

	bool old = false;

	for ( int i = 0; i < data->m_records.size(); i++ )
	{
		LagRecord* cur = data->m_records[ i ].get();
		if ( cur->m_left_dormancy )
			break;

		if ( cur->m_layers[ 3 ].m_weight == 0.f && cur->m_layers[ 3 ].m_cycle == 0.f )
		{
			if ( old )
				++old_count;
			else
				++new_count;

			if ( i == 0 && data->m_player->GetSequenceActivity( cur->m_layers[ 3 ].m_sequence ) == 979 && !data->m_cant_predict_lby )
			{
				data->m_update_offsets.push_front( { data->m_updates_post_lby, record->m_anim_time } );
			}
		}

		if ( cur->m_mode == RESOLVE_UPDATE )
		{
			if ( old )
			{
				break;
			}
			else
			{
				old = true;
			}
		}
	}

	if ( !old )
		return;

	data->m_979_count = new_count;
	data->m_old_979_count = old_count;
}

int Resolver::GetTimingOffset( AimPlayer* data )
{
	constexpr int max_ticks = 37;

	if ( !data->m_blacklisted_timings.empty() )
	{
		for ( int i = 0; i <= max_ticks; i++ )
		{
			bool found_timing = false;

			for ( int j = 0; j < data->m_blacklisted_timings.size(); j++ )
			{
				if ( data->m_blacklisted_timings[ j ] == i )
				{
					found_timing = true;
					break;
				}
			}

			if ( !found_timing )
			{
				return i;
			}
		}

		// we tried every possibility bruzzz... how we havnt run out of ammo? Genius.
		return -1;
	}

	if ( data->m_logged_timer != -1 )
		return data->m_logged_timer;

	if ( data->m_update_offsets.empty() )
		return 0;

	static std::vector<std::pair<int, int>> options;
	options.clear();

	for ( int i = 0; i < data->m_update_offsets.size(); i++ )
	{
		auto* cur = &data->m_update_offsets[ i ];

		// we probably dont want to use anim timing if they stopped triggering anim.
		if ( i == 0 && data->m_last_update_time - cur->second > 1.1f )
			return 0;

		for ( int j = i + 1; j < data->m_update_offsets.size(); j++ )
		{
			auto* old = &data->m_update_offsets[ j ];

			for ( int k = j + 1; k < data->m_update_offsets.size(); k++ )
			{
				auto* older = &data->m_update_offsets[ k ];

				if ( cur->first == old->first && cur->first == older->first )
					options.push_back( { cur->first, AimPlayer::LBYDelayType_t::CONSTANT } );
				else if ( cur->first - old->first == old->first - older->first )
					options.push_back( { cur->first - old->first, AimPlayer::LBYDelayType_t::REPEATING } );
			}
		}
	}

	if ( options.empty() )
		return 0;

	struct frequency_t
	{
		int constant_count = 0;
		int repeating_count = 0;
	};

	static std::unordered_map<int, frequency_t> freq;
	freq.clear();

	for ( const auto& option : options )
	{
		// this is impossible.
		// 0.58 / 37.12 is max time.
		if ( option.first > max_ticks || option.first < 0 )
			continue;

		if ( option.second == AimPlayer::LBYDelayType_t::CONSTANT )
			freq[ option.first ].constant_count++;
		else if ( option.second == AimPlayer::LBYDelayType_t::REPEATING )
			freq[ option.first ].repeating_count++;
	}

	int best_total = 0;

	int best_offset = 0;
	int best_type = 0;

	for ( const auto& entry : freq )
	{
		int total = entry.second.constant_count + entry.second.repeating_count;

		if ( total > best_total )
		{
			best_total = total;
			best_offset = entry.first;
			best_type = entry.second.repeating_count > entry.second.constant_count ? AimPlayer::LBYDelayType_t::REPEATING : AimPlayer::LBYDelayType_t::CONSTANT;
		}
	}

	// since 979 only triggers to one side
	// we want to see if this is likely to be happening.
	if ( best_offset > 0.f )
	{
		for ( const auto& entry : freq )
		{
			if ( entry.first == best_offset - 1 )
			{
				if ( entry.second.constant_count + entry.second.repeating_count > best_total * 0.5f )
				{
					best_offset -= 1;
					break;
				}
			}
		}
	}

	// clear this down here so the ting above ^ has a larger sample size to detect with.
	while ( data->m_update_offsets.size() > 64 )
		data->m_update_offsets.pop_back();

	// this repeating shit is bungus as fuck bruzz...
	// todo: figure out a way to get this workinnn... idk if anyone uses this anyways tho maybe fam or astral idk wtf Kill me
	if ( best_type == AimPlayer::LBYDelayType_t::REPEATING )
		return -INT_MAX;

	return best_offset;
}

void Resolver::Resolve( AimPlayer* data, LagRecord* record )
{
	if ( !g_ctx.m_local->enemy( record->m_player ) )
		return;

	MatchShot( data, record );

	SetFacingPlayer( data, record );

	if ( !g_ctx.m_processing && data->m_facing_player && g_ctx.m_local->GetObserverTarget() == data->m_player )
	{
		g_csgo.m_debug_overlay->AddLineOverlay( record->m_origin, data->m_facing_player->m_vecOrigin(), 255, 0, 0, true, 0.1f );
	}

	if ( data->m_went_dormant && data->m_records.size() > 1 )
	{
		LagRecord* previous = data->m_records[ 1 ].get();

		// animtime is old simtime + 1 tick
		// and if they just came out of dormancy
		// old simtime will be fucked lol.
		if ( ( record->m_origin - previous->m_origin ).length() > 10.f
			|| record->m_sim_time > data->m_lby_timer )
		{
			data->m_cant_predict_lby = true;
		}
	}

	// lol shit but it lets us keep updating data if they go to 0 choke.
	const float old_angles = record->m_eye_angles.y;

	if ( record->m_flags & FL_ONGROUND )
	{
		if ( record->m_velocity.length_2d() <= 0.1f )
		{
			ResolveStand( data, record );
		}
		else
		{
			ResolveWalk( data, record );
		}

		data->m_air_angle = record->m_eye_angles.y;
	}
	else
	{
		ResolveAir( data, record );
	}

	if (record->m_lag <= 1 || !g_Variables.m_bools["aimbot_resolve_fake"])
	{
		record->m_eye_angles.y = old_angles;
	}

	if ( data->m_override != 0.f && record->m_mode <= RESOLVE_THRESHOLD )
	{
		data->m_override_agreed = abs( math::NormalizedAngle( record->m_eye_angles.y - data->m_override ) ) < 45.f;
		record->m_eye_angles.y = data->m_override;
	}
}

std::pair<float, bool>  Resolver::FreestandYaw( AimPlayer* data, Player* from_player, std::vector<AdaptiveAngle> angles, bool get_worst )
{
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	bool valid{ false };

	vec3_t shoot_pos = g_ctx.m_shoot_pos;

	if ( from_player && from_player != g_ctx.m_local )
	{
		from_player->GetEyePos( &shoot_pos );
	}

	vec3_t enemy_pos; data->m_player->GetEyePos( &enemy_pos );

	vec3_t closest_pos{};
	int lowest_iterator = std::numeric_limits<int>::max();

	for ( auto it = angles.begin(); it != angles.end(); ++it )
	{
		vec3_t trace_start{ enemy_pos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			enemy_pos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			enemy_pos.z };

		vec3_t dir = shoot_pos - trace_start;
		float len = dir.normalize();

		for ( float i{ 0.f }; i < len; i += STEP )
		{
			if ( g_csgo.m_engine_trace->GetPointContents( trace_start + ( dir * i ), MASK_SHOT_HULL ) & MASK_SHOT_HULL )
			{
				it->m_dist += STEP;
				valid = true;

				if ( i < lowest_iterator )
				{
					closest_pos = trace_start + ( dir * i );
					lowest_iterator = i;
				}
			}
		}
	}

	if ( !valid )
	{
		return { get_worst ? angles.back().m_yaw : angles[ 0 ].m_yaw, false };
	}

	std::sort( angles.begin(), angles.end(), [&get_worst]( const AdaptiveAngle& a, const AdaptiveAngle& b )
		{
			return get_worst ? a.m_dist < b.m_dist : a.m_dist > b.m_dist;
		} );

	return { angles.front().m_yaw, ( closest_pos - shoot_pos ).length() * 0.25f > ( closest_pos - enemy_pos ).length() };
}

void AimPlayer::OnImpact( ImpactRecord* impact )
{
	int target_hitbox = impact->m_shot->m_hitbox;
	int actual_hitgroup = impact->m_ACTUALLY_HITGROUP_TAPPER;
	
	if ( target_hitbox != HITBOX_HEAD )
		return;
	
	LagRecord* shot_record = impact->m_shot->m_record;
	
	if ( !shot_record 
		|| !( shot_record->m_flags & FL_ONGROUND ) 
		|| !( shot_record->m_mode == Resolver::Modes::RESOLVE_STAND || shot_record->m_mode == Resolver::Modes::RESOLVE_UPDATE ) )
		return;
	
	CCSGOPlayerAnimState* animstate = shot_record->m_player->m_PlayerAnimState();
	if ( !animstate )
		return;
	
	CCSGOPlayerAnimState backup_state = *animstate;
	
	float old_poses[ 24 ]; 
	shot_record->m_player->GetPoseParameters( old_poses );
	
	C_AnimationLayer old_layers[ 13 ]; 
	shot_record->m_player->GetAnimLayers( old_layers );
	
	LagRecord* record = &g_resolver.m_resolver_record;
	
	{
		record->m_player = shot_record->m_player;
		record->m_immune = shot_record->m_immune;
		record->m_tick = shot_record->m_tick;
		record->m_lag = shot_record->m_lag;
		record->m_sim_time = shot_record->m_sim_time;
		record->m_old_sim_time = shot_record->m_old_sim_time;
		record->m_flags = shot_record->m_flags;
		record->m_origin = shot_record->m_origin;
		record->m_old_origin = shot_record->m_old_origin;
		record->m_velocity = shot_record->m_velocity;
		record->m_mins = shot_record->m_mins;
		record->m_maxs = shot_record->m_maxs;
		record->m_eye_angles = shot_record->m_eye_angles;
		record->m_fake_angles = shot_record->m_fake_angles;
		record->m_body = shot_record->m_body;
		record->m_duck = shot_record->m_duck;
	
		for ( int i = 0; i < 13; ++i )
		{
			record->m_layers[ i ] = shot_record->m_layers[ i ];
		}
	
		for ( int i = 0; i < 24; ++i )
		{
			record->m_poses[ i ] = shot_record->m_poses[ i ];
		}
	
		record->m_old_state = shot_record->m_old_state;
	
		record->m_anim_velocity = shot_record->m_anim_velocity;
		record->m_max_feet_yaw = shot_record->m_max_feet_yaw;
		record->m_min_feet_yaw = shot_record->m_min_feet_yaw;
	
		if ( !record->m_bones )
		{
			record->m_bones = (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );
		}
	
		record->m_abs_ang = shot_record->m_abs_ang;
		record->m_broke_lc = shot_record->m_broke_lc;
		record->m_extrapolated = shot_record->m_extrapolated;
		record->m_lby_offset = shot_record->m_lby_offset;
		record->m_mode = shot_record->m_mode;
		record->m_fake_walk = shot_record->m_fake_walk;
		record->m_away = shot_record->m_away;
		record->m_anim_time = shot_record->m_anim_time;
		record->m_interp_time = shot_record->m_interp_time;
	}
	
	auto trace_start = impact->m_shot->m_pos;
	auto shot_dir = ( impact->m_pos - trace_start ).normalized();
	auto trace_end = trace_start + ( shot_dir * impact->m_shot->m_weapon_dist );

	if ( !impact->m_hit )
	{
		if ( record->m_mode == Resolver::Modes::RESOLVE_UPDATE )
		{
			if ( m_logged_timer == record->m_lby_offset )
			{
				m_logged_timer = -1;
			}

			m_blacklisted_timings.emplace_back( record->m_lby_offset );
		}
		else
			++m_missed_shots;
	
		BackupRecord backup_record;
	
		for ( int i = 0; i < 360; i += 10 )
		{
			if ( i != 0 )
			{
				record->m_eye_angles.y += i;

				*animstate = record->m_old_state;

				animstate->m_last_update_frame = g_csgo.m_globals->m_frame - 1;
				animstate->update_( record->m_eye_angles );

				record->m_player->GetPoseParameters( record->m_poses );
				record->m_abs_ang = { 0.f, animstate->m_abs_yaw, 0.f };

				if ( g_bones.BuildBones( record->m_player, BONE_USED_BY_HITBOX, record->m_bones, record ) )
				{
					CGameTrace tr;

					// realllyyyy shit way of doing this
					// but as far as i can tell it fixes the bone setup issues
					// so idgaf LOL XD LOLOL XD ROFL LMAO
					backup_record.store( m_player );
					record->cache();
					g_csgo.m_engine_trace->ClipRayToEntity( Ray( trace_start, trace_end ), MASK_SHOT, record->m_player, &tr );
					backup_record.restore( m_player );

					if ( tr.m_entity != record->m_player )
					{
						continue;
					}
				}
			}

			AimPlayer::ResolverLog_t log{};
			log.m_reason = AimPlayer::LogReason_t::MISS;
			log.m_on_ground = ( record->m_flags & FL_ONGROUND );

			log.m_confidence = 1.f;

			float body_delta = math::NormalizedAngle( record->m_eye_angles.y - record->m_body );
			log.m_lby_side = abs( body_delta ) <= 35.f ? AimPlayer::LbySide_t::EQUAL : ( body_delta > 0.f ? AimPlayer::LbySide_t::RIGHT : AimPlayer::LbySide_t::LEFT );

			log.m_deltas[ LBY_EYE ] = abs( math::NormalizedAngle( record->m_body - record->m_eye_angles.y ) );
			log.m_deltas[ BACKWARDS_EYE ] = math::NormalizedAngle( record->m_backwards_angle - record->m_eye_angles.y );
			log.m_deltas[ FREESTAND_EYE ] = math::NormalizedAngle( record->m_freestand_angle - record->m_eye_angles.y );

			m_shot_log.emplace_back( log );

			m_blacklisted_angles.emplace_back( record->m_eye_angles.y );
		}
	}
	else
	{
		if ( record->m_mode == Resolver::Modes::RESOLVE_UPDATE )
		{
			m_logged_timer = record->m_lby_offset;
		}

		BackupRecord backup_record;
	
		for ( int i = 10; i < 360; i += 10 )
		{
			record->m_eye_angles.y += i;
	
			*animstate = record->m_old_state;
	
			animstate->m_last_update_frame = g_csgo.m_globals->m_frame - 1;
			animstate->update( record->m_eye_angles );
	
			record->m_player->GetPoseParameters( record->m_poses );
			record->m_abs_ang = { 0.f, animstate->m_abs_yaw, 0.f };
	
			if ( g_bones.BuildBones( record->m_player, BONE_USED_BY_HITBOX, record->m_bones, record ) )
			{
				CGameTrace tr;
	
				// realllyyyy shit way of doing this
                // but as far as i can tell it fixes the bone setup issues
                // so idgaf LOL XD LOLOL XD ROFL LMAO
				backup_record.store( m_player );
				record->cache();
				g_csgo.m_engine_trace->ClipRayToEntity( Ray( trace_start, trace_end ), MASK_SHOT, record->m_player, &tr );
				backup_record.restore(m_player);
	
				if ( tr.m_entity == record->m_player && tr.m_hitgroup == actual_hitgroup )
				{
					AimPlayer::ResolverLog_t log{};
					log.m_reason = actual_hitgroup == HITGROUP_HEAD ? AimPlayer::LogReason_t::HIT : AimPlayer::LogReason_t::OVERLAP;
					log.m_on_ground = ( record->m_flags & FL_ONGROUND );

					log.m_confidence = 1.f;

					float body_delta = math::NormalizedAngle( record->m_eye_angles.y - record->m_body );
					log.m_lby_side = abs( body_delta ) <= 35.f ? AimPlayer::LbySide_t::EQUAL : ( body_delta > 0.f ? AimPlayer::LbySide_t::RIGHT : AimPlayer::LbySide_t::LEFT );

					log.m_deltas[ LBY_EYE ] = abs( math::NormalizedAngle( record->m_body - record->m_eye_angles.y ) );
					log.m_deltas[ BACKWARDS_EYE ] = math::NormalizedAngle( record->m_backwards_angle - record->m_eye_angles.y );
					log.m_deltas[ FREESTAND_EYE ] = math::NormalizedAngle( record->m_freestand_angle - record->m_eye_angles.y );

					m_shot_log.emplace_back( log );
				}
			}
		}
	}
	
	*animstate = backup_state;
	shot_record->m_player->SetPoseParameters( old_poses );
	shot_record->m_player->SetAnimLayers( old_layers );
}

void Resolver::SetFacingPlayer( AimPlayer* data, LagRecord* record )
{
	// want it to work when ppl are fakewalking, so manually calc it with origins.
	vec3_t velocity = record->m_origin - record->m_old_origin;

	if ( velocity.length_2d() <= 0.1f )
	{
		if ( data->m_facing_player && data->m_facing_player->alive() )
			return;
	}
	else
	{
		data->m_velocity_dir = math::rad_to_deg( std::atan2( velocity.y, velocity.x ) );
	}

	// adding some weighting, since it wasnt perfect before.
	constexpr float distance_exponent = 0.5f;
	constexpr float angle_exponent = 2.f;

	float best_score = FLT_MAX;
	Player* best_player = nullptr;

	for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ )
	{
		Player* player = g_csgo.m_entlist->GetClientEntity<Player*>( i );
		if ( !player || !player->alive() || !player->enemy( data->m_player ) )
			continue;

		float raw_dist = ( record->m_origin - player->m_vecOrigin() ).length();
		float angle_diff = fabsf(
			math::NormalizedAngle(
				math::CalcAngle( record->m_origin, player->m_vecOrigin() ).y
				- data->m_velocity_dir
			)
		) / 180.f;

		float d = std::pow( raw_dist, distance_exponent );
		float a = std::pow( 1.f - angle_diff, angle_exponent );
		float score = d * a;

		if ( score < best_score )
		{
			best_score = score;
			best_player = player;
		}
	}

	data->m_facing_player = best_player;
}

constexpr int group_seperation = 10;
constexpr int group_size = 180 / group_seperation;
constexpr int num_buckets = 360 / group_seperation;

int get_bucket( float angle, float range = 360.f )
{
	float n = math::NormalizedAngle( angle );
	float shifted = n + 180.f;

	shifted = std::fmod( shifted, range );
	if ( shifted < 0.f )
		shifted += range;

	return static_cast<int>( shifted / group_seperation );
}

std::array<float, num_buckets> GetTrack( AimPlayer* data, LagRecord* record, const int& clamp_preference )
{
	auto GetAngle = [&]( int i ) -> float
		{
			switch ( i )
			{
			case 0: return record->m_body;
			case 1: return record->m_backwards_angle;
			case 2: return record->m_freestand_angle;
			default:
				g_notify.add( XOR( "unexpected iterator in GetTrack!!\n" ) );
				return 0.f;
			}
		};

	std::array<float, num_buckets> track = {};

	float fs_side = math::NormalizedAngle( record->m_freestand_angle - record->m_body );

	for ( int i = 0; i < data->m_shot_log.size(); i++ )
	{
		auto* cur = &data->m_shot_log[ i ];
		if ( !cur->m_on_ground && ( record->m_flags & FL_ONGROUND ) )
			continue;

		float confidence = cur->m_confidence;

		if ( record->m_flags & FL_ONGROUND )
		{
			bool likely_side = ( abs( fs_side ) <= 35.f && cur->m_lby_side == AimPlayer::LbySide_t::EQUAL )
				|| ( fs_side > 0.f && cur->m_lby_side == AimPlayer::LbySide_t::RIGHT )
				|| ( fs_side < 0.f && cur->m_lby_side == AimPlayer::LbySide_t::LEFT );

			if ( likely_side )
			{
				confidence *= 1.25f;
			}
		}

		for ( int j = 0; j < 3; j++ )
		{
			auto IncrementTrack = [&]( float amt ) -> void
				{
					if ( j == AimPlayer::LogDelta_t::LBY_EYE )
					{
						track[ get_bucket( GetAngle( j ) + cur->m_deltas[ j ] ) ] += amt;
					}

					track[ get_bucket( GetAngle( j ) - cur->m_deltas[ j ] ) ] += amt;
				};

			switch ( cur->m_reason )
			{
			case AimPlayer::LogReason_t::DEATH_CLAMPED:
			case AimPlayer::LogReason_t::HIT:
			case AimPlayer::LogReason_t::OVERLAP:
				IncrementTrack( confidence );
				break;
			case AimPlayer::LogReason_t::DEATH_UNCLAMPED:
				switch ( clamp_preference )
				{
				case Resolver::unsure:
				case Resolver::neutral:
					IncrementTrack( confidence * 0.5f );
					break;
				case Resolver::ClampPreference_t::bias_unclamped:
					IncrementTrack( confidence );
					break;
				}
				break;
			case AimPlayer::LogReason_t::MISS:
				IncrementTrack( -confidence );
				break;
			}
		}
	}

	return track;
}

std::pair<float, float> SelectBest( std::array<float, num_buckets> track, AimPlayer* data, LagRecord* record, std::vector<float>& angles, const int& clamp_preference )
{
	float best_chance = -FLT_MAX;
	float best_angle = -1.f;

	for ( int i = 0; i < angles.size(); i++ )
	{
		const float& ang = angles[ i ];

		if ( !data->m_blacklisted_angles.empty() )
		{
			bool bad_angle = false;

			for ( int j = 0; j < data->m_blacklisted_angles.size(); j++ )
			{
				if ( abs( math::NormalizedAngle( ang - data->m_blacklisted_angles[ j ] ) ) <= group_seperation )
				{
					bad_angle = true;
					break;
				}
			}

			if ( bad_angle )
			{
				continue;
			}
		}

		auto& cur = track[ get_bucket( ang ) ];

		if ( cur > best_chance )
		{
			best_chance = cur;
			best_angle = ang;
		}
	}

	// if this happened, we tried all the bruteforce angles and couldnt find a valid one.
	// in that case, restard the bruteforce.
	if ( best_chance == -FLT_MAX )
	{
		data->m_blacklisted_angles.clear();
		return SelectBest( track, data, record, angles, clamp_preference );
	}

	return { best_chance, best_angle };
}

void PushUnclampedAngles( std::vector<float>& test_angles, std::vector<float>& angles, float body_yaw )
{
	for ( int i = 0; i < test_angles.size(); i++ )
	{
		if ( abs( math::NormalizedAngle( test_angles[ i ] - body_yaw ) ) > 58.f )
			continue;

		angles.emplace_back( test_angles[ i ] );
	}
}

void PushClampedAngles( AimPlayer* data, LagRecord* record, std::vector<float>& angles )
{
	// future-proofing incase i forget, i intent on adding to unclamped/clamp counter using twist resolver.
	if ( data->m_shot_log.empty() )
		return;

	float fs_side = math::NormalizedAngle( record->m_freestand_angle - record->m_body );

	std::array<float, group_size> lby_groups{};

	for ( int i = 0; i < data->m_shot_log.size(); i++ )
	{
		auto* log = &data->m_shot_log[ i ];
		if ( log->m_reason != AimPlayer::LogReason_t::DEATH_CLAMPED && log->m_reason != AimPlayer::LogReason_t::HIT && log->m_reason != AimPlayer::LogReason_t::MISS )
			continue;

		if ( !log->m_on_ground && ( record->m_flags & FL_ONGROUND ) )
			continue;

		float confidence = log->m_confidence;

		if ( record->m_flags & FL_ONGROUND )
		{
			bool likely_side = ( abs( fs_side ) <= 35.f && log->m_lby_side == AimPlayer::LbySide_t::EQUAL )
				|| ( fs_side > 0.f && log->m_lby_side == AimPlayer::LbySide_t::RIGHT )
				|| ( fs_side < 0.f && log->m_lby_side == AimPlayer::LbySide_t::LEFT );

			if ( likely_side )
			{
				confidence *= 1.25f;
			}
		}

		if ( log->m_reason == AimPlayer::LogReason_t::MISS )
		{
			lby_groups[ get_bucket( log->m_deltas[ AimPlayer::LogDelta_t::LBY_EYE ], 180.f ) ] -= confidence;
		}
		else
			lby_groups[ get_bucket( log->m_deltas[ AimPlayer::LogDelta_t::LBY_EYE ], 180.f ) ] += confidence;
	}

	std::pair<float, float> best_lby = { 0.f, -FLT_MAX };

	for ( int i = 0; i < lby_groups.size(); i++ )
	{
		if ( lby_groups[ i ] > best_lby.second )
		{
			best_lby.first = i * group_seperation;
			best_lby.second = lby_groups[ i ];
		}
	}

	if ( best_lby.second == -FLT_MAX )
		return;

	// the logging with backwards & freestand considered should handle the side-selection.
	angles.emplace_back( record->m_body - best_lby.first );
	angles.emplace_back( record->m_body + best_lby.first );
}

void Resolver::ResolveStand( AimPlayer* data, LagRecord* record )
{
	// its fine to only set this inside of ResolveStand since we dont log any other states.
	float backwards = math::CalcAngle( data->m_facing_player ? data->m_facing_player->m_vecOrigin() : g_ctx.m_shoot_pos, record->m_origin ).y;
	auto fs_data = FreestandYaw( data, data->m_facing_player, { backwards, backwards - 90.f, backwards + 90.f, backwards + 180.f } );
	data->m_freestand_angle = fs_data.first;

	record->m_backwards_angle = backwards;
	record->m_freestand_angle = data->m_freestand_angle;

	if ( data->m_cant_predict_lby && data->m_body == data->m_old_body )
	{
		data->m_updates_post_lby = -1;
	}
	else
	{
		data->m_cant_predict_lby = false;

		if ( record->m_anim_time > data->m_lby_timer || data->m_body != data->m_old_body )
		{
			data->m_updates_post_lby = 0;
			data->m_lby_timer = record->m_anim_time + 1.1f;
		}
	}

	CheckAnimations( record, data );

	record->m_lby_offset = GetTimingOffset( data );

	data->m_scary_lby = record->m_lby_offset != 0 || data->m_blacklisted_timings.size() > 0;

	if ( data->m_updates_post_lby == record->m_lby_offset || data->m_body != data->m_old_body )
	{
		record->m_eye_angles.y = record->m_body;
		record->m_mode = RESOLVE_UPDATE;
	}

	++data->m_updates_post_lby;

	if ( record->m_mode == RESOLVE_UPDATE )
		return;

	record->m_mode = RESOLVE_STAND;

	int clamp_preference = unsure;

	float total = data->m_clamp_log.m_times_clamped + data->m_clamp_log.m_times_unclamped;

	// we can be sure that one is happening more often than the other.
	if ( total >= 4.f )
	{
		float clamp_percent = data->m_clamp_log.m_times_clamped / total;

		if ( clamp_percent >= 0.66f )
		{
			clamp_preference = bias_clamped;
		}
		else if ( clamp_percent <= 0.33f )
		{
			clamp_preference = bias_unclamped;
		}
		else
		{
			clamp_preference = neutral;	
		}
	}

	auto* layer3 = &record->m_layers[ 3 ];

	bool anim_adjust = record->m_player->GetSequenceActivity( layer3->m_sequence ) == 979
		// should check < 0.92 i think, not <=, but the actual value here is dependant on how much they lag, not sure how to calculate it (0.92 seems to be the lowest it will go).
		&& ( layer3->m_weight > 0.f || ( layer3->m_cycle == 0.f && layer3->m_prev_cycle < 0.92f ) );

	float back_body = math::NormalizedAngle( record->m_backwards_angle - record->m_body );
	float fs_body = math::NormalizedAngle( record->m_freestand_angle - record->m_body );
	float fs_side = math::NormalizedAngle( record->m_freestand_angle - backwards );

	static std::vector<float> angles;
	angles.clear();

	constexpr float danger_tolerance = 105.f;
	constexpr float half_break = 17.5f;

	if ( data->m_lby_changed )
	{
		// 979 could trigger
		if ( data->m_last_lby_delta / data->m_update_increment > 120.0f )
		{
			if ( data->m_979_count )
			{
				if ( !data->m_old_979_count )
				{
					if ( abs( math::NormalizedAngle( data->m_body - fs_data.second ) ) >= danger_tolerance )
					{
						if ( fs_data.first == backwards + 180.f && fs_data.second )
						{
							angles.emplace_back( backwards + 180.f );
						}
						else
						{
							angles.emplace_back( backwards );
						}
					}
					else
						angles.emplace_back( data->m_body );
				}
				else
				{
					if ( fs_data.second )
					{
						angles.emplace_back( fs_data.first );
					}

					angles.emplace_back( backwards );

					if ( abs( back_body ) >= half_break )
					{
						angles.emplace_back( back_body > 0.f ? backwards + 45.f : backwards - 45.f );
						angles.emplace_back( back_body > 0.f ? backwards + 60.f : backwards - 60.f );
						angles.emplace_back( back_body > 0.f ? backwards + 20.f : backwards - 20.f );
					}
					else
					{
						angles.emplace_back( FreestandYaw( data, data->m_facing_player, { backwards + 45.f, backwards - 45.f } ).first );
						angles.emplace_back( FreestandYaw( data, data->m_facing_player, { backwards + 60.f, backwards - 60.f } ).first );
						angles.emplace_back( FreestandYaw( data, data->m_facing_player, { backwards + 20.f, backwards - 20.f } ).first );
					}
				}
			}
			else
			{
				// some foot yaw tomfoolery has to be happening here.

				// TODO MAYBE CHECK WHATS ACTULLY POSSIBLE
				float bruh = ( record->m_body + ( record->m_body + data->m_last_lby_delta ) ) * 0.5f;
				angles.emplace_back( FreestandYaw( data, data->m_facing_player, { bruh, bruh + 180.f } ).first );
			}
		}
		else
		{
			// most likely breaking lbyyy
			if ( data->m_979_count )
			{
				// uhhhh idk do something here
				//

				// ok bro switched back it was just a joke lol
				if ( !data->m_old_979_count && data->m_979_count % 2 == 0 )
				{
					angles.emplace_back( record->m_body );
				}
				else
				{
					if ( abs( fs_body ) > 45.f )
					{
						angles.emplace_back( fs_data.first );
					}
					else
					{
						angles.emplace_back( backwards );
					}
				}
			}
			else
			{
				// theres like a 1 billion % chance that they fail breaked here.
				if ( data->m_old_979_count )
				{
					angles.emplace_back( record->m_body );
				}
				else
				{
					// they have to be breaking lby for this to happen.
					if ( abs( data->m_last_lby_delta ) <= 35.f )
					{
						if ( abs( math::NormalizedAngle( backwards - record->m_body ) ) > 35.f )
						{
							angles.emplace_back( backwards );
						}
						else
						{
							angles.emplace_back( fs_data.first );
						}
					}
					else
					{
						// lets check if the lby angle was away from a likely angle, to see if it theyre likely to be breaking lby

						float closest = SnapToNearest( record->m_body, { backwards, backwards + 90.f, backwards - 90.f, backwards + 180.f } );
						float closest_delta = math::NormalizedAngle( record->m_body - closest );

						if ( abs( closest_delta ) >= half_break )
						{
							if ( closest_delta > 0.f )
							{
								angles.emplace_back( closest - 90.f );
							}
							else
							{
								angles.emplace_back( closest + 90.f );
							}
						}
						else
						{
							angles.emplace_back( record->m_body );
						}
					}
				}
			}
		}
	}
	else
	{
		if ( data->m_lby_change_standing )
		{
			angles.emplace_back( data->m_last_change_yaw );
		}
		else
		{
			if ( ( data->m_979_count % 2 == 1 && data->m_old_979_count ) || ( data->m_979_count % 2 == 0 && !data->m_old_979_count ) )
			{
				switch ( clamp_preference )
				{
				case bias_unclamped:
					if ( !data->m_979_count )
					{
						angles.emplace_back( record->m_body );
					}

					angles.emplace_back( back_body > 0.f ? record->m_body + 45.f : record->m_body - 45.f );
					angles.emplace_back( back_body > 0.f ? record->m_body - 45.f : record->m_body + 45.f );
					angles.emplace_back( back_body > 0.f ? record->m_body + 20.f : record->m_body - 20.f );
					angles.emplace_back( back_body > 0.f ? record->m_body - 20.f : record->m_body + 20.f );
					break;
				case bias_clamped:
					PushClampedAngles( data, record, angles );
					break;
				default:
					if ( !data->m_979_count )
					{
						angles.emplace_back( record->m_body );
					}

					if ( abs( back_body ) >= half_break )
					{
						angles.emplace_back( back_body > 0.f ? backwards + 45.f : backwards - 45.f );
						angles.emplace_back( back_body > 0.f ? backwards + 20.f : backwards - 20.f );
						angles.emplace_back( backwards );
						angles.emplace_back( back_body > 0.f ? backwards + 60.f : backwards - 60.f );
					}
					else
					{
						angles.emplace_back( FreestandYaw( data, data->m_facing_player, { backwards + 45.f, backwards - 45.f } ).first );
						angles.emplace_back( FreestandYaw( data, data->m_facing_player, { backwards + 20.f, backwards - 20.f } ).first );
						angles.emplace_back( backwards );
						angles.emplace_back( FreestandYaw( data, data->m_facing_player, { backwards + 60.f, backwards - 60.f } ).first );
					}
					break;
				}
			}
			else
			{
				if ( data->m_old_979_count )
				{
					if ( fs_data.second )
					{
						angles.emplace_back( fs_data.first );
					}
					else
					{
						angles.emplace_back( backwards );
					}
				}
				else
				{
					angles.emplace_back( record->m_body );
				}
			}
		}
	}

	for ( int i = 0; i < 360; i += group_seperation )
	{
		angles.emplace_back( record->m_freestand_angle + fs_side > 0 ? -i : i );
	}

	record->m_eye_angles.y = SelectBest( GetTrack( data, record, clamp_preference ), data, record, angles, clamp_preference ).second;

	if ( data->m_lby_changed )
		data->m_last_change_yaw = record->m_eye_angles.y;

	if ( fs_data.second && abs( math::NormalizedAngle( record->m_eye_angles.y - fs_data.first ) ) >= danger_tolerance )
	{
		record->m_eye_angles.y = fs_data.first;
		data->m_resolver_confidence = AimPlayer::ResolverConfidence_t::LOW;
	}
	else
	{
		SetConfidence( data, record, anim_adjust, clamp_preference );
	}
}

void Resolver::ResolveWalk( AimPlayer* data, LagRecord* record )
{
	record->m_eye_angles.y = record->m_body;
	record->m_mode = RESOLVE_MOVE;

	data->m_missed_shots = 0;

	data->m_lby_timer = record->m_anim_time + 0.22f;
	data->m_cant_predict_lby = false;
	data->m_updates_post_lby = 0;

	data->m_lby_change_standing = false;

	data->m_blacklisted_angles.clear();
	data->m_blacklisted_timings.clear();

	data->m_resolver_confidence = AimPlayer::ResolverConfidence_t::MAX;
}

void Resolver::ResolveAir( AimPlayer* data, LagRecord* record )
{
	record->m_mode = RESOLVE_AIR;

	data->m_lby_change_standing = false;

	if ( data->m_body != data->m_old_body )
	{
		data->m_updates_post_lby = 0;
		data->m_lby_timer = record->m_anim_time + 0.22f;

		record->m_mode = RESOLVE_UPDATE;
		record->m_eye_angles.y = data->m_air_angle = data->m_body;
	}
	else
	{
		record->m_eye_angles.y = data->m_air_angle;
		data->m_resolver_confidence = AimPlayer::ResolverConfidence_t::LOW;
	}
}

// the ~45 degree accuracy from the cycle resolving is too inaccurate to push an exact angle from.
void Resolver::CheckUnclamped( AimPlayer* data, float angle )
{
	constexpr float bro = 58.f - 45.f;
	if ( abs( math::NormalizedAngle( data->m_body - angle ) ) >= bro )
		return;

	data->m_clamp_log.m_times_unclamped += 0.33f;
}

// should be better than multiplying then rounding? If not I doubt it will make a huge difference.
bool equal_compressed( float a, float b )
{
	constexpr float tolerance = 0.00001f;
	return fabs( a - b ) <= tolerance;
}

void Resolver::UpdateTransition( AimPlayer* data, LagRecord* record, LagRecord* previous, CCSGOPlayerAnimState* state )
{
	m_test_angles.clear();

	if (!g_Variables.m_bools["aimbot_resolve_desync"])
		return;

	if ( record->m_mode != RESOLVE_MOVE || !previous || ( previous->m_mode != RESOLVE_STAND && previous->m_mode != RESOLVE_UPDATE ) || previous->m_layers[ 6 ].m_weight )
		return;

	const float local_cycle_increment = record->m_layers[ 6 ].m_playback_rate;

	float raw_yaw_ideal = atan2( -record->m_anim_velocity.y, -record->m_anim_velocity.x ) * 180.f / math::pi;
	if ( raw_yaw_ideal < 0.f )
		raw_yaw_ideal += 360.f;

	const int move_seq = record->m_layers[ 6 ].m_sequence;

	constexpr std::array angles = { 180.f, 135.f, 90.f, 45.f, 0.f, -45.f, -90.f, -135.f };

	float& server = record->m_layers[ 6 ].m_cycle;

	for ( int i = ANIMTAG_STARTCYCLE_N; i <= ANIMTAG_STARTCYCLE_NW; i++ )
	{
		const float pred_cycle = math::clamp_cycle( data->m_player->GetFirstSequenceAnimTag( move_seq, i, 0, 1 ) + local_cycle_increment );

		// we can get multiple possible matches. if this is the case, we need to find which side is more likely.
		if ( equal_compressed( pred_cycle, server ) )
		{
			m_test_angles.emplace_back( math::NormalizedAngle( math::AngleDiff( raw_yaw_ideal, angles[ i - ANIMTAG_STARTCYCLE_N ] ) ) );
		}
	}
}

void Resolver::ResolveTwist( AimPlayer* data, LagRecord* record, LagRecord* previous, CCSGOPlayerAnimState* state )
{
	if (!g_Variables.m_bools["aimbot_resolve_desync"])
		return;

	if ( abs( math::NormalizedAngle( record->m_max_feet_yaw - record->m_min_feet_yaw ) ) <= 5.f )
		return;

	if ( m_test_angles.size() == 1 )
	{
		// we are better off to just re-calculate the pose manually here
		// since we want to avoid being an update ahead
		// because our new footyaw is calculated from layers updated after footyaw.
		state->m_abs_yaw = m_test_angles.front();

		CheckUnclamped( data, state->m_abs_yaw );

		float flAimYaw = math::AngleDiff( state->m_eye_yaw, state->m_abs_yaw );
		if ( flAimYaw >= 0 && state->m_aim_yaw_max != 0 )
		{
			flAimYaw = ( flAimYaw / state->m_aim_yaw_max ) * -60.0f;
		}
		else if ( state->m_aim_yaw_min != 0 )
		{
			flAimYaw = ( flAimYaw / state->m_aim_yaw_min ) * 60.0f;
		}

		g_csgo.m_model_cache->BeginLock();
		{
			data->m_player->SetPoseParameter( PLAYER_POSE_PARAM_BODY_YAW, flAimYaw );
		}
		g_csgo.m_model_cache->EndLock();

		data->m_detected_desync = true;
		return;
	}

	float& server = record->m_layers[ 6 ].m_playback_rate;

	if ( m_test_angles.size() > 0 )
	{
		std::pair<float, float> best = { record->m_body, FLT_MAX };

		for ( auto& angle : m_test_angles )
		{
			float current_offset = math::NormalizedAngle( angle - record->m_body );

			// this angle is impossible.
			if ( current_offset > math::NormalizedAngle( data->m_max_feet_yaw + record->m_body ) || current_offset < math::NormalizedAngle( data->m_min_feet_yaw - record->m_body ) )
				continue;

			data->m_player->SetAnimLayers( previous->m_layers );

			*state = record->m_old_state;

			state->m_abs_yaw = angle;

			data->UpdatePlayerAnims( state );

			float delta = abs( data->m_player->m_AnimOverlay()[ 6 ].m_playback_rate - server );
			if ( delta < best.second )
			{
				best.first = angle;
				best.second = delta;
			}
		}

		data->m_player->SetAnimLayers( previous->m_layers );

		*state = record->m_old_state;

		state->m_abs_yaw = best.first;

		data->UpdatePlayerAnims( state );

		CheckUnclamped( data, state->m_abs_yaw );
		
		data->m_detected_desync = data->m_accurate_velocity;
		return;
	}

	if ( data->m_detected_desync || !data->m_accurate_velocity )
		return;

	bool bro1 = abs( math::NormalizedAngle( record->m_body - record->m_max_feet_yaw ) ) >= abs( math::NormalizedAngle( previous->m_body - previous->m_max_feet_yaw ) );
	bool bro2 = abs( math::NormalizedAngle( record->m_body + record->m_min_feet_yaw ) ) >= abs( math::NormalizedAngle( previous->m_body - previous->m_min_feet_yaw ) );

	if ( !bro1 && !bro2 )
		return;

	for ( int i = 58; i >= 0; i-- )
	{
		for ( int j = -1; j <= 1; j += 2 )
		{
			float offset = i * j;

			if ( offset > math::NormalizedAngle( data->m_max_feet_yaw + record->m_body ) || offset < math::NormalizedAngle( data->m_min_feet_yaw - record->m_body ) )
				continue;

			data->m_player->SetAnimLayers( previous->m_layers );

			*state = record->m_old_state;

			state->m_abs_yaw = record->m_body + offset;

			data->UpdatePlayerAnims( state );

			// this matches what we got networked.
			if ( equal_compressed( data->m_player->m_AnimOverlay()[ 6 ].m_playback_rate, server ) )
			{
				data->m_detected_desync = true;
				return;
			}
		}
	}

	// we couldnt find a good twist angle (Niggas hacking?), re-animate them back to where they were.

	data->m_player->SetAnimLayers( previous->m_layers );

	*state = record->m_old_state;

	data->UpdatePlayerAnims( state );

	data->m_detected_desync = true;
}

void Resolver::process_override()
{
	bool held = GetAsyncKeyState( g_Variables.m_ints[ "aimbot_resolveroverride" ] );

	static bool old_held;
	static float unheld_yaw;

	static AimPlayer* overriding;

	constexpr float min_change = 0.5f;

	if ( held )
	{
		if ( !old_held || !overriding )
		{
			float closest = FLT_MAX;

			for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ )
			{
				AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
				if ( !data || !data->m_player || data->m_ignore )
					continue;

				constexpr int head_bone = 6;

				vec3_t bone = data->m_player->m_vecOrigin();
				if ( data->m_player->m_BoneCache().m_pCachedBones )
					data->m_player->m_BoneCache().m_pCachedBones->get_bone( bone, head_bone );

				ang_t ang;
				math::VectorAngles( bone - g_ctx.m_shoot_pos, ang );

				auto dist = abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.x - ang.x ) + math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - ang.y ) );

				if ( dist < closest )
				{
					overriding = data;
					closest = dist;
				}
			}
		}
		else if ( abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - unheld_yaw ) ) >= min_change )
		{
			vec3_t base = overriding->m_player->GetAbsOrigin() + vec3_t{ 0.f, 0.f, 48.f };

			ang_t Bro;
			math::VectorAngles( base - g_ctx.m_shoot_pos, Bro );

			overriding->m_override = Bro.y;

			if ( math::NormalizedAngle( Bro.x - g_ctx.m_cmd->m_view_angles.x ) < 0.f )
			{
				overriding->m_override += 180.f;
				overriding->m_override += std::clamp( ( Bro.y - g_ctx.m_cmd->m_view_angles.y ) * 30.f, -90.f, 90.f );
			}
			else
				overriding->m_override -= std::clamp( ( Bro.y - g_ctx.m_cmd->m_view_angles.y ) * 30.f, -90.f, 90.f );
		}
	}
	else
	{
		if ( old_held )
		{
			if ( abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - unheld_yaw ) ) < min_change )
			{
				for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ )
				{
					AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
					if ( !data )
						continue;

					data->m_override = 0.f;
				}
			}
		}

		unheld_yaw = g_ctx.m_cmd->m_view_angles.y;
	}

	old_held = held;
}