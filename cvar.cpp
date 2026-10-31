#include "includes.h"

int Hooks::DebugSpreadGetInt()
{
	if ( g_Variables.m_bools[ "force_crosshair" ] && g_ctx.m_processing && !g_ctx.m_local->m_bIsScoped() || (g_Variables.m_ints[ "penetration_crosshair_style" ] == 4 && g_Variables.m_ints[ "skip_scope_lines_pen" ] != 1 ) )
		return 3;

	return g_hooks.m_debug_spread.GetOldMethod< GetInt_t >( ConVar::GETINT )( this );
}