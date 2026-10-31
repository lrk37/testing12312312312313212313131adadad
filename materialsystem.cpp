#include "includes.h"

bool Hooks::OverrideConfig( MaterialSystem_Config_t* config, bool update )
{
	if ( g_Variables.m_bools[ "world_fullbright" ] )
		config->m_nFullbright = true;

	return g_hooks.m_material_system.GetOldMethod< OverrideConfig_t >( IMaterialSystem::OVERRIDECONFIG )( this, config, update );
}