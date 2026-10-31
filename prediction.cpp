#include "includes.h"

bool Hooks::InPrediction()
{
	ang_t* angles;

	// note - dex; first 2 'test al, al' instructions in C_BasePlayer::CalcPlayerView.
	static Address CalcPlayerView_ret1{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C" ) ) };
	static Address CalcPlayerView_ret2{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 08 57 8B CE E8 ? ? ? ? 8B 06" ) ) };

	// kick is viewpunch
	// smooth is aim_punch
	if ( g_ctx.m_local && (g_Variables.m_bools[ "remove_kick_recoil" ] || g_Variables.m_bools[ "remove_smooth_recoil" ] ) )
	{
		// view->DriftPitch().
		if ( _ReturnAddress() == CalcPlayerView_ret1 )
			return true;

		if ( _ReturnAddress() == CalcPlayerView_ret2 )
		{
			// at this point, angles are copied into the CalcPlayerView's eyeAngles argument.
			// (ebp) InPrediction -> (ebp) CalcPlayerView + 0xC = eyeAngles.
			uintptr_t stack = *(uintptr_t*)( ( *(uintptr_t*)( (uintptr_t)_AddressOfReturnAddress() - sizeof( uintptr_t ) ) ) + 0xC );
			angles = reinterpret_cast<ang_t*>( stack );

			if ( angles )
			{
				if (g_Variables.m_bools[ "remove_kick_recoil" ])
					*angles -= g_ctx.m_local->m_viewPunchAngle();
				 
				if (g_Variables.m_bools["remove_smooth_recoil"])
					*angles -= (g_ctx.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat()) * g_csgo.view_recoil_tracking->GetFloat();

				//*angles -= g_ctx.m_local->m_viewPunchAngle()
				//	+ ( g_ctx.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat() )
				//	* g_csgo.view_recoil_tracking->GetFloat();
			}

			return true;
		}
	}

	return g_hooks.m_prediction.GetOldMethod< InPrediction_t >( CPrediction::INPREDICTION )( this );
}

void Hooks::PostEntityPacketReceived()
{
	return;
}

void Hooks::PostNetworkDataReceived( int commands_acknowledged )
{
	if ( g_ctx.m_processing && commands_acknowledged > 0 )
	{  
		g_netdata.StoreAnims( commands_acknowledged - 1, false );

		// this is the point where our info gets overrided with bungus shit from server
		// so restore our clients pred info from this cmd.
		// also, idk if we need to check tolerance or anytinnnnggg like that lool.
		//g_netdata.SetViewmodel( commands_acknowledged ); THIS DOES NOT WORK
	}

	return g_hooks.m_prediction.GetOldMethod< PostNetworkDataReceived_t >( CPrediction::POSTNETWORKDATARECEIVED )( this, commands_acknowledged );
}

// Nice.
static float bone_yaw_pose = 0.f;
static bool apply_foot_yaw = true;

void Hooks::RunCommand( Entity* ent, CUserCmd* cmd, IMoveHelper* movehelper )
{
	Player* player = reinterpret_cast<Player*>( ent );
	if ( !player || !player->m_bIsLocalPlayer() || !player->alive() )
		return g_hooks.m_prediction.GetOldMethod< RunCommand_t >( CPrediction::RUNCOMMAND )( this, ent, cmd, movehelper );

	static ConVar* sv_max_usercmd_future_ticks = g_csgo.m_cvar->FindVar( HASH( "sv_max_usercmd_future_ticks" ) );
	if ( cmd->m_tick >= ( g_csgo.m_globals->m_tick_count + sv_max_usercmd_future_ticks->GetInt() ) )
		return;

	g_netdata.RestoreAnims( cmd->m_command_number - 1 );
	g_netdata.SetViewmodel( cmd->m_command_number - 1 );

	g_hooks.m_prediction.GetOldMethod< RunCommand_t >( CPrediction::RUNCOMMAND )( this, ent, cmd, movehelper );

    // https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/baseplayer_shared.cpp#L3267
	// it is very likely retarded to just always set this to zero
	// you can see in the link above that sometimes (when m_pPhysicsController != nullptr) m_vPhysicsCollisionState can be != 0 on server
	// todo: if any problems arise with this method (ive not seen any), hook the func above and check m_pPhysicsController for a proper fix.
	g_ctx.m_local->m_vphysicsCollisionState() = 0;

	if ( !g_ctx.m_update_records.empty() )
	{
		for ( int i = 0; i < g_ctx.m_update_records.size(); i++ )
		{
			auto& current = g_ctx.m_update_records[ i ];
			if ( current.m_cmd_num != cmd->m_command_number )
				continue;

			g_ctx.UpdateAnimstate( cmd, cmd->m_view_angles, current.m_viewmodel_act, current.m_jumping );

			if ( i == 0 )
			{
				switch ( current.m_hide_angle )
				{
				case 0:
					if ( !apply_foot_yaw )
					{
						if ( g_ctx.m_local->m_vecVelocity().length() > 0.1f )
						{
							g_ctx.m_render_abs = g_ctx.m_abs_yaw;
							bone_yaw_pose = g_ctx.m_local->m_flPoseParameter()[ 11 ];
							apply_foot_yaw = true;
						}
						else
						{
							CCSGOPlayerAnimState* state = g_ctx.m_local->m_PlayerAnimState();
							if ( state && state->m_abs_yaw == state->m_abs_yaw_last )
							{
								apply_foot_yaw = true;
							}

							// i think theres a problem caused by the foot yaw approaching
							// which causes the float equals to never be true
							// so just check if its within 1 update's limit instead.
							//if ( state && abs( math::NormalizedAngle( state->m_abs_yaw - g_ctx.m_local->m_flLowerBodyYawTarget() ) ) 
							//	== abs( math::NormalizedAngle( state->m_abs_yaw_last - g_ctx.m_local->m_flLowerBodyYawTarget() ) ) )
							//{
							//	apply_foot_yaw = true;
							//}
						}
					}

					if ( apply_foot_yaw )
					{
						g_ctx.m_render_abs = g_ctx.m_abs_yaw;
						bone_yaw_pose = g_ctx.m_local->m_flPoseParameter()[ 11 ];
					}
					break;
				case 1:
					apply_foot_yaw = false;
					break;
				case 2:
					apply_foot_yaw = false;
					g_bones.Bro( g_ctx.m_local, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE, g_chams.local_hidden );
					g_chams.last_hidden_time = g_csgo.m_globals->m_curtime;
					break;
				}

				if ( g_Variables.m_bools[ "force_local_footplant" ] && current.m_jumping )
					g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_weight = 0.f; // brohio

				// aimbot matrix.
				{
					float backup_pitch = g_ctx.m_local->m_flPoseParameter()[ 12 ];
					g_ctx.m_local->m_flPoseParameter()[ 12 ] = 0.5f; // math
					g_bones.Bro( g_ctx.m_local, BONE_USED_BY_HITBOX, g_server_animations.local_matrix );
					g_ctx.m_local->m_flPoseParameter()[ 12 ] = backup_pitch;
				}

				g_ctx.m_local->m_flPoseParameter()[ 11 ] = bone_yaw_pose;

				if ( g_Variables.m_bools[ "remove_model_sway" ] )
					g_ctx.m_local->m_AnimOverlay()[ 12 ].m_weight = 0.f;

				if ( g_csgo.m_globals->m_curtime != g_ctx.m_smooth_data.m_time )
				{
					memcpy( g_ctx.m_smooth_data.m_old_poses, g_ctx.m_smooth_data.m_goal_poses, sizeof( float ) * 24 );
					memcpy( g_ctx.m_smooth_data.m_old_layers, g_ctx.m_smooth_data.m_goal_layers, sizeof( C_AnimationLayer ) * 13 );

					g_ctx.m_local->GetPoseParameters( g_ctx.m_smooth_data.m_goal_poses );
					g_ctx.m_local->GetAnimLayers( g_ctx.m_smooth_data.m_goal_layers );

					g_ctx.m_smooth_data.m_old_abs = g_ctx.m_smooth_data.m_goal_abs;
					g_ctx.m_smooth_data.m_goal_abs = g_ctx.m_render_abs;

					g_ctx.m_smooth_data.m_old_time = g_ctx.m_smooth_data.m_time;
					g_ctx.m_smooth_data.m_time = g_csgo.m_globals->m_curtime;

					if ( g_Variables.m_bools[ "smooth_local_anims" ] )
					{
						if ( g_ctx.m_local->m_PlayerAnimState() )
							g_ctx.m_smooth_data.m_do_smoothing = current.m_force_smoothing || g_ctx.m_local->m_PlayerAnimState()->m_velocity_length_xy <= 0.1f;
					}
					else
						g_ctx.m_smooth_data.m_do_smoothing = false;
				}

				if ( !g_ctx.m_smooth_data.m_do_smoothing )
				{
					g_ctx.m_local->SetAbsAngles( ang_t( 0.f, g_ctx.m_render_abs, 0.f ) );
					g_bones.Bro( g_ctx.m_local, BONE_USED_BY_ANYTHING, g_server_animations.local_render_matrix );
					g_server_animations.last_render_setup = g_ctx.m_local->GetAbsOrigin();
				}
			}

			break;
		}
	}

	g_inputpred.predict_postpone_time( cmd );

	g_netdata.StoreAnims( cmd->m_command_number );
	g_netdata.StoreViewmodel( cmd->m_command_number );
}