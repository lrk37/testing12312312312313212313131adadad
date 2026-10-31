#include "includes.h"

void Hooks::Shutdown()
{
	g_threading.shutdown();

	// i think this is needed to pause the main thread so we can make sure others exit in time.
	// we (ideally) wont have queued tasks here, though.
	g_threading.join_all();

	g_hooks.m_client.GetOldMethod<void( __thiscall* )( void* )>( CHLClient::SHUTDOWN )( this );
}

void Hooks::LevelInitPreEntity( const char* map )
{
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	g_csgo.cl_updaterate->SetValue( rate );
	g_csgo.cl_cmdrate->SetValue( rate );

	g_aimbot.reset();
	g_visuals.m_hit_start = 0.f;
	g_shots.m_pre_hitmarker_impacts.clear();
	g_shots.m_post_hitmarker_impacts.clear();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >( CHLClient::LEVELINITPREENTITY )( this, map );
}

void Hooks::LevelInitPostEntity()
{
	g_ctx.OnMapload();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >( CHLClient::LEVELINITPOSTENTITY )( this );
}

void Hooks::LevelShutdown()
{
	g_aimbot.reset();

	g_ctx.m_local = nullptr;
	g_ctx.m_weapon = nullptr;
	g_ctx.m_processing = false;
	g_ctx.m_weapon_info = nullptr;
	g_ctx.m_round_end = false;

	g_ctx.m_sequences.clear();

	g_skins.OnLevelShutdown();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >( CHLClient::LEVELSHUTDOWN )( this );
}

/*int Hooks::IN_KeyEvent( int evt, int key, const char* bind ) {
	// see if this key event was fired for the drop bind.
	/*if( bind && FNV1a::get( bind ) == HASH( "drop" ) ) {
		// down.
		if( evt ) {
			g_ctx.m_drop = true;
			g_ctx.m_drop_query = 2;
			g_ctx.print( "drop\n" );
		}

		// up.
		else
			g_ctx.m_drop = false;

		// ignore the event.
		return 0;
	}

	return g_hooks.m_client.GetOldMethod< IN_KeyEvent_t >( CHLClient::INKEYEVENT )( this, evt, key, bind );
}*/

void DrawServerHitboxes(int player_index) {
	if (g_csgo.m_cl->m_net_channel && g_csgo.m_engine->IsInGame() && g_csgo.m_cl->m_net_channel->IsLoopback()) {

		// server hitboxes.
		float fDuration = -1.f;

		static auto DrawServerHitboxes = pattern::find(g_csgo.m_server_dll, XOR("E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 84 ? ? ? ? 33 FF 39 BB")).rel32(0x1).as < void* >();
		static auto UTIL_PlayerByIndex = pattern::find(g_csgo.m_server_dll, XOR("85 C9 7E 2A A1")).as < void* (__fastcall*) (int) >();

		//for (int i = 1; i <= 32; ++i) {
			//auto e = g_csgo.m_entlist->GetClientEntity(i);
			//
			//if (!e)
			//	continue;

			auto ent = (Player*)UTIL_PlayerByIndex(player_index);

			if (!ent)
				return;

			__asm {
				pushad
				movss xmm1, fDuration
				push 1 //bool monoColor
				mov ecx, ent
				call DrawServerHitboxes
				popad
			}
		//}
	}
}

void Hooks::FrameStageNotify( Stage_t stage )
{
	if ( stage != FRAME_START )
		g_ctx.m_stage = stage;

	g_ctx.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer() );

	if ( stage == FRAME_RENDER_START ) // only one, better to not use a switch
	{
		//DrawServerHitboxes(g_csgo.m_engine->GetLocalPlayer());

		g_visuals.DrawBeams();
		g_visuals.ModulateWorld();
	}

	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >( CHLClient::FRAMESTAGENOTIFY )( this, stage );

	switch ( stage )
	{
	case FRAME_RENDER_START:
		g_ctx.SmoothAnimations();

		// events are fired in OG frame_render_start.
		// so call here after calling OG.
		// we also manually fire events in ProcessPacket but here should still work.
		g_shots.Process();

		if ( g_Variables.m_bools[ "draw_impacts" ] && g_ctx.m_processing )
		{
			static int last_count = 0;
			auto& client_impact_list = *( CUtlVector< client_hit_verify_t >* )( (uintptr_t)g_ctx.m_local + 0xBA84 );
		
			int count = client_impact_list.Count();
		
			if ( count > 0 )
			{
				Color color = g_Variables.m_colors[ "client_impact_color" ];
		
				for ( auto i = count; i > last_count; i-- )
				{
					float size = g_Variables.m_floats[ "impacts_size" ] / 100.f;
					
				}
			}
		
			if ( count != last_count )
				last_count = count;
		}
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		g_ctx.Skybox();
		g_ctx.ClanTag();

		if ( g_csgo.sv_cheats->GetInt() != 1 )
			g_csgo.sv_cheats->SetValue( 1 );

		//if ( g_csgo.r_drawallrenderables->GetInt() != 1 )
		//	g_csgo.r_drawallrenderables->SetValue( 1 ); // chams

		g_visuals.RaceChanger();

		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		g_visuals.NoSmoke();
		g_visuals.ServersideEspBySpectatorsLolThanksNaphack();
		break;
	//case FRAME_NET_UPDATE_END:
	//{
	//	auto* l3 = &g_ctx.m_local->m_AnimOverlay()[ 3 ];
	//	g_ctx.print( tfm::format( "weight %s | cycle %s\n", l3->m_weight, l3->m_cycle ) );
	//}
	//	break;
	default:
		break;
	}
}