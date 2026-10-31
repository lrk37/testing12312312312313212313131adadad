#include "includes.h"
#define	EQUAL_EPSILON	0.001 

void Hooks::DoExtraBoneProcessing( int a2, int a3, int a4, int a5, int a6, int a7 )
{
	return;
}

void Hooks::StandardBlendingRules( int a2, int a3, int a4, int a5, int a6 )
{
	// cast thisptr to player ptr.
	Player* player = ( Player* )this;

	// These are To Fix Arms Flickering. Bone Building, Stuff.
	if ( player->enemy( g_ctx.m_local ) || player->m_bIsLocalPlayer( ) )
		a6 = BONE_USED_BY_SERVER;

	if ( player->m_bIsLocalPlayer( ) )
		a6 |= BONE_USED_BY_BONE_MERGE;

	player->m_fEffects( ) |= EF_NOINTERP;
	g_hooks.m_StandardBlendingRules( this, a2, a3, a4, a5, a6 );
	player->m_fEffects( ) &= ~EF_NOINTERP;
}

void Hooks::BuildTransformations( CStudioHdr* a2, int a3, int a4, int a5, int a6, int a7 )
{
	Player* player = ( Player* )this;

	int bone_jiggle = *reinterpret_cast< int* >( uintptr_t( player ) + 0x291C );
	const auto bone_flags = a2->m_boneFlags;

	*reinterpret_cast< int* >( uintptr_t( player ) + 0x291C ) = 0;
	for ( auto i = 0; i < a2->m_boneFlags.Count( ); i++ )
		a2->m_boneFlags.Element( i ) &= ~0x04; // BONE_ALWAYS_PROCEDURAL

	g_hooks.m_BuildTransformations( this, a2, a3, a4, a5, a6, a7 );

	*reinterpret_cast< int* >( uintptr_t( player ) + 0x291C ) = bone_jiggle;
	a2->m_boneFlags = bone_flags;
}

void Hooks::UpdateClientSideAnimation( )
{
	// call some shit for local player, but prevent updating animstate.
	// this wont be done now because we only call CCSGOPlayerAnimstate->Update instead of UpdateClientSideAnimation.
	Player* player = ( Player* )this;
	if ( player == g_ctx.m_local )
	{
		CCSGOPlayerAnimState* state = g_ctx.m_local->m_PlayerAnimState( );
		if ( !state )
			return;

		const int old_frame = state->m_last_update_frame;
		state->m_last_update_frame = g_csgo.m_globals->m_frame;

		g_hooks.m_UpdateClientSideAnimation( this );

		state->m_last_update_frame = old_frame;
	}
}

typedescription_t* GetTypeDescription( datamap_t* map, const char* name )
{
	while ( map )
	{
		for ( int i = 0; i < map->dataNumFields; i++ )
		{
			if ( map->dataDesc[ i ].field_name == nullptr )
				continue;

			if ( strcmp( name, map->dataDesc[ i ].field_name ) == 0 )
				return &map->dataDesc[ i ];

			if ( map->dataDesc[ i ].field_type == FIELD_EMBEDDED )
			{
				if ( map->dataDesc[ i ].td )
				{
					typedescription_t* offset { };
					if ( ( offset = GetTypeDescription( map->dataDesc[ i ].td, name ) ) != nullptr )
						return offset;
				}
			}
		}

		map = map->baseMap;
	}

	return nullptr;
}

datamap_t* Hooks::GetPredDescMap( )
{
	static datamap_t customMap;

	if ( !customMap.baseMap )
	{
		customMap = { 0, 0, "Grimace", g_hooks.m_GetPredDescMap( this ) };

		static typedescription_t predDesc[ ] = {
				{ FIELD_VOID, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ FIELD_FLOAT, "m_flVelocityModifier", static_cast< int >( g_entoffsets.m_flVelocityModifier ), 1, 0x0100, NULL, NULL, NULL, NULL, sizeof( float ), NULL, 0, g_csgo.m_globals->m_interval * 2.f }, //0.003922f isnt accurate enough bud heh
				{ FIELD_FLOAT, "m_flThirdpersonRecoil", static_cast< int >( g_entoffsets.m_flThirdpersonRecoil ), 1, 0x0100, NULL, NULL, NULL, NULL, sizeof( float ), NULL, 0, g_csgo.m_globals->m_interval },
				{ FIELD_BOOLEAN, "m_bWaitForNoAttack", static_cast< int >( g_entoffsets.m_bWaitForNoAttack ), 1, 0x0100, NULL, NULL, NULL, NULL, sizeof( bool ), NULL, 0, 0 },
				// ..
		};

		customMap.dataNumFields = ARRAYSIZE( predDesc ) - 1;
		customMap.dataDesc = &predDesc[ 1 ];

		customMap.m_pOptimizedDataMap = nullptr;

		( ( Player* )this )->PrepareDatamap( &customMap );
	}

	return &customMap;
}

void Hooks::PostDataUpdate( DataUpdateType_t type )
{
	Player* player = ( Player* )( ( uintptr_t )this - 0x8 );
	if ( player )
	{
		if ( player->m_bIsLocalPlayer( ) )
		{
			g_skins.OnPostDataUpdate( );
		}
		else if ( player->IsPlayer( ) )
		{
			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
			if ( data )
			{
				data->OnNetUpdate( player );
			}
		}
	}

	g_hooks.m_PostDataUpdate( this, type );
}

void CustomEntityListener::OnEntityCreated( Entity* ent )
{
	if ( ent )
	{
		// player created.
		if ( ent->IsPlayer( ) )
		{
			Player* player = ent->as< Player* >( );

			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
			if ( data )
				data->reset( );

			VMT* player_vmt = &g_hooks.m_player[ player->index( ) - 1 ];
			if ( player_vmt )
			{
				player_vmt->reset( );
				player_vmt->init( player );

				g_hooks.m_DoExtraBoneProcessing = player_vmt->add< Hooks::DoExtraBoneProcessing_t >( Player::DOEXTRABONEPROCESSING, util::force_cast( &Hooks::DoExtraBoneProcessing ) );
				g_hooks.m_StandardBlendingRules = player_vmt->add< Hooks::StandardBlendingRules_t >( Player::STANDARDBLENDINGRULES, util::force_cast( &Hooks::StandardBlendingRules ) );
				g_hooks.m_BuildTransformations = player_vmt->add< Hooks::BuildTransformations_t >( Player::BUILDTRANSFORMATIONS, util::force_cast( &Hooks::BuildTransformations ) );
				g_hooks.m_UpdateClientSideAnimation = player_vmt->add< Hooks::UpdateClientSideAnimation_t >( Player::UPDATECLIENTSIDEANIMATION, util::force_cast( &Hooks::UpdateClientSideAnimation ) );

				if ( player->index( ) == g_csgo.m_engine->GetLocalPlayer( ) )
				{
					//g_hooks.m_GetActiveWeapon = player_vmt->add< Hooks::GetActiveWeapon_t >( Player::GETACTIVEWEAPON, util::force_cast( &Hooks::GetActiveWeapon ) ); // we don't want this, it disables glow and is cringe
					g_hooks.m_GetPredDescMap = player_vmt->add< Hooks::GetPredDescMap_t >( 17, util::force_cast( &Hooks::GetPredDescMap ) );
				}
			}

			VMT* networkable_vmt = &g_hooks.m_player_networkable[ player->index( ) - 1 ];
			if ( networkable_vmt )
			{
				networkable_vmt->reset( );
				networkable_vmt->init( ( void* )( ( uintptr_t )player + 0x8 ) );

				g_hooks.m_PostDataUpdate = networkable_vmt->add<Hooks::PostDataUpdate_t>( 7, util::force_cast( &Hooks::PostDataUpdate ) );
			}
		}
		// dont think this makes a difference
		else if ( ent->IsBaseCombatWeapon( ) )
		{
			auto dmap = ent->GetPredDescMap( );

			auto m_flNextPrimaryAttack = GetTypeDescription( dmap, "m_flNextPrimaryAttack" );
			m_flNextPrimaryAttack->flags = 0x0100; // FTYPEDESC_INSENDTABLE

			auto m_flNextSecondaryAttack = GetTypeDescription( dmap, "m_flNextSecondaryAttack" );
			m_flNextSecondaryAttack->flags = 0x0100; // FTYPEDESC_INSENDTABLE

			auto m_flPostponeFireReadyTime = GetTypeDescription( dmap, "m_flPostponeFireReadyTime" );
			m_flPostponeFireReadyTime->flags = 0x0100; // FTYPEDESC_INSENDTABLE
		}

		//else if (ent->get_class_id() == _CPredictedViewModel) {
		//	g_ctx.print(tfm::format("%i\n", ent->index()));
		//	auto dmap = ent->GetPredDescMap();
		//
		//	//auto m_flAnimTime{ GetTypeDescription(dmap, "m_flAnimTime") };
		//	//if (m_flAnimTime->flags != (0x0200 | 0x0080 | 0x0400))
		//	//	m_flAnimTime->flags = 0x0200 | 0x0080 | 0x0400;
		//	//
		//	//auto m_nAnimationParity{ GetTypeDescription(dmap, "m_nAnimationParity") };
		//	//m_nAnimationParity->flags |= 0x0400; // FTYPEDESC_NOERRORCHECK		
		//
		//	auto m_nSequence{ GetTypeDescription(dmap, "m_nSequence") };
		//	m_nSequence->flags |= 0x0400;
		//
		//}
	}
}

void CustomEntityListener::OnEntityDeleted( Entity* ent )
{
	// note; IsPlayer doesn't work here because the ent class is CBaseEntity.
	if ( ent && ent->index( ) >= 1 && ent->index( ) <= 64 )
	{
		Player* player = ent->as< Player* >( );

		AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
		if ( data ) {
			data->reset( );
			data->m_ignore = false;
			data->m_player = nullptr;
		}

		VMT* player_vmt = &g_hooks.m_player[ player->index( ) - 1 ];
		if ( player_vmt )
			player_vmt->reset( );

		VMT* networkable_vmt = &g_hooks.m_player_networkable[ player->index( ) - 1 ];
		if ( networkable_vmt )
			networkable_vmt->reset( );
	}
}