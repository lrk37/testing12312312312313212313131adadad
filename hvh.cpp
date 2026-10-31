#include "includes.h"

HVH g_hvh{ };;

void HVH::ManualAntiaim()
{
	if ( !g_input.m_should_listen )
		return;

	static int press_times;

	if ( GetAsyncKeyState( g_Variables.m_ints[ "antiaim_manual_left" ] ) & 1 )
	{
		m_queued_dir = LEFT;
		m_last_press_time = g_csgo.m_globals->m_realtime;
		++press_times;
	}
	else if ( GetAsyncKeyState( g_Variables.m_ints[ "antiaim_manual_right" ] ) & 1 )
	{
		m_queued_dir = RIGHT;
		m_last_press_time = g_csgo.m_globals->m_realtime;
		++press_times;
	}
	else if ( GetAsyncKeyState( g_Variables.m_ints[ "antiaim_manual_back" ] ) & 1 )
	{
		m_queued_dir = BACK;
		m_last_press_time = g_csgo.m_globals->m_realtime;
		++press_times;
	}
	else if ( GetAsyncKeyState( g_Variables.m_ints[ "antiaim_manual_forward" ] ) & 1 )
	{
		m_queued_dir = FORWARD;
		m_last_press_time = g_csgo.m_globals->m_realtime;
		++press_times;
	}

	if ( m_queued_dir > NONE 
		&& ( g_csgo.m_globals->m_realtime - m_last_press_time >= game::TICKS_TO_TIME( 16 ) 
		|| press_times > 1 
		|| !g_Variables.m_bools[ "antiaim_manual_lastmove" ] ) )
	{
		if ( press_times > 1 && g_Variables.m_bools[ "antiaim_manual_lastmove" ] )
		{
			m_manual_lastmove = m_queued_dir;
		}
		else
		{
			if ( m_manual_direction == m_queued_dir || m_manual_lastmove == m_queued_dir )
				m_manual_direction = NONE;
			else
				m_manual_direction = m_queued_dir;

			m_manual_lastmove = NONE;
		}

		press_times = 0;
		m_queued_dir = NONE;
	}
}

float HVH::DynamicFreestand( Player* target, std::vector<AdaptiveAngle> angles, bool get_worst )
{
	if ( !target )
		return angles[ 0 ].m_yaw;

	constexpr float STEP = 4.f;

	vec3_t start = target->GetShootPosition();

	for ( auto it = angles.begin(); it != angles.end(); ++it )
	{
		vec3_t end{ g_ctx.m_shoot_pos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * it->m_range,
			g_ctx.m_shoot_pos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * it->m_range,
			g_ctx.m_shoot_pos.z + 16.f };

	

		vec3_t dir = end - start;
		float len = dir.normalize();
		if ( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		float i = 0.f;
		for ( ; i < len; i += STEP )
		{
			int contents = g_csgo.m_engine_trace->GetPointContents( start + ( dir * i ), MASK_SHOT_HULL );
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			it->m_dist += STEP;
		}

		vec3_t endpos = start + ( dir * i );
		if ( endpos.dist_to( start ) < endpos.dist_to( end ) )
		{
			it->m_dist = std::numeric_limits<float>::max();
		}
	}

	// put the most distance at the front of the container.
	std::sort( angles.begin(), angles.end(), []( const AdaptiveAngle& a, const AdaptiveAngle& b )
		{
			return a.m_dist > b.m_dist;
		} );

	return get_worst ? angles.back().m_yaw : angles.front().m_yaw;
}

void HVH::SetFreestandInfo( Player* target )
{
	constexpr float STEP = 4.f;

	static std::vector< AdaptiveAngle > angles;
	angles.clear();

	angles.emplace_back( m_view + 90.f, 48.f );
	angles.emplace_back( m_view - 90.f, 48.f );

	angles.emplace_back( m_view + 90.f, 16.f );
	angles.emplace_back( m_view - 90.f, 16.f );

	vec3_t start = target->GetShootPosition();

	for ( auto it = angles.begin(); it != angles.end(); ++it )
	{
		vec3_t end{ g_ctx.m_shoot_pos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * it->m_range,
			g_ctx.m_shoot_pos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * it->m_range,
			g_ctx.m_shoot_pos.z + 16.f };



		vec3_t dir = end - start;
		float len = dir.normalize();
		if ( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		float i = 0.f;
		for ( ; i < len; i += STEP )
		{
			int contents = g_csgo.m_engine_trace->GetPointContents( start + ( dir * i ), MASK_SHOT_HULL );
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			it->m_dist += STEP;
		}

		vec3_t endpos = start + ( dir * i );
		if ( endpos.dist_to( start ) < endpos.dist_to( end ) )
		{
			it->m_dist = std::numeric_limits<float>::max();
		}
	}

	m_left_dist = angles[ 0 ].m_dist;
	m_right_dist = angles[ 1 ].m_dist;

	m_left_close_dist = angles[ 2 ].m_dist;
	m_right_close_dist = angles[ 3 ].m_dist;
}

Player* HVH::GetFOVTarget()
{
	Player* best_player = nullptr;

	float best_fov = 180.f;
	bool pred_damage = false;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i )
	{
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !g_aimbot.IsValidTarget( player ) )
			continue;

		float fov = math::GetFOV( g_ctx.m_view_angles, g_ctx.m_shoot_pos, player->WorldSpaceCenter() );
		if ( player->dormant() && fov > 45.f )
			continue;

		bool has_pred_damage = false;

		if ( fov < best_fov && has_pred_damage >= pred_damage )
		{
			best_player = player;
			best_fov = fov;
			pred_damage = has_pred_damage;
		}
	}

	return best_player;
}

bool HVH::IsYawSideways( vec3_t from, vec3_t to, float yaw )
{
	float base = math::CalcAngle( from, to ).y;
	float front_delta = std::min( abs( math::NormalizedAngle( yaw - ( base + 180.f ) ) ), abs( math::NormalizedAngle( yaw - base ) ) );
	return front_delta > 45.f;
}

void HVH::ApplySupression()
{
	// dont need to check activiation, only gets called when we want it.

	// could check this to only flick to 1 side but its wtv
	//float diff = math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - m_lastmove_yaw );

	const int ticks_after_upd = game::TIME_TO_TICKS( g_csgo.m_globals->m_curtime - ( g_ctx.m_body_pred - 1.1f ) );
	const int ticks_until_upd = game::TIME_TO_TICKS( g_ctx.m_body_pred - g_csgo.m_globals->m_curtime );

	if ( m_manual_direction > NONE )
	{
		if ( ticks_after_upd <= 4 || ticks_until_upd <= 4 )
		{
			g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw + 180.f;
		}
	}

	if ( ticks_after_upd <= 2 )
	{
		g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw - 90.f;
		g_ctx.m_hide_angle = 1;
		return;
	}

	if ( ticks_until_upd <= 2 )
	{
		g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw + 90.f;
		g_ctx.m_hide_angle = 1;
	}
}

void HVH::ApplyTwist()
{
	if ( !g_Variables.m_bools[ "antiaim_twist" ] )
		return;

	static int ticks_landed;

	if ( m_mode != WALK )
	{
		if ( m_mode == AIR )
		{
			ticks_landed = 0;
		}

		return;
	}

	if ( ticks_landed < 2 && m_manual_direction != ManualAAMode::LEFT && m_manual_direction != ManualAAMode::RIGHT )
	{
		switch ( ticks_landed )
		{
		case 0:
			g_ctx.m_cmd->m_view_angles.y = ( ( g_ctx.m_abs_yaw + g_server_animations.m_fake_feet_yaw ) * 0.5f ) + 180.f;
			//g_ctx.m_hide_angle = 2; ugly
			break;
		case 1:
			g_ctx.m_cmd->m_view_angles.y = ( g_ctx.m_abs_yaw + g_server_animations.m_fake_feet_yaw ) * 0.5f;
			m_twist_lag = 1;
			break;
		}

		++ticks_landed;
		return;
	}

	const float delta = math::NormalizedAngle( g_ctx.m_abs_yaw - g_server_animations.m_fake_feet_yaw );
	if ( abs( delta ) < 10.f )
		return;

	bool in_use = ( g_ctx.m_cmd->m_buttons & IN_USE );

	static float target_angle;

	float angle_1337 = in_use ? m_view : m_view + 180.f;

	if ( m_manual_direction > NONE )
	{
		target_angle = angle_1337 = g_ctx.m_cmd->m_view_angles.y;
	}
	else if ( g_Variables.m_bools[ "antiaim_twist_tilt_freestand" ] )
	{
		if ( m_left_dist != m_right_dist )
		{
			if ( m_left_dist > m_right_dist )
			{
				angle_1337 -= abs( delta * 0.5f );
			}
			else
			{
				angle_1337 += abs( delta * 0.5f );
			}
		}
	}

	if ( abs( math::NormalizedAngle( target_angle - angle_1337 ) ) > g_Variables.m_floats[ "antiaim_twist_lock_threshold" ] )
		target_angle = angle_1337;

	// dont want pred animstate here since abs yaw wont update yet.
	float flEyeFootDelta = math::NormalizedAngle( target_angle - g_ctx.m_abs_yaw );

	if ( abs( flEyeFootDelta ) > 105.f )
		return;

	float width = g_ctx.m_pred_aim_matrix_width_range - 1.f;

	float enemy_min = g_server_animations.m_fake_feet_yaw - width;
	float enemy_max = g_server_animations.m_fake_feet_yaw + width;

	float local_min = g_ctx.m_abs_yaw - width;
	float local_max = g_ctx.m_abs_yaw + width;

	if ( delta > 0.f )
	{
		g_ctx.m_cmd->m_view_angles.y = flEyeFootDelta > 0.f ? enemy_max : local_min;
	}
	else
	{
		g_ctx.m_cmd->m_view_angles.y = flEyeFootDelta > 0.f ? local_max : enemy_min;
	}

	if ( g_ctx.m_local->m_vecVelocity().length_2d() > 0.1f && abs( delta ) > g_Variables.m_floats[ "antiaim_twist_minimum_desync" ] )
	{
		m_twist_lag = g_Variables.m_bools[ "antiaim_twist_disable_lag" ] ? 1 : 0;
	}
	else
		m_twist_lag = 0;
}

void HVH::ApplyFlick()
{
	static float flick_angle;

	if ( g_movement.m_transition_flick )
	{
		// first part is to flick and update lby so the enemy sees this.
		if ( !g_movement.m_transition_stage.first )
		{
			float freestand = DynamicFreestand( m_best_target, { g_ctx.m_angle.y + 90.f, g_ctx.m_angle.y - 90.f } );

			g_ctx.m_cmd->m_view_angles.y = flick_angle = freestand;
			g_movement.m_transition_stage.first = true;

			g_ctx.m_hide_angle = 2;
		}
		else if ( !g_movement.m_transition_stage.second )
		{
			g_ctx.m_cmd->m_view_angles.y = flick_angle + 180.f;
			g_movement.m_transition_stage.second = true;

			g_ctx.m_hide_angle = 1;
		}
		else
		{
			g_movement.m_transition_reset = false;
		}
	}

	if ( g_ctx.m_local->m_vecVelocity().length_2d() <= 0.1f && !g_ctx.m_pressing_move )
	{
		g_movement.m_transition_reset = true;
	}
}

void HVH::AntiAim()
{
	if ( !g_Variables.m_bools[ "antiaim_enabled" ] )
		return;

	ManualAntiaim();

	if ( g_Variables.m_bools[ "antiaim_face_targets" ] && m_best_target )
	{
		m_view = math::CalcAngle( g_ctx.m_local->m_vecOrigin(), m_best_target->m_vecOrigin() ).y;
	}
	else
		m_view = g_ctx.m_cmd->m_view_angles.y;

	m_fresh_jump = false;

	if ( ( ( g_ctx.m_buttons & IN_JUMP ) || !( g_ctx.m_flags & FL_ONGROUND ) ) && m_mode != AntiAimMode::AIR ) // on the first tick we should send to get rid of our lastmove being the angle we update in-air with
		m_fresh_jump = true;

	m_mode = AntiAimMode::STAND;

	if ( ( g_ctx.m_buttons & IN_JUMP ) || !( g_ctx.m_flags & FL_ONGROUND ) )
		m_mode = AntiAimMode::AIR;
	else if ( g_ctx.m_local->m_vecVelocity().length_2d() > 0.1f && !g_movement.m_fakewalking )
	{
		m_mode = AntiAimMode::WALK;
		m_lby_timing_broken = false;
	}

	// disable conditions.
	if ( g_ctx.m_local->m_MoveType() == MOVETYPE_LADDER || g_ctx.m_local->m_MoveType() == MOVETYPE_NOCLIP )
		return;

	// grenade throwing
	// CBaseCSGrenade::ItemPostFrame()
	// https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/shared/cstrike/weapon_basecsgrenade.cpp#L209
	if ( g_ctx.m_weapon_type == WEAPONTYPE_GRENADE && !g_ctx.m_weapon->m_bPinPulled()
		&& g_csgo.m_globals->m_curtime - g_ctx.m_weapon->m_fThrowTime() <= ( g_ctx.m_latency * 0.5f ) + g_csgo.m_globals->m_interval )
	{
		*g_ctx.m_packet = true;
		return;
	}

	if ( ( g_ctx.m_round_end && !g_Variables.m_bools[ "antiaim_enabled_round_end" ] )
		|| ( g_csgo.m_gamerules->m_bFreezePeriod() && !g_Variables.m_bools[ "antiaim_enabled_freezetime" ] ) )
	{
		g_movement.m_fakewalk_lby_stop = false;
		g_ctx.m_hide_angle = 0;
		return;
	}

	if ( g_ctx.m_weapon && g_ctx.m_weapon_fire )
	{
		bool knife = g_ctx.m_weapon_type == WEAPONTYPE_KNIFE && g_ctx.m_weapon_id != ZEUS;

		const bool attack = g_ctx.m_cmd->m_buttons & IN_ATTACK;
		const bool attack2 = g_ctx.m_cmd->m_buttons & IN_ATTACK2;

		// Hi we are not going to be manual firing with the R8 Revolver
		if ( g_ctx.m_weapon_id != REVOLVER && ( attack || ( attack2 && knife ) ) )
			return;
	}

	bool in_use = g_ctx.m_cmd->m_buttons & IN_USE;

	if ( in_use && !g_Variables.m_bools[ "antiaim_enabled_use" ] )
	{
		g_movement.m_fakewalk_lby_stop = false;
		g_ctx.m_hide_angle = 0;
		return;
	}

	if ( *g_ctx.m_packet )
	{
		if ( in_use )
		{
			g_ctx.m_cmd->m_view_angles.y = m_view;
		}
		else
		{
			switch ( g_Variables.m_ints[ "antiaim_pitch" ] )
			{
			case 0:
				g_ctx.m_cmd->m_view_angles.x = -89.f;
				break;
			case 1:
				g_ctx.m_cmd->m_view_angles.x = 89.f;
				break;
			case 2:
				g_ctx.m_cmd->m_view_angles.x = 0.f;
				break;
			}

			if ( m_mode == STAND && g_Variables.m_bools[ "antiaim_adaptive_lby" ] )
			{
				if ( g_ctx.m_enemy_body_pred > g_ctx.m_body_pred )
				{
					g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw;
				}
				else
				{
					g_ctx.m_cmd->m_view_angles.y = DynamicFreestand( m_best_target, { m_view + 180.f, m_view - 90.f, m_view + 90.f }, g_Variables.m_ints[ "antiaim_lby_orientation" ] == 0 );

					if ( abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - m_real_angles.y ) ) <= 35.f )
					{
						g_ctx.m_cmd->m_view_angles.y += g_csgo.RandomFloat( -90.f, 90.f );
					}
				}
			}
			else
			{
				g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw;
			}
		}

		return;
	}

	if ( g_ctx.m_lag > 0 )
		return;

	if ( !in_use )
	{
		switch ( g_Variables.m_ints[ "antiaim_pitch" ] )
		{
		case 0:
			g_ctx.m_cmd->m_view_angles.x = -89.f;
			break;
		case 1:
			g_ctx.m_cmd->m_view_angles.x = 89.f;
			break;
		case 2:
			g_ctx.m_cmd->m_view_angles.x = 0.f;
			break;
		}
	}

	m_twist_lag = 0;

	g_movement.m_fakewalk_lby_stop = false;

	bool skip_normal_aa = false;
	bool skip_twist = false;
	bool disable_manual = false;
	bool supress_979 = false;

	if ( g_Variables.m_bools[ "antiaim_adaptive_lby" ] )
	{
		if ( ( g_Variables.m_bools[ "antiaim_extend_lastmoving" ] || m_manual_lastmove > NONE ) && m_mode == WALK && ( !g_ctx.m_pressing_move || g_Variables.get_keystate( g_Variables.m_binds[ "fakewalk" ] ) ) )
		{
			switch ( m_manual_lastmove )
			{
			case NONE:
				if ( g_Variables.m_ints[ "antiaim_lby_angle" ] )
				{
					g_ctx.m_cmd->m_view_angles.y = DynamicFreestand( m_best_target,
						{ m_view - 90.f, m_view + 90.f },
						g_Variables.m_ints[ "antiaim_lby_orientation" ] == 0 );
				}
				else
				{
					g_ctx.m_cmd->m_view_angles.y = DynamicFreestand( m_best_target,
						{ g_ctx.m_abs_yaw + g_ctx.m_pred_aim_matrix_width_range, g_ctx.m_abs_yaw - g_ctx.m_pred_aim_matrix_width_range },
						g_Variables.m_ints[ "antiaim_lby_orientation" ] == 0 );
				}
				break;
			case BACK:
				g_ctx.m_cmd->m_view_angles.y = m_view + 180.f;
				break;
			case RIGHT:
				g_ctx.m_cmd->m_view_angles.y = m_view - 90.f;
				break;
			case LEFT:
				g_ctx.m_cmd->m_view_angles.y = m_view + 90.f;
				break;
			case FORWARD:
				g_ctx.m_cmd->m_view_angles.y = m_view;
				break;
			}

			g_ctx.m_hide_angle = 1;

			// test, make head stick out less.
			//g_ctx.m_cmd->m_view_angles.x = -89.f;

			skip_normal_aa = true;
			skip_twist = true;
			disable_manual = true;
		}
		else if ( m_mode == STAND )
		{
			/*
				m_flFootYaw = ApproachAngle( m_pPlayer->m_flLowerBodyYawTarget.Get(), m_flFootYaw, m_flLastUpdateIncrement * CSGO_ANIM_LOWER_CATCHUP_IDLE );

				What i want:
				Have footyaw and eyeyaw within 35 degrees on update tick so we can delay it.
				Then have lby flick the next tick, without changing m_flLowerBodyYawTarget value.

				How to do it:
				m_flFootYaw would need to be far away enough from our goal lby angle on the two important ticks (update tick and the one after)
				so that we can set our eyeangles to footyaw on the update tick, and then flick the tick after.
			*/

		    //m_lby_timing_broken = abs( g_ctx.m_enemy_body_pred - g_ctx.m_body_pred ) >= 4 * g_csgo.m_globals->m_interval;

			g_movement.m_fakewalk_lby_stop = true;

			if ( !m_lby_timing_broken )
			{
				if ( g_csgo.m_globals->m_curtime > g_ctx.m_body_pred + ( g_csgo.m_globals->m_interval * 4 ) )
				{
					g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw;
					m_lby_timing_broken = true;
					g_ctx.m_hide_angle = 2;
				}
				else if ( g_csgo.m_globals->m_curtime > g_ctx.m_body_pred/* - ( g_csgo.m_globals->m_interval * 4 )*/ )
				{
					g_ctx.m_cmd->m_view_angles.y = g_ctx.m_abs_yaw;
					g_ctx.m_hide_angle = 1;
				}
				else
				{
					g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw + 180.f;
					g_ctx.m_hide_angle = 1;
				}

				disable_manual = true;
			}
			else if ( g_csgo.m_globals->m_curtime > g_ctx.m_body_pred )
			{
				g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw;
				g_ctx.m_hide_angle = 2;
				disable_manual = true;
			}
			else if ( !g_Variables.m_ints[ "antiaim_lby_anim" ] )
			{
				if ( g_csgo.m_globals->m_curtime >= g_ctx.m_body_pred - ( g_csgo.m_globals->m_interval * 2 ) )
				{
					g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw + 180.f;
					g_ctx.m_hide_angle = 1;
					disable_manual = true;
				}
				else
				{
					float back = in_use ? m_view : m_view + 180.f;
					float lby_side = math::NormalizedAngle( back - m_lastmove_yaw );

					float add = ( g_Variables.m_ints[ "antiaim_real_angle" ] && !in_use ) ? 30.f : 45.f;

					g_ctx.m_cmd->m_view_angles.y = lby_side > 0.f ? back + add : back - add;
				}
			}
			else
			{
				g_ctx.m_cmd->m_view_angles.y = m_lastmove_yaw + 180.f;
				supress_979 = true;
			}

			skip_normal_aa = true;
		}
	}

	if ( !skip_normal_aa )
	{
		if ( m_best_target )
		{
			SetFreestandInfo( m_best_target );
		}
		else
			m_left_dist = m_left_close_dist = m_right_dist = m_right_close_dist = 0.f;

		const float min_dist = ( m_left_dist + m_right_dist ) * 0.75f;

		if ( g_Variables.m_bools[ "antiaim_adaptive_air" ] && !( g_ctx.m_flags & FL_ONGROUND ) )
		{
			g_ctx.m_cmd->m_view_angles.y = m_view + 180.f;
			vec3_t vel = g_ctx.m_local->m_vecVelocity();
			float velyaw = math::rad_to_deg( std::atan2( vel.y, vel.x));

			while ( abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - ( m_view + 180 ) ) ) <= 45.f
				|| abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - g_ctx.m_local->m_flLowerBodyYawTarget() ) ) <= 45.f
				|| abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - m_lastmove_yaw ) ) <= 45.f 
				|| abs( math::NormalizedAngle( g_ctx.m_cmd->m_view_angles.y - velyaw ) ) <= 45.f )
			{
				// using outgoing instead of g_ctx.m_cmd number to fix jitter with anim smoothing.
				g_ctx.m_cmd->m_view_angles.y += g_csgo.m_cl->m_last_outgoing_command % 2 == 0 ? 10.f : -10.f;
			}

			disable_manual = true;
		}
		else if ( in_use )
		{
			g_ctx.m_cmd->m_view_angles.y = m_view;
		}
		else if ( g_Variables.m_bools[ "antiaim_backwards_duck" ] && g_ctx.m_cmd->m_buttons & IN_DUCK )
		{
			g_ctx.m_cmd->m_view_angles.y = m_view + 180.f;
		}
		else
		{
			switch ( g_Variables.m_ints[ "antiaim_yaw" ] )
			{
			case 0:
				if ( m_left_dist > min_dist && m_right_close_dist < min_dist )
				{
					g_ctx.m_cmd->m_view_angles.y = m_view + 90.f;
				}
				else if ( m_right_dist > min_dist && m_left_close_dist < min_dist )
				{
					g_ctx.m_cmd->m_view_angles.y = m_view - 90.f;
				}
				else
				{
					g_ctx.m_cmd->m_view_angles.y = m_view + 180.f;
				}

				break;
			case 1:
				g_ctx.m_cmd->m_view_angles.y += 180.f;
				break;
			}
		}
	}

	if ( m_manual_direction > NONE && !disable_manual && !in_use )
	{
		switch ( m_manual_direction )
		{
		case FORWARD:
			g_ctx.m_cmd->m_view_angles.y = m_view;
			break;
		case LEFT:
			g_ctx.m_cmd->m_view_angles.y = m_view + 90.f;
			break;
		case RIGHT:
			g_ctx.m_cmd->m_view_angles.y = m_view - 90.f;
			break;
		case BACK:
			g_ctx.m_cmd->m_view_angles.y = m_view + 180.f;
			break;
		}
	}

	if ( supress_979 )
		ApplySupression();

	if ( !skip_twist )
		ApplyTwist();

	ApplyFlick();
}

std::vector<vec3_t> local_points;

void PerFormTheLocaLPoints( BoneArray* bones )
{
	const model_t* model = g_ctx.m_local->GetModel();
	if ( !model )
		return;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	mstudiohitboxset_t* set = hdr->GetHitboxSet( g_ctx.m_local->m_nHitboxSet() );
	if ( !set )
		return;

	local_points.reserve( 8 );

	local_points.clear();

	constexpr int hitboxes[ 6 ] = { HITBOX_HEAD, HITBOX_BODY, HITBOX_L_FOOT, HITBOX_R_FOOT, HITBOX_R_UPPER_ARM, HITBOX_L_UPPER_ARM };

	for ( int i = 0; i < 6; i++ )
	{
		mstudiobbox_t* bbox = set->GetHitbox( hitboxes[ i ] );
		if ( !bbox )
			return; // dont continue cus we'd never get other hitboxes if we cant get dis wan xddddddd

		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) * 0.5f;

		switch ( hitboxes[ i ] )
		{
		case HITBOX_HEAD:
			static float rotation_full = std::cos( math::deg_to_rad( 45.f ) );

			local_points.push_back( { bbox->m_maxs.x + ( rotation_full * bbox->m_radius ), bbox->m_maxs.y + ( -rotation_full * bbox->m_radius ), bbox->m_maxs.z } );
			break;
		case HITBOX_BODY:
			local_points.push_back( { center.x, bbox->m_maxs.y - bbox->m_radius, center.z } );
			break;

		case HITBOX_R_UPPER_ARM:
		case HITBOX_L_UPPER_ARM:
			local_points.push_back( { bbox->m_maxs.x + bbox->m_radius, center.y, center.z } );
			break;

		case HITBOX_L_FOOT:
		case HITBOX_R_FOOT:
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );
			
			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms( bones[ bbox->m_bone ], rot_matrix, matrix );
			
			// extract origin from matrix.
			vec3_t origin = matrix.GetOrigin();
			
			float d1 = ( bbox->m_mins.z - center.z ) * 0.875f;
			
			// invert.
			if ( hitboxes[ i ] == HITBOX_L_FOOT )
				d1 *= -1.f;
			
			// side is more optimal then center.
			local_points.push_back( { center.x, center.y, center.z + d1 } );
			
			// get point offset relative to center point
			// and factor in hitbox scale.
			float d2 = ( bbox->m_mins.x - center.x );
			float d3 = ( bbox->m_maxs.x - center.x );

			// heel.
			local_points.push_back( { center.x + d2, center.y, center.z } );

			// move into world-space.
			for ( int j = 1; j <= 2; j++ )
			{
				auto& bruh = local_points[ local_points.size() - j ];
				bruh = { bruh.dot( matrix[ 0 ] ), bruh.dot( matrix[ 1 ] ), bruh.dot( matrix[ 2 ] ) };
				bruh += origin;
			}

			break;
		}

		// if we ever add more than 1 point at a time on a capsule box (radius > 0)
		// then make this iterative, but for now its fine.
		if ( bbox->m_radius > 0.f )
		{
			math::VectorTransform( local_points.back(), bones[ bbox->m_bone ], local_points.back() );
		}
	}
}

PeekResult_t HVH::DoPeek( Player* player )
{
	if ( !player || player->dormant() )
		return PeekResult_t::BRO;

	if ( !g_ctx.m_pressing_move || g_movement.m_fakewalking || !( g_ctx.m_local->m_fFlags() & FL_ONGROUND ) )
		return PeekResult_t::BRO;

	auto* bonecache = &g_ctx.m_local->m_BoneCache();
	if ( !bonecache )
		return PeekResult_t::BRO;

	static int tick_found_hit_at;
	static bool scan_reset;
	if ( tick_found_hit_at > -1 && ( g_ctx.m_cmd->m_command_number <= tick_found_hit_at || g_ctx.m_cmd->m_command_number > tick_found_hit_at && !scan_reset ) )
	{
		if ( g_ctx.m_max_lag <= g_csgo.m_cl->m_choked_commands )
		{
			scan_reset = true;
			return PeekResult_t::BRO;
		}

		if ( tick_found_hit_at - 1 == g_ctx.m_cmd->m_command_number )
		{
			// WE SHOULD CHECK HERE AGAIN IF WE SHOULD UNCHOKE THE NEXT TICK!!!!!
			return PeekResult_t::UNCHOKE;
		}

		

		return PeekResult_t::FORCE;
	}

	// The Scanning Process 1337... 
	// NOTE: I believe there is a blunder in the fact we do not re-check the fakelag once we SET our found tick, this current code assumes we will MAINTAIN a peek
	{
		tick_found_hit_at = -1;
		scan_reset = false;
		int ticks_remaining = g_ctx.m_max_lag - g_csgo.m_cl->m_choked_commands;
		// WAIT, what if we called ENGINEPREDICIONT......... HAHHAAHA.
		vec3_t initial_origin = g_ctx.m_local->GetAbsOrigin();

		static BoneArray* hello = nullptr;

		if ( !hello )
		{
			hello = (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );
		}

		BoneArray* old_cache = bonecache->m_pCachedBones;

		vec3_t shoot_pos;
		player->GetEyePos( &shoot_pos );

		const CCSGOPlayerAnimState state = *g_ctx.m_local->m_PlayerAnimState(); // Copy our state
		C_AnimationLayer restore[ 13 ]; g_ctx.m_local->GetAnimLayers( restore );

		for ( int i = 0; i <= ticks_remaining; i++ )
		{	
			if ( tick_found_hit_at > -1 ) // prevent us from scanning unneeded records...
				break;

			vec3_t new_origin = initial_origin + ( g_ctx.m_local->m_vecVelocity() * game::TICKS_TO_TIME( i ) );
			g_ctx.m_local->SetAbsOrigin( new_origin );
			game::UpdateAnimationState( g_ctx.m_local->m_PlayerAnimState(), g_ctx.m_angle );
			g_bones.Bro( g_ctx.m_local, BONE_USED_BY_HITBOX, hello );

			PerFormTheLocaLPoints( hello );

			bonecache->m_pCachedBones = hello;
			for ( int i = 0; i < local_points.size(); i++ )
			{
				

				if ( g_autowall.run( player, shoot_pos, local_points[ i ] ) )
				{
					
					tick_found_hit_at = g_ctx.m_cmd->m_command_number + i;
					break;
				}
			}
		}

		g_ctx.m_local->SetAnimLayers( restore );
		*g_ctx.m_local->m_PlayerAnimState() = state;
		bonecache->m_pCachedBones = old_cache;

		g_ctx.m_local->SetAbsOrigin( initial_origin );
	}

	return PeekResult_t::BRO;
}

void HVH::SendPacket()
{
	if ( !*g_ctx.m_final_packet )
		*g_ctx.m_packet = false;

	// yepppp... run this here since aa runs after.
	m_best_target = GetFOVTarget();

	if ( g_Variables.m_bools[ "fakelag_enabled" ] )
	{
		int limit = 1;

		bool in_air = ( g_ctx.m_buttons & IN_JUMP ) || !( g_ctx.m_local->m_fFlags() & FL_ONGROUND );

		if ( g_Variables.m_bools[ "fakelag_move" ] && g_ctx.m_local->m_vecVelocity().length_2d() > 0.1f )
		{
			limit = g_movement.m_transition_flick ? 1 : std::min( g_Variables.m_ints[ "fakelag_limit" ], g_ctx.m_max_lag );
		}
		else if ( g_Variables.m_bools[ "fakelag_air" ] && in_air )
		{
			limit = g_movement.m_transition_flick ? 1 : std::min( g_Variables.m_ints[ "fakelag_limit" ], g_ctx.m_max_lag );
		}
		else if ( g_Variables.m_bools[ "fakelag_duck" ] && g_ctx.m_local->m_bDucking() )
		{
			limit = g_movement.m_transition_flick ? 1 : std::min( g_Variables.m_ints[ "fakelag_limit" ], g_ctx.m_max_lag );
		}

		if ( !g_movement.m_transition_flick )
		{
			switch ( m_twist_lag )
			{
			case 1:
			case 2:
				limit = 1;
				break;
			case 3:
				limit = 14 - int( g_ctx.m_view_yaw_change );

				if ( limit < 1 )
					limit = 1;

				break;
			}
		}

		*g_ctx.m_packet = false;

		if ( g_Variables.m_bools[ "fakelag_adaptive_limit" ] )
		{
			vec3_t cur = g_ctx.m_local->m_vecOrigin();
			vec3_t prev = g_ctx.m_net_pos.empty() ? g_ctx.m_local->m_vecOrigin() : g_ctx.m_net_pos.front().m_pos;

			if ( m_mode == AIR )
			{
				if ( m_updates_cycle > 3 )
					m_updates_cycle = 0;

				switch ( m_updates_cycle )
				{ // AstralAir Meme Force family Delay Shotted
				case 0:
					limit = 4096.f / g_ctx.m_local->m_vecVelocity().length_sqr();
					break;
				case 1:
					limit = 13;break;
				case 2:
					limit = 2;break;
					//case 3:
					//	limit = 5;break;
				}
			}

		}
		else
		{
			vec3_t cur = g_ctx.m_local->m_vecOrigin();
			vec3_t prev = g_ctx.m_net_pos.empty() ? g_ctx.m_local->m_vecOrigin() : g_ctx.m_net_pos.front().m_pos;

			if ( ( cur - prev ).length_sqr() > 4096.0f )
				*g_ctx.m_packet = true;
		}

		// yepppppppppppp
		if ( m_mode == WALK && ( !g_ctx.m_pressing_move || g_Variables.get_keystate( g_Variables.m_binds[ "fakewalk" ] ) ) && g_Variables.m_bools[ "antiaim_adaptive_lby" ] && !g_ctx.m_old_packet )
		{
			limit = std::min( g_ctx.m_max_lag - 1, g_movement.m_fake_walk_ticks_left - 2 );
		}

		if ( g_Variables.m_bools[ "fakelag_on_peek" ] && !in_air )
		{
			m_peek_result = DoPeek( m_best_target );

			switch ( m_peek_result )
			{
			case PeekResult_t::UNCHOKE:
				*g_ctx.m_packet = true;
				break;

			case PeekResult_t::FORCE:
				limit = g_ctx.m_max_lag;
				*g_ctx.m_packet = false;
				break;
			}
		}

		if ( g_Variables.m_bools[ "antiaim_adaptive_lby" ] && m_mode == STAND && !g_ctx.m_pressing_move )
		{
			int lby_limit = g_Variables.m_ints[ "antiaim_lby_lag" ];

			if ( lby_limit > 1 && m_lby_timing_broken )
			{
				if ( g_movement.m_fakewalk_lby_stop )
				{
					const int ticks_until_upd = game::TIME_TO_TICKS( g_ctx.m_body_pred - g_csgo.m_globals->m_curtime );
					const int ticks_after_upd = game::TIME_TO_TICKS( g_csgo.m_globals->m_curtime - ( g_ctx.m_body_pred - 1.1f ) );

					if ( ticks_until_upd > 1 && ticks_after_upd > 1 )
					{
						*g_ctx.m_packet = false;
						limit = lby_limit;
					}
				}
				else
				{
					*g_ctx.m_packet = false;
					limit = lby_limit;
				}
			}
		}

		if ( g_ctx.m_lag >= limit )
		{
			*g_ctx.m_packet = true;
		}
	}

	if ( g_ctx.m_old_shot )
	{
		if ( m_peek_result != PeekResult_t::FORCE || g_aimbot.m_skip_hold_fakelag )
			*g_ctx.m_packet = true;
	}

	if ( g_movement.m_fakewalking )
	{
		if ( g_movement.m_fake_walk_ticks_left > 0 )
		{
			*g_ctx.m_packet = false;
		}
		else if ( g_movement.m_stop_the_fakewalk && !g_ctx.m_old_packet )
		{
			*g_ctx.m_packet = true;
			g_movement.m_stop_the_fakewalk = false;
		}
	}

	if ( g_ctx.m_lag >= g_ctx.m_max_lag || g_ctx.m_landed_on_anim_update || m_fresh_jump )
	{
		*g_ctx.m_packet = true;
		g_ctx.m_weapon_fire = false;
	}

	if ( *g_ctx.m_packet )
	{
		m_updates_cycle++;
	}
}