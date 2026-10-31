#include "includes.h"

NetData g_netdata{};;

void NetData::StoreAnims( int cmd_num, bool store_state )
{
	if ( !g_ctx.m_processing )
		return;

	PredictionData_t* data = &m_prediction_data[ cmd_num % MULTIPLAYER_BACKUP ];
	data->m_cmd_num = cmd_num;

	g_ctx.m_local->GetAnimLayers( data->m_layers );

	if ( store_state )
	{
		CCSGOPlayerAnimState* state = g_ctx.m_local->m_PlayerAnimState();
		if ( state )
		{
			data->m_state = *state;
		}
		data->m_fake_feet_yaw = g_server_animations.m_fake_feet_yaw;
	}

	data->m_timebase = g_csgo.m_globals->m_curtime;
	data->m_shoot_pos = g_ctx.m_local->GetShootPosition();
	data->m_inaccuracy = 1.f;

	Weapon* weapon = g_ctx.m_local->GetActiveWeapon();
	if ( !weapon || weapon != g_ctx.m_weapon )
		return;

	if ( cmd_num == g_csgo.m_cl->m_last_command_ack && g_ctx.m_need_revolver_timing )
	{
		g_ctx.m_revolver_time = weapon->m_flPostponeFireReadyTime();
	}

	data->m_postpone_fire_ready_time = g_ctx.m_revolver_time;

	if ( !g_ctx.CanFireWeapon() )
		return;         

	data->m_inaccuracy = weapon->GetInaccuracy();
}

void NetData::RestoreAnims( int cmd_num )
{
	if ( !g_ctx.m_processing )
		return;

	PredictionData_t* data = &m_prediction_data[ cmd_num % MULTIPLAYER_BACKUP ];
	if ( data->m_cmd_num != cmd_num )
	{
		return;
	}

	for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ )
	{
		if ( i != ANIMATION_LAYER_WHOLE_BODY && i != ANIMATION_LAYER_FLASHED && i != ANIMATION_LAYER_FLINCH )
		{
			memcpy( &g_ctx.m_local->m_AnimOverlay()[ i ], &data->m_layers[ i ], sizeof( C_AnimationLayer ) );
		}
	}

	CCSGOPlayerAnimState* state = g_ctx.m_local->m_PlayerAnimState();
	// frame check should work fine.
	if ( state && data->m_state.m_last_update_frame > 0.f )
	{
		*state = data->m_state;
	}
	g_server_animations.m_fake_feet_yaw = data->m_fake_feet_yaw;

	g_ctx.m_revolver_time = data->m_postpone_fire_ready_time;
}
 
void NetData::MaintainClientLayers( C_AnimationLayer* layers )
{
	// client does shit on this layer (since normally the client wouldnt play it), but we rebuild it, so dont.
	memcpy( &g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ], &layers[ ANIMATION_LAYER_ADJUST ], sizeof( C_AnimationLayer ) );
}

float& NetData::GetTimebase( int cmd_num )
{
	return m_prediction_data[ cmd_num % MULTIPLAYER_BACKUP ].m_timebase;
}

vec3_t& NetData::GetShootPos( int cmd_num )
{
	return m_prediction_data[ cmd_num % MULTIPLAYER_BACKUP ].m_shoot_pos;
}

float& NetData::GetInaccuracy( int cmd_num )
{
	return m_prediction_data[ cmd_num % MULTIPLAYER_BACKUP ].m_inaccuracy;
}

void NetData::StoreViewmodel( int cmd_num )
{
	auto viewmodel = g_csgo.m_entlist->GetClientEntityFromHandle( g_ctx.m_local->m_hViewModel() );
	if ( viewmodel )
	{
		m_viewmodel_data[ cmd_num % MULTIPLAYER_BACKUP ] = {
			viewmodel->m_flCycle(),
			viewmodel->m_flAnimTime()
		};
	}
}

void NetData::SetViewmodel( int cmd_num )
{
	auto viewmodel = g_csgo.m_entlist->GetClientEntityFromHandle( g_ctx.m_local->m_hViewModel() );
	if ( viewmodel )
	{
		auto bro = &m_viewmodel_data[ cmd_num % MULTIPLAYER_BACKUP ];
		viewmodel->m_flCycle() = bro->first;
		viewmodel->m_flAnimTime() = bro->second;
	}
}