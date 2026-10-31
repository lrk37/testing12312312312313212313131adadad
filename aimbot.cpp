#include "includes.h"

Aimbot g_aimbot{ };;

WeaponConfig_t GetWeaponConfig()
{
	switch ( g_ctx.m_weapon_type )
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		if ( g_ctx.m_weapon_id == SSG08 )
			return WeaponConfig_t::CONFIG_SSG08;
		else if ( g_ctx.m_weapon_id == AWP )
			return WeaponConfig_t::CONFIG_AWP;

		return WeaponConfig_t::CONFIG_AUTO;
	case WEAPONTYPE_PISTOL:
		if ( g_ctx.m_weapon_id == DEAGLE || g_ctx.m_weapon_id == REVOLVER )
			return WeaponConfig_t::CONFIG_HEAVYPISTOL;

		return WeaponConfig_t::CONFIG_PISTOL;
		break;
	default:
		return WeaponConfig_t::CONFIG_DEFAULT;
	}
}

void AimbotSettings_t::Init()
{
	WeaponConfig_t weapon = GetWeaponConfig();

	std::string add = g_Variables.GetAimbotWeaponPrefix( weapon );
	if ( !g_GUI.m_is_open && g_GUI.m_selected_aimweapon != weapon )
		g_GUI.m_selected_aimweapon = weapon;

	hitboxes[ AimbotHitbox_t::AIM_HEAD ] = g_Variables.m_bools[ "hitbox_head" + add ];
	hitboxes[ AimbotHitbox_t::AIM_CHEST ] = g_Variables.m_bools[ "hitbox_chest" + add ];
	hitboxes[ AimbotHitbox_t::AIM_BODY ] = g_Variables.m_bools[ "hitbox_body" + add ];
	hitboxes[ AimbotHitbox_t::AIM_ARMS ] = g_Variables.m_bools[ "hitbox_arms" + add ];
	hitboxes[ AimbotHitbox_t::AIM_LEGS ] = g_Variables.m_bools[ "hitbox_legs" + add ];
	hitboxes[ AimbotHitbox_t::AIM_FEET ] = g_Variables.m_bools[ "hitbox_feet" + add ];

	multipoint[ AimbotHitbox_t::AIM_HEAD ] = g_Variables.m_bools[ "multipoint_head" + add ];
	multipoint[ AimbotHitbox_t::AIM_CHEST ] = g_Variables.m_bools[ "multipoint_chest" + add ];
	multipoint[ AimbotHitbox_t::AIM_BODY ] = g_Variables.m_bools[ "multipoint_body" + add ];
	multipoint[ AimbotHitbox_t::AIM_ARMS ] = g_Variables.m_bools[ "multipoint_arms" + add ];
	multipoint[ AimbotHitbox_t::AIM_LEGS ] = g_Variables.m_bools[ "multipoint_legs" + add ];
	multipoint[ AimbotHitbox_t::AIM_FEET ] = g_Variables.m_bools[ "multipoint_feet" + add ];

	multipoint_head_scale = g_Variables.m_floats[ "multipoint_head_scale" + add ];
	multipoint_normal_scale = g_Variables.m_floats[ "multipoint_scale" + add ];
	multipoint_dynamic_scale = g_Variables.m_bools[ "multipoint_dynamic_scale" + add ];
	hitchance = ( g_ctx.m_buttons & IN_ATTACK ) ? 0.f : g_Variables.m_floats[ "hitchance" + add ];

	prefer_overlap = g_Variables.m_floats[ "prefer_overlap" + add ];
	prefer_hitchance = g_Variables.m_floats[ "prefer_hitchance" + add ];
	conditional_standing = g_Variables.m_bools[ "conditional_standing" + add ];
	conditional_unsafe_moving = g_Variables.m_bools[ "conditional_unsafe_moving" + add ];
	conditional_in_air = g_Variables.m_bools[ "conditional_in_air" + add ];
	conditional_already_missed = g_Variables.m_bools[ "conditional_already_missed" + add ];
	conditional_prefer_overlap = g_Variables.m_floats[ "conditional_prefer_overlap" + add ];
	conditional_prefer_hitchance = g_Variables.m_floats[ "conditional_prefer_hitchance" + add ];

	avoid_head_shots = g_Variables.m_ints[ "avoid_head_after" + add ];
	minimum_damage = g_Variables.get_keystate( g_Variables.m_binds[ "aimbot_mindamageoverride" ] ) ? g_Variables.m_floats[ "minimum_damage_override" + add ] : g_Variables.m_floats[ "minimum_damage" + add ];
	g_movement.m_should_quickswitch = g_Variables.m_bools[ "quick_switch" + add ];
	delay_shot = g_Variables.m_bools[ "delay_shot" + add ];
	delay_shot_disable_vulnerable = g_Variables.m_bools[ "delay_shot_disable_vulnerable" + add ];
	delay_shot_damage = g_Variables.m_bools[ "delay_shot_damage" + add ];
	delay_shot_safety = g_Variables.m_bools[ "delay_shot_safety" + add ];
	delay_shot_strictness = g_Variables.m_floats[ "delay_shot_strictness" + add ];
}

bool AimPlayer::AllowHitbox( int hitbox, float mindmg, int resolver_mode )
{
	if ( g_autowall.ScaleDamage( m_player, g_ctx.m_weapon_info->m_damage, g_ctx.m_weapon_info->m_armor_ratio, game::HitboxToHitgroup( hitbox ) ) < mindmg )
		return false;

	int x_misses = g_aimbot.m_settings.avoid_head_shots;

	if ( resolver_mode < Resolver::Modes::RESOLVE_THRESHOLD
		&& hitbox == HITBOX_HEAD
		&& x_misses > 0
		&& m_missed_shots >= x_misses )
	{
		return false;
	}

	if ( g_Variables.m_bools[ "aimbot_headshot_only" ] )
	{
		if ( hitbox > HITBOX_HEAD )
			return false;
	}

	if ( g_Variables.get_keystate( g_Variables.m_binds[ "aimbot_forcebaim" ] ) )
	{
		if ( hitbox < HITBOX_PELVIS || hitbox > HITBOX_UPPER_CHEST )
			return false;
	}

	switch ( hitbox )
	{
	default:
		return false;
	case HITBOX_HEAD:
		return g_aimbot.m_settings.hitboxes[ AimbotHitbox_t::AIM_HEAD ];
	case HITBOX_THORAX:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		return g_aimbot.m_settings.hitboxes[ AimbotHitbox_t::AIM_CHEST ];
	case HITBOX_PELVIS:
	case HITBOX_BODY:
		return g_aimbot.m_settings.hitboxes[ AimbotHitbox_t::AIM_BODY ];
	case HITBOX_L_UPPER_ARM:
	case HITBOX_R_UPPER_ARM:
	case HITBOX_L_FOREARM:
	case HITBOX_R_FOREARM:
		return g_aimbot.m_settings.hitboxes[ AimbotHitbox_t::AIM_ARMS ];
	case HITBOX_L_THIGH:
	case HITBOX_R_THIGH:
	case HITBOX_L_CALF:
	case HITBOX_R_CALF:
		return g_aimbot.m_settings.hitboxes[ AimbotHitbox_t::AIM_LEGS ];
	case HITBOX_L_FOOT:
	case HITBOX_R_FOOT:
		return g_aimbot.m_settings.hitboxes[ AimbotHitbox_t::AIM_FEET ];
	}

	return false;
}

void Aimbot::init()
{
	m_aimplayer = nullptr;
	m_aim = vec3_t{ };
	m_angle = ang_t{ };
	m_damage = 0.f;
	m_record = nullptr;
	m_stop_next_tick = false;
	m_skip_hold_fakelag = false;

	m_best_dist = std::numeric_limits< float >::max();
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = std::numeric_limits< float >::max();
	m_best_height = std::numeric_limits< float >::max();
}

// dont need to call prediction store/restore since we are already predicted here.
void Aimbot::UpdateEarlyStop()
{
	m_stop_ticks = 0;

	if ( g_ctx.m_local->m_vecVelocity().length_2d() <= 0.1f || g_movement.m_fakewalking || !( g_ctx.m_local->m_fFlags() & FL_ONGROUND ) || g_ctx.m_cmd->m_buttons & IN_JUMP )
	{
		m_allow_auto_stop = false;
		m_stop_inaccuracy = g_ctx.m_weapon->GetInaccuracy();
		return;
	}

	const CUserCmd backup_cmd = *g_ctx.m_cmd;

	while ( true )
	{
		if ( g_movement.QuickStop() )
			break;

		g_inputpred.predict();
		++m_stop_ticks;
	}

	//m_allow_auto_stop = m_stop_ticks > 0;
	m_allow_auto_stop = true;

	m_stop_inaccuracy = g_ctx.m_weapon->GetInaccuracy();

	//g_ctx.print( std::to_string( ticks ) + "\n" );

	// is it really needed to call ModifyEyePosition here?
	g_ctx.m_local->GetEyePos( &m_early_stop_shootpos );

	*g_ctx.m_cmd = backup_cmd;
	g_inputpred.SetDatamap( g_inputpred.og_predicted );
}

bool Aimbot::record_meets_safety( LagRecord* record )
{
	// head can still be slightly desynced while sideways but for the most part its fine.
	float threshold = g_hvh.IsYawSideways( g_ctx.m_shoot_pos, record->m_origin, record->m_eye_angles.y ) ? 45.f : 15.f;
	return abs( math::NormalizedAngle( record->m_max_feet_yaw - record->m_min_feet_yaw ) ) <= threshold;
}

float Aimbot::GetMinDamage( int health )
{
	if ( g_ctx.m_weapon_id == ZEUS )
		return health;

	if ( health < m_settings.minimum_damage )
		return health;

	return m_settings.minimum_damage;
}

void Aimbot::StripAttack()
{
	if ( g_ctx.m_weapon_id == REVOLVER )
		g_ctx.m_cmd->m_buttons &= ~IN_ATTACK2;
	else
		g_ctx.m_cmd->m_buttons &= ~IN_ATTACK;
}

bool Aimbot::DelayShot( AimPlayer* data, LagRecord* record, float damage, float safety )
{
	if ( !g_aimbot.m_settings.delay_shot || m_stop_next_tick || g_ctx.m_local->m_vecVelocity().length_2d() <= 0.1f || g_movement.m_fakewalking || !record || !( record->m_flags & FL_ONGROUND ) )
		return false;

	struct StrippedTarget_t
	{
		vec3_t pos;
		int hitbox;
		float damage;
		float hitchance;
		float fake_amount;
	};

	StrippedTarget_t shot_data{};

	// needs to be float for std::min lol
	float hp = data->m_player->m_iHealth();

	bool history_aim = ( record->m_origin - data->m_player->m_vecOrigin() ).length() > 0.1f || record->m_player->m_vecVelocity().length() > 0.1f;

	vec3_t restore_pos = g_ctx.m_shoot_pos;

	// we can just go up to max lag, the data update check should keep things reasonable.
	for ( int i = 0; i < g_ctx.m_max_lag; i++ )
	{
		if ( history_aim && !record->valid( record->m_sim_time + game::TICKS_TO_TIME( i ) ) )
			break;

		// a new data update would have come in by now.
		// this seems like a good way of estimating without throwing in a shot ton of tracing, but if this is shit then lmk lol.
		if ( m_settings.delay_shot_disable_vulnerable && ( g_csgo.m_cl->m_server_tick + i ) - record->m_tick >= std::max( record->m_lag, 8 ) )
			break;

		g_inputpred.predict();

		if ( g_Variables.m_bools[ "aimbot_reduce_delay_shot_intensity" ] && i % ( g_Variables.m_ints[ "aimbot_delay_shot_intensity" ] + 1 ) == 0 )
			continue;

		g_ctx.m_local->GetEyePos( &g_ctx.m_shoot_pos );

		// no point to keep predicting if we can no longer hit.
		if ( !data->AimAtRecord( record, shot_data.pos, shot_data.hitbox, shot_data.damage, shot_data.hitchance, shot_data.fake_amount, true ) )
			break;

		// check if the firerate reduction on an automatic weapon is worth it.
		if ( g_ctx.m_weapon_info->m_cycletime <= 0.5f )
		{
			float cycle_ticks = 64.f * g_ctx.m_weapon_info->m_cycletime;
			float scaler = std::min( 1.f, 1.f - ( ( cycle_ticks - i ) / cycle_ticks ) );

			// scale player damage so we can still prioritize some safety without instantly breaking.
			if ( std::min( damage, hp * ( m_settings.delay_shot_strictness * 0.01f ) ) + ( shot_data.damage * scaler ) >= hp )
				break;
		}	

		// lets favor results closer to where we are at in our choke cycle.
		//float choke_progress = 1.f - ( ( ( g_ctx.m_lag + static_cast<float>( i + 1 ) ) / g_ctx.m_max_lag ) * 0.5f );
		float choke_progress = 1.f - ( ( static_cast<float>( i + 1 ) / g_ctx.m_max_lag ) * ( m_settings.delay_shot_strictness * 0.01f ) );

		// fake amt name is kinda misleading lol
		// higher = safer

		bool damage_increase = m_settings.delay_shot_damage && shot_data.damage * choke_progress > damage && shot_data.fake_amount >= safety;
		bool safety_increase = m_settings.delay_shot_safety && shot_data.fake_amount * choke_progress > safety && std::min( shot_data.damage, hp ) >= std::min( damage, hp );

		if ( damage_increase || safety_increase )
		{
			g_ctx.m_shoot_pos = restore_pos;
			g_inputpred.SetDatamap( g_inputpred.og_predicted );
			return true;
		}
	}

	g_ctx.m_shoot_pos = restore_pos;
	g_inputpred.SetDatamap( g_inputpred.og_predicted );
	return false;
}

bool Aimbot::ScanPlayer( AimPlayer* data, BestTarget_t& target_data )
{
	enum record_order
	{
		BEST_MOVE = 0,
		STAND_LBY,
		MIDDLE,
		FIRST,
		LAST
	};

	std::array<LagRecord*, 5> target_records{};

	if ( data->m_breaking_lc )
	{
		auto lc_data = g_lagcomp.GetLagcompData( data );

		if ( lc_data.m_can_fire )
		{
			LagRecord* record = ( lc_data.m_needs_extrap && data->UpdateExtrapolation( lc_data.m_extrap_ticks ) ) ? &data->m_extrapolated_record : data->m_records[ 0 ].get();

			if ( record && data->AimAtRecord( record, target_data.pos, target_data.hitbox, target_data.damage, target_data.hitchance, target_data.fake_amount ) )
			{
				target_data.player = data;
				target_data.record = record;
				return true;
			}
		}

		return false;
	}

	float best_move = FLT_MAX;
	for ( int j = 0; j < data->m_records.size(); j++ )
	{
		LagRecord* current = data->m_records[ j ].get();
		if ( !current || current->immune() )
			continue;

		if ( current->m_broke_lc )
			break;

		LagRecord* previous = ( j > 0 ) ? data->m_records[ j - 1 ].get() : nullptr;

		if ( !current->valid() )
		{
			if ( target_records[ FIRST ] )
			{
				if ( ( target_records[ FIRST ]->m_origin - previous->m_origin ).length() <= 10.f )
					break;

				target_records[ LAST ] = previous;

				if ( j > 2 )
				{
					int middle_idx = std::floor( j / 2 );

					target_records[ MIDDLE ] = data->m_records[ middle_idx ].get();
					while ( target_records[ MIDDLE ] && !target_records[ MIDDLE ]->valid() )
					{
						middle_idx += 1;

						if ( middle_idx >= j - 1 )
						{
							target_records[ MIDDLE ] = nullptr;
							break;
						}

						target_records[ MIDDLE ] = data->m_records[ middle_idx ].get();
					}
				}

				break;
			}

			continue;
		}

		if ( !target_records[ FIRST ] )
			target_records[ FIRST ] = current;

		if ( current->m_mode == Resolver::Modes::RESOLVE_UPDATE )
			target_records[ STAND_LBY ] = current;

		if ( current->m_mode == Resolver::Modes::RESOLVE_MOVE )
		{
			float move_factor = abs( math::NormalizedAngle( current->m_max_feet_yaw - current->m_min_feet_yaw ) );
			if ( g_hvh.IsYawSideways( g_ctx.m_shoot_pos, current->m_origin, current->m_eye_angles.y ) )
				move_factor *= 0.25f;

			if ( move_factor < best_move )
			{
				if ( current != target_records[ FIRST ] )
					target_records[ BEST_MOVE ] = current;

				best_move = move_factor;
			}
		}
	}

	for ( int i = 0; i < target_records.size(); i++ )
	{
		LagRecord* record = target_records[ i ];
		if ( !record )
		{
			
			continue;
		}

		switch ( data->AimAtRecord( record, target_data.pos, target_data.hitbox, target_data.damage, target_data.hitchance, target_data.fake_amount ) )
		{
		case AimPlayer::ScanResult_t::HITCHANCE_TOO_LOW:
			
			return false;
		case AimPlayer::ScanResult_t::PASSED:
			target_data.player = data;
			target_data.record = record;
			return true;
		}
	}

	return false;
}

void Aimbot::think()
{
	init();

	if ( !g_ctx.m_weapon || g_ctx.m_weapon_type == WEAPONTYPE_GRENADE || g_ctx.m_weapon_type == WEAPONTYPE_C4 )
		return;

	if ( !g_ctx.m_weapon_fire )
	{
		StripAttack();

		// if it isnt full auto -> stop here, we dont need to run between shots
		// we dont use m_is_full_auto because its not set on certain weapons
		/*if ( g_ctx.m_weapon_info->m_cycletime > 0.5f )
		{
			m_skip_repred_stop = false;
			return;
		}*/

		// ^^ we are gonna do sigma pred stop with single fire guns too
		// so we can pred next fire time with scout and stop early yahhhh.
	}

	if ( !g_Variables.m_bools[ "aimbot_enabled" ] || ( g_ctx.m_weapon_id == ZEUS && !g_Variables.m_bools[ "aimbot_zeus" ] ) )
		return;

	if ( g_ctx.m_weapon_type == WEAPONTYPE_KNIFE && g_ctx.m_weapon_id != ZEUS )
	{
		if ( g_Variables.m_bools[ "aimbot_knife" ] )
		{
			DoKnifeBot();
		}

		return;
	}

	m_settings.Init();

	UpdateEarlyStop();

	if ( m_allow_auto_stop )
	{
		float pred_time = g_csgo.m_globals->m_curtime + game::TICKS_TO_TIME( m_stop_ticks + 1 );

		if ( pred_time < g_ctx.m_local->m_flNextAttack() || pred_time < g_ctx.m_weapon->m_flNextPrimaryAttack() )
		{
			
			return;
		}
	}
	else if ( !g_ctx.m_weapon_fire )
	{
		
		return;
	}

	// handle creation of our targ table (WE were getting weeird ass bugs w the array so we doing it this way)
	{
		m_sortable_players.clear();

		for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ )
		{
			AimPlayer* data = &m_players[ i - 1 ];
			if ( !IsValidTarget( data->m_player ) || data->m_player->dormant() || data->m_records.empty() || data->m_ignore )
				continue;

			m_sortable_players.push_back( data );
		}
	}

	std::sort( m_sortable_players.begin(), m_sortable_players.end(), []( const AimPlayer* a, const AimPlayer* b )
		{
			if ( a->m_player->m_iHealth() != b->m_player->m_iHealth() )
				return a->m_player->m_iHealth() < b->m_player->m_iHealth();

			return g_ctx.m_shoot_pos.dist_to( a->m_player->m_vecOrigin() )
				< g_ctx.m_shoot_pos.dist_to( b->m_player->m_vecOrigin() );
		} );

	const bool OnTheFuckingGround = g_ctx.m_local->m_fFlags() & FL_ONGROUND && ( g_ctx.m_flags & FL_ONGROUND || !( g_ctx.m_buttons & IN_JUMP ) );

	BestTarget_t best;

	for ( auto& player : m_sortable_players )
	{
		if ( *g_ctx.m_packet )
			player->m_held_damage = 0.f;

		if ( ScanPlayer( player, best ) )
		{
			if ( DelayShot( player, best.record, best.damage, best.fake_amount ) )
			{
				best.player = nullptr;
				
				continue;
			}

			break;
		}

		// > 1 because we repredict for autostop and stuff yeah
		if ( m_stop_ticks > 1 && g_Variables.m_bools[ "aimbot_early_stop" ] && !m_stop_next_tick && OnTheFuckingGround )
		{
			// its fine to just set these, they'll get overriden if we find a good target anyways.
			if ( !player->CanEarlyStop( best.record, best.damage, best.fake_amount ) )
				continue;

			if ( DelayShot( player, best.record, best.damage, best.fake_amount ) )
			{
				
				continue;
			}

			m_stop_next_tick = true;
		
		}
	}

	if ( !best.player || !best.record )
		return;

	

	m_aimplayer = best.player;
	m_aim = best.pos;
	m_damage = best.damage;
	m_hitchance = best.hitchance;
	m_record = best.record;
	m_hitbox = best.hitbox;

	math::VectorAngles( m_aim - g_ctx.m_shoot_pos, m_angle );

	m_record->cache();

	// m_early_stopping is never gonna be true here since we return.
	if ( g_Variables.m_bools[ "aimbot_autostop" ] && m_allow_auto_stop && !m_stop_next_tick && OnTheFuckingGround )
	{
		if ( !RepredictAutostop() )
		{
			
			return;
		}

		

		// fix with repred yay.
		g_ctx.m_weapon->UpdateAccuracyPenalty();

		m_stop_next_tick = true;
	}

	if ( !g_ctx.m_weapon_fire )
		return;

	if ( g_ctx.m_max_lag > 1 && g_ctx.m_weapon_fire && !g_ctx.m_lag )
	{
		*g_ctx.m_packet = false;
		return;
	}

	// Sickcvnt rapid fire.
	if ( TryOldCmd() )
		return;

	// we might want to replace this with a real hitchance check if we notice any delayed firing.
	if ( g_ctx.m_weapon->GetInaccuracy() > m_stop_inaccuracy )
		return;

	ang_t vis_angle = m_angle;

	m_angle -= g_ctx.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat();

	if ( !RecalcShootpos( m_angle ) )
	{
		
		return;
	}

	g_ctx.m_cmd->m_view_angles = m_angle;

	if ( !g_Variables.m_bools[ "aimbot_silent" ] )
		g_csgo.m_engine->SetViewAngles( vis_angle );

	if ( g_ctx.m_weapon_fire )
	{
		if ( g_Variables.m_bools[ "aimbot_autofire" ] )
		{
			//if ( g_menu.main.config.mode.get() == 1 && g_ctx.m_weapon_id == REVOLVER ) // nospread
			//{
			//	g_ctx.m_cmd->m_buttons |= IN_ATTACK2;
			//}
			//else
			//{
			g_ctx.m_cmd->m_buttons |= IN_ATTACK;
			//}
		}

		if ( g_ctx.m_cmd->m_buttons & IN_ATTACK || ( g_ctx.m_weapon_id == REVOLVER && g_ctx.m_cmd->m_buttons & IN_ATTACK2 ) && !g_ctx.m_old_shot )
		{
			*g_ctx.m_packet = false;

			if ( m_record && ( !m_record->m_broke_lc /* || m_record->m_extrapolated */ ) )
				g_ctx.m_cmd->m_tick = game::TIME_TO_TICKS( m_record->m_sim_time + g_ctx.m_lerp );

			g_shots.OnShotFire( m_aimplayer->m_player, m_damage, g_ctx.m_weapon_info->m_bullets, m_record, m_hitbox );

			g_csgo.m_engine->GetPlayerInfo( m_aimplayer->m_player->index(), &m_player_info );

			std::string str = tfm::format( "Fired at %s in the %s for %.f damage and %.f hitchance", m_player_info.m_name, g_shots.m_groups[ game::HitboxToHitgroup( m_hitbox ) ], m_damage, m_hitchance * 100.f );
			int bt = game::TIME_TO_TICKS( m_aimplayer->m_player->m_flSimulationTime() - m_record->m_sim_time );

			if ( bt == 0 )
			{
				str += "\n";
			}
			else
			{
				str += tfm::format( " with %i", abs( bt ) );
				str += abs( bt ) > 1 ? " ticks of" : " tick of";

				if ( bt < 0 )
					str += " extrapolation\n";
				// doing this when u extrapolate looks weird as fuck
				// also i would get rid of this anyways, getting impacts on backtracked shots does not really make sense.
				else
				{
					str += " backtrack\n";

					if ( g_Variables.m_bools[ "draw_impacts" ] ) // im a lazy cunt
					{
						// the idea for this is that what SHOULD happen is that when we call record->cache() an impact should appear when the firebullet
						// intersects with the hitbox. Although, this doesn't actually happen because FireBullet runs at a different time (after we restore bone caches)
						// thus, we are left with this ghetto fix 
						Color color = g_Variables.m_colors[ "client_impact_color" ];
						float size = g_Variables.m_floats[ "impacts_size" ] / 100.f;
		
					}
				}
			}

			g_ctx.print( colors::grey, str );

			g_ctx.m_shot = true;

			m_aimplayer->m_held_damage += m_damage;

			float max_speed = m_aimplayer->m_player->m_bIsScoped() ? m_aimplayer->m_player->GetActiveWeapon()->GetWpnData()->m_max_player_speed_alt : m_aimplayer->m_player->GetActiveWeapon()->GetWpnData()->m_max_player_speed;
			float curr_speed = m_record->m_anim_velocity.length();

			m_skip_hold_fakelag = m_aimplayer->m_held_damage >= m_aimplayer->m_player->m_iHealth() || m_record->m_broke_lc || ( curr_speed > max_speed * 0.6f ); // we can reasonably assume theyre peeking if theyre going this fast. worst case scenario, we should still aimpunch them and kill them anyways

			g_movement.m_stop_the_fakewalk = true;

			if ( g_Variables.m_bools[ "draw_aimbot_matrix" ] && !g_ctx.m_old_shot )
				g_visuals.DrawHitboxMatrix( m_aimplayer->m_player, m_record->m_bones, g_Variables.m_colors[ "aimbot_matrix_color" ], 4.f );
		}
	}
}

bool AimPlayer::SetupHitboxPoints( LagRecord* record, std::vector< AimPoint_t >& points, float min_dmg )
{
	points.clear();

	const model_t* model = m_player->GetModel();
	if ( !model )
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet( m_player->m_nHitboxSet() );
	if ( !set )
		return false;

	float scale = g_aimbot.m_settings.multipoint_normal_scale * 0.01f;
	float hscale = g_aimbot.m_settings.multipoint_head_scale * 0.01f;

	const auto player_centre = ( ( record->m_origin + record->m_maxs ) - ( record->m_origin + record->m_mins ) ).length() * 0.5f;

	const auto head_target = record->m_origin + vec3_t{ 0.f, 0.f, 64.f };
	const auto body_target = record->m_origin + vec3_t{ 0.f, 0.f, 32.f };



	for ( int index = 0; index < HITBOX_MAX; index++ )
	{
		if ( !AllowHitbox( index, min_dmg, record->m_mode ) )
			continue;

		mstudiobbox_t* bbox = set->GetHitbox( index );
		if ( !bbox )
			return false;

		int old_size = points.size();

		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) * 0.5f;

		float pointscale = index == HITBOX_HEAD ? hscale : scale;

		if ( bbox->m_radius > 0.f )
		{
			math::VectorTransform( center, record->m_bones[ bbox->m_bone ], center );
			pointscale *= bbox->m_radius;
		}
		else
		{
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

			matrix3x4_t matrix;
			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], rot_matrix, matrix );

			// ghettooo.
			pointscale *= center.length();

			center = { center.dot( matrix[ 0 ] ), center.dot( matrix[ 1 ] ), center.dot( matrix[ 2 ] ) };
			center += matrix.GetOrigin();
		}

		auto AddAimpoint = [&]( vec3_t positionz, bool edge = true ) -> void
			{
				AimPoint_t temp;
				temp.m_pos = positionz;
				temp.m_edge_point = edge;

				// limbs (specifically lower body, but i wanna avoid arms n shit anyways) are the exception to this rule, idk how i didnt realise this earlier.
				if ( index > HITBOX_UPPER_CHEST )
				{
					// there isnt really any way to safe point feet unfortunately, but avoiding edges should help regardless.
					temp.m_fake_amt = edge ? -1.f : 0.f;
				}
				// how close is this point (horizontally) to the centre of the players bounding box
				// imperfect way of detecting how much this point is changed by fake angles.
				else
				{
					float aura;

					if ( index == HITBOX_HEAD )
					{
						vec3_t bro = ( temp.m_pos - head_target );

						// WE WANT IT A LIL HIGHER UP.
						bro.z *= 4.f;

						aura = bro.length();
					}
					else
						aura = ( temp.m_pos - body_target ).length();


					temp.m_fake_amt = 1.f - ( aura / player_centre );
				}

				temp.m_damage = temp.m_hitchance = -1.f;
				temp.m_hitbox = index;

				points.emplace_back( temp );
			};

		AddAimpoint( center, false );

		ang_t center_angle;
		math::VectorAngles( center - g_ctx.m_shoot_pos, center_angle );

		vec3_t forward, right, up;
		math::AngleVectors( center_angle, &forward, &right, &up );

		right *= pointscale;
		up *= pointscale;

		bool always_scan = index == HITBOX_HEAD || index == HITBOX_BODY || index > HITBOX_UPPER_CHEST;

		AddAimpoint( center + right, always_scan );
		AddAimpoint( center - right, always_scan );

		if ( index == HITBOX_HEAD )
		{
			AddAimpoint( center + up );

			// add 3 points around the edges.
			for ( int i = 1; i < 4; i++ )
			{
				AddAimpoint( center + up.slerp( right, static_cast<float>( i ) / 3.f ) );
				AddAimpoint( center + up.slerp( -right, static_cast<float>( i ) / 3.f ) );
			}
		}

		if ( g_aimbot.m_settings.multipoint_dynamic_scale && !( record->m_flags & FL_DUCKING ) && bbox->m_radius > 0.f )
		{
			for ( int i = old_size; i < points.size(); i++ )
			{
				auto* p = &points[ i ];

				float effective_rad = ( bbox->m_radius * 2.f ) - ( p->m_pos - center ).length();
				float distance = ( p->m_pos - g_ctx.m_shoot_pos ).length();

				float probability = std::min( std::powf( effective_rad / ( g_aimbot.m_stop_inaccuracy * distance ), 2 ), 1.f );

				p->m_pos = math::Lerp( 1.f - probability, p->m_pos, center );
			}
		}
	}

	return points.size();
}

// featuring chatgpt psychosis code
void Aimbot::OnInject()
{
	m_sortable_players.reserve( 6 );

	struct SeedData
	{
		float inaccuracy;  // the random value between 0 and 1
		float sin_val;     // sin( pi_seed )
		float cos_val;     // cos( pi_seed )
		float spread_x;    // computed: cos_val * inaccuracy
		float spread_y;    // computed: sin_val * inaccuracy
		float r;           // radial distance (== inaccuracy in this case)
	};

	std::vector<SeedData> seeds;

	// Generate 256 seeds
	for ( int i = 0; i < 256; i++ )
	{
		float inaccuracy = g_csgo.RandomFloat( 0.f, 1.f );
		float pi_seed = g_csgo.RandomFloat( 0.f, 6.28318531f );
		float sin_val = sin( pi_seed );
		float cos_val = cos( pi_seed );
		float spread_x = cos_val * inaccuracy;
		float spread_y = sin_val * inaccuracy;
		float r = inaccuracy; // since sqrt(sin^2 + cos^2) == 1

		seeds.push_back( { inaccuracy, sin_val, cos_val, spread_x, spread_y, r } );
	}

	// Sort seeds by radial distance descending (outermost first)
	std::sort( seeds.begin(), seeds.end(), []( const SeedData& a, const SeedData& b )
		{
			return a.r > b.r;
		} );

	precomputed_seeds.clear();

	// Partition seeds into groups of 4 and reorder each group
	for ( size_t i = 0; i < seeds.size(); i += 4 )
	{
		// Get a group of 4 seeds
		std::vector<SeedData> group;
		for ( size_t j = 0; j < 4 && ( i + j ) < seeds.size(); j++ )
			group.push_back( seeds[ i + j ] );

		std::vector<SeedData> orderedGroup;

		// 1. Top-most: seed with maximum spread_y
		if ( !group.empty() )
		{
			auto it = std::max_element( group.begin(), group.end(), []( const SeedData& a, const SeedData& b )
				{
					return a.spread_y < b.spread_y;
				} );
			orderedGroup.push_back( *it );
			group.erase( it );
		}

		// 2. Bottom-most: seed with minimum spread_y
		if ( !group.empty() )
		{
			auto it = std::min_element( group.begin(), group.end(), []( const SeedData& a, const SeedData& b )
				{
					return a.spread_y < b.spread_y;
				} );
			orderedGroup.push_back( *it );
			group.erase( it );
		}

		// 3. Left-most: seed with minimum spread_x
		if ( !group.empty() )
		{
			auto it = std::min_element( group.begin(), group.end(), []( const SeedData& a, const SeedData& b )
				{
					return a.spread_x < b.spread_x;
				} );
			orderedGroup.push_back( *it );
			group.erase( it );
		}

		// 4. Right-most: remaining seed (or seed with maximum spread_x if more than one remains)
		if ( !group.empty() )
		{
			auto it = std::max_element( group.begin(), group.end(), []( const SeedData& a, const SeedData& b )
				{
					return a.spread_x < b.spread_x;
				} );
			orderedGroup.push_back( *it );
			group.erase( it );
		}

		// Add the ordered group to precomputed_seeds
		// (We only need to store the original three values so that the ScanPoint code remains unchanged.)
		for ( const auto& seed : orderedGroup )
		{
			precomputed_seeds.emplace_back( seed.inaccuracy, seed.sin_val, seed.cos_val );
		}
	}
}

void AimPlayer::ScanPoint( AimPoint_t* point, float damage, vec3_t& shootpos, LagRecord* record, int* result )
{
	point->m_damage = g_autowall.run( g_ctx.m_local, shootpos, point->m_pos );

	if ( point->m_damage < damage )
		return;

	constexpr int max_seeds = 64;
	constexpr float fmaxseeds = 64.f;

	const float hitchance = g_aimbot.m_settings.hitchance * 0.01f;

	ang_t aim_angle;
	math::VectorAngles( point->m_pos - shootpos, aim_angle );

	vec3_t forward, right, up;
	math::AngleVectors( aim_angle, &forward, &right, &up );

	vec3_t total_spread;
	float inaccuracy, spread_x, spread_y;
	ang_t spread_angle;

	vec3_t end;

	int hits = 0;
	int last_miss = 0;

	constexpr float bro = 32.f;
	constexpr float bro3 = ( bro * 2.f ) + 10.f; // extend it a lil, had issues shooting feet.

	vec3_t max_right = vec3_t( record->m_origin.x, record->m_origin.y, record->m_origin.z + record->m_maxs.z ) + right * bro;
	vec3_t max_left = vec3_t( record->m_origin.x, record->m_origin.y, record->m_origin.z + record->m_maxs.z ) + -right * bro;

	bool within_bounds = false;

	for ( int i = 0; i < max_seeds; i++ )
	{
		if ( static_cast<float>( hits + ( max_seeds - i ) ) / fmaxseeds < hitchance )
			break;

		inaccuracy = std::get<0>( g_aimbot.precomputed_seeds[ i ] ) * g_aimbot.m_stop_inaccuracy;
		spread_x = std::get<2>( g_aimbot.precomputed_seeds[ i ] ) * inaccuracy;
		spread_y = std::get<1>( g_aimbot.precomputed_seeds[ i ] ) * inaccuracy;
		total_spread = ( forward + right * spread_x + up * spread_y ).normalized();

		end = shootpos + total_spread * g_ctx.m_weapon_info->m_range;

		// we iterate inwards, so we dont need to check this if we found a point within bounds.
		if ( !within_bounds && math::SegmentToSegment( g_ctx.m_shoot_pos, end, max_left, max_right ) >= bro3 )
			continue;

		within_bounds = true;

		if ( g_autowall.run( g_ctx.m_local, shootpos, end ) >= damage )
		{
			if ( i - last_miss >= 4 )
			{
				hits += max_seeds - i;
				break;
			}
			else
				++hits;
		}
		else
		{
			last_miss = i;
		}
	}

	point->m_hitchance = static_cast<float>( hits ) / fmaxseeds;

	if ( result )
	{
		g_threading.m_mutex.lock();
		if ( *result == 0 )
		{
			*result = ( point->m_hitchance < g_aimbot.m_settings.hitchance * 0.0005 ) ? HITCHANCE_TOO_LOW : PASSED;
		}
		g_threading.m_mutex.unlock();
	}
}

int AimPlayer::AimAtRecord( LagRecord* record, vec3_t& aim_pos, int& hitbox, float& damage, float& chance, float& fake_amount, bool skip_edges )
{
	float hp = m_player->m_iHealth();
	float dmg = g_aimbot.GetMinDamage( hp );

	if ( SetupHitboxPoints( record, g_aimbot.m_aim_points, dmg ) )
	{
		record->cache();

		int scan_result = 0;

		for ( auto& p : g_aimbot.m_aim_points )
		{
			if ( p.m_edge_point == skip_edges )
				continue;

			g_threading.enqueue_task( &AimPlayer::ScanPoint, this, &p, dmg, g_ctx.m_shoot_pos, record, &scan_result );
		}

		if ( !skip_edges )
		{
			for ( auto& p : g_aimbot.m_aim_points )
			{
				if ( p.m_edge_point )
					continue;

				g_threading.enqueue_task( &AimPlayer::ScanPoint, this, &p, dmg, g_ctx.m_shoot_pos, record, nullptr );
			}
		}

		g_threading.wait_for_task_completion();

		std::pair<AimPoint_t*, float> best_point = { nullptr, -FLT_MAX };

		float hitchance = g_aimbot.m_settings.hitchance * 0.01f;

		float prefer_overlap;
		float prefer_hitchance;

		if ( ( g_aimbot.m_settings.conditional_standing && record->m_mode <= Resolver::Modes::RESOLVE_THRESHOLD )
			|| ( g_aimbot.m_settings.conditional_unsafe_moving && record->m_mode == Resolver::Modes::RESOLVE_MOVE && !g_aimbot.record_meets_safety( record ) )
			|| ( g_aimbot.m_settings.conditional_in_air && record->m_mode == Resolver::Modes::RESOLVE_AIR )
			|| ( g_aimbot.m_settings.conditional_already_missed && m_missed_shots > 0 ) )
		{
			prefer_overlap = g_aimbot.m_settings.conditional_prefer_overlap;
			prefer_hitchance = g_aimbot.m_settings.conditional_prefer_hitchance;
		}
		else
		{
			prefer_overlap = g_aimbot.m_settings.prefer_overlap;
			prefer_hitchance = g_aimbot.m_settings.prefer_hitchance;
		}

		for ( auto it = g_aimbot.m_aim_points.begin(); it != g_aimbot.m_aim_points.end(); ++it )
		{
			if ( it->m_hitchance < hitchance )
				continue;

			float score = ( ( it->m_hitchance * prefer_hitchance ) + std::min( 1.f, it->m_damage / hp ) ) * ( it->m_fake_amt * prefer_overlap );
			if ( score > best_point.second )
			{
				best_point.first = &*it;
				best_point.second = score;
			}
		}

		if ( best_point.first )
		{
			aim_pos = best_point.first->m_pos;
			hitbox = best_point.first->m_hitbox;
			damage = best_point.first->m_damage;
			chance = best_point.first->m_hitchance;
			fake_amount = best_point.first->m_fake_amt;
			return PASSED;
		}
	}

	return 0;
}

bool AimPlayer::CanEarlyStop( LagRecord*& out_record, float& damage, float& fake_amount )
{
	LagRecord* record = nullptr;
	for ( int i = 0; i < m_records.size(); i++ )
	{
		record = m_records[ i ].get();

		if ( record && record->valid( record->m_sim_time + game::TICKS_TO_TIME( g_aimbot.m_stop_ticks ) ) )
			break;
	}

	if ( !record || record->immune() )
		return false;

	out_record = record;

	float min_damage = g_aimbot.GetMinDamage( m_player->m_iHealth() );

	SetupHitboxPoints( record, g_aimbot.m_aim_points, min_damage );

	if ( g_aimbot.m_aim_points.empty() )
		return false;

	std::array< std::pair<AimPoint_t*, float>, 3> extremes;
	extremes[ 0 ] = extremes[ 1 ] = extremes[ 2 ] = { &g_aimbot.m_aim_points[ 0 ], 0.f };

	float base_dir = ( record->m_origin - g_aimbot.m_early_stop_shootpos ).y;

	for ( auto& p : g_aimbot.m_aim_points )
	{
		// its never gonna be one of deez.
		if ( !p.m_edge_point )
			continue;

		float relative_dir = math::NormalizedAngle( ( p.m_pos - g_aimbot.m_early_stop_shootpos ).y - base_dir );

		if ( p.m_pos.z > extremes[ 0 ].first->m_pos.z )
		{
			extremes[ 0 ].first = &p;
		}

		if ( relative_dir > extremes[ 1 ].second )
		{
			extremes[ 1 ].first = &p;
			extremes[ 1 ].second = relative_dir;
		}
		else if ( relative_dir < extremes[ 2 ].second )
		{
			extremes[ 2 ].first = &p;
			extremes[ 2 ].second = relative_dir;
		}
	}

	AimPoint_t* point = nullptr;

	for ( int i = 0; i < 3; i++ )
	{
		point = extremes[ i ].first;

		ScanPoint( point, min_damage, g_aimbot.m_early_stop_shootpos, record, nullptr );

		if ( point->m_damage > 0.f )
			break;
	}

	damage = point->m_damage;
	fake_amount = point->m_fake_amt;

	return point->m_hitchance >= g_aimbot.m_settings.hitchance * 0.01f;
}

bool AimPlayer::UpdateExtrapolation( int ticks, bool broke_lc )
{
	if ( m_extrapolated_record.m_tick >= m_records.front().get()->m_tick )
	{
	
		return true;
	}

	if ( ticks <= 0 || m_records.size() < 2 )
	{
		
		return false;
	}

	LagRecord* first = m_records[ 0 ].get();

	if ( first->m_anim_velocity.length_2d() <= 0.1f )
	{
		
		return false;
	}

	CCSGOPlayerAnimState* animstate = m_player->m_PlayerAnimState();
	if ( !animstate )
	{

		return false;
	}

	static datamap_t* backup_datamap = nullptr;

	if ( !backup_datamap )
	{
		//m_player->GetPredDescMap()->m_pOptimizedDataMap = nullptr;
		m_player->PrepareDatamap( m_player->GetPredDescMap() );
		backup_datamap = (datamap_t*)g_csgo.m_mem_alloc->Alloc( std::max( m_player->GetPredDescMap()->m_nPackedSize, 4 ) );
	}

	{
		CPredictionCopy CopyHelper( 2, (byte*)backup_datamap, true, (const byte*)m_player, false, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "Nigga part 1", m_player->index(), m_player->GetPredDescMap() );
	}

	C_AnimationLayer backup_layers[ 13 ]; m_player->GetAnimLayers( backup_layers );
	float backup_poses[ 24 ]; m_player->GetPoseParameters( backup_poses );
	CCSGOPlayerAnimState backup_state = *animstate;

	float time = first->m_sim_time;

	LagRecord* previous = m_records[ 1 ].get();
	const auto dir = std::remainder( math::rad_to_deg( atan2( first->m_velocity.y, first->m_velocity.x ) ), 360.f );

	LagCompensation::PreviousData_t old_data = { previous->m_duck, previous->m_flags };

	int upd_diff = broke_lc ? previous->m_lag : first->m_lag;
	if ( upd_diff == 0 )
		upd_diff = ticks;

	for ( int i = 0; i < ticks; i++ )
	{
		g_lagcomp.SimulatePlayer( m_player, time + game::TICKS_TO_TIME( i ), 0.f, dir, old_data );

		// first command, animate the player.
		if ( i % upd_diff == 0 )
		{
			const float curtime = g_csgo.m_globals->m_curtime;
			const float frametime = g_csgo.m_globals->m_frametime;

			g_csgo.m_globals->m_curtime = time + game::TICKS_TO_TIME( i );
			g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

			// todo: check if these vars are in the datamap. if so, this isnt needed.
			AnimationBackup_t backup;
			backup.store( m_player );

			// ehhhhhhhhhhh lol
			m_player->m_iEFlags() &= ~0x1000;
			m_player->m_vecAbsVelocity() = m_player->m_vecVelocity();
			m_player->SetAbsOrigin( m_player->m_vecOrigin() );

			UpdatePlayerAnims( animstate );

			m_extrapolated_record.m_anim_time = g_csgo.m_globals->m_curtime;

			g_csgo.m_globals->m_curtime = curtime;
			g_csgo.m_globals->m_frametime = frametime;

			backup.restore( m_player );
		}
		else
		{
			++m_extrapolated_record.m_lag;
		}

		old_data = { m_player->m_flDuckAmount(), m_player->m_fFlags() };
	}

	m_extrapolated_record.store( m_player, !m_extrapolated_record.m_bones ); // only allocate once.
	m_extrapolated_record.m_lag = upd_diff;
	m_player->GetPoseParameters( m_extrapolated_record.m_poses );

	m_extrapolated_record.m_sim_time += game::TICKS_TO_TIME( ticks );

	m_extrapolated_record.m_setup = g_bones.BuildBones( m_player, BONE_USED_BY_HITBOX, m_extrapolated_record.m_bones, &m_extrapolated_record );
	m_extrapolated_record.m_extrapolated = true;

	//g_visuals.DrawHitboxMatrix( m_player, m_extrapolated_record.m_bones, colors::red, 0.25f );

	m_player->SetAnimLayers( backup_layers );
	m_player->SetPoseParameters( backup_poses );
	*animstate = backup_state;

	{
		CPredictionCopy CopyHelper( 2, (byte*)m_player, false, (const byte*)backup_datamap, true, CPredictionCopy::TRANSFERDATA_COPYONLY );
		CopyHelper.TransferData( "Nigga part 2", m_player->index(), m_player->GetPredDescMap() );
	}

	// store new bone count so the hdr bone count ting dont get fucked when we copy our matrix in setupbones.
	g_aimbot.m_backup[ m_player->index() - 1 ].store( m_player );

	return true;
}

bool Aimbot::TryOldCmd()
{
	if ( !g_Variables.m_bools[ "increase_fire_rate" ] || m_record->m_broke_lc )
		return false;

	// pre-increment will avoid animating the shot.
	int oldest_seq = g_csgo.m_cl->m_last_outgoing_command + 1;
	do
	{
		if ( ++oldest_seq >= g_ctx.m_cmd->m_command_number )
			return false;
	} while ( !m_record->valid( m_record->m_sim_time, g_netdata.GetTimebase( oldest_seq ) ) );

	CUserCmd* cmd = g_csgo.m_input->GetUserCmd( oldest_seq );
	if ( !cmd )
		return false;

	auto& inaccuracy = g_netdata.GetInaccuracy( cmd->m_command_number );

	// theres like some weird imprecision that happens, so lets do some mathematics.
	if ( static_cast<int>( inaccuracy * 1000.f ) > static_cast<int>( g_aimbot.m_stop_inaccuracy * 1000.f ) )
		return false;

	auto& shootpos = g_netdata.GetShootPos( cmd->m_command_number );

	if ( g_autowall.run( g_ctx.m_local, shootpos, m_aim ) >= g_aimbot.GetMinDamage( m_aimplayer->m_player->m_iHealth() ) )
	{
		cmd->m_view_angles = m_angle;
		cmd->m_buttons |= IN_ATTACK;
		cmd->m_tick = game::TIME_TO_TICKS( m_record->m_sim_time + g_ctx.m_lerp );

		// this is only used for firing events anyways, but i want it to sound like we dt lol.
		cmd->m_predicted = false;

		// we dont need to verify our newest command because of the CreateMove hook we use :thumbsup:
		CVerifiedUserCmd* verified = g_csgo.m_input->GetVerifiedCmd( oldest_seq );
		verified->m_cmd = *cmd;
		verified->m_crc = cmd->get_checksum();

		// fuck our data off.
		g_inputpred.restore();

		// repredict all of our historical commands (since we just induced a prediction error... lol)
		// also this automatically restores us to prediction frames, no need to store any datamaps ourselves.
		g_inputpred.update();

		g_inputpred.GetDatamap( g_inputpred.cm_start );

		g_inputpred.store();
		g_inputpred.predict();

		g_inputpred.GetDatamap( g_inputpred.og_predicted );

		// dont fuck our newest command.
		g_ctx.m_weapon->UpdateAccuracyPenalty();

		g_csgo.m_engine->GetPlayerInfo( m_aimplayer->m_player->index(), &m_player_info );

		// xyren posts clips with the console open so lets confuse people.
		g_ctx.print( colors::grey, tfm::format( "Fired at %s in the %s for %.f damage and %.f hitchance with %i ticks of extrapolation\n", m_player_info.m_name, g_shots.m_groups[ game::HitboxToHitgroup( m_hitbox ) ], m_damage, m_hitchance * 100.f, g_ctx.m_cmd->m_command_number - cmd->m_command_number ) );

		// if we can already fire again, we want to go ahead and set our current cmd too.
		return !g_ctx.CanFireWeapon();
	}

	return false;
}

bool Aimbot::RepredictAutostop()
{
	CUserCmd backup_cmd = *g_ctx.m_cmd;

	// restore our datamap from createmove start.
	g_inputpred.SetDatamap( g_inputpred.cm_start );

	// set our autostop.
	g_movement.QuickStop();

	// repredict with our new cmd.
	g_inputpred.predict();

	vec3_t new_pos = g_ctx.m_local->GetShootPosition();

	if ( g_autowall.run( g_ctx.m_local, new_pos, m_aim ) >= g_aimbot.GetMinDamage( m_aimplayer->m_player->m_iHealth() ) )
	{
		g_ctx.m_shoot_pos = new_pos;
		return true;
	}

	// if autostop wasnt valid
	// set us back to where we were.
	*g_ctx.m_cmd = backup_cmd;
	g_inputpred.SetDatamap( g_inputpred.og_predicted );

	return false;
}

bool Aimbot::RecalcShootpos( ang_t& aim_angle )
{
	constexpr int max_setups = 2;

	float backup_pitch = g_ctx.m_local->m_flPoseParameter()[ 12 ];

	for ( int i = 0; i < max_setups; i++ )
	{
		g_ctx.m_local->m_flPoseParameter()[ 12 ] = ( aim_angle.x + 90.f ) / 180.f;

		vec3_t old_pos = g_ctx.m_shoot_pos;

		g_bones.Bro( g_ctx.m_local, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE, g_server_animations.local_matrix );

		g_ctx.m_shoot_pos = g_ctx.m_local->GetShootPosition();

		math::VectorAngles( m_aim - g_ctx.m_shoot_pos, aim_angle );

		aim_angle -= g_ctx.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat();

		if ( ( g_ctx.m_shoot_pos - old_pos ).length() <= 0.1f )
		{
			//g_ctx.print( std::to_string( i ) + "\n" );
			break;
		}
	}

	return g_autowall.run( g_ctx.m_local, g_ctx.m_shoot_pos, m_aim ) >= g_aimbot.GetMinDamage( m_aimplayer->m_player->m_iHealth() );
}