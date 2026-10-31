#include "includes.h"

TickbaseManager g_tickbase{ };

// todo: make this our guns cycle time.
int TickbaseManager::GetShiftAmount()
{
	return m_extra_process_ticks;
}

// to make sure we get proper record validity etc.
void TickbaseManager::UpdateTicks()
{
	m_pre_shift_ticks = 0;

	//if ( g_Variables.m_bools[ "tickbase_shift_fire" ] )
	//	m_pre_shift_ticks = GetShiftAmount();
	//
	//if ( !g_movement.m_transition_flick && !g_movement.m_fakewalking )
	//{
	//	static bool old_moving;
	//
	//	if ( g_Variables.m_bools[ "tickbase_shift_move" ] && g_ctx.m_pressing_move && !old_moving )
	//	{
	//		m_pre_shift_ticks = m_shift_ticks = m_extra_process_ticks;
	//	}
	//
	//	old_moving = g_ctx.m_pressing_move;
	//}
}

void TickbaseManager::FinishShift()
{
	if ( g_ctx.m_cmd->m_buttons & IN_ATTACK && g_ctx.m_weapon_fire )
	{
		m_shift_ticks = m_pre_shift_ticks;
	}
}

void TickbaseManager::UpdateTickbase( int command )
{
	if ( m_tickbase_log.empty() )
		return;

	while ( m_tickbase_log.size() > 128 )
		m_tickbase_log.pop_back();

	for ( int i = 0; i < m_tickbase_log.size(); i++ )
	{
		auto& cur = m_tickbase_log[ i ];

		if ( cur.command == command )
		{
			int estimated = g_ctx.m_local->m_nTickBase() + cur.queued;

			if ( estimated > cur.high || estimated < cur.low )
			{
				g_ctx.m_local->m_nTickBase() = cur.ideal - cur.queued + g_csgo.m_globals->m_frame_simulation_ticks;
			}

			break;
		}
	}
}

void TickbaseManager::GetHighLowTickbase( int* low, int* high, int* ideal )
{
	static ConVar* sv_clockcorrection_msecs = g_csgo.m_cvar->FindVar( HASH( "sv_clockcorrection_msecs" ) );
	float flCorrectionSeconds = std::clamp( sv_clockcorrection_msecs->GetFloat() / 1000.0f, 0.0f, 1.0f );
	int nCorrectionTicks = game::TIME_TO_TICKS( flCorrectionSeconds );

	// Set the target tick flCorrectionSeconds (rounded to ticks) ahead in the future. this way the client can
	//  alternate around this target tick without getting smaller than gpGlobals->tickcount.
	// After running the commands simulation time should be equal or after current gpGlobals->tickcount, 
	//  otherwise the simulation time drops out of the client side interpolated var history window.

	// correct the server tick.
	int lat_ticks = 0;
	if ( g_csgo.m_net )
		lat_ticks = game::TIME_TO_TICKS( g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING ) );

	int	nIdealFinalTick = g_csgo.m_cl->m_server_tick + lat_ticks + nCorrectionTicks;

	*high = nIdealFinalTick + nCorrectionTicks;
	*low = nIdealFinalTick - nCorrectionTicks;

	*ideal = nIdealFinalTick;
}

void TickbaseManager::AddTickShift( int command, int low, int high, int ideal, int queued )
{
	m_tickbase_log.push_front( { command, low, high, ideal, queued } );
}