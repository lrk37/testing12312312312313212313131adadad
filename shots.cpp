#include "includes.h"

Shots g_shots{ };

void Shots::Process()
{
	if ( m_impacts.empty() )
		return;

	for ( auto it = m_impacts.begin(); it != m_impacts.end(); it = m_impacts.erase( it ) )
	{
		ImpactRecord* cur = &( *it );
		AimPlayer* aimplayer = &g_aimbot.m_players[ cur->m_shot->m_target->index() - 1 ];

		if ( cur->m_hit )
		{
			if ( aimplayer )
			{
				aimplayer->OnImpact( cur );
			}
			continue;
		}

		if ( !cur->m_shot->m_target->alive() )
		{
			continue;
		}

		BackupRecord backup;
		backup.store( cur->m_shot->m_target );

		cur->m_shot->m_record->cache();

		auto start = cur->m_shot->m_pos;
		auto dir = ( cur->m_pos - start ).normalized();
		auto end = start + ( dir * cur->m_shot->m_weapon_dist );

		CGameTrace trace;
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT, cur->m_shot->m_target, &trace );

		backup.restore( cur->m_shot->m_target );

		if ( trace.m_entity != cur->m_shot->m_target )
		{
			g_notify.add( g_ctx.m_last_prediction_error_time > cur->m_shot->m_time ? XOR( "Missed shot due to prediction error\n" ) : XOR( "Missed shot due to spread\n" ) );
		}
		else if ( trace.m_entity == cur->m_shot->m_target )
		{
			if ( cur->m_shot->m_record->m_mode == Resolver::Modes::RESOLVE_MOVE )
			{
				g_notify.add( XOR( "Missed shot due to animations\n" ) );
			}
			else if ( cur->m_shot->m_record->m_mode == Resolver::Modes::RESOLVE_NONE )
			{
				g_notify.add( XOR( "Missed shot due to ?\n" ) );
			}
			else
			{
				if ( cur->m_shot->m_record->m_broke_lc )
				{
					LagRecord* front = g_aimbot.m_players[ cur->m_shot->m_target->index() - 1 ].m_records[ 0 ].get();

					// dont need to store a backup record here since we restore the og after anyways.
					front->cache();

					auto start = cur->m_shot->m_pos;
					auto dir = ( cur->m_pos - start ).normalized();
					auto end = start + ( dir * cur->m_shot->m_weapon_dist );

					CGameTrace tr2;
					g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT, cur->m_shot->m_target, &tr2 );

					backup.restore( cur->m_shot->m_target );

					if ( !tr2.m_entity || !tr2.m_entity->IsPlayer() || tr2.m_entity != cur->m_shot->m_target )
					{
						g_notify.add( cur->m_shot->m_record->m_extrapolated ? XOR( "Missed shot due to extrapolation\n" ) : XOR( "Missed shot due to latency\n" ) );
					}
					else
					{
						g_notify.add( XOR( "Missed shot due to resolver\n" ) );
					}
				}
				else
				{
					if ( cur->m_shot->m_record->m_mode == Resolver::Modes::RESOLVE_UPDATE )
					{
						// ts (this) aint animations 
						g_notify.add( XOR( "Missed shot due to lby prediction\n" ) );
					}
					else
					{
						g_notify.add( XOR( "Missed shot due to resolver\n" ) );
					}

					if ( aimplayer )
					{
						aimplayer->OnImpact( cur );
					}
				}
			}
		}
	}
}

void Shots::OnShotFire( Player* target, float damage, int bullets, LagRecord* record, int hitbox )
{
	for ( int i{ }; i < bullets; ++i )
	{
		// setup new shot data.
		ShotRecord shot;
		shot.m_target = target;
		shot.m_record = record;
		shot.m_time = game::TICKS_TO_TIME( g_ctx.m_local->m_nTickBase() );
		shot.m_lat = g_ctx.m_latency;
		shot.m_damage = damage;
		shot.m_hitbox = hitbox;
		shot.m_pos = g_ctx.m_shoot_pos;
		shot.m_command_number = g_ctx.m_cmd->m_command_number;

		if ( g_ctx.m_weapon_info )
		{
			shot.m_weapon_dist = g_ctx.m_weapon_info->m_range;
		}

		m_unapproved_shots.push_front( shot );
	}
}

void Shots::OnImpact( IGameEvent* evt )
{
	int        attacker;
	vec3_t     dir, start, end;
	float      time;
	CGameTrace trace;

	if ( !evt || !g_ctx.m_local )
		return;

	vec3_t pos = {
		evt->m_keys->FindKey( HASH( "x" ) )->GetFloat(),
		evt->m_keys->FindKey( HASH( "y" ) )->GetFloat(),
		evt->m_keys->FindKey( HASH( "z" ) )->GetFloat()
	};

	attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt() );
	if ( !attacker )
		return;

	auto ent = (Player*)g_csgo.m_entlist->GetClientEntity( attacker );

	if ( ent )
	{
		if ( ent->enemy( g_ctx.m_local ) )
		{
			auto* data = &g_aimbot.m_players[ ent->index() - 1 ];

			// make the resolver face the player closest to their shot.
			if ( data )
			{
				float best_angle = FLT_MAX;
				Player* best_player = nullptr;

				for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ )
				{
					Player* player = g_csgo.m_entlist->GetClientEntity<Player*>( i );
					if ( !player || !player->alive() || !player->enemy( ent ) )
						continue;

					float angle = abs( math::NormalizedAngle( math::CalcAngle( ent->m_vecOrigin(), player->m_vecOrigin() ).y - math::CalcAngle( ent->m_vecOrigin(), pos ).y ) );

					if ( angle < best_angle )
					{
						best_player = player;
						best_angle = angle;
					}
				}

				data->m_facing_player = best_player;
			}
		}

		if ( g_Variables.m_bools[ "bullet_beams" ] )
		{
			Color beam_color;

			if ( ent == g_ctx.m_local )
			{
				if ( g_Variables.m_bools[ "local_beams" ] )
					beam_color = g_Variables.m_colors[ "local_beam_color" ];
			}
			else if ( ent->visual_enemy( g_ctx.m_local ) )
			{
				if ( g_Variables.m_bools[ "enemy_beams" ] )
					beam_color = g_Variables.m_colors[ "enemy_beam_color" ];
			}
			else if ( g_Variables.m_bools[ "friendly_beams" ] )
			{
				beam_color = g_Variables.m_colors[ "friendly_beam_color" ];
			}

			vec3_t eye_pos;
			ent->GetEyePos( &eye_pos );
			if ( ent == g_ctx.m_local )
			{
				if ( !m_vis_impacts.empty() && m_vis_impacts.back().m_tickbase == float( g_csgo.m_globals->m_curtime ) && m_vis_impacts.back().m_ent_index == attacker )  // if the currently back one has the same time and is us
					eye_pos = m_vis_impacts.back().m_shoot_pos;

				m_vis_impacts.push_back( { pos, eye_pos, int(g_csgo.m_globals->m_curtime), beam_color, attacker });
			}
			else
				m_vis_impacts.push_back( { pos, eye_pos, int( g_csgo.m_globals->m_curtime ), beam_color, attacker } );
		}
	}

	if ( attacker != g_csgo.m_engine->GetLocalPlayer() )
		return;

	if ( g_Variables.m_bools[ "draw_impacts" ] )
	{
		Color color = g_Variables.m_colors[ "server_impact_color" ];
		float size = g_Variables.m_floats["impacts_size"] / 100.f;
		
	}

	time = game::TICKS_TO_TIME( g_ctx.m_local->m_nTickBase() );

	m_pre_hitmarker_impacts.push_back( { pos, g_csgo.m_globals->m_curtime, 0 } );

	if ( m_real_shots.empty() )
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	for ( int i = 0; i < m_real_shots.size(); i++ )
	{
		auto& s = m_real_shots[ i ];
		if ( s.m_matched )
		{
			continue;
		}

		float predicted = s.m_time + s.m_lat;
		float delta = std::abs( time - predicted );

		if ( delta > 0.99f )
		{
			m_real_shots.erase( m_real_shots.begin() + i );
			continue;
		}

		if ( delta < match.delta )
		{
			match.delta = delta;
			match.shot = &s;
		}
	}

	ShotRecord* shot = match.shot;
	if ( !shot )
		return;

	shot->m_matched = true;

	ImpactRecord impact;
	impact.m_shot = shot;
	impact.m_tick = g_csgo.m_cl->m_client_tick;
	impact.m_pos = pos;

	m_impacts.push_front( impact );

	if ( g_Variables.m_bools[ "bullet_beams" ] && g_Variables.m_bools[ "local_beams" ] && !m_vis_impacts.empty() )
	{
		if ( m_vis_impacts.back().m_ent_index == attacker )
			m_vis_impacts.back().m_shoot_pos = g_netdata.GetShootPos( shot->m_command_number );

	}

	while ( m_impacts.size() > 128 )
		m_impacts.pop_back();
}

void Shots::OnHurt( IGameEvent* evt )
{
	int         attacker, victim, group, hp;
	float       damage;
	std::string name;

	if ( !evt || !g_ctx.m_local )
		return;

	attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt() );
	victim = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt() );

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if ( attacker < 1 || attacker > 64 || victim < 1 || victim > 64 )
		return;

	else if ( attacker != g_csgo.m_engine->GetLocalPlayer() || victim == g_csgo.m_engine->GetLocalPlayer() )
		return;

	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	group = evt->m_keys->FindKey( HASH( "hitgroup" ) )->GetInt();

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if ( group == HITGROUP_GEAR )
		return;

	Player* target = g_csgo.m_entlist->GetClientEntity< Player* >( victim );
	if ( !target )
		return;

	player_info_t info;
	if ( !g_csgo.m_engine->GetPlayerInfo( victim, &info ) )
		return;

	name = std::string( info.m_name ).substr( 0, 24 );
	damage = (float)evt->m_keys->FindKey( HASH( "dmg_health" ) )->GetInt();
	hp = evt->m_keys->FindKey( HASH( "health" ) )->GetInt();

	g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;

	if (!target->alive())
		g_visuals.m_hit_type = HitType_t::LETHAL;
	else if (group == HITGROUP_HEAD)
		g_visuals.m_hit_type = HitType_t::HEADSHOT;
	else 
		g_visuals.m_hit_type = HitType_t::NORMAL;

	if ( g_Variables.m_bools[ "hitmarker_sound" ] )
	{
		g_csgo.m_sound->EmitAmbientSound( XOR( "buttons/arena_switch_press_02.wav" ), 1.f );
	}

	if ( g_Variables.m_bools["log_damage"] )
	{
		std::string out = tfm::format( XOR( "Hit %s in the %s for %i damage (%i health remaining)\n" ), name, m_groups[ group ], (int)damage, hp );
		g_notify.add( out );
	}

	if (g_Variables.m_bools["center_damage_logs"]) {
		wchar_t buffer[4096] = { };
		std::string thing = m_groups[group];
		thing.at(0) = std::toupper(thing.at(0));

		wsprintfW(buffer,
			L"<font size='24' color='#9e6a4a'>%S.</font>\n<font size='24' color='#FFFFFF'>Health: </font><font size='24' color='#3b693d'>-%i</font>",
			thing.data(), (int)damage);

		g_csgo.m_center_print->Print(buffer);
	}

	if ( group == HITGROUP_GENERIC )
		return;

	if ( m_impacts.empty() )
		return;

	ImpactRecord* impact{ nullptr };

	for ( auto& i : m_impacts )
	{
		if ( i.m_tick != g_csgo.m_cl->m_client_tick )
			continue;

		if ( i.m_shot->m_target != target )
			continue;

		impact = &i;
		break;
	}

	if ( !impact )
		return;

	/*if ( group == HITGROUP_HEAD && impact->m_shot->m_record->m_mode == Resolver::Modes::RESOLVE_UPDATE )
	{
		g_notify.add( tfm::format( XOR( "Hit lby predicted shot [%s, %i, %s]\n" ), name, impact->m_shot->m_record->m_lby_pred_offset, impact->m_shot->m_record->m_ignored_lby_pred_offset ) );
	}*/

	impact->m_ACTUALLY_HITGROUP_TAPPER = group;

	float best_dist = std::numeric_limits<float>::max();
	vec3_t best_pos = {};

	LagRecord* shot_rec = impact->m_shot->m_record;
	if ( shot_rec )
	{
		for ( auto& i : m_pre_hitmarker_impacts )
		{
			float dist = ( i.m_pos - shot_rec->m_origin ).length();
			if ( dist < best_dist )
			{
				best_dist = dist;
				best_pos = i.m_pos;
			}
		}
	}

	m_pre_hitmarker_impacts.clear();
	m_post_hitmarker_impacts.push_back( { best_pos, g_csgo.m_globals->m_curtime, int( damage ), target->m_iHealth() < 1, group == HITGROUP_HEAD});

	impact->m_hit = true;

	HitRecord hit;
	hit.m_impact = impact;
	hit.m_group = group;
	hit.m_damage = damage;

	m_hits.push_front( hit );

	while ( m_hits.size() > 128 )
		m_hits.pop_back();
}