#include "includes.h"

Skins g_skins{};

void Skins::load()
{
	// update model indexes on mapload.
	m_knife_data[ knives_t::BAYONET ].setup( KNIFE_BAYONET,
		XOR( "models/weapons/v_knife_bayonet.mdl" ),
		XOR( "models/weapons/w_knife_bayonet.mdl" ),
		XOR( "bayonet" ) );

	m_knife_data[ knives_t::BOWIE ].setup( KNIFE_BOWIE,
		XOR( "models/weapons/v_knife_survival_bowie.mdl" ),
		XOR( "models/weapons/w_knife_survival_bowie.mdl" ),
		XOR( "knife_survival_bowie" ) );

	m_knife_data[ knives_t::BUTTERFLY ].setup( KNIFE_BUTTERFLY,
		XOR( "models/weapons/v_knife_butterfly.mdl" ),
		XOR( "models/weapons/w_knife_butterfly.mdl" ),
		XOR( "knife_butterfly" ) );

	m_knife_data[ knives_t::FALCHION ].setup( KNIFE_FALCHION,
		XOR( "models/weapons/v_knife_falchion_advanced.mdl" ),
		XOR( "models/weapons/w_knife_falchion_advanced.mdl" ),
		XOR( "knife_falchion" ) );

	m_knife_data[ knives_t::FLIP ].setup( KNIFE_FLIP,
		XOR( "models/weapons/v_knife_flip.mdl" ),
		XOR( "models/weapons/w_knife_flip.mdl" ),
		XOR( "knife_flip" ) );

	m_knife_data[ knives_t::GUT ].setup( KNIFE_GUT,
		XOR( "models/weapons/v_knife_gut.mdl" ),
		XOR( "models/weapons/w_knife_gut.mdl" ),
		XOR( "knife_gut" ) );

	m_knife_data[ knives_t::HUNTSMAN ].setup( KNIFE_HUNTSMAN,
		XOR( "models/weapons/v_knife_tactical.mdl" ),
		XOR( "models/weapons/w_knife_tactical.mdl" ),
		XOR( "knife_tactical" ) );

	m_knife_data[ knives_t::KARAMBIT ].setup( KNIFE_KARAMBIT,
		XOR( "models/weapons/v_knife_karam.mdl" ),
		XOR( "models/weapons/w_knife_karam.mdl" ),
		XOR( "knife_karambit" ) );

	m_knife_data[ knives_t::M9 ].setup( KNIFE_M9_BAYONET,
		XOR( "models/weapons/v_knife_m9_bay.mdl" ),
		XOR( "models/weapons/w_knife_m9_bay.mdl" ),
		XOR( "knife_m9_bayonet" ) );

	m_knife_data[ knives_t::DAGGER ].setup( KNIFE_SHADOW_DAGGERS,
		XOR( "models/weapons/v_knife_push.mdl" ),
		XOR( "models/weapons/w_knife_push.mdl" ),
		XOR( "knife_push" ) );

	// update glove model indexes on mapload.
	m_glove_data[ gloves_t::BLOODHOUND ].setup( 5027,
		XOR( "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_bloodhound.mdl" ) ); // WORLD MODEL DIRECTORY IS FUCKED LOL NOW WE HAVE THIRDPERSON GLOVES

	m_glove_data[ gloves_t::SPORTY ].setup( 5030,
		XOR( "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_sporty.mdl" ) );

	m_glove_data[ gloves_t::DRIVER ].setup( 5031,
		XOR( "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_slick.mdl" ) );

	m_glove_data[ gloves_t::HANDWRAP ].setup( 5032,
		XOR( "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl" ) );

	m_glove_data[ gloves_t::MOTO ].setup( 5033,
		XOR( "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_motorcycle.mdl" ) );

	m_glove_data[ gloves_t::SPECIALIST ].setup( 5034,
		XOR( "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_specialist.mdl" ) );

	m_glove_data[ gloves_t::HYDRA ].setup( 5035,
		XOR( "models/weapons/v_models/arms/glove_specialist/v_glove_bloodhound_hydra.mdl" ),
		XOR( "models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl" ) );

	m_update_time = 0.f;
}

int ConvertKnifetoWeaponDefinition()
{
	switch ( g_Variables.m_ints[ "knife_index" ] )
	{
	case 1:
		return KNIFE_BAYONET;
	case 2:
		return KNIFE_BOWIE;
	case 3:
		return KNIFE_BUTTERFLY;
	case 4:
		return KNIFE_FALCHION;
	case 5:
		return KNIFE_FLIP;
	case 6:
		return KNIFE_GUT;
	case 7:
		return KNIFE_HUNTSMAN;
	case 8:
		return KNIFE_KARAMBIT;
	case 9:
		return KNIFE_M9_BAYONET;
	case 10:
		return KNIFE_SHADOW_DAGGERS;
	default:
		return -1;
	}


	return -1;
}

int equip_wearable( void* wearable, void* player )
{
	static auto fn = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9" ) ).as<int( __thiscall* )( void*, void* )>();
	return fn( wearable, player );
}

int initialize_attributes( void* wearable )
{
	static auto fn = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 0C 53 56 8B F1 8B 86" ) ).as<int( __thiscall* )( void* )>();
	return fn( wearable );
}

void Skins::OnLevelShutdown()
{
	const auto our_glove = reinterpret_cast<Weapon*>( g_csgo.m_entlist->GetClientEntityFromHandle( m_glove_handle ) );
	for ( int i = 64; i < g_csgo.m_entlist->GetHighestEntityIndex(); i++ )
	{
		auto ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent || ent->GetClientClass()->m_ClassID != _CEconWearable || ent != our_glove )
			continue;

		g_csgo.m_leaf_system->RemoveRenderable( ent->GetRefEHandle() );
		ent->SetDestroyedOnRecreateEntities();
		ent->Release();
		m_glove_handle = 0;
		break;
	}
}

void Skins::OnPostDataUpdate()
{
	if ( !g_ctx.m_local->alive() )
		return;

	player_info_t player_info;
	if ( !g_csgo.m_engine->GetPlayerInfo( g_ctx.m_local->index(), &player_info ) )
		return;

	if ( !first_handle )
	{
		m_glove_handle.Set( nullptr );
		first_handle = true;
	}

	if ( g_Variables.m_ints[ "glove_index" ] > 0 )
	{
		ClientClass* list{ g_csgo.m_client->GetAllClasses() };
		CBaseHandle* wearables = g_ctx.m_local->m_hMyWearables();

		// get pointer to entity from wearable handle 0.
		// glove is at wearable 0, valve will likely add more wearables like hats and boots. thats why.
		if ( wearables )
		{
			Weapon* glove = static_cast<Weapon*>( g_csgo.m_entlist->GetClientEntityFromHandle( wearables[ 0 ] ) );
			const auto custom_glove = reinterpret_cast<Weapon*>( g_csgo.m_entlist->GetClientEntityFromHandle( m_glove_handle ) );

			if ( !glove )
			{
				if ( custom_glove )
				{
					wearables[ 0 ] = m_glove_handle;
					glove = custom_glove;
				}

			}

			// if we at this point still not have a glove entity.
			// we should create one.
			if ( !glove )
			{
				if ( !g_ctx.m_local->alive() )
					return;

				int index = g_csgo.m_entlist->GetHighestEntityIndex() + 1;
				for ( int i = g_csgo.m_globals->m_max_clients; i < g_csgo.m_entlist->GetHighestEntityIndex(); i++ )
				{
					auto entity = g_csgo.m_entlist->GetClientEntity( i );

					if ( entity && entity->GetClientClass()->m_ClassID == _CPlasma )
					{
						index = i;
						break;
					}
				}

				// iterate list.
				for ( ; list != nullptr; list = list->m_pNext )
				{
					// break if we found wearable
					if ( list->m_ClassID == _CEconWearable )
					{
						// create an ent index and serial no for the new glove entity.
						int serial = 4095;

						list->m_pCreate( index, serial );
						wearables[ 0 ] = index | ( serial << 16 );

						break;
					}
				}

				glove = (Weapon*)g_csgo.m_entlist->GetClientEntity( index );

				if ( glove )
				{
					GloveData* data = &m_glove_data[ g_Variables.m_ints[ "glove_index" ] ];
					int id = 10006 + m_weapon_name_map[ 48 ].set_paintkit;

					glove->m_iItemIDHigh() = -1;
					glove->m_iItemDefinitionIndex() = data->m_id;
					glove->m_nFallbackPaintKit() = id;
					glove->m_iEntityQuality() = 4;
					glove->m_iAccountID() = player_info.m_xuid_low;
					glove->m_bInitialized() = true;
					glove->m_nFallbackSeed() = 0;
					glove->SetGloveModelIndex( data->m_world_model_index );
					equip_wearable( glove, g_ctx.m_local ); // follow entity, owner
					*(int*)( (uintptr_t)g_ctx.m_local + 0xA20 ) = 1; //(int)"m_nBody";
					initialize_attributes( glove );
					g_csgo.m_leaf_system->_CreateRenderableHandle( glove );
					if ( glove->GetClientRenderable() )
						glove_render_handle = glove->GetClientRenderable()->RenderHandle();

					auto networkable = glove->networkable();
					if ( networkable )
					{
						glove->PreDataUpdate( DataUpdateType_t::DATA_UPDATE_CREATED );
					}
				}
			}
			else
			{
				if ( glove->GetClientRenderable() )
				{
					if ( glove->GetClientRenderable()->RenderHandle() == 0xffff ||
						glove->GetClientRenderable()->RenderHandle() != glove_render_handle )
					{

						g_csgo.m_leaf_system->RemoveRenderable( glove_render_handle );
						g_csgo.m_leaf_system->_CreateRenderableHandle( glove );
						glove_render_handle = glove->GetClientRenderable()->RenderHandle();
					}
				}
			}
		}
	}
	else
	{
		const auto custom_glove = reinterpret_cast<Weapon*>( g_csgo.m_entlist->GetClientEntityFromHandle( m_glove_handle ) );
		if ( custom_glove && m_glove_handle.IsValid() )
		{
			auto pNetworkable = custom_glove->networkable();
			if ( pNetworkable )
			{
				custom_glove->SetDestroyedOnRecreateEntities();
				custom_glove->Release();
			}

			m_glove_handle.Set( nullptr );
		}
	}
	// handle weapons
	{
		if ( !g_GUI.m_is_open )
			g_GUI.iSelectedSkinWeapon = ConvertDefinitionIndexToMapID( g_ctx.m_weapon_id );

		auto weapons = g_ctx.m_local->m_hMyWeapons();
		KnifeData* knife = &m_knife_data[ g_Variables.m_ints[ "knife_index" ] ];
		for ( auto i = 0; weapons[ i ].IsValid(); i++ )
		{
			Weapon* weapon = (Weapon*)g_csgo.m_entlist->GetClientEntityFromHandle( weapons[ i ] );
			if ( !weapon )
				continue;

			// our knife will always return the normal one, but to save multiple knife types at our discretion we have to do this. No problem honestly.
			// only fail i think i could see is if we DO have a certain type of knife and we dont override
			int def_index = weapon->IsKnife() ? ConvertKnifetoWeaponDefinition() : weapon->m_iItemDefinitionIndex();
			int index = ConvertDefinitionIndexToMapID( def_index );

			if ( weapon->IsKnife() )
			{ // supremacy shit it works idc
				if ( g_Variables.m_ints[ "knife_index" ] > 0 )
				{
					weapon->m_iItemDefinitionIndex() = knife->m_id;
					weapon->m_nModelIndex() = knife->m_model_index;
					weapon->m_iViewModelIndex() = knife->m_model_index;
					weapon->m_iWorldModelIndex() = knife->m_world_model_index;
					weapon->m_iEntityQuality() = 3;
				}
				else
				{
					if ( index == -1 )
					{ // don't reset unless we have the default knife (returns -1)
						if ( g_ctx.m_local->m_iTeamNum() == TEAM_TERRORISTS )
							weapon->m_iItemDefinitionIndex() = KNIFE_T;

						else if ( g_ctx.m_local->m_iTeamNum() == TEAM_COUNTERTERRORISTS )
							weapon->m_iItemDefinitionIndex() = KNIFE_CT;

						weapon->m_iEntityQuality() = 0;
						weapon->m_nFallbackPaintKit() = -1;
						weapon->m_nFallbackStatTrak() = -1;
					}
				}
			}

			if ( index == -1 )
				continue;

			auto weapon_info = m_weapon_name_map[ index ];

			if ( weapon_info.set_paintkit > 0 )
			{
				weapon->m_iItemIDHigh() = -1;
				weapon->m_iAccountID() = player_info.m_xuid_low;
				weapon->m_nFallbackPaintKit() = weapon_info.set_paintkit;
				weapon->m_flFallbackWear() = FLT_EPSILON;
			}

			if ( m_update && g_csgo.m_globals->m_curtime >= m_update_time && !g_GUI.m_is_open )
			{
				UpdateItem( weapon );
			}
		}

		if ( m_update && g_csgo.m_globals->m_curtime >= m_update_time && !g_GUI.m_is_open )
		{
			m_update = false;
			m_update_time = g_csgo.m_globals->m_curtime + 1.f;
		}

		// knife-specific world model stuff
		if ( !m_update )
		{
			Weapon* view_model = g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >( g_ctx.m_local->m_hViewModel() );

			if ( !view_model )
				return;

			Weapon* view_model_weapon = view_model->GetWeapon();

			if ( !view_model_weapon )
				return;

			if ( IsWeaponKnife( view_model_weapon->m_iItemDefinitionIndex() ) )
			{
				// should barely happen unless you pick up someones knife.
				// possible on servers with 'mp_drop_knife_enable 1'.
				if ( player_info.m_xuid_low != view_model_weapon->m_OriginalOwnerXuidLow() || player_info.m_xuid_high != view_model_weapon->m_OriginalOwnerXuidHigh() )
					return;

				// get and validate world model handle from the weapon
				Weapon* weapon_world_model = view_model_weapon->GetWeaponWorldModel();
				if ( !weapon_world_model )
					return;

				// set model index of the predicted viewmodel.
				view_model->m_nModelIndex() = knife->m_model_index;

				// set the world model index.
				// do this to have knifechanger in third person. verry p.
				weapon_world_model->m_nModelIndex() = knife->m_world_model_index;

				// correct m_nSequence and m_flCycle.
				UpdateAnimations( view_model );
			}
		}
	}
}

void Skins::Init()
{
	constexpr int DESYNC_MAX = 1264;
	constexpr int LEGACY_MAX = 706;
	constexpr int LEGACY_GLOVES = 10064;
	constexpr int DESYNC_GLOVES = 10089;
	//m_desync = true;
	
	const auto map_head = reinterpret_cast< Head_t< int, CPaintKit * >* >( std::uintptr_t( g_csgo.m_item_schema ) + 0x28C );
	const auto V_UCS2ToUTF8 = reinterpret_cast<int( * )( const wchar_t* ucs2, char* utf8, int len )>( GetProcAddress( GetModuleHandleA( XorStr( "vstdlib.dll" ) ), XorStr( "V_UCS2ToUTF8" ) ) );
	for ( auto i = 0; i <= map_head->last_element; ++i )
	{
		const auto kit = map_head->memory[ i ].value;

		if ( kit->id == 9001 )
			continue;

		#ifdef _DEBUG
			const wchar_t* n = g_csgo.m_localize->Find( kit->des_tag.buf );
			std::string final = util::WideToMultiByte( n );
			#else
				const wchar_t* n = g_csgo.m_localize->Find( kit->des_tag.buf );
				char name[ 256 ];
				V_UCS2ToUTF8( n, name, sizeof( name ) );
				std::string final = name;
		#endif

		std::transform( final.begin(), final.end(), final.begin(), ::tolower ); // F*ck you Xyron

		if ( kit->id < 10006 )
		{
			std::strcpy( saved_kits[ kit->id ].name, final.c_str() );
			saved_kits[ kit->id ].fill( kit->col[ 0 ], kit->col[ 1 ], kit->col[ 2 ], kit->col[ 3 ] );
		}
		else
			std::strcpy( saved_gloves[ kit->id - 10006 ], final.c_str() );
	}

	m_finished = true;
}

void Skins::UpdateItem( Weapon* item )
{
	if ( !item || !item->IsBaseCombatWeapon() )
		return;

	if ( g_csgo.m_cl->m_delta_tick == -1 )
		return;

	item->m_bCustomMaterialInitialized() = item->m_nFallbackPaintKit() <= 0;

	item->m_CustomMaterials().RemoveAll();
	item->m_CustomMaterials2().RemoveAll();

	size_t count = item->m_VisualsDataProcessors().Count();
	for ( size_t i{}; i < count; ++i )
	{
		auto& elem = item->m_VisualsDataProcessors()[ i ];
		if ( elem )
		{
			elem->unreference();
			elem = nullptr;
		}
	}

	item->m_VisualsDataProcessors().RemoveAll();

	item->PostDataUpdate( DATA_UPDATE_CREATED );
	item->OnDataChanged( DATA_UPDATE_CREATED );

	CHudElement* SFWeaponSelection = g_csgo.m_hud->FindElement( HASH( "SFWeaponSelection" ) );
	g_csgo.ShowAndUpdateSelection( SFWeaponSelection, 0, item, false );
}

void Skins::UpdateAnimations( Entity* ent )
{
	int knife = g_Variables.m_ints[ "knife_index" ];

	int seq = ent->m_nSequence();

	// check if this knife needs extra fixing.
	if ( knife == knives_t::BUTTERFLY || knife == knives_t::FALCHION || knife == knives_t::DAGGER || knife == knives_t::BOWIE )
	{
		// fix the idle sequences.
		if ( seq == sequence_default_idle1 || seq == sequence_default_idle2 )
		{
			// set the animation to be completed.
			ent->m_flCycle() = 0.999f;

			// cycle change, re-render.
			ent->InvalidatePhysicsRecursive( ANIMATION_CHANGED );
		}
	}

	// fix sequences.
	if ( m_last_seq != seq )
	{
		if ( knife == knives_t::BUTTERFLY )
		{
			switch ( seq )
			{
			case sequence_default_draw:
				seq = g_csgo.RandomInt( sequence_butterfly_draw, sequence_butterfly_draw2 );
				break;

			case sequence_default_lookat01:
				seq = g_csgo.RandomInt( sequence_butterfly_lookat01, sequence_butterfly_lookat03 );
				break;

			default:
				seq++;
				break;
			}
		}

		else if ( knife == knives_t::FALCHION )
		{
			switch ( seq )
			{
			case sequence_default_draw:
			case sequence_default_idle1:
				break;

			case sequence_default_idle2:
				seq = sequence_falchion_idle1;
				break;

			case sequence_default_heavy_miss1:
				seq = g_csgo.RandomInt( sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip );
				break;

			case sequence_default_lookat01:
				seq = g_csgo.RandomInt( sequence_falchion_lookat01, sequence_falchion_lookat02 );
				break;
			}
		}

		else if ( knife == knives_t::DAGGER )
		{
			switch ( seq )
			{
			case sequence_default_idle2:
				seq = sequence_push_idle1;
				break;

			case sequence_default_heavy_hit1:
			case sequence_default_heavy_backstab:
			case sequence_default_lookat01:
				seq += 3;
				break;

			case sequence_default_heavy_miss1:
				seq = sequence_push_heavy_miss2;
				break;
			}
		}

		else if ( knife == knives_t::BOWIE )
		{
			if ( seq > sequence_default_idle1 )
				seq--;
		}

		m_last_seq = seq;
	}

	// write back fixed sequence.
	ent->m_nSequence() = seq;
}