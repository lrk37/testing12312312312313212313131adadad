#include "includes.h"

bool Hooks::ShouldDrawParticles()
{
	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawParticles_t >( IClientMode::SHOULDDRAWPARTICLES )( this );
}

bool Hooks::ShouldDrawFog()
{
	// remove fog.
	if ( g_Variables.m_bools[ "remove_fog" ] )
		return false;

	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawFog_t >( IClientMode::SHOULDDRAWFOG )( this );
}

void Hooks::OverrideView(CViewSetup* view)
{
	// damn son.
	g_ctx.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	if ( g_Variables.m_bools[ "fov_changer" ] )
	{
		float remaining = g_Variables.m_floats[ "view_fov" ] - view->m_fov;
		if ( g_ctx.m_processing )
		{
			if ( g_ctx.m_weapon && g_ctx.m_local->m_bIsScoped() )
			{
				remaining *= ( ( 100.f - g_Variables.m_floats[ "fov_scope_percent" ] ) * 0.01f );
			}
		}
		
		view->m_fov += remaining;
	}

	// g_grenades.think( );
	g_visuals.ThirdpersonThink();

	// call original.
	g_hooks.m_client_mode.GetOldMethod< OverrideView_t >(IClientMode::OVERRIDEVIEW)(this, view);

	if (g_ctx.m_local) {
		static auto DisablePostProcess = *pattern::find(g_csgo.m_client_dll, XOR("80 3D ? ? ? ? ? 53 56 57 0F 85")).add(0x2).as<bool**>();
		if (g_Variables.m_bools["remove_post_processing"]) {
			if (*DisablePostProcess == false)
				*DisablePostProcess = true;
		}
		else {
			if (*DisablePostProcess == true)
				*DisablePostProcess = false;
		}
	}
}

float Hooks::GetViewmodelFOV()
{
	return g_Variables.m_floats[ "viewmodel_fov" ];
}

bool Hooks::CreateMove( float time, CUserCmd* cmd )
{
	// called from CInput::ExtraMouseSample -> return original.
	if ( !cmd || !cmd->m_command_number )
		return g_hooks.m_client_mode.GetOldMethod< CreateMove_t >( IClientMode::CREATEMOVE )( this, time, cmd );

	// if we arrived here, called from -> CInput::CreateMove
	// call EngineClient::SetViewAngles according to what the original returns.
	if ( g_hooks.m_client_mode.GetOldMethod< CreateMove_t >( IClientMode::CREATEMOVE )( this, time, cmd ) )
		g_csgo.m_engine->SetViewAngles( cmd->m_view_angles );

	// random_seed isn't generated in ClientMode::CreateMove yet, we must set generate it ourselves.
	cmd->m_random_seed = g_csgo.MD5_PseudoRandom( cmd->m_command_number ) & 0x7fffffff;

	// get bSendPacket off the stack.
	const auto ebp = *reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress()) - sizeof(std::uintptr_t*));
	g_ctx.m_packet = reinterpret_cast<bool*>(ebp - 0x1c);
	g_ctx.m_final_packet = reinterpret_cast<bool*>(ebp - 0x1b); // the .local function breaks in debug for some reason. idc.

	// invoke move function.

	g_ctx.OnTick( cmd );

	return false;
}

bool Hooks::DoPostScreenSpaceEffects( CViewSetup* setup )
{
	g_visuals.RenderGlow();
	return g_hooks.m_client_mode.GetOldMethod< DoPostScreenSpaceEffects_t >( IClientMode::DOPOSTSPACESCREENEFFECTS )( this, setup );
}