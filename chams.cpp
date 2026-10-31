#include "includes.h"

Chams g_chams{ };;

void Chams::SetColor( int material, Color col )
{
	if ( material > -1 )
		m_materials[ material ]->ColorModulate(col);

	else
		g_csgo.m_render_view->SetColorModulation( col );
}

void Chams::SetAlpha( int material, float alpha )
{
	if ( material > -1 )
		m_materials[ material ]->AlphaModulate( alpha );

	else
		g_csgo.m_render_view->SetBlend( alpha );
}

void Chams::SetupMaterial( int mat, Color col, bool z_flag )
{
	SetColor( mat, col );
	SetAlpha( mat, col.a() / 255.f );

	if ( mat == -2 )
		return;

	// mat->SetFlag( MATERIAL_VAR_HALFLAMBERT, flags );
	if ( mat == -1 )
	{
		g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
		return;
	}

	m_materials[ mat ]->SetFlag( MATERIAL_VAR_ZNEARER, z_flag );
	m_materials[ mat ]->SetFlag( MATERIAL_VAR_NOFOG, z_flag );
	m_materials[ mat ]->SetFlag( MATERIAL_VAR_IGNOREZ, z_flag );

	bool doalpha = false;
	switch ( m_draw_side )
	{
		case drawside_t::local: doalpha = g_Variables.m_bools[ "chams_local_alpha_phong" ]; break;
		case drawside_t::enemy: doalpha = g_Variables.m_bools[ "chams_enemy_alpha_phong" ]; break;
		case drawside_t::friendly: doalpha = g_Variables.m_bools[ "chams_friendly_alpha_phong" ]; break;
	}

	if ( mat == materialtype_t::shine )
	{
		if ( !doalpha )
		{ // ghetto fix but idc
			mat++;

			m_materials[ mat ]->SetFlag( MATERIAL_VAR_ZNEARER, z_flag );
			m_materials[ mat ]->SetFlag( MATERIAL_VAR_NOFOG, z_flag );
			m_materials[ mat ]->SetFlag( MATERIAL_VAR_IGNOREZ, z_flag );

			SetColor( mat, col );
			SetAlpha( mat, col.a() / 255.f );
		}

		auto reflectivity = m_materials[ mat ]->FindVar( "$envmaptint", nullptr );
		if ( reflectivity )
		{
			Color col = colors::white;
			float ratio = 1.f;

			switch ( m_draw_side )
			{
			case drawside_t::local:
				col = g_Variables.m_colors[ "chams_local_shine_color" ];
				ratio = col.a() / 255.f;
				break;
			case drawside_t::enemy:
				col = g_Variables.m_colors[ "chams_enemy_shine_color" ];
				ratio = col.a() / 255.f;
				break;
			case drawside_t::friendly:
				col = g_Variables.m_colors[ "chams_friendly_shine_color" ];
				ratio = col.a() / 255.f;
				break;
			}

			reflectivity->SetVecValue(
				( col.r() / 255.f ) * ratio,
				( col.g() / 255.f ) * ratio,
				( col.b() / 255.f ) * ratio );
		}

		if ( mat == materialtype_t::shine2 )
		{
			auto phongexponent = m_materials[ mat ]->FindVar( "$phongexponent", nullptr );
			if ( phongexponent )
			{
				int amt = 0;

				switch ( m_draw_side )
				{
				case drawside_t::local: amt = g_Variables.m_ints[ "chams_local_phong_amount" ]; break;
				case drawside_t::enemy: amt = g_Variables.m_ints[ "chams_enemy_phong_amount" ]; break;
				case drawside_t::friendly: amt = g_Variables.m_ints[ "chams_friendly_phong_amount" ]; break;
				}

				phongexponent->SetIntValue( amt );
			}
		}
	}

	g_csgo.m_studio_render->ForcedMaterialOverride( m_materials[ mat ] );
}

void Chams::init()
{
	std::ofstream( "csgo\\materials\\shine.vmt" ) << ( R"#("VertexLitGeneric"{
		"$basetexture" "vgui/white_additive"
		"$phong" "1"
		"$phongexponent" "15"
		"$basemapalphaphongmask" "1"
		"basemapluminancephongmask" "1"
		"$normalmapalphaenvmask" "1"
		"$envmap" "editor/cube_vertigo"
		"$nofog" "1"
		"$ingorez" "1"
		"$model" "1"
		"$nocull" "0"
		"$selfillum" "1"
		"$halflambert" "1"
		"$znearer" "0"
		"$flat" "1"
	})#" );

	std::ofstream( "csgo\\materials\\shine2.vmt" ) << ( R"#("VertexLitGeneric"{
		"$basetexture" "vgui/white_additive"
		"$phong" "1"
		"$phongexponent" "15"
		"$phongboost" "0.4"
		"$normalmapalphaenvmask" "1"
		"$envmap" "editor/cube_vertigo"
		"$nofog" "1"
		"$ingorez" "1"
		"$model" "1"
		"$nocull" "0"
		"$selfillum" "1"
		"$halflambert" "1"
		"$znearer" "0"
		"$flat" "1"
	})#" );

	// find stupid materials.
	m_materials[ materialtype_t::textured ] = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugambientcube" ), XOR( "Model textures" ) );
	m_materials[ materialtype_t::textured ]->IncrementReferenceCount();

	m_materials[ materialtype_t::flat ] = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugdrawflat" ), XOR( "Model textures" ) );
	m_materials[ materialtype_t::flat ]->IncrementReferenceCount();

	m_materials[ materialtype_t::shine ] = g_csgo.m_material_system->FindMaterial( XOR( "shine" ), XOR( "Model textures" ) );
	m_materials[ materialtype_t::shine ]->IncrementReferenceCount();

	m_materials[ materialtype_t::shine2 ] = g_csgo.m_material_system->FindMaterial( XOR( "shine2" ), XOR( "Model textures" ) );
	m_materials[ materialtype_t::shine2 ]->IncrementReferenceCount();
}

bool Chams::GenerateLerpedMatrix( int index, BoneArray* out, float& alphamod )
{
	LagRecord* current_record;
	AimPlayer* data;

	data = &g_aimbot.m_players[ index - 1 ];
	if ( !data || data->m_records.size() < 2 || data->m_went_dormant )
		return false;

	LagRecord* last_valid = nullptr;
	LagRecord* first_invalid = nullptr;

	bool had_valid = false;

	for ( int i = 0; i < data->m_records.size() - 1; i++ )
	{
		if ( data->m_records[ i ].get()->m_broke_lc )
		{
			if ( i > 0 )
			{
				last_valid = data->m_records[ i - 1 ].get();
				first_invalid = data->m_records[ i ].get();
			}
			break;
		}

		if ( !had_valid )
		{
			if ( data->m_records[ i ].get()->valid() )
				had_valid = true;
			else
				continue;
		}

		if ( !data->m_records[ i + 1 ].get()->valid() )
		{
			last_valid = data->m_records[ i ].get();
			first_invalid = data->m_records[ i + 1 ].get();
			break;
		}
	}

	if ( !last_valid || !first_invalid || last_valid->m_broke_lc )
		return false;

	if ( (last_valid->m_origin - first_invalid->m_origin).length() > 4096.f )
		return false;

	auto flDelta = 1.f - ( g_csgo.m_globals->m_curtime - last_valid->m_interp_time ) / ( last_valid->m_sim_time - first_invalid->m_sim_time );
	if ( flDelta < 0.f || flDelta > 1.f )
	{
		last_valid->m_interp_time = g_csgo.m_globals->m_curtime;
		flDelta = 1.f - ( g_csgo.m_globals->m_curtime - last_valid->m_interp_time ) / ( last_valid->m_sim_time - first_invalid->m_sim_time );
	}

	const auto lerp = math::Interpolate( last_valid->m_origin, first_invalid->m_origin, std::clamp( flDelta, 0.f, 1.f ) );

	constexpr float max_dist = 30.f;
	alphamod = std::min( ( lerp - last_valid->m_player->GetAbsOrigin() ).length(), max_dist ) / max_dist;

	memcpy( out, first_invalid->m_bones, sizeof( BoneArray ) * 128 );

	for ( int i = 0; i < 128; i++ )
	{
		const auto matrix_delta = first_invalid->m_bones[ i ].GetOrigin() - first_invalid->m_origin;
		out[ i ].SetOrigin( matrix_delta + lerp );
	}

	return true;
}

ShaderStencilState_t GenerateStencilState( bool ignorez, bool disable = false )
{
	ShaderStencilState_t state;

	if ( disable )
		return state;

	if ( !ignorez )
	{
		state.m_bEnable = true;
		state.m_CompareFunc = SHADER_STENCILFUNC_ALWAYS;
		state.m_PassOp = SHADER_STENCILOP_SET_TO_REFERENCE;
		state.m_FailOp = SHADER_STENCILOP_KEEP;
		state.m_ZFailOp = SHADER_STENCILOP_KEEP;
		state.m_nReferenceValue = 1;
		state.m_nWriteMask = 0xFF;
		state.m_nTestMask = 0x0;
		return state;
	}

	state.m_bEnable = true;
	state.m_CompareFunc = SHADER_STENCILFUNC_EQUAL;
	state.m_PassOp = SHADER_STENCILOP_KEEP;
	state.m_FailOp = SHADER_STENCILOP_KEEP;
	state.m_ZFailOp = SHADER_STENCILOP_KEEP;
	state.m_nReferenceValue = 0;
	state.m_nWriteMask = 0x0;
	state.m_nTestMask = 0xFF;
	return state;
} // currently causes some fps shite not botherin

Entity* GetMoveParentEntity( Entity* pEntity )
{
	if ( !pEntity )
		return false;

	if ( !pEntity->moveparent().IsValid() )
		return false;

	return g_csgo.m_entlist->GetClientEntityFromHandle( pEntity->moveparent() );
}

bool AttachmentBelongsToPlayer( Entity* pEntity, Entity* pOwnerEntity )
{
	if ( !pEntity )
		return false;

	if ( !pOwnerEntity )
		return false;

	auto pAttachmentParent = GetMoveParentEntity( pEntity );
	if ( !pAttachmentParent )
		return false;

	return pAttachmentParent->index() == pOwnerEntity->index();
}

bool Chams::OnDrawModelExecute( Player* ent, void* ecx, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld )
{
	if (!g_ctx.m_local)
		return false;

	int id = ent->GetClientClass()->m_ClassID;

	if ( id != _CEconWearable ) // EconWearables are permanently dormant
	{
		if ( ent->dormant() )
			return false;
	}

	auto restore = [ ]() -> void
	{
			g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
			g_csgo.m_render_view->SetColorModulation( colors::white );
			g_csgo.m_render_view->SetBlend( 1.f ); // dont even know if we need to do this
	};

	bool blend = ( g_Variables.m_bools[ "local_blend_scope" ] && g_ctx.m_local->m_bIsScoped() ) || ( g_Variables.m_bools[ "local_blend_grenade" ] && g_ctx.m_weapon_type == WEAPONTYPE_GRENADE );
	if ( blend )
	{
		if ( AttachmentBelongsToPlayer( ent, g_ctx.m_local ) )
		{
			//Color col( 255, 255, 255, g_Variables.m_floats[ "local_blend_amt" ] * 0.01f );
			//SetupMaterial( -2, col, false );

			g_csgo.m_render_view->SetBlend( g_Variables.m_floats[ "local_blend_amt" ] * 0.01f );

			oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

			restore();

			return true;
		}
	}

	// Check if the model is a glove
	if ( id == _CEconWearable )
	{
		bool should_color_gloves = g_Variables.m_bools[ "local_override_gloves" ] && g_Variables.m_bools[ "chams_local" ];
		if ( blend || should_color_gloves )
		{
			const auto parent = ent->GetShadowParent();
			const auto parent_unknown = parent != nullptr ? parent->GetIClientUnknown() : nullptr;
			const auto owner = parent_unknown != nullptr ? reinterpret_cast<Player* const>( parent_unknown->GetIClientEntity() ) : nullptr;
			if ( owner && owner->m_bIsLocalPlayer() )
			{
				if ( blend ) // idk why we didnt write the other one like this but whatever lol
							// i realize now why but im too lazy to fix it;
					        // we wrote it the other way in order for the alpha to scale with transparency on the slider
					g_csgo.m_render_view->SetBlend( g_Variables.m_floats[ "local_blend_amt" ] * 0.01f );
				else
				{
					SetupMaterial( g_Variables.m_ints[ "chams_local_material" ], g_Variables.m_colors[ "chams_local_color" ], false);
				}

				oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

				restore();

				return true;
			}
		}
	}

	if ( g_csgo.m_model_render->IsForcedMaterialOverride() )
		return false;

	switch ( id )
	{
	case _CCSPlayer:
	{
		if ( !ent->alive() )
			return false;

		if ( ent->m_bIsLocalPlayer() )
		{
			m_draw_side = drawside_t::local;

			int mat = -1;
			Color col = colors::white;

			if ( g_Variables.m_bools[ "chams_local" ] )
			{
				mat = g_Variables.m_ints[ "chams_local_material" ];
				col = g_Variables.m_colors[ "chams_local_color" ];
				if ( blend )
				{
					if ( g_Variables.m_bools[ "disable_chams_on_blend" ] )
						mat = -1;

					if ( mat > -1 )
						col.a() *= ( g_Variables.m_floats[ "local_blend_amt" ] * 0.01f ); // we want it to be a percentage of the current alpha
					else
						col = Color( 255, 255, 255, 255 * ( g_Variables.m_floats[ "local_blend_amt" ] * 0.01f ) );

				}
			}
			else if ( blend )
			{
				int amt = 255 * ( g_Variables.m_floats[ "local_blend_amt" ] * 0.01f );
				col.a() = amt;
			}

			SetupMaterial( mat, col, false );

			oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

			restore();

			if ( g_Variables.m_bools[ "chams_local_lby" ] && g_csgo.m_globals->m_curtime > last_hidden_time && g_csgo.m_globals->m_curtime - last_hidden_time < .5f )
			{
				Color col = g_Variables.m_colors[ "chams_local_lby_color" ];

				float delta = 4.f * ( g_csgo.m_globals->m_curtime - last_hidden_time );
				float alpha = delta <= 1.f ? delta * std::min( (int)col.a(), 255 )
					: fabsf( 2.f - delta ) * std::min( (int)col.a(), 255 ); // i am a psychopath

				col.a() = alpha;
				SetupMaterial( materialtype_t::flat, col, false );

				oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, local_hidden );

				restore();
			}

			return true; // always return true here so that blending works...
		}
		else if ( ent->visual_enemy( g_ctx.m_local ) )
			{

				if ( g_Variables.m_bools[ "chams_enemy_history" ] )
				{
					BoneArray out[ 128 ];
					float alpha_mod = 1.f;

					if ( GenerateLerpedMatrix( ent->index(), out, alpha_mod ) )
					{
						Color col = g_Variables.m_colors[ "chams_enemy_history_color" ];
						col.a() *= alpha_mod;
						SetupMaterial( materialtype_t::flat, col, true );

						oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, out );

						restore(); // dont return, we might want to see the original model with bt still
					}
				}

				if ( g_Variables.m_bools[ "chams_enemy" ] )
				{
					m_draw_side = drawside_t::enemy;

					if ( g_Variables.m_bools[ "chams_enemy_occluded" ] )
					{
						SetupMaterial( g_Variables.m_ints[ "chams_enemy_material" ], g_Variables.m_colors[ "chams_enemy_occluded_color" ], true );

						oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

						restore();
					}

					SetupMaterial( g_Variables.m_ints[ "chams_enemy_material" ], g_Variables.m_colors[ "chams_enemy_color" ], false );

					oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

					restore();
					return true;
				}
			}
		else // teammates
		{
			if ( g_Variables.m_bools[ "chams_friendly" ] )
			{
				m_draw_side = drawside_t::friendly;
				if ( g_Variables.m_bools[ "chams_friendly_occluded" ] )
				{
					SetupMaterial( g_Variables.m_ints[ "chams_friendly_material" ], g_Variables.m_colors[ "chams_friendly_occluded_color" ], true );

					oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );
				}

				SetupMaterial( g_Variables.m_ints[ "chams_friendly_material" ], g_Variables.m_colors[ "chams_friendly_color" ], false );

				oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

				restore();
				return true;
			}
		}
		break;
	}
	case _CCSRagdoll:
	{
		if ( ent->enemy( g_ctx.m_local ) && g_Variables.m_bools[ "chams_enemy_ragdoll" ] )
		{
			m_draw_side = drawside_t::enemy;
			if ( g_Variables.m_bools[ "chams_enemy_ragdoll_occluded" ] )
			{

				SetupMaterial( g_Variables.m_ints[ "chams_enemy_material" ], g_Variables.m_colors[ "chams_enemy_ragdoll_occluded_color" ], true );

				oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

				restore();
			}

			SetupMaterial( g_Variables.m_ints[ "chams_enemy_material" ], g_Variables.m_colors[ "chams_enemy_ragdoll_occluded_color" ], false );

			oDrawModelExecute( ecx, MatRenderContext, DrawModelState, RenderInfo, pCustomBoneToWorld );

			restore();

			return true;
		}
		return false;
	}
	default:
		return false;
	}

	return false;
}