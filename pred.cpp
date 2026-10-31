#include "includes.h"
#include "pred.h"

InputPrediction g_inputpred{};;

void CPrediction::SuppressHostEvents(void* player) {
	static auto addr = **pattern::find(g_csgo.m_client_dll, "A1 ? ? ? ? 0F 94 C1 85 C0").as< DWORD** >(1);
	auto next = addr;
	while (next)
	{
		*(DWORD*)(next + 12) = (int)player;
		*(DWORD*)(next + 8) = player != nullptr;
		if (*(DWORD*)(next + 16) > 0)
			*(DWORD*)(next + 16) = 0;
		next = *(DWORD*)(next + 4);
	}
}

void InputPrediction::GetDatamap( datamap_t*& map )
{
	if ( !map )
	{
		g_ctx.m_local->PrepareDatamap( g_ctx.m_local->GetPredDescMap() );
		map = (datamap_t*)g_csgo.m_mem_alloc->Alloc( std::max( g_ctx.m_local->GetPredDescMap()->m_nPackedSize, 4 ) );        
	}

	CPredictionCopy CopyHelper( 2, (byte*)map, true, (const byte*)g_ctx.m_local, false, CPredictionCopy::TRANSFERDATA_COPYONLY );   
	CopyHelper.TransferData( "InputPrediction::GetDatamap", g_ctx.m_local->index(), g_ctx.m_local->GetPredDescMap() );
}

void InputPrediction::SetDatamap( datamap_t*& map )
{
	CPredictionCopy CopyHelper( 2, (byte*)g_ctx.m_local, false, (const byte*)map, true, CPredictionCopy::TRANSFERDATA_COPYONLY );
	CopyHelper.TransferData( "InputPrediction::SetDatamap", g_ctx.m_local->index(), g_ctx.m_local->GetPredDescMap() );
}

void InputPrediction::update()
{
	//g_csgo.m_prediction->m_last_server_world_timestamp = 0.f;
	if ( g_csgo.cl_pred_optimize->GetInt() != 0 )
		g_csgo.cl_pred_optimize->SetValue( 0 );

	g_csgo.m_prediction->Update(
		g_csgo.m_cl->m_delta_tick,
		g_csgo.m_cl->m_delta_tick > 0,
		g_csgo.m_cl->m_last_command_ack,
		g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands
	);
}

void InputPrediction::store()
{
	m_in_prediction = g_csgo.m_prediction->m_in_prediction;
	m_first_time_predicted = g_csgo.m_prediction->m_first_time_predicted;

	g_csgo.m_prediction->m_in_prediction = true;
	g_csgo.m_prediction->m_first_time_predicted = false;

	m_curtime = g_csgo.m_globals->m_curtime;
	m_frametime = g_csgo.m_globals->m_frametime;
}

// CPrediction::RunCommand
void InputPrediction::predict()
{
	static CMoveData data{};
	memset( &data, 0, sizeof( data ) );

	g_csgo.m_globals->m_curtime = g_ctx.m_local->m_nTickBase() * g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frametime = g_csgo.m_prediction->m_engine_paused ? 0.f : g_csgo.m_globals->m_interval;

	m_dummy_cmd = *g_ctx.m_cmd;

	// CPrediction::StartCommand
	g_ctx.m_local->m_pCurrentCommand() = &m_dummy_cmd;
	g_ctx.m_local->m_PlayerCommand() = m_dummy_cmd;

	*g_csgo.m_nPredictionRandomSeed = m_dummy_cmd.m_random_seed;
	g_csgo.m_pPredictionPlayer = g_ctx.m_local;

	g_csgo.m_prediction->SuppressHostEvents( g_ctx.m_local );
	g_csgo.m_move_helper->SetHost( g_ctx.m_local );
	g_csgo.m_game_movement->StartTrackPredictionErrors( g_ctx.m_local );

	m_dummy_cmd.m_buttons |= IN_ATTACK; // fix weapon timing.
	m_dummy_cmd.m_buttons |= *reinterpret_cast<uint8_t*>( uintptr_t( g_ctx.m_local ) + 0x3310 );

	if ( m_dummy_cmd.m_impulse )
		*reinterpret_cast<uint8_t*>( uintptr_t( g_ctx.m_local ) + 0x31EC ) = m_dummy_cmd.m_impulse;

	g_ctx.m_local->UpdateButtonState( m_dummy_cmd.m_buttons );

	g_csgo.m_prediction->SetLocalViewAngles( m_dummy_cmd.m_view_angles );
	typedef char( __thiscall* PhysicsRunThink_t )( void*, int );
	static auto physics_run_think = pattern::find( g_csgo.m_client_dll, "55 8B EC 83 EC ? 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8 ?" ).as<PhysicsRunThink_t>();
	if ( physics_run_think( g_ctx.m_local, 0 ) )
		g_ctx.m_local->PreThink();

	const auto next_think = *reinterpret_cast<int*>( uintptr_t( g_ctx.m_local ) + ( 0xF8 ) );
	if ( next_think > 0 && next_think <= *(int*)( g_ctx.m_local + 0x3430 ) )
	{
		*reinterpret_cast<int*>( uintptr_t( g_ctx.m_local ) + ( 0xF8 ) ) = -1; // next_think

		typedef void( __thiscall* SetNextThink_t )( void*, char );
		static auto set_next_think = pattern::find( g_csgo.m_client_dll, "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6" ).as<SetNextThink_t>();
		set_next_think( g_ctx.m_local, next_think );

		g_ctx.m_local->Think();
	}

	g_csgo.m_prediction->SetupMove( g_ctx.m_local, &m_dummy_cmd, g_csgo.m_move_helper, &data );
	g_csgo.m_game_movement->ProcessMovement( g_ctx.m_local, &data );
	g_csgo.m_prediction->FinishMove( g_ctx.m_local, &m_dummy_cmd, &data );

	// needed from PostThink
	if ( g_ctx.m_local->m_fFlags() & FL_DUCKING )
		g_ctx.m_local->SetCollisionBounds( { -16, -16, 0 }, { 16, 16, 36 } );
	else
		g_ctx.m_local->SetCollisionBounds( { -16, -16, 0 }, { 16, 16, 72 } );

	g_csgo.m_game_movement->FinishTrackPredictionErrors( g_ctx.m_local );
	g_csgo.m_move_helper->SetHost( nullptr );

	g_csgo.m_prediction->SuppressHostEvents( nullptr );
}

void InputPrediction::restore()
{
	g_csgo.m_prediction->m_in_prediction = m_in_prediction;
	g_csgo.m_prediction->m_first_time_predicted = m_first_time_predicted;

	*g_csgo.m_nPredictionRandomSeed = -1;
	g_csgo.m_pPredictionPlayer = nullptr;

	g_csgo.m_globals->m_curtime = m_curtime;
	g_csgo.m_globals->m_frametime = m_frametime;
}

void InputPrediction::predict_postpone_time( CUserCmd* cmd )
{
	constexpr float magic = 0.234375f;

	if ( !g_ctx.m_need_revolver_timing )
	{
		g_ctx.m_revolver_time = g_csgo.m_globals->m_curtime + magic;
		return;
	}

	Weapon* weapon = g_ctx.m_local->GetActiveWeapon();
	if ( !weapon || weapon->m_iItemDefinitionIndex() != REVOLVER || g_csgo.m_globals->m_curtime >= g_ctx.m_revolver_time || cmd->m_buttons & IN_RELOAD )
		g_ctx.m_revolver_time = g_csgo.m_globals->m_curtime + magic;
}

CPredictionCopy::CPredictionCopy( int type, byte* dest, bool dest_packed, const byte* src, bool src_packed, optype_t opType, FN_FIELD_COMPARE func )
{
	m_OpType = opType;
	m_nType = type;
	m_pDest = dest;
	m_pSrc = src;
	m_nDestOffsetIndex = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
	m_nSrcOffsetIndex = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;

	m_nErrorCount = 0;
	m_nEntIndex = -1;

	m_pWatchField = nullptr;
	m_FieldCompareFunc = func;
}

int CPredictionCopy::TransferData( const char* operation, int entindex, datamap_t* dmap )
{
	using TransferDataFn = int( __thiscall* )( CPredictionCopy*, const char*, int, datamap_t* );
	static auto pat = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 8B 45 10 53 56 8B F1 57" ) ).as< TransferDataFn >();
	return pat( this, operation, entindex, dmap );
}