#include "includes.h"

Movement g_movement{ };;

void Movement::EdgeJump()
{
	if ( !( g_ctx.m_local->m_fFlags() & FL_ONGROUND ) )
		return;

	g_inputpred.predict();

	if ( g_ctx.m_local->m_fFlags() & FL_ONGROUND )
	{
		g_inputpred.SetDatamap( g_inputpred.og_predicted );
		return;
	}

	g_ctx.m_cmd->m_buttons |= IN_JUMP;

	g_inputpred.SetDatamap( g_inputpred.cm_start );
	g_inputpred.predict();
	g_inputpred.GetDatamap( g_inputpred.og_predicted );
}

void Movement::JumpRelated()
{
	int bad_mts[ ] = { MOVETYPE_LADDER, MOVETYPE_NOCLIP, MOVETYPE_OBSERVER };
	bool bad_movement = std::find( std::begin( bad_mts ), std::end( bad_mts ), g_ctx.m_local->m_MoveType() ) != std::end( bad_mts );

	if ( bad_movement )
		return;

	if ( ( g_ctx.m_cmd->m_buttons & IN_JUMP ) && !( g_ctx.m_flags & FL_ONGROUND ) )  
	{
		// bhop.
		if ( g_Variables.m_bools["autojump"] )
			g_ctx.m_cmd->m_buttons &= ~IN_JUMP;

		// duck jump ( crate jump ).
		if ( g_Variables.m_bools["air_duck"] )
			g_ctx.m_cmd->m_buttons |= IN_DUCK;
	}
}

void Movement::AutoStrafe()
{
	if ( !g_Variables.m_bools[ "autostrafe" ] )
		return;

	if ( g_ctx.m_flags & FL_ONGROUND )
		return;

	if ( g_ctx.m_local->m_MoveType() == MOVETYPE_NOCLIP || g_ctx.m_buttons & IN_SPEED )
		return;

	vec3_t vel = g_ctx.m_local->m_vecVelocity();

	auto speed2d = vel.length_2d();

	if ( speed2d < 2.f) //&& !g_ctx.m_cmd->m_forward_move && !g_ctx.m_cmd->m_side_move )
		return;

	auto ideal_rot = std::min( math::rad_to_deg( std::asinf( 15.f / speed2d ) ), 90.f );
	auto sign = g_ctx.m_cmd->m_command_number % 2 ? 1.f : -1.f;

	bool move_forward = !g_ctx.m_pressing_move || g_ctx.m_cmd->m_buttons & IN_FORWARD;
	bool move_backward = g_ctx.m_cmd->m_buttons & IN_BACK;

	bool move_left = g_ctx.m_cmd->m_buttons & IN_MOVELEFT, move_right = g_ctx.m_cmd->m_buttons & IN_MOVERIGHT;

	g_ctx.m_cmd->m_forward_move = speed2d > 0.1f ? 0.f : 450.f;

	if ( move_forward )
		g_ctx.m_strafe_angles.y += move_left ? 45.f : move_right ? -45.f : 0.f;
	else if ( move_backward )
		g_ctx.m_strafe_angles.y += move_left ? 135.f : move_right ? -135.f : 180.f;
	else if ( move_left || move_right )
		g_ctx.m_strafe_angles.y += move_left ? 90.f : -90.f;

	static auto old_yaw = 0.f;
	auto yaw_delta = std::remainder( g_ctx.m_strafe_angles.y - old_yaw, 360.f ), abs_yaw_delta = std::abs( yaw_delta );
	old_yaw = g_ctx.m_strafe_angles.y;

	if ( yaw_delta > 0.f ) g_ctx.m_cmd->m_side_move = -450.f;
	else if ( yaw_delta < 0.f ) g_ctx.m_cmd->m_side_move = 450.f;

	if ( abs_yaw_delta <= ideal_rot || abs_yaw_delta >= 30.f )
	{
		const auto vel_ang = math::rad_to_deg( std::atan2( vel.y, vel.x ) );
		const auto vel_delta = std::remainder( g_ctx.m_strafe_angles.y - vel_ang, 360.f );

		auto retrack_speed = ideal_rot * ( ( g_Variables.m_floats[ "autostrafe_speed" ] / 100.f ) * 3 );

		if ( vel_delta <= retrack_speed || speed2d <= 15.f )
		{
			if ( -retrack_speed <= vel_delta || speed2d <= 15.f )
			{
				g_ctx.m_strafe_angles.y += ideal_rot * sign;
				g_ctx.m_cmd->m_side_move = sign * 450.f;
			}
			else
			{
				g_ctx.m_strafe_angles.y = vel_ang - retrack_speed;
				g_ctx.m_cmd->m_side_move = 450.f;
			}
		}
		else
		{
			g_ctx.m_strafe_angles.y = vel_ang + retrack_speed;
			g_ctx.m_cmd->m_side_move = -450.f;
		}
	}
}

void Movement::MoonWalk( CUserCmd* cmd )
{
	if ( g_ctx.m_local->m_MoveType() == MOVETYPE_LADDER )
		return;

	const float forwardmove = cmd->m_forward_move;
	const float sidemove = cmd->m_side_move;
	int new_buttons = cmd->m_buttons & ~( IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD );

	if ( !g_Variables.m_ints[ "antiaim_leg_movement" ] )
	{

		if ( forwardmove <= 0.f )
		{
			if ( forwardmove < 0.f )
				new_buttons |= IN_BACK;
		}
		else
			new_buttons |= IN_FORWARD;

		if ( sidemove >= 0.f )
		{

			if ( sidemove > 0.f )
				goto LABEL_15;

			goto LABEL_18;
		}

		goto LABEL_17;
	}

	if ( g_Variables.m_ints[ "antiaim_leg_movement" ] != 1 )
		goto LABEL_18;

	if ( forwardmove <= 0.f )
	{
		if ( forwardmove < 0.f )
			new_buttons |= IN_FORWARD;
	}
	else
		new_buttons |= IN_BACK;

	if ( sidemove > 0.f )
	{

	LABEL_17:
		new_buttons |= IN_MOVELEFT;
		goto LABEL_18;
	}

	if ( sidemove < 0.f )
	{
	LABEL_15:
		new_buttons |= IN_MOVERIGHT;
	}

LABEL_18:
	cmd->m_buttons = new_buttons;
}

// todo: probably add the shit back that checks local player stuff lol
// just removed for ease while making new extrapolation.
void Movement::FixMove( CUserCmd* cmd, const ang_t& wish_angles, bool local )
{
	vec3_t  move, dir;
	float   delta, len;
	ang_t   move_angle;

	// roll nospread fix.
	//if ( !( g_ctx.m_flags & FL_ONGROUND ) && cmd->m_view_angles.z != 0.f )
	//	cmd->m_side_move = 0.f;

	// convert movement to vector.
	move = { cmd->m_forward_move, cmd->m_side_move, 0.f };

	// get move length and ensure we're using a unit vector ( vector with length of 1 ).
	len = move.normalize();
	if ( !len )
		return;

	// convert move to an angle.
	math::VectorAngles( move, move_angle );

	// calculate yaw delta.
	delta = ( cmd->m_view_angles.y - wish_angles.y );

	// accumulate yaw delta.
	move_angle.y += delta;

	// calculate our new move direction.
	// dir = move_angle_forward * move_length
	math::AngleVectors( move_angle, &dir );

	// scale to og movement.
	dir *= len;

	// strip old flags.
	cmd->m_buttons &= ~( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT );

	float max_fwd_speed = g_csgo.cl_forwardspeed->GetFloat();
	float max_back_speed = g_csgo.cl_backspeed->GetFloat();
	float max_side_speed = g_csgo.cl_sidespeed->GetFloat();

	cmd->m_forward_move = std::clamp(cmd->m_forward_move, -max_back_speed, max_fwd_speed);
	cmd->m_side_move = std::clamp(cmd->m_forward_move, -max_side_speed, max_side_speed);

	// fix ladder and noclip.
	if (local && g_ctx.m_local->m_MoveType() == MOVETYPE_LADDER )
	{
		// invert directon for up and down.
		if ( cmd->m_view_angles.x >= 45.f && wish_angles.x < 45.f && std::abs( delta ) <= 65.f )
			dir.x = -dir.x;

		// write to movement.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if ( cmd->m_forward_move > 200.f )
			cmd->m_buttons |= IN_FORWARD;

		else if ( cmd->m_forward_move < -200.f )
			cmd->m_buttons |= IN_BACK;

		if ( cmd->m_side_move > 200.f )
			cmd->m_buttons |= IN_MOVERIGHT;

		else if ( cmd->m_side_move < -200.f )
			cmd->m_buttons |= IN_MOVELEFT;
	}

	// we are moving normally.
	else
	{
		// we must do this for pitch angles that are out of bounds.
		if ( cmd->m_view_angles.x < -90.f || cmd->m_view_angles.x > 90.f )
			dir.x = -dir.x;

		// set move.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if ( cmd->m_forward_move > 0.f )
			cmd->m_buttons |= IN_FORWARD;

		else if ( cmd->m_forward_move < 0.f )
			cmd->m_buttons |= IN_BACK;

		if ( cmd->m_side_move > 0.f )
			cmd->m_buttons |= IN_MOVERIGHT;

		else if ( cmd->m_side_move < 0.f )
			cmd->m_buttons |= IN_MOVELEFT;
	}
}

void Movement::AutoPeek()
{
	bool retreating = false;

	if ( g_Variables.get_keystate( g_Variables.m_binds[ "autopeek" ] ) )
	{
		if ( ( g_ctx.m_local->m_vecOrigin() - m_autopeek_pos ).length_2d() < 10.f )
		{
			m_autopeek_retreat = false;
		}

		if ( m_autopeek_retreat )
		{
			retreating = true;

			if (m_should_quickswitch) {
				auto weapons = g_ctx.m_local->m_hMyWeapons();

				for (auto i = 0; weapons[i].IsValid(); i++)
				{
					Weapon* weapon = (Weapon*)g_csgo.m_entlist->GetClientEntityFromHandle(weapons[i]);
					if (!weapon)
						continue;

					WeaponInfo* data = weapon->GetWpnData();
					if (!data)
						continue;

					if ( data->m_weapon_type == WEAPONTYPE_KNIFE )
					{
						if ( g_Variables.m_bools[ "quick_switch_zeus" ] )
						{
							if ( weapon->m_iItemDefinitionIndex() == ZEUS )
							{
								g_ctx.m_cmd->m_weapon_select = weapon->index();
								break;
							}
						}

						if ( weapon->m_iItemDefinitionIndex() != ZEUS )
							g_ctx.m_cmd->m_weapon_select = weapon->index();
					}
				}

				m_should_quickswitch = false;
			}

			vec3_t ang = math::CalcAngle( g_ctx.m_local->m_vecOrigin(), m_autopeek_pos );

			g_ctx.m_cmd->m_forward_move = 450.f;
			g_ctx.m_cmd->m_side_move = 0.f;
			g_ctx.m_strafe_angles.y = ang.y;
		}
	}
	else
	{
		m_autopeek_pos = g_ctx.m_local->m_vecOrigin();
		m_autopeek_retreat = true;
	}

	if ( g_Variables.m_bools[ "aimbot_autostop" ] )
	{
		if ( !g_ctx.m_pressing_move && !retreating )
		{
			NullVelocity( g_ctx.m_cmd );
		}
		else if ( g_aimbot.m_stop_next_tick )
		{
			QuickStop();
		}
	}
}

void Movement::NullVelocity( CUserCmd* cmd )
{
	vec3_t vel = g_ctx.m_local->m_vecVelocity();

	if ( vel.length() <= 30.f )
	{
		cmd->m_forward_move = cmd->m_side_move = 0.f;
		return;
	}

	ang_t direction;
	math::VectorAngles( vel, direction );

	ang_t view_angles;
	g_csgo.m_engine->GetViewAngles( view_angles );

	direction.y = view_angles.y - direction.y;

	vec3_t forward;
	math::AngleVectors( direction, &forward );

	const vec3_t negative_forward_direction = forward * -g_csgo.cl_forwardspeed->GetFloat();
	const vec3_t negative_side_direction = forward * -g_csgo.cl_sidespeed->GetFloat();

	cmd->m_forward_move = negative_forward_direction.x;
	cmd->m_side_move = negative_side_direction.y;
}

bool Movement::QuickStop()
{
	if ( !g_ctx.m_weapon_info )
		return false;

	float flMaxSpeed = g_ctx.m_local->m_bIsScoped() > 0 ? g_ctx.m_weapon_info->m_max_player_speed_alt : g_ctx.m_weapon_info->m_max_player_speed;
	float flDesiredSpeed = flMaxSpeed * 0.33f;

	float speedlength = g_ctx.m_local->m_vecVelocity().length_2d();

	if ( speedlength > flDesiredSpeed )
	{
		NullVelocity( g_ctx.m_cmd );
		return false;
	}

	ClampMovementSpeed( g_ctx.m_cmd, flDesiredSpeed );
	return true;
}

void Movement::ClampMovementSpeed( CUserCmd* cmd, float speed )
{
	float squirt = std::sqrtf( ( cmd->m_forward_move * cmd->m_forward_move ) + ( cmd->m_side_move * cmd->m_side_move ) );

	if ( squirt >= speed )
	{
		float cock1 = cmd->m_forward_move / squirt;
		float cock2 = cmd->m_side_move / squirt;

		cmd->m_forward_move = cock1 * speed;
		cmd->m_side_move = cock2 * speed;
	}
}

void Movement::FakeWalk()
{
	vec3_t velocity{ g_ctx.m_local->m_vecVelocity() };

	m_fake_walk_ticks_left = -1;

	bool fakewalking = g_Variables.get_keystate( g_Variables.m_binds[ "fakewalk" ] );

	if ( !( fakewalking || !g_ctx.m_pressing_move ) )
	{
		m_fakewalking = false;
		return;
	}

	if ( !g_ctx.m_local->GetGroundEntity() )
	{
		if ( fakewalking && g_Variables.m_bools[ "aimbot_jump_stop" ] )
		{
			NullVelocity( g_ctx.m_cmd );
		}

		m_fakewalking = false;
		return;
	}

	// this shit makes u slow as fuck lol.
	if ( fakewalking )
	{
		g_ctx.m_cmd->m_buttons &= ~IN_SPEED;
	}
	else
	{
		m_stop_the_fakewalk = false;

		if ( g_ctx.m_local->m_vecVelocity().length_2d() <= 0.1f )
			return;
	}

	CUserCmd bak_usrcmd = *g_ctx.m_cmd;

	// not updated in CreateMove yet.
	g_inputpred.store();

	int ticks = 0;
	while ( ticks <= g_ctx.m_max_lag )
	{
		if ( g_ctx.m_local->m_vecVelocity().length_2d() <= 0.1f )
			break;

		NullVelocity( g_ctx.m_cmd );

		// keep predicting until we stop moving.
		g_inputpred.predict();
		++ticks;
	}

	if ( ticks == 0 )
		m_fakewalking = true;

	m_fake_walk_ticks_left = ticks;

	g_inputpred.SetDatamap( g_inputpred.cm_start );
	g_inputpred.restore();
	*g_ctx.m_cmd = bak_usrcmd;

	// we just want the stop ticks here.
	if ( !fakewalking )
		return;

	const float curtime = g_ctx.m_local->m_nTickBase() * g_csgo.m_globals->m_interval;

	const int ticks_until_upd = game::TIME_TO_TICKS( g_ctx.m_body_pred + 0.25f - curtime );
	const int ticks_after_upd = game::TIME_TO_TICKS( curtime - ( g_ctx.m_body_pred - 1.145f ) );

	// -1 kinda weird break sometimeszzzzzzzzzz.
	int choke_remaining = ( g_ctx.m_max_lag - g_ctx.m_lag ) - 2;

	int& max_delay = g_Variables.m_ints[ "fakewalk_max_delay" ];
	bool limit_speed = max_delay > 0 && ticks > max_delay;

	if ( ticks >= choke_remaining
		|| ( ( ticks_until_upd <= choke_remaining || ticks_after_upd <= 1 ) && m_fakewalk_lby_stop )
		|| !g_ctx.m_lag
		|| g_movement.m_stop_the_fakewalk
		|| !m_fakewalking
		|| limit_speed )
	{
		NullVelocity( g_ctx.m_cmd );
	}
}