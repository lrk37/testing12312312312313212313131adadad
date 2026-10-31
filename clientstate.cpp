#include "includes.h"

bool Hooks::TempEntities( void* msg )
{
	if ( !g_ctx.m_processing )
	{
		return g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );
	}

	const bool ret = g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );

	CEventInfo* ei = g_csgo.m_cl->m_events;
	CEventInfo* next = nullptr;

	if ( !ei )
	{
		return ret;
	}

	do
	{
		next = *reinterpret_cast<CEventInfo**>( reinterpret_cast<uintptr_t>( ei ) + 0x38 );

		uint16_t classID = ei->m_class_id - 1;

		auto m_pCreateEventFn = ei->m_client_class->m_pCreateEvent;
		if ( !m_pCreateEventFn )
		{
			continue;
		}

		void* pCE = m_pCreateEventFn();
		if ( !pCE )
		{
			continue;
		}

		if ( classID == 170 )
		{
			ei->m_fire_delay = 0.0f;
		}
		ei = next;
	} while ( next != nullptr );

	return ret;
}

bool CorrectIncomingSequence( int outgoing_command )
{
	if ( !g_ctx.m_local )
		return true;

	for ( auto it = g_ctx.m_packet_sequences.begin(); it != g_ctx.m_packet_sequences.end();) // clear old sequences
	{
		if ( *it < outgoing_command )
		{
			it = g_ctx.m_packet_sequences.erase( it );
		}
		else
		{
			++it;
		}
	}

	for ( auto it = g_ctx.m_packet_sequences.begin(); it != g_ctx.m_packet_sequences.end(); ++it ) // return true when we should start packetingering
	{
		if ( *it == outgoing_command )
		{
			g_ctx.m_packet_sequences.erase( it );
			return true;
		}
	}

	return false;
}


void Hooks::PacketStart(int incoming_sequence, int outgoing_acknowledged) {

	if (CorrectIncomingSequence(outgoing_acknowledged))
	{
		g_hooks.m_client_state.GetOldMethod< PacketStart_t >(CClientState::PACKETSTART)(this, incoming_sequence, outgoing_acknowledged);
	}
}