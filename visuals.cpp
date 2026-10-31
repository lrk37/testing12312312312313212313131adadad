#include "includes.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld()
{
	if ( !g_ctx.m_change_world )
		return;

	auto world_col = g_Variables.m_bools[ "world_modulation" ] ? g_Variables.m_colors[ "world_color" ] : Color( 255, 255, 255, 255 );
	auto props_col = g_Variables.m_bools[ "world_modulation" ] ? g_Variables.m_colors[ "prop_color" ] : Color( 255, 255, 255, 255 );
	auto sky_col = g_Variables.m_bools[ "world_modulation" ] ? g_Variables.m_colors[ "sky_color" ] : Color( 255, 255, 255, 255 );
	auto ambient_col = g_Variables.m_bools[ "ambient_light" ] ? g_Variables.m_colors[ "ambient_light_color" ] : Color( 0, 0, 0 );

	std::vector< IMaterial* > world, props, sky;

	// iterate material handles.
	for ( uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial( h ) )
	{
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial( h );
		if ( !mat )
			continue;

		auto group = mat->GetTextureGroupName();

		if ( *group == 'W' )
		{ // world textures
			if ( group[ 4 ] == 'd' )
			{
				world.push_back( mat );
			}
		}
		else if ( *group == 'S' )
		{ // staticprops & skybox
			auto thirdCharacter = group[ 3 ];
			if ( thirdCharacter == 'B' )
			{
				sky.push_back( mat );
			}
			else if ( thirdCharacter == 't' && group[ 6 ] == 'P' )
			{
				props.push_back( mat );
			}
		}
	}

	for ( const auto& w : world )
	{
		w->ColorModulate( world_col.r() / 255.f, world_col.g() / 255.f, world_col.b() / 255.f );
		w->AlphaModulate( world_col.a() / 255.f );
	}

	for ( const auto& s : sky )
	{
		s->ColorModulate( sky_col.r() / 255.f, sky_col.g() / 255.f, sky_col.b() / 255.f );
		s->AlphaModulate( sky_col.a() / 255.f );
	}

	// this check didnt work.
	if ( props_col != colors::white )
	{
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt() != 0 )
		{
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}
	}
	else
	{
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt() != -1 )
		{
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}
	}

	for ( const auto& p : props )
	{
		p->ColorModulate( props_col.r() / 255.f, props_col.g() / 255.f, props_col.b() / 255.f );
		p->AlphaModulate( props_col.a() / 255.f );
	}

	static ConVar* mat_ambient_light_r = g_csgo.m_cvar->FindVar( HASH( "mat_ambient_light_r" ) );
	static ConVar* mat_ambient_light_g = g_csgo.m_cvar->FindVar( HASH( "mat_ambient_light_g" ) );
	static ConVar* mat_ambient_light_b = g_csgo.m_cvar->FindVar( HASH( "mat_ambient_light_b" ) );

	mat_ambient_light_r->SetValue( ambient_col.r() / 255.f );
	mat_ambient_light_g->SetValue( ambient_col.g() / 255.f );
	mat_ambient_light_b->SetValue( ambient_col.b() / 255.f );

	/*
	Checkbox	FogOverride; // butt
	Colorpicker	FogColor; // color
	Slider		FogStart; // slider
	Slider		FogEnd; // slider
	Slider		Fogdensity; // slider
	*/
	//g_menu.main.visuals.FogColor.get().r(), g_menu.main.visuals.FogColor.get().g(), g_menu.main.visuals.FogColor.get().b()

	float destiny = g_Variables.m_floats["fog_density"] / 100.f;

	static const auto fog_enable = g_csgo.m_cvar->FindVar(HASH("fog_enable"));
	fog_enable->SetValue(1); //Включает туман на карте если он выключен по дефолту
	static const auto fog_override = g_csgo.m_cvar->FindVar(HASH("fog_override"));
	fog_override->SetValue(g_Variables.m_bools["fog_modulation"]); // Разрешает кастомизацию тумана
	static const auto fog_color = g_csgo.m_cvar->FindVar(HASH("fog_color"));
	fog_color->SetValue(std::string(std::to_string(g_Variables.m_colors["fog_color"].r()) + " " + std::to_string(g_Variables.m_colors["fog_color"].g()) + " " + std::to_string(g_Variables.m_colors["fog_color"].b())).c_str()); //Цвет тумана rgb
	static const auto fog_start = g_csgo.m_cvar->FindVar(HASH("fog_start"));
	fog_start->SetValue(g_Variables.m_floats["fog_start"]); // Дистанция с которой туман появляется
	static const auto fog_end = g_csgo.m_cvar->FindVar(HASH("fog_end"));
	fog_end->SetValue(g_Variables.m_floats["fog_end"]); // Дистанция с которой туман пропадает
	static const auto fog_destiny = g_csgo.m_cvar->FindVar(HASH("fog_maxdensity")); // The Fog Destiny Guides Us Through The Path of FOgs Destiny When we Desntity Density Destiny of Awesome.
	fog_destiny->SetValue(destiny); //Максимальная насыщенность тумана(0-1)

	g_ctx.m_change_world = false;
}

void Visuals::ThirdpersonThink()
{
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if ( !g_csgo.m_engine->IsInGame() )
		return;

	// check if we have a local player and he is alive.
	bool alive = g_ctx.m_local && g_ctx.m_local->alive();

	// camera should be in thirdperson.
	if ( g_Variables.get_keystate( g_Variables.m_binds[ "thirdperson" ] ) )
	{
		// if alive and not in thirdperson already switch to thirdperson.
		if ( alive && !g_csgo.m_input->CAM_IsThirdPerson() )
			g_csgo.m_input->CAM_ToThirdPerson();

		// if dead and spectating in firstperson switch to thirdperson.
		else if ( g_ctx.m_local->m_iObserverMode() == 4 )
		{
			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if ( g_csgo.m_input->CAM_IsThirdPerson() )
			{
				g_csgo.m_input->CAM_ToFirstPerson();
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_ctx.m_local->m_iObserverMode() = 5;
		}
	}
	// camera should be in firstperson.
	else if ( g_csgo.m_input->CAM_IsThirdPerson() )
	{
		g_csgo.m_input->CAM_ToFirstPerson();
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if ( g_csgo.m_input->CAM_IsThirdPerson() )
	{
		// get camera angles.
		g_csgo.m_engine->GetViewAngles( offset );

		// get our viewangle's forward directional vector.
		math::AngleVectors( offset, &forward );

		// cam_idealdist convar.
		offset.z = g_Variables.m_floats[ "thirdperson_distance" ];

		// start pos.
		origin = g_ctx.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity( g_ctx.m_local );

		g_csgo.m_engine_trace->TraceRay(
			Ray( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
			MASK_NPCWORLDSTATIC,
			(ITraceFilter*)&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp( tr.m_fraction, 0.f, 1.f );
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

std::string Visuals::GetWeaponIcon( int id )
{
	auto search = m_weapon_icon_map.find( id );
	if ( search != m_weapon_icon_map.end() )
	{
		return std::string( &search->second, 1 );
	}
	return XOR( "" );
}

void Visuals::NoSmoke()
{
	if ( !g_Variables.m_bools[ "remove_smoke_grenades" ] )
		return;

	//colado https://www.unknowncheats.me/forum/counterstrike-global-offensive/262635-epic-wireframe-smoke.html
	std::vector<const char*> vistasmoke_mats =
	{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	for ( auto mat_s : vistasmoke_mats )
	{
		IMaterial* mat = g_csgo.m_material_system->FindMaterial( mat_s, XOR( "Other textures" ) );
		mat->SetFlag( MATERIAL_VAR_NO_DRAW, true );
	}
}

void Visuals::think()
{
	// don't run anything if our local player isn't valid.
	if ( !g_ctx.m_local )
		return;

	if ( g_Variables.m_bools[ "remove_scope" ]
		&& g_ctx.m_local->alive()
		&& g_ctx.m_local->GetActiveWeapon()
		&& g_ctx.m_local->GetActiveWeapon()->GetWpnData()->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_ctx.m_local->m_bIsScoped() 
		&& (g_Variables.m_ints[ "penetration_crosshair_style" ] != 4 || ( g_Variables.m_ints[ "penetration_crosshair_style" ] == 4 && g_Variables.m_ints[ "skip_scope_lines_pen" ] != 0 ) ) )
	{
		// rebuild the original scope lines.
		int w = g_ctx.m_width,
			h = g_ctx.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt();

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if ( size > 1 )
		{
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		// draw our lines.
		render::rect_filled( 0, y, w, size, colors::black );
		render::rect_filled( x, 0, size, h, colors::black );
	}

	if ( !g_aimbot.m_debug_overlay.empty() )
	{
		for ( int i = 0; i < g_aimbot.m_debug_overlay.size(); i++ )
		{
			render::esp.string( g_ctx.m_width - 20, 50 + ( render::esp.m_size.m_height * i ), colors::white, g_aimbot.m_debug_overlay[ i ], render::ALIGN_RIGHT );
		}
	}

	// draw esp on ents.
	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i )
	{
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		draw( ent );
	}

	// draw everything else.
	DrawPlantedC4();
	DrawHitmarkers();
	Spectators();
	SpreadCrosshair();
	StatusIndicators();
	ManualAntiAim();
	PenetrationCrosshair();
}

void Visuals::Spectators()
{
	if ( !g_Variables.m_bools[ "spectator_list" ] )
		return;

	std::vector< std::string > spectators{ XOR( "" ) };
	int h = render::menu_shade.m_size.m_height;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i )
	{
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !player )
			continue;

		if ( player->m_bIsLocalPlayer() )
			continue;

		if ( player->dormant() )
			continue;

		if ( player->m_lifeState() == LIFE_ALIVE )
			continue;

		if ( g_ctx.m_processing )
		{
			if ( player->GetObserverTarget() != g_ctx.m_local )
			{
				continue;
			}
		}
		else
		{
			if ( player->GetObserverTarget() != g_ctx.m_local->GetObserverTarget() )
			{
				continue;
			}
		}

		player_info_t info;
		if ( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
			continue;

		spectators.push_back( std::string( info.m_name ).substr( 0, 24 ) );
	}

	if ( spectators.empty() )
		return;

	size_t total_size = spectators.size() * ( h - 1 );
	float add_offset = 0.f;
	if ( g_Variables.m_bools[ "watermark" ] )
	{
		add_offset += g_ctx.watermark_textsize.second + 12;
	}

	for ( size_t i{ }; i < spectators.size(); ++i )
	{
		const std::string& name = spectators[ i ];

		render::esp.string( g_ctx.m_width - 15, ( i * 15 ) - 10 + add_offset, { 255, 255, 255, 200 }, name, render::ALIGN_RIGHT );
	}
}

void Visuals::StatusIndicators()
{
	// dont do if dead.
	if ( !g_ctx.m_processing )
		return;

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	// PING
	if (g_Variables.m_bools["ping_indicator"] && (g_Variables.m_bools["fakelatency"] || g_Variables.get_keystate(g_Variables.m_binds["fakelatency_spike"])))
	{
		bool spiking = g_Variables.get_keystate( g_Variables.m_binds[ "fakelatency_spike" ] );

		float flAmt = std::min( g_csgo.sv_maxunlag->GetFloat() * 1000.f, spiking ? g_Variables.m_floats["fakelatency_spike"] : g_Variables.m_floats["fakelatency_default"] );
		float flRatio = std::clamp( g_ctx.m_fakelatency_amt / flAmt, 0.f, 1.f );

		Color scnd = spiking ? Color(125, 195, 15, 240) : Color(255, 255, 255, flRatio * 220.f);

		Indicator_t ind{ };
		ind.color = Color(125, 0, 0, 210).LerpRGB(scnd, flRatio);
		ind.color.brighten();
		ind.text = XOR("PING");

		indicators.push_back(ind);
	}

	// LBY
	if ( g_Variables.m_bools[ "lby_indicator" ] )
	{
		// get the absolute change between current lby and animated angle.
		bool change = std::abs( math::NormalizedAngle( g_ctx.m_local->m_flLowerBodyYawTarget() - g_ctx.m_angle.y ) ) > 35.f;

		Indicator_t ind{ };
		if ( change )
		{
			ind.color = 0xff15c27b;//change ? 0xff15c27b : 0xff0000ff;
		}
		else
		{
			CCSGOPlayerAnimState* animstate = g_ctx.m_local->m_PlayerAnimState();
			if ( animstate )
			{
				ind.color = abs( math::NormalizedAngle( animstate->m_abs_yaw - g_server_animations.m_fake_feet_yaw ) ) > 10.f ? 0xFFB4B900 : 0xff0000ff;
			}
			else
			{
				ind.color = 0xff0000ff;
			}
		}
		ind.text = XOR( "LBY" );
		indicators.push_back( ind );
	}

	// LC
	if ( g_Variables.m_bools[ "lc_indicator" ] && ( ( g_ctx.m_buttons & IN_JUMP ) || !( g_ctx.m_flags & FL_ONGROUND ) ) )
	{
		Indicator_t ind{ };
		ind.color = g_ctx.m_lagcomp ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LC" );

		indicators.push_back( ind );
	}

	if ( g_Variables.m_bools["dmg_indicator"] && g_Variables.get_keystate(g_Variables.m_binds["aimbot_mindamageoverride"]))
	{
		Indicator_t ind{ };
		ind.color = Color( 255, 255, 255, 210 );
		ind.text = XOR( "DMG" );

		indicators.push_back( ind );
	}

	if ( g_Variables.m_bools[ "baim_indicator" ] && g_Variables.get_keystate( g_Variables.m_binds[ "aimbot_forcebaim" ] ) )
	{
		Indicator_t ind{ };
		ind.color = Color( 255, 255, 255, 210 );
		ind.text = XOR( "BAIM" );

		indicators.push_back( ind );
	}

	if ( indicators.empty() )
		return;

	// iterate and draw indicators.
	float velmod = g_ctx.m_local->m_flVelocityModifier();
	for ( size_t i{ }; i < indicators.size(); ++i )
	{
		auto& indicator = indicators[ i ];
		indicator.color.a() = 255 * velmod;

		render::indicator.string( 12, g_ctx.m_height - 64 - ( 30 * i ), indicator.color, indicator.text );
	}
}

void Visuals::SpreadCrosshair()
{
	// dont do if dead.
	if ( !g_ctx.m_processing )
		return;

	if ( !g_Variables.m_bools[ "spread_crosshair" ] )
		return;

	// get active weapon.
	Weapon* weapon = g_ctx.m_local->GetActiveWeapon();
	if ( !weapon )
		return;

	WeaponInfo* data = weapon->GetWpnData();
	if ( !data )
		return;

	// do not do this on: bomb, knife and nades.
	int type = weapon->m_iItemDefinitionIndex();
	if ( type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE )
		return;

	int w, h;
	g_csgo.m_engine->GetScreenSize( w, h );

	float spreadDist = ( ( weapon->GetInaccuracy() + weapon->GetSpread() ) * 320.f ) / std::tan( math::deg_to_rad( g_ctx.m_local->GetFOV() ) * 0.5f );
	float spreadRadius = ( spreadDist * ( h / 480.f ) ) * 50 / 250.f;

	for ( float i = 0; i <= spreadRadius; i++ )
	{
		Color col = g_Variables.m_colors[ "spread_crosshair_color" ];
		col.a() = static_cast<int>( i * ( col.a() / spreadRadius ) );

		g_csgo.m_surface->DrawSetColor( col );
		g_csgo.m_surface->DrawOutlinedCircle( w / 2, h / 2, static_cast<int>( i ), 240 );
	}
}

void RenderPolygonSup( float x, float y, float rotation, Color color )
{
	Vertex verts[ 3 ];

	float size = 0.8f;

	verts[ 0 ] = { x, y };        // 0,  0
	verts[ 1 ] = { x - ( 12.f * size ), y + ( 24.f * size ) }; // -1, 1
	verts[ 2 ] = { x + ( 12.f * size ), y + ( 24.f * size ) }; // 1,  1

	verts[ 0 ] = render::RotateVertex( vec2_t( x, y ), verts[ 0 ], rotation * 90 );
	verts[ 1 ] = render::RotateVertex( vec2_t( x, y ), verts[ 1 ], rotation * 90 );
	verts[ 2 ] = render::RotateVertex( vec2_t( x, y ), verts[ 2 ], rotation * 90 );

	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawTexturedPolygon( 3, verts );
}

void Visuals::ManualAntiAim()
{
	if ( !g_ctx.m_processing )
		return;

	if ( ( g_hvh.m_queued_dir < 0 && g_hvh.m_manual_direction < 0 && g_hvh.m_manual_lastmove < 0 ) || !g_Variables.m_bools[ "antiaim_manual_arrows" ] )
		return;

	int x = g_ctx.m_width / 2;
	int y = g_ctx.m_height / 2;

	Color color = g_Variables.m_colors[ "antiaim_manual_arrows_color" ];

	switch ( g_Variables.m_ints[ "antiaim_manual_arrows_style" ] )
	{
	case 0:
		if ( g_hvh.m_manual_lastmove != g_hvh.m_manual_direction )
		{
			switch ( g_hvh.m_manual_lastmove )
			{
			case ManualAAMode::BACK:
				RenderPolygonSup( x, y + 60, 2, g_Variables.m_colors[ "antiaim_manual_lastmove_color" ] );
				break;
			case ManualAAMode::RIGHT:
				RenderPolygonSup( x + 60, y, 1, g_Variables.m_colors[ "antiaim_manual_lastmove_color" ] );
				break;
			case ManualAAMode::LEFT:
				RenderPolygonSup( x - 60, y, 3, g_Variables.m_colors[ "antiaim_manual_lastmove_color" ] );
				break;
			case ManualAAMode::FORWARD:
				RenderPolygonSup( x, y - 60, 4, g_Variables.m_colors[ "antiaim_manual_lastmove_color" ] );
				break;
			}
		}

		switch ( g_hvh.m_manual_direction )
		{
		case ManualAAMode::BACK:
			RenderPolygonSup( x, y + 60, 2, color );
			break;
		case ManualAAMode::RIGHT:
			RenderPolygonSup( x + 60, y, 1, color );
			break;
		case ManualAAMode::LEFT:
			RenderPolygonSup( x - 60, y, 3, color );
			break;
		case ManualAAMode::FORWARD:
			RenderPolygonSup( x, y - 60, 4, color );
			break;
		}

		if ( g_hvh.m_queued_dir > 0 )
		{
			color.a() *= std::min( ( g_csgo.m_globals->m_realtime - g_hvh.m_last_press_time ) / game::TICKS_TO_TIME( 16 ), 1.f );

			switch ( g_hvh.m_queued_dir )
			{
			case ManualAAMode::BACK:
				RenderPolygonSup( x, y + 60, 2, color );
				break;
			case ManualAAMode::RIGHT:
				RenderPolygonSup( x + 60, y, 1, color );
				break;
			case ManualAAMode::LEFT:
				RenderPolygonSup( x - 60, y, 3, color );
				break;
			case ManualAAMode::FORWARD:
				RenderPolygonSup( x, y - 60, 4, color );
				break;
			}
		}

		break;
	case 1:
		Color inactive = Color( 170, 170, 170 );

		render::KibitNapHackArrow.string( x - 60, y - 13, g_hvh.m_manual_direction == LEFT || g_hvh.m_manual_direction == FORWARD ? color : inactive, "<" );
		render::KibitNapHackArrow.string( x + 40, y - 13, g_hvh.m_manual_direction == RIGHT || g_hvh.m_manual_direction == FORWARD ? color : inactive, ">" );
		render::KibitNapHackArrow.string( x - 7, y + 30, g_hvh.m_manual_direction == BACK || g_hvh.m_manual_direction == FORWARD ? color : inactive, "v" );
		break;
	}
}

void Visuals::PrePaint()
{
	g_csgo.m_prediction->SetLocalViewAngles( g_ctx.m_sent_angles );
}

void Visuals::PenetrationCrosshair()
{
	if ( !g_Variables.m_bools[ "penetration_crosshair" ] || !g_ctx.m_processing || g_ctx.m_pen_crosshair_bullshit_shit_feature == 0 )
		return;

	int x = g_ctx.m_width / 2;
	int y = g_ctx.m_height / 2;

	Color final_color;

	switch ( g_ctx.m_pen_crosshair_bullshit_shit_feature )
	{
	case 3:
		final_color = g_Variables.m_colors[ "pen_player" ];
		break;
	case 2:
		final_color = g_Variables.m_colors[ "pen_active" ];
		break;
	case 1:
		final_color = g_Variables.m_colors[ "pen_inactive" ];
		break;
	}

	switch ( g_Variables.m_ints[ "penetration_crosshair_style" ] )
	{
	case 0:
		render::circle( x, y, 2.f, 16, { 10, 10, 10, 220 } );
		render::circle( x, y, 1.f, 16, final_color );
		break;
	case 1:
		render::rect( x - 1, y - 1, 3, 3, { 0, 0, 0, 125 } );
		render::rect( x, y - 1, 1, 3, final_color );
		render::rect( x - 1, y, 3, 1, final_color );
		break;
	case 2:
		render::rect_filled( x - 1, y - 1, 3, 3, final_color );
		break;
	case 3:
		render::line( x - 1, y + 1, x - 1, y - 1, final_color ); // left
		render::line( x + 1, y + 1, x + 1, y - 1, final_color ); // right
		render::line( x + 1, y - 1, x - 1, y - 1, final_color ); // top
		render::line( x + 1, y + 1, x - 1, y + 1, final_color ); // bottom
		break;
	}
}

void Visuals::HandleLocal(Player* local)
{
	if ( !local->alive() )
		return;

	if ( g_Variables.m_bools[ "esp_local_skeleton" ] && g_csgo.m_input->m_camera_in_third_person )
		DrawSkeleton(local, g_Variables.m_colors[ "esp_local_skeleton_color" ], 1);
}

void Visuals::draw( Entity* ent )
{
	if ( ent->IsPlayer() )
	{
		Player* player = ent->as< Player* >();
		if ( player->m_bIsLocalPlayer() )
		{
			return HandleLocal(player);
		}

		if ( !g_Variables.m_bools[ "esp_enabled" ] )
			return;

		if ( !player->alive() )
			return;

		if ( !player->visual_enemy( g_ctx.m_local ) && !g_Variables.m_bools[ "esp_draw_teammates" ] )
			return;

		// draw player esp.
		DrawPlayer( player );
	}

	else if ( g_Variables.m_bools[ "dropped_weapons" ] && ent->IsBaseCombatWeapon() && !ent->dormant() )
		DrawItem( ent->as< Weapon* >() );

	else if ( g_Variables.m_bools[ "draw_grenades" ] )
		DrawProjectile( ent->as< Weapon* >() );
}

void Visuals::DrawProjectile( Weapon* ent )
{
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();

	if ( !render::WorldToScreen( origin, screen ) )
		return;

	Color col = g_Variables.m_colors[ "draw_grenades_color" ];

	int id = ent->GetClientClass()->m_ClassID;

	switch ( id )
	{
	case _CHostage:
	{
		int dist = ( ent->m_vecOrigin() - g_ctx.m_local->m_vecOrigin() ).length();

		if ( dist < 150 )
		{
			render::esp_small.string( screen.x, screen.y, colors::light_blue, XOR( "HOSTAGE" ), render::ALIGN_CENTER );
		}
	}
		break;
	case _CDecoyProjectile:
		render::esp_small.string( screen.x, screen.y, col, XOR( "DECOY" ), render::ALIGN_CENTER );
		break;
	case _CMolotovProjectile:
		render::esp_small.string( screen.x, screen.y, col, XOR( "MOLLY" ), render::ALIGN_CENTER );
		break;
	case _CBaseCSGrenadeProjectile:
	{
		const model_t* model = ent->GetModel();

		if ( model )
		{
			// grab modelname.
			std::string name{ ent->GetModel()->m_name };

			if ( name.find( XOR( "flashbang" ) ) != std::string::npos )
			{
				render::esp_small.string( screen.x, screen.y, col, XOR( "FLASH" ), render::ALIGN_CENTER );
			}
			else if ( name.find( XOR( "fraggrenade" ) ) != std::string::npos )
			{
				render::esp_small.string( screen.x, screen.y, col, XOR( "FRAG" ), render::ALIGN_CENTER );
			}
		}
	}
		break;
	case _CInferno:
	{
		auto owner = g_csgo.m_entlist->GetClientEntityFromHandle( ent->m_hOwnerEntity() );

		if ( owner )
		{
			if ( owner->as<Player*>()->visual_enemy( g_ctx.m_local ) || owner == g_ctx.m_local )
				render::esp_small.string( screen.x, screen.y, col, XOR( "FIRE" ), render::ALIGN_CENTER );
		}
	}
		break;
	case _CSmokeGrenadeProjectile:
		render::esp_small.string( screen.x, screen.y, col, XOR( "SMOKE" ), render::ALIGN_CENTER );
		break;
	}
}

void Visuals::DrawItem( Weapon* item )
{
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle( item->m_hOwnerEntity() );
	if ( owner )
		return;

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin();
	if ( !render::WorldToScreen( origin, screen ) )
		return;

	WeaponInfo* data = item->GetWpnData();
	if ( !data )
		return;

	// render bomb in green.
	if ( item->is( HASH( "CC4" ) ) )
	{
		render::esp_small.string( screen.x, screen.y, { 150, 200, 60, 180 }, XOR( "BOMB" ), render::ALIGN_CENTER );
		return;
	}

	float delta = (g_ctx.m_local->GetAbsOrigin() - origin).length() * 0.001f;

	if (delta > 1.f)
		return; // lets not even bother 

	int offset = 0;

	if ( g_Variables.m_bools[ "dropped_weapon_text" ] )
	{
		Color col = g_Variables.m_colors[ "dropped_weapon_text_color" ];
		col.a() = math::Lerp(std::clamp(delta, 0.f, 1.f), (float)col.a(), 0.f);

		// if not bomb
        // normal item, get its name.
		std::string name{ item->GetLocalizedName() };

		// smallfonts needs uppercase.
		std::transform( name.begin(), name.end(), name.begin(), ::toupper );

		render::esp_small.string( screen.x, screen.y, col, name, render::ALIGN_CENTER );

		offset += render::esp_small.m_size.m_height - 1;
	}

	if ( g_Variables.m_bools[ "dropped_weapon_icon" ] )
	{
		Color col = g_Variables.m_colors[ "dropped_weapon_icon_color" ];
		col.a() = math::Lerp(std::clamp(delta, 0.f, 1.f), (float)col.a(), 0.f);

		std::string icon = GetWeaponIcon( item->m_iItemDefinitionIndex() );

		offset += 2;

		render::weapon_icons.string( screen.x, screen.y + offset, col, icon, render::ALIGN_CENTER );
	}
}

void Visuals::OffScreen( Player* player, float alpha )
{
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[ 3 ], verts_outline[ 3 ];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = []( const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation )
		{
			ang_t  view_angles( g_csgo.m_view_render->m_view.m_angles );
			vec3_t fwd, right, up( 0.f, 0.f, 1.f );
			float  front, side, yaw_rad, sa, ca;

			// get viewport angles forward directional vector.
			math::AngleVectors( view_angles, &fwd );

			// convert viewangles forward directional vector to a unit vector.
			fwd.z = 0.f;
			fwd.normalize();

			// calculate front / side positions.
			right = up.cross( fwd );
			front = delta.dot( fwd );
			side = delta.dot( right );

			// setup offscreen position.
			out_offscreen_pos.x = radius * -side;
			out_offscreen_pos.y = radius * -front;

			// get the rotation ( yaw, 0 - 360 ).
			out_rotation = math::rad_to_deg( std::atan2( out_offscreen_pos.x, out_offscreen_pos.y ) + math::pi );

			// get needed sine / cosine values.
			yaw_rad = math::deg_to_rad( -out_rotation );
			sa = std::sin( yaw_rad );
			ca = std::cos( yaw_rad );

			// rotate offscreen position around.
			out_offscreen_pos.x = (int)( ( g_ctx.m_width / 2.f ) + ( radius * sa ) );
			out_offscreen_pos.y = (int)( ( g_ctx.m_height / 2.f ) - ( radius * ca ) );
		};

	if ( !g_Variables.m_bools[ "oof_arrows" ] )
		return;

	if ( !g_ctx.m_processing || !g_ctx.m_local->visual_enemy( player ) )
		return;

	// get the player's center screen position.
	target_pos = origin_cache[ player->index() - 1 ];
	is_on_screen = render::WorldToScreen( target_pos, screen_pos );

	// give some extra room for screen position to be off screen.
	leeway_x = g_ctx.m_width / 18.f;
	leeway_y = g_ctx.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if ( !is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >( g_ctx.m_width + leeway_x )
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >( g_ctx.m_height + leeway_y ) )
	{
		float size = g_Variables.m_floats[ "oof_arrows_size" ] * 0.1f;
		float pos = (g_ctx.m_height * 0.15f) * (g_Variables.m_floats["oof_arrows_distance"] * 0.01f);

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = ( target_pos - view_origin ).normalized();

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = pos * ( g_ctx.m_height / 480.f );

		// get the data we need for rendering.
		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference:
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[ 0 ] = { offscreen_pos.x + ( 1 * size ) , offscreen_pos.y + ( 1 * size ) };        // 0,  0
		verts[ 1 ] = { offscreen_pos.x - ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // -1, 1
		verts[ 2 ] = { offscreen_pos.x + ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[ 0 ] = { verts[ 0 ].m_pos.x - 1.f, verts[ 0 ].m_pos.y - 1.f };
		verts_outline[ 1 ] = { verts[ 1 ].m_pos.x - 1.f, verts[ 1 ].m_pos.y + 1.f };
		verts_outline[ 2 ] = { verts[ 2 ].m_pos.x + 1.f, verts[ 2 ].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[ 0 ] = render::RotateVertex( offscreen_pos, verts[ 0 ], offscreen_rotation );
		verts[ 1 ] = render::RotateVertex( offscreen_pos, verts[ 1 ], offscreen_rotation );
		verts[ 2 ] = render::RotateVertex( offscreen_pos, verts[ 2 ], offscreen_rotation );

		// render!
		float alpha1337 = abs( sin( g_csgo.m_globals->m_curtime ) );

		color = g_Variables.m_colors[ "oof_arrows_color" ];
		color.a() *= alpha1337 * alpha;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, verts );
	}
}

void Color::brighten()
{
	float h, s, v = 0.f;
	float r, g, b = 0.f;

	ImGui::ColorConvertRGBtoHSV( (float)this->r(), (float)this->g(), (float)this->b(), h, s, v );
	ImGui::ColorConvertHSVtoRGB( h, s, 1.f, r, g, b );

	this->r() = r * 255;
	this->g() = g * 255;
	this->b() = b * 255;
}

enum renderFont_t
{
	VERDANA = 0,
	VERDANABOLD,
	TAHOMABOLD,
	SMALLFONTS
};

render::Font get_name_font()
{
	// idc rn
	switch ( g_Variables.m_ints[ "esp_name_font" ] )
	{
	case VERDANA:
		return render::esp;
	case VERDANABOLD:
		return render::esp2;
	case TAHOMABOLD:
		return render::esp3;
	case SMALLFONTS:
		return render::esp_small;
	}

	return render::esp;
}

void Visuals::DrawPlayer( Player* player )
{
	constexpr float MAX_DORMANT_TIME = 15.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;
	constexpr float LOWER_ALPHA = 200.f;

	player_info_t info;
	Rect          box;

	// get player index.
	int index = player->index();

	// get reference to array variable.
	bool& draw = m_draw[ index - 1 ];
	float& opacity = m_opacities[ index - 1 ];

	// opacity should reach 1 in 100 milliseconds.
	constexpr int frequency = 1.f / 0.1f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool enemy = player->visual_enemy( g_ctx.m_local );
	bool dormant = player->dormant();

	if ( enemy && !dormant )
		player->m_bSpotted() = true;

	// we can draw this player again.
	if ( !dormant )
	{
		draw = true;
		origin_cache[ player->index() - 1 ] = player->GetAbsOrigin();
		m_time_cache[ player->index() - 1 ] = g_csgo.m_globals->m_realtime;
	}

	if ( !draw )
	{
		opacity = 0.f;
		return;
	}

	dormant ? opacity -= step : opacity += step;

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy && g_Variables.m_bools[ "esp_draw_dormant" ];

	// clamp the opacity.
	math::clamp( opacity, 0.f, 1.f );
	if ( !opacity && !dormant_esp )
		return;

	// stay for x seconds max.
	float dt = g_csgo.m_globals->m_realtime - m_time_cache[ player->index() - 1 ];
	if ( dormant && dt > MAX_DORMANT_TIME )
		return;

	Color box_color = g_Variables.m_colors[ "esp_box_color" ];

	float alpha_mod = opacity;

	if ( dormant && dormant_esp )
	{
		alpha_mod = 0.5f;

		if ( dt > DORMANT_FADE_TIME )
		{
			float faded = ( dt - DORMANT_FADE_TIME );
			float scale = 1.f - ( faded / DORMANT_FADE_TIME );

			alpha_mod *= scale;
		}

		box_color = g_Variables.m_colors[ "esp_dormant" ];
	}

	box_color.a() *= alpha_mod;

	// get player info.
	if ( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	// run offscreen ESP.
	OffScreen( player, alpha_mod );

	if ( !GetPlayerBoxRect( player, box ) )
	{
		return;
	}

	AimPlayer* aimplayer = &g_aimbot.m_players[ index - 1 ];

	// player closest to our crosshair.
	//if ( index == g_aimbot.m_resolver_override_index && aimplayer && !aimplayer->resolver_debug_data.empty() )
	//{
	//	render::menu.string( 12, g_ctx.m_height * 0.5f, colors::white, aimplayer->resolver_debug_data );
	//}

	//DebugAimbotPoints( player );

	bool bone_esp = g_Variables.m_bools[ "esp_skeleton" ];
	if ( bone_esp )
	{
		DrawSkeleton( player, g_Variables.m_colors[ "esp_skeleton_color" ], opacity );
	}

	int alpha_bro = LOWER_ALPHA * alpha_mod;

	// is box esp enabled for this player.
	bool box_esp = g_Variables.m_bools[ "esp_box" ];

	// render box if specified.
	if ( box_esp )
		render::rect_outlined( box.x, box.y, box.w, box.h, box_color, { 10, 10, 10, alpha_bro } );

	// is name esp enabled for this player.
	bool name_esp = g_Variables.m_bools[ "esp_name" ];

	// draw name.
	if ( name_esp )
	{
		// fix retards with their namechange meme
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		std::string name{ std::string( info.m_name ).substr( 0, 24 ) };
		render::Font f = get_name_font();

		if ( g_Variables.m_ints[ "esp_name_font" ] == SMALLFONTS )
			std::transform( name.begin(), name.end(), name.begin(), ::toupper );

		Color clr = g_Variables.m_colors[ "esp_name_color" ];

		if ( dormant && g_Variables.m_bools[ "esp_override_dormant" ] )
			clr = g_Variables.m_colors[ "esp_dormant" ];

		clr.a() *= alpha_mod;

		f.string( box.x + box.w / 2, box.y - f.m_size.m_height, clr, name, render::ALIGN_CENTER );

		if ( aimplayer && aimplayer->m_override != 0.f )
		{
			render::esp_small.string( box.x + box.w / 2, box.y - f.m_size.m_height - render::esp_small.m_size.m_height,
				aimplayer->m_override_agreed ? Color( 150, 200, 60, alpha_bro ) : Color( 255, 0, 0, alpha_bro ), XOR( "OVERRIDE" ), render::ALIGN_CENTER );
		}
	}

	// is health esp enabled for this player.
	bool health_esp = g_Variables.m_bools[ "esp_health" ];

	if ( health_esp )
	{
		int y = box.y + 1;
		int h = box.h - 2;

		int health = player->m_iHealth();

		Color firstcol = g_Variables.m_colors[ "esp_health_start_color" ];
		Color secondcol = g_Variables.m_colors[ "esp_health_end_color" ];

		Color col = Color( secondcol ).LerpRGB( firstcol, health * 0.01f );
		col.brighten();
		col.a() *= alpha_mod;

		// render background.
		float bg_alpha = 255 * ( g_Variables.m_floats[ "esp_health_bar_alpha" ] * 0.01f ) * alpha_mod;

		render::rect_filled( box.x - 6, y - 2, 4, h + 3 + 1, { 10, 10, 10, (int)bg_alpha } );

		// get hp bar height.
		int fill = (int)std::round( health * h / 100.f );

		// render actual bar.
		if ( dormant && g_Variables.m_bools[ "esp_override_dormant" ] )
		{
			col = g_Variables.m_colors[ "esp_dormant" ];
			col.a() *= alpha_mod;
			render::rect( box.x - 5, y + h - fill - 1, 2, fill + 2, g_Variables.m_colors[ "esp_dormant" ] );
		}
		else
			render::rect( box.x - 5, y + h - fill - 1, 2, fill + 2, col );

		if ( g_Variables.m_bools[ "esp_health_text" ] )
		{
			bool lethal = false;
			if ( g_ctx.m_weapon_info )
			{
				lethal = g_autowall.ScaleDamage( player, g_ctx.m_weapon_info->m_damage, g_ctx.m_weapon_info->m_armor_ratio, HITGROUP_STOMACH ) >= health;
			}

			// < 100 check so its not ugly with AWP.
			if ( health < 100 && lethal )
			{
				if ( dormant )
				{ // im not overriding this one itd be nasty as fuq
					render::esp_small.string( box.x - 5, y + ( h - fill ) - 5, { 180, 180, 180, alpha_bro }, std::to_string( health ), render::ALIGN_CENTER );
				}
				else
				{
					render::esp_small.string( box.x - 5, y + ( h - fill ) - 5, { 255, 255, 255, alpha_bro }, std::to_string( health ), render::ALIGN_CENTER );
				}
			}
		}
	}

	// draw flags.
	{
		std::vector< std::pair< std::string, Color > > flags;

		bool skip_flags = !g_Variables.m_bools[ "esp_flags" ];

		if ( g_Variables.m_bools[ "esp_flags_money" ] && !skip_flags )
		{
			flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount() ),  Color( 155, 210, 100, alpha_bro ) } );
		}

		if ( aimplayer )
		{
			if ( aimplayer->m_ignore )
			{
				flags.push_back( { XOR( "WL" ),  Color( 155, 210, 100, alpha_bro ) } );
				skip_flags = true;
			}
		}

		if ( !skip_flags )
		{
			if ( g_Variables.m_bools[ "esp_flags_ping" ] && g_ctx.player_resource )
			{
				auto latency = std::clamp( g_ctx.player_resource->m_iPing( player->index() ), 0, 999 );
				float rate = ( float( latency ) * 0.001f ) / g_csgo.sv_maxunlag->GetFloat();
				rate = std::clamp( rate, 0.f, 1.f );

				Color color = Color( 155, 210, 100, alpha_bro ).LerpRGB( Color( 255, 0, 0, alpha_bro ), rate );

				if ( g_Variables.m_bools[ "brighten_ping" ] )
					color.brighten();

				std::string txt = tfm::format( XOR( "%i" ), latency );

				if ( g_Variables.m_bools[ "capitalize_ping" ] )
					txt += "MS";
				else
					txt += "ms";

				flags.push_back( { txt, color } ); // prob a nicer way to do this
			}

			if ( g_Variables.m_bools[ "esp_flags_armor" ] )
			{
				if ( player->m_bHasHelmet() && player->m_ArmorValue() > 0 )
				{
					flags.push_back( { XOR( "HK" ), Color( 255, 255, 255, alpha_bro ) } );
				}
				// only helmet.
				else if ( player->m_bHasHelmet() )
				{
					flags.push_back( { XOR( "H" ), Color( 255, 255, 255, alpha_bro ) } );
				}
				// only kevlar.
				else if ( player->m_ArmorValue() > 0 )
				{
					flags.push_back( { XOR( "K" ), Color( 255, 255, 255, alpha_bro ) } );
				}
			}

			// scoped.
			if ( g_Variables.m_bools[ "esp_flags_scoped" ] && player->m_bIsScoped() )
			{
				flags.push_back( { XOR( "ZOOM" ), Color( 0, 175, 255, alpha_bro ) } );
			}

			// flashed.
			if ( g_Variables.m_bools[ "esp_flags_flashed" ] && player->m_flFlashBangTime() > 0.f )
			{
				flags.push_back( { XOR( "FLASHED" ), Color( 0, 175, 255, alpha_bro ) } );
			}

			// reload.
			if ( g_Variables.m_bools[ "esp_flags_reload" ] )
			{
				C_AnimationLayer* layer1 = &player->m_AnimOverlay()[ 1 ];

				// check if reload animation is going on.
				if ( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 )
				{
					flags.push_back( { XOR( "RELOAD" ), Color( 0, 175, 255, alpha_bro ) } );
				}
			}

			// bomb.
			if ( g_Variables.m_bools[ "esp_flags_c4" ] && player->HasC4() )
			{
				flags.push_back( { XOR( "C4" ), Color( 255, 0, 0, alpha_bro ) } );
			}

			if ( aimplayer && !dormant )
			{
				if ( g_Variables.m_bools[ "esp_flags_exploit" ] && aimplayer->m_exploit )
				{
					flags.push_back( { XOR( "X" ), Color( 255, 0, 0, alpha_bro ) } );
				}

				if ( !aimplayer->m_records.empty() )
				{
					LagRecord* front = aimplayer->m_records.front().get();

					if ( g_Variables.m_bools[ "esp_flags_lagcomp" ] && front->m_broke_lc )
					{
						flags.push_back( { XOR( "LC" ), Color( 255, 0, 0, alpha_bro ) } );
					}

					if ( g_Variables.m_bools[ "esp_flags_resolver_confidence" ] )
					{
						switch ( aimplayer->m_resolver_confidence )
						{
						case AimPlayer::ResolverConfidence_t::VERY_LOW:
							flags.push_back( { XOR( "VLOW" ), Color( 255, 0, 0, alpha_bro ) } );
							break;
						case AimPlayer::ResolverConfidence_t::LOW:
							flags.push_back( { XOR( "LOW" ), Color( 255, 0, 0, alpha_bro ) } );
							break;
						case AimPlayer::ResolverConfidence_t::MEDIUM:
							flags.push_back( { XOR( "MED" ), Color( 255, 255, 255, alpha_bro ) } );
							break;
						case AimPlayer::ResolverConfidence_t::HIGH:
							flags.push_back( { XOR( "HIGH" ), Color( 255, 255, 255, alpha_bro ) });
							break;
						case AimPlayer::ResolverConfidence_t::VERY_HIGH:
							flags.push_back( { XOR( "VHIGH" ), Color( 155, 210, 100, alpha_bro ) } );
							break;
						case AimPlayer::ResolverConfidence_t::MAX:
							flags.push_back( { XOR( "FULL" ), Color( 155, 210, 100, alpha_bro ) } );
							break;
						}
					}
				}

				if ( g_Variables.m_bools[ "esp_flags_desync" ] && aimplayer->m_detected_desync )
				{
					float bro = abs( math::NormalizedAngle( aimplayer->m_max_feet_yaw - aimplayer->m_min_feet_yaw ) );
					//if ( bro >= 10.f )
					{
						flags.push_back( { XOR( "C" ), Color( 155, 210, 100, alpha_bro ).LerpRGB( Color( 255, 0, 0, alpha_bro ), bro / 58.f ) } );
					}
				}

				if ( g_Variables.m_bools[ "esp_flags_distortion" ] && aimplayer->m_resolver_confidence != AimPlayer::ResolverConfidence_t::MAX )
				{
					constexpr int min_979_updates = 3;

					if ( aimplayer->m_979_count >= min_979_updates )
					{
						flags.push_back( { XOR( "DISTORTION" ), Color( 255, 0, 0, alpha_bro ) } );
					}
					else if ( aimplayer->m_old_979_count >= min_979_updates )
					{
						flags.push_back( { XOR( "DISTORTION" ), Color( 255, 255, 255, alpha_bro ) } );
					}
				}
			}
		}

		// iterate flags.
		for ( size_t i{ }; i < flags.size(); ++i )
		{
			// get flag job (pair).
			auto& f = flags[ i ];

			int offset = i * render::esp_small.m_size.m_height;

			// draw flag.
			Color dormant_col = g_Variables.m_colors[ "esp_dormant" ];
			dormant_col.a() *= alpha_mod;

			render::esp_small.string( box.x + box.w + 2, box.y + offset, dormant && g_Variables.m_bools[ "esp_override_dormant" ] ? dormant_col : f.second, f.first );
		}
	}

	// draw bottom bars.
	int  offset{ 0 };

	// draw lby update bar.
	if ( enemy && g_Variables.m_bools[ "esp_lby_update" ] )
	{
		// make sure everything is valid.
		if ( aimplayer && !aimplayer->m_cant_predict_lby && aimplayer->m_records.size() )
		{
			// grab lag record.
			LagRecord* current = aimplayer->m_records.front().get();

			if ( current && current->m_velocity.length_2d() <= 0.1f && ( current->m_flags & FL_ONGROUND ) )
			{
				// calculate box width
				float cycle = std::clamp( 1.f - ( ( aimplayer->m_lby_timer - current->m_anim_time ) / 1.1f ), 0.f, 1.f );
				float width = box.w * cycle;

				if ( width >= 0.f )
				{
					Color clr = aimplayer->m_scary_lby ? g_Variables.m_colors[ "esp_lby_update_unsafe_color" ] : g_Variables.m_colors[ "esp_lby_update_color" ];

					if ( dormant && g_Variables.m_bools[ "esp_override_dormant" ] )
						clr = g_Variables.m_colors[ "esp_dormant" ];

					clr.a() *= alpha_mod;

					// draw.
					float bg_alpha = 255 * ( g_Variables.m_floats[ "esp_lby_update_bar_alpha" ] * 0.01f ) * alpha_mod;

					render::rect_filled( box.x - 1, box.y + box.h + 2, box.w + 2, 4, { 10, 10, 10, (int)bg_alpha } );

					render::rect( box.x, box.y + box.h + 3, width, 2, clr );

					// move down the offset to make room for the next bar.
					offset += 6;
				}
			}
		}
	}

	// draw weapon.
	if ( g_Variables.m_bools[ "esp_weapon" ] )
	{
		Weapon* weapon = player->GetActiveWeapon();
		if ( weapon )
		{
			WeaponInfo* data = weapon->GetWpnData();
			if ( data )
			{
				int bar;
				float scale;

				// the maxclip1 in the weaponinfo
				int max = data->m_max_clip1;
				int current = weapon->m_iClip1();

				C_AnimationLayer* layer1 = &player->m_AnimOverlay()[ 1 ];

				// set reload state.
				bool reload = ( layer1->m_weight != 0.f ) && ( player->GetSequenceActivity( layer1->m_sequence ) == 967 );

				// ammo bar.
				if ( max != -1 && g_Variables.m_bools[ "esp_weapon_ammo" ] )
				{
					// check for reload.
					if ( reload )
						scale = layer1->m_cycle;
					// not reloading.
					// make the division of 2 ints produce a float instead of another int.
					else
						scale = static_cast<float>( current ) / max;

					// relative to bar.
					bar = (int)std::round( ( box.w - 2 ) * scale );

					Color clr = g_Variables.m_colors[ "esp_weapon_ammo_start_color" ];
					clr = g_Variables.m_colors[ "esp_weapon_ammo_end_color" ].LerpRGB( clr, scale );
					//clr.brighten();

					if ( dormant && g_Variables.m_bools[ "esp_override_dormant" ] )
						clr = Color( 130, 130, 130 );

					clr.a() *= alpha_mod;

					// draw.
					float bg_alpha = 255 * (g_Variables.m_floats[ "esp_weapon_ammo_bar_alpha" ] * 0.01f) * alpha_mod;
					render::rect_filled( box.x - 1, box.y + box.h + 2 + offset, box.w + 2, 4, { 10, 10, 10, (int)bg_alpha } );

					render::rect( box.x, box.y + box.h + 3 + offset, bar + 2, 2, clr );

					// less then a 5th of the bullets left.
					if ( g_Variables.m_bools[ "esp_weapon_ammo_text" ] && current <= (int)std::round( max / 5 ) && !reload )
						if ( dormant ) // no dormant override on bar strings
							render::esp_small.string( box.x + bar, box.y + box.h + offset, { 130, 130, 130, alpha_bro }, std::to_string( current ), render::ALIGN_CENTER );
						else
							render::esp_small.string( box.x + bar, box.y + box.h + offset, { 255, 255, 255, alpha_bro }, std::to_string( current ), render::ALIGN_CENTER );

					offset += 6;
				}

				// text.
				if ( g_Variables.m_bools[ "esp_weapon_text" ] )
				{
					// construct std::string instance of localized weapon name.
					std::string name{ weapon->GetLocalizedName() };

					std::transform( name.begin(), name.end(), name.begin(), ::toupper );

					if ( dormant && g_Variables.m_bools[ "esp_override_dormant" ] )
					{
						Color col = g_Variables.m_colors[ "esp_dormant" ];
						col.a() *= alpha_mod;
						render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset, col, name, render::ALIGN_CENTER );
					}
					else
					{
						Color bro_got_bands = g_Variables.m_colors[ "esp_weapon_text_color" ];
						bro_got_bands.a() *= alpha_mod;

						render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset, bro_got_bands, name, render::ALIGN_CENTER );
					}

					// MOVE DOWN FOR THE NEXT ICONS OKAY :D
					offset += render::esp_small.m_size.m_height - 1;
				}

				// icons.
				if ( g_Variables.m_bools[ "esp_weapon_icon" ] )
				{
					std::string icon = GetWeaponIcon( weapon->m_iItemDefinitionIndex() );

					// icons are a lil big...
					offset += 2;

					if ( !icon.empty() )
					{
						if ( dormant && g_Variables.m_bools[ "esp_override_dormant" ] )
						{
							Color ohioThamp = g_Variables.m_colors[ "esp_dormant" ];
							ohioThamp.a() *= alpha_mod;
							render::weapon_icons.string( box.x + box.w / 2, box.y + box.h + offset, ohioThamp, icon, render::ALIGN_CENTER );
						}
						else
						{
							Color bro_got_bands = weapon->m_iItemDefinitionIndex() == ZEUS ? g_Variables.m_colors[ "esp_weapon_zeus_color" ] : g_Variables.m_colors[ "esp_weapon_icon_color" ];
							bro_got_bands.a() *= alpha_mod;

							render::weapon_icons.string( box.x + box.w / 2, box.y + box.h + offset, bro_got_bands, icon, render::ALIGN_CENTER );
						}
					}
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4()
{
	bool        mode_2d, mode_3d, is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = []( float damage, int armor_value )
		{
			float new_damage, armor;

			if ( armor_value > 0 )
			{
				new_damage = damage * 0.5f;
				armor = ( damage - new_damage ) * 0.5f;

				if ( armor > (float)armor_value )
				{
					armor = (float)armor_value * 2.f;
					new_damage = damage - armor;
				}

				damage = new_damage;
			}

			return std::max( 0, (int)std::floor( damage ) );
		};

	mode_2d = g_Variables.m_bools[ "planted_bomb_screen" ];
	mode_3d = g_Variables.m_bools[ "planted_bomb_world" ];

	if ( !mode_2d && !mode_3d )
		return;

	if ( !m_c4_planted || !g_Variables.m_bools[ "planted_bomb" ] )
		return;

	// calculate bomb damage.
	// references:
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//     https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_ctx.m_local->WorldSpaceCenter();
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length();

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp( ( dist * dist ) / ( ( m_planted_c4_radius_scaled * -2.f ) * m_planted_c4_radius_scaled ) );

		// now finally, scale the damage based on our armor (if we have any).
		final_damage = scale_damage( range_damage, g_ctx.m_local->m_ArmorValue() );
	}

	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	time_str = tfm::format( XOR( "%.2f" ), explode_time_diff );
	damage_str = tfm::format( XOR( "%i" ), final_damage );

	// get damage color.
	damage_color = ( final_damage < g_ctx.m_local->m_iHealth() ) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen( m_planted_c4_explosion_origin, screen_pos );

	std::string bomb = m_last_bombsite.c_str();

	// 'on screen (2D)'.
	if ( mode_2d )
	{
		std::string bomb = m_last_bombsite.c_str();
		std::string timer = tfm::format( XOR( "%s - %.1fs" ), bomb.substr( 0, 1 ), explode_time_diff );
		std::string dmg = tfm::format( XOR( "%i" ), final_damage );

		Color colortimer = { 135, 172, 10, 255 };
		if ( explode_time_diff < 10 ) colortimer = { 200, 200, 110, 255 };
		if ( explode_time_diff < 5 ) colortimer = { 192, 32, 17, 255 };

		if ( m_c4_planted )
		{
			if ( explode_time_diff > 0.f )
			{
				render::indicator.string( 6, 11, { 0, 0, 0, 125 }, timer );
				Color display = colortimer.LerpRGB( 0xff15c27b, std::clamp( explode_time_diff / 20.f, 0.f, 1.f ) );
				display.brighten();
				render::indicator.string( 5, 10, display, timer );
			}
			if ( g_ctx.m_local->m_iHealth() <= final_damage )
			{
				render::indicator.string( 6, 31, { 0, 0, 0, 125 }, tfm::format( XOR( "FATAL" ) ) );
				render::indicator.string( 5, 30, 0xff0000ff, tfm::format( XOR( "FATAL" ) ) );
			}
			else if ( final_damage > 1 )
			{
				render::indicator.string( 5, 31, { 0, 0, 0, 125 }, tfm::format( XOR( "- %iHP" ), dmg ) );
				render::indicator.string( 6, 30, Color( 0xff0000ff ).LerpRGB( 0xff15c27b, std::clamp( ( g_ctx.m_local->m_iHealth() - final_damage ) * 0.01f, 0.f, 1.f ) ), tfm::format( XOR( "- %iHP" ), dmg ) );
			}
		}
	}

	// 'on bomb (3D)'.
	if ( mode_3d && is_visible )
	{
		if ( explode_time_diff > 0.f )
			render::esp_small.string( screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER );

		// only render damage string if we're alive.
		if ( g_ctx.m_local->alive() )
			render::esp_small.string( screen_pos.x, (int)screen_pos.y + render::esp_small.m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER );
	}
}

void Visuals::DrawHitmarkers()
{
	auto DrawHitmarker = []( int x, int y, int alpha, int length, bool antialiased, Color col ) -> void
		{
			col.a() = std::min(col.a(), (uint8_t)alpha);

			constexpr int thickness = 1;

			if (antialiased) {
				// length, spacing, color
				render::line(vec2_t(x - length, y - length), vec2_t(x - 4, y - 4), col);
				render::line(vec2_t(x - length, y + length), vec2_t(x - 4, y + 4), col);
				render::line(vec2_t(x + length, y - length), vec2_t(x + 4, y - 4), col);
				render::line(vec2_t(x + length, y + length), vec2_t(x + 4, y + 4), col);
			}
			else {

				for (int i = 0; i < length; i++)
				{
					int offset = 5 + i;

					// (\)
					render::rect_filled(x + offset, y + offset, thickness, thickness, col);

					// (/)
					render::rect_filled(x - offset, y - offset, thickness, thickness, col);

					// (/)
					render::rect_filled(x - offset, y + offset, thickness, thickness, col);

					// (\)
					render::rect_filled(x + offset, y - offset, thickness, thickness, col);
				}
			}
		};

	constexpr float hit_duration = 1.f;

	if ( g_csgo.m_globals->m_curtime <= m_hit_start + hit_duration && g_Variables.m_bools[ "hitmarker_screen" ] )
	{
		float elapsed = g_csgo.m_globals->m_curtime - m_hit_start;

		int alpha = 255.f * ( 1.f - elapsed );

		Color col = g_Variables.m_colors["hitmarker_screen_default"];
		switch (m_hit_type) {
		case HitType_t::LETHAL:
			col = g_Variables.m_colors["hitmarker_screen_lethal"];
			break;
		case HitType_t::HEADSHOT:
			col = g_Variables.m_colors["hitmarker_screen_headshot"];
			break;
		default:
			break;
		}

		int x = g_ctx.m_width / 2;
		int y = g_ctx.m_height / 2;

		DrawHitmarker( x, y, alpha, g_Variables.m_ints["hitmarker_screen_length"], g_Variables.m_bools["hitmarker_antialias_screen"], col );
	}

	bool world_enabled = g_Variables.m_bools[ "hitmarker_world" ];
	bool damage_enabled = g_Variables.m_bools[ "hitmarker_damage" ];

	if ( g_shots.m_post_hitmarker_impacts.size() && ( world_enabled || damage_enabled ) )
	{
		for ( int i = 0; i < g_shots.m_post_hitmarker_impacts.size(); i++ )
		{
			HitmarkerImpacts* current = &g_shots.m_post_hitmarker_impacts[ i ];
			if ( !current )
				continue;

			if ( g_csgo.m_globals->m_curtime > current->m_time + hit_duration )
			{
				g_shots.m_post_hitmarker_impacts.erase( g_shots.m_post_hitmarker_impacts.begin() + i );
				continue;
			}

			vec2_t pos2D;
			if ( !render::WorldToScreen( current->m_pos, pos2D ) )
				continue;

			float elapsed = current->m_time + hit_duration - g_csgo.m_globals->m_curtime;

			if ( world_enabled )
			{
				Color col = g_Variables.m_colors["hitmarker_world_default"];
				if (current->m_headshot)
					col = g_Variables.m_colors["hitmarker_world_headshot"];
				else if (current->m_lethal)
					col = g_Variables.m_colors["hitmarker_world_lethal"];

				DrawHitmarker( pos2D.x, pos2D.y, 255.f * elapsed, g_Variables.m_ints["hitmarker_world_length"], g_Variables.m_bools["hitmarker_antialias_world"], col);
			}

			if ( damage_enabled )
			{
				Color col = g_Variables.m_colors["hitmarker_damage_default"];
				if (current->m_headshot)
					col = g_Variables.m_colors["hitmarker_damage_headshot"];
				else if (current->m_lethal)
					col = g_Variables.m_colors["hitmarker_damage_lethal"];

				col.a() *= elapsed;

				render::world_damage.string( pos2D.x, pos2D.y, col, std::to_string( int( current->m_damage * std::min( 1.f, ( elapsed / hit_duration ) * 2 ) ) ), render::ALIGN_CENTER );
			}
		}
	}
}

bool Visuals::GetPlayerBoxRect( Player* player, Rect& box )
{
	if (g_Variables.m_bools["esp_wide_boxes"]) {
		float x1 = g_ctx.m_width * 2;
		float x2 = -g_ctx.m_width;
		float y1 = g_ctx.m_height * 2;
		float y2 = -g_ctx.m_width;

		vec3_t pos = origin_cache[ player->index() - 1];
		vec3_t mins = player->m_vecMins();
		vec3_t maxs = player->m_vecMaxs();
		std::vector<vec3_t> corners = {
			vec3_t( mins.x, mins.y, mins.z ),
			vec3_t( mins.x, maxs.y, mins.z ),
			vec3_t( maxs.x, maxs.y, mins.z ),
			vec3_t( maxs.x, mins.y, mins.z ),
			vec3_t( mins.x, mins.y, maxs.z ),
			vec3_t( mins.x, maxs.y, maxs.z ),
			vec3_t( maxs.x, maxs.y, maxs.z ),
			vec3_t( maxs.x, mins.y, maxs.z )
		};

		for ( int i = 0; i < corners.size(); i++ )
		{
			vec3_t currCorner = pos + corners[ i ];
			vec2_t screen;
			if ( !render::WorldToScreen( currCorner, screen ) )
				break;

			x1 = std::min( x1, screen.x );
			x2 = std::max( x2, screen.x );
			y1 = std::min( y1, screen.y );
			y2 = std::max( y2, screen.y );
		}

		int diff1 = std::abs( x2 - x1 );
		int diff2 = std::abs( y2 - y1 );

		box.h = diff2;
		box.w = diff1;
		box.x = x1;
		box.y = y1;
	}
	else {
		vec3_t origin, mins, maxs;
		vec2_t bottom, top;

		// get interpolated origin.
		origin = origin_cache[player->index() - 1];

		// get hitbox bounds.
		player->ComputeHitboxSurroundingBox(&mins, &maxs);

		// correct x and y coordinates.
		mins = { origin.x, origin.y, mins.z };
		maxs = { origin.x, origin.y, maxs.z + 8.f };

		if (!render::WorldToScreen(mins, bottom) || !render::WorldToScreen(maxs, top))
			return false;

		box.h = bottom.y - top.y;
		box.w = box.h / 2.f;
		box.x = bottom.x - (box.w / 2.f);
		box.y = bottom.y - box.h;
	}

	return true;
}

void Visuals::DrawSkeleton( Player* player, Color color, int opacity )
{
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel();
	if ( !model )
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	if ( player->dormant() )
		return;

	BoneArray* matrix = player->m_BoneCache().m_pCachedBones;

	color.a() *= opacity;

	for ( int i{ }; i < hdr->m_num_bones; ++i )
	{
		// get bone.
		bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		matrix->get_bone( bone_pos, i );
		matrix->get_bone( parent_pos, parent );

		// world to screen both the bone parent bone then draw.
		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, color );
	}
}

void Visuals::RenderGlow()
{
	Color   color;
	int style = 0;

	if ( !g_ctx.m_local )
		return;

	if ( !g_csgo.m_glow->m_object_definitions.Count() )
		return;

	static bool ran = false; 
	if (!ran) {
		IMaterial* pMatTeamIdShape = g_csgo.m_material_system->FindMaterial(XOR("dev/glow_edge_highlight"), TEXTURE_GROUP_OTHER);
		IMaterial* pmat2 = g_csgo.m_material_system->FindMaterial(XOR("dev/glow_color"), TEXTURE_GROUP_OTHER);
		IMaterial* pmat_weirdhealthboxthing = g_csgo.m_material_system->FindMaterial(XOR("dev/glow_health_color"), TEXTURE_GROUP_OTHER); // box that appears on players being hurt

		pMatTeamIdShape->SetFlag(MATERIAL_VAR_IGNOREZ, true);
		pmat2->SetFlag(MATERIAL_VAR_IGNOREZ, true);
		pmat_weirdhealthboxthing->SetFlag(MATERIAL_VAR_NO_DRAW, true);
		ran = true;
	}

	for ( int i{ }; i < g_csgo.m_glow->m_object_definitions.Count(); ++i )
	{
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[ i ];
		// skip non-players.
		if ( obj->unused() || !obj->m_pEntity )
			continue;

		auto class_id = obj->m_pEntity->GetClientClass()->m_ClassID;

		if (class_id == _CCSPlayer) 
		{
			// get player ptr.
			Player* player = obj->m_pEntity->as< Player* >();

			// get reference to array variable.
			float& opacity = m_opacities[player->index() - 1];

			bool enemy = player->visual_enemy(g_ctx.m_local);

			if (player->m_bIsLocalPlayer() && !g_Variables.m_bools["glow_local"])
				continue;

			if (enemy && !g_Variables.m_bools["glow_enemy"])
				continue;

			if (!enemy && !g_Variables.m_bools["glow_friendly"])
				continue;

			// enemy color
			if (enemy) {
				color = g_Variables.m_colors["glow_enemy_color"];
				style = g_Variables.m_ints["glow_enemy_style"];
			}
			else if (player->m_bIsLocalPlayer()) {
				color = g_Variables.m_colors["glow_local_color"];
				style = g_Variables.m_ints["glow_local_style"];
			}
			// friendly color
			else {
				color = g_Variables.m_colors["glow_friendly_color"];
				style = g_Variables.m_ints["glow_friendly_style"];
			}

			obj->m_nRenderStyle = style < 1 ? 0 : style - 1;
			obj->m_bRenderWhenOccluded = true;
			obj->m_bRenderWhenUnoccluded = false;
			obj->m_bFullBloomRender = style == 0;
			obj->m_vGlowColor = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
			obj->m_flGlowAlpha = (opacity * color.a()) / 255.f;
		}
		else if (obj->m_pEntity->IsBaseCombatWeapon()) 
		{
			float delta = (g_ctx.m_local->GetAbsOrigin() - obj->m_pEntity->GetAbsOrigin()).length() * 0.001f;

			if (delta > 1.f)
				continue; // lets not even bother 

			if (g_Variables.m_bools["glow_weapons"]) 
			{
				color = g_Variables.m_colors["glow_weapons_color"];
				color.a() = math::Lerp(std::clamp(delta, 0.f, 1.f), (float)color.a(), 0.f);

				style = g_Variables.m_ints["glow_weapons_style"];
				obj->m_bFullBloomRender = style == 0;
				obj->m_nRenderStyle = style < 1 ? 0 : style - 1;
				obj->m_vGlowColor = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
				obj->m_flGlowAlpha = color.a() / 255.f;
				obj->m_bRenderWhenOccluded = true;
			}
			else {
				obj->m_bRenderWhenOccluded = false;
				obj->m_flGlowAlpha = 0.f;
			}
		}

	}
}

void Visuals::DrawHitboxMatrix( Player* player, BoneArray* bones, Color col, float time )
{
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = player->GetModel();
	if ( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	set = hdr->GetHitboxSet( player->m_nHitboxSet() );
	if ( !set )
		return;

	for ( int i{ }; i < set->m_hitboxes; ++i )
	{
		bbox = set->GetHitbox( i );
		if ( !bbox )
			continue;

		// bbox.
		if ( bbox->m_radius <= 0.f )
		{
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms( bones[ bbox->m_bone ], rot_matrix, matrix );

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles( matrix, bbox_angle );

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin();

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r(), col.g(), col.b(), col.a(), time );
		}

		// capsule.
		else
		{
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix( bbox->m_angle, matrix );

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms( bones[ bbox->m_bone ], matrix, matrix );

			// get world positions from new matrix.
			math::VectorTransform( bbox->m_mins, matrix, mins );
			math::VectorTransform( bbox->m_maxs, matrix, maxs );

			g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r(), col.g(), col.b(), col.a(), time, 0, 1 );
		}
	}
}

void Visuals::DrawBeams()
{
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	auto vis_impacts = &g_shots.m_vis_impacts;
	impact_count = vis_impacts->size();

	if ( !impact_count )
		return;

	curtime = g_csgo.m_globals->m_curtime;

	for ( size_t i{ impact_count }; i-- > 0; )
	{
		auto impact = &vis_impacts->operator[ ]( i );
		if ( !impact )
			continue;

		// impact is too old, erase it.
		if ( std::abs( curtime - impact->m_tickbase ) > g_Variables.m_floats[ "beam_lifetime" ] )
		{
			vis_impacts->erase( vis_impacts->begin() + i );
			continue;
		}

		// already rendering this impact, skip over it.
		if ( impact->m_ignore )
			continue;

		// is this the final impact?
		// last impact in the vector, it's the final impact.
		if ( i == ( impact_count - 1 ) )
			is_final_impact = true;

		// the current impact's tickbase is different than the next, it's the final impact.
		else if ( ( i + 1 ) < impact_count && impact->m_tickbase != vis_impacts->operator[ ]( i + 1 ).m_tickbase )
			is_final_impact = true;

		else
			is_final_impact = false;

		// is this the final impact?
		// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

		if ( is_final_impact )
		{
			// calculate start and end position for beam.
			start = impact->m_shoot_pos;

			dir = ( impact->m_impact_pos - start ).normalized();
			dist = ( impact->m_impact_pos - start ).length();

			end = start + ( dir * dist );

			// setup beam info.
			// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
			const char* model = "";
			switch ( g_Variables.m_ints[ "beam_type" ] )
			{
			case 0:
				model = XOR( "sprites/purplelaser1.vmt" );
				break;
			case 1:
				model = XOR( "sprites/physbeam.vmt" );
				break;
			case 2:
				model = XOR( "sprites/white.vmt" );
				break;
			default:
				break;
			}

			beam_info.m_vecStart = start;
			beam_info.m_vecEnd = end;
			beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( model );
			beam_info.m_pszModelName = model;
			beam_info.m_flHaloScale = 0.f;
			beam_info.m_flLife = g_Variables.m_floats[ "beam_lifetime" ]; 
			beam_info.m_flWidth = g_Variables.m_floats[ "beam_thickness" ];
			beam_info.m_flEndWidth = g_Variables.m_floats[ "beam_thickness" ];
			beam_info.m_flFadeLength = 1.f;
			beam_info.m_flAmplitude = 0.f;   // beam 'jitter'.
			beam_info.m_flBrightness = impact->m_color.a();
			beam_info.m_flSpeed = 0.5f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
			beam_info.m_nStartFrame = 0;
			beam_info.m_flFrameRate = 0.f;
			beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
			beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
			beam_info.m_nFlags = 0;//FBEAM_FADEIN | FBEAM_FADEOUT; // fadeout makes it look like bnunguzz

			beam_info.m_flRed = impact->m_color.r();
			beam_info.m_flGreen = impact->m_color.g();
			beam_info.m_flBlue = impact->m_color.b();

			beam = game::CreateGenericBeam( beam_info );
			if ( beam )
			{
				g_csgo.m_beams->DrawBeam( beam );
				impact->m_ignore = true;
			}
		}
	}
}

void Visuals::DebugAimbotPoints( Player* player )
{
	AimPlayer* data = &g_aimbot.m_players[ player->index() - 1 ];
	if ( !data || data->m_records.empty() )
		return;
	
	LagRecord* front = data->m_records.front().get();
	if ( !front )
		return;

	std::vector<AimPoint_t> BroPoints;

	if ( !data->SetupHitboxPoints( front, BroPoints, 0.f ) )
		return;
	
	for ( auto& p : BroPoints )
	{
		vec2_t screen;
	
		if ( render::WorldToScreen( p.m_pos, screen ) )
			render::rect_filled( screen.x, screen.y, 2, 2, p.m_edge_point ? Color{ 255, 0, 0, 255 } : Color{ 0, 0, 255, 255 } );
	}
}

void Visuals::ServersideEspBySpectatorsLolThanksNaphack()
{
	if ( !g_ctx.m_local )
		return;

	for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; ++i )
	{
		auto player = g_csgo.m_entlist->GetClientEntity<Player*>( i );
		if ( !player || player->alive() || player->dormant() )
			continue;

		//if ( g_ExtendedVisuals.m_arrOverridePlayers.at( i ).m_eOverrideType == EOverrideType::ESP_SHARED )
		//	continue;

		// this player is spectating someone
		if ( player->m_hObserverTarget().IsValid() )
		{
			// grab the player who is being spectated (pSpectatee) by the spectator (player)
			const auto pSpectatee = (Player*)player->m_hObserverTarget().Get();
			if ( pSpectatee && pSpectatee->index() <= 64 )
			{
				// if the guy that's being spectated is dormant, set his origin
				// to the spectators origin (and ofc set dormancy state for esp to draw him)
				if ( pSpectatee->dormant() )
				{
					if ( player->m_iObserverMode() == 4 || player->m_iObserverMode() == 5 )
					{
						//m_arrOverridePlayers.at( pSpectatee->EntIndex() ).m_eOverrideType = EOverrideType::ESP_SPECTATOR;
						origin_cache[ pSpectatee->index() - 1 ] = player->m_vecOrigin();
						m_time_cache[ pSpectatee->index() - 1 ] = g_csgo.m_globals->m_realtime;
						player->m_bSpotted() = true;
					}
				}
			}
		}
	}
}

void Visuals::RaceChanger()
{ // Only the most useful of a feature
	if ( g_csgo.r_skin->GetInt() != g_Variables.m_ints[ "race_changer" ] )
		g_csgo.r_skin->SetValue( g_Variables.m_ints[ "race_changer" ] );
}