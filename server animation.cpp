#include "includes.h"

ServerAnimations g_server_animations{ };

void Player::ModifyEyePosition(CCSGOPlayerAnimState* state, vec3_t* pos)
{
	if (!state)
	{
		return;
	}
	//  if ( *(this + 0x50) && (*(this + 0x100) || *(this + 0x94) != 0.0 || !sub_102C9480(*(this + 0x50))) )
	if (state->m_player &&
		(state->m_landing || state->m_player->m_flDuckAmount() != 0.f || !state->m_player->GetGroundEntity()))
	{
		if (state->m_player->m_BoneCache().m_pCachedBones)
		{
			vec3_t head_pos(
				state->m_player->m_BoneCache().m_pCachedBones[8][0][3],
				state->m_player->m_BoneCache().m_pCachedBones[8][1][3],
				state->m_player->m_BoneCache().m_pCachedBones[8][2][3]);
			if (state->m_player->m_bIsLocalPlayer()) { // fuckin hell
				head_pos =
					vec3_t(g_server_animations.local_matrix[8][0][3],
						g_server_animations.local_matrix[8][1][3],
						g_server_animations.local_matrix[8][2][3]);
			}

			auto v12 = head_pos;
			auto v7 = v12.z + 1.7;

			auto v8 = pos->z;
			if (v8 > v7) // if (v8 > (v12 + 1.7))
			{
				float v13 = 0.f;
				float v3 = (*pos).z - v7;

				float v4 = (v3 - 4.f) * 0.16666667;
				if (v4 >= 0.f)
					v13 = std::fminf(v4, 1.f);

				(*pos).z = (((v7 - (*pos).z)) * (((v13 * v13) * 3.0) - (((v13 * v13) * 2.0) * v13))) + (*pos).z;
			}
		}
	}
}

using fnAddStringFindString = __int16* (__thiscall*)(void* this_ptr, __int16* symbol, const char* string);
__int16* CUtlSymbolTable_t::find(__int16* symbol, const char* lookup)
{
	static auto add_string = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 56 8B F1 8B 4D ? 85 C9 75 ? 8B 45"));
	return add_string.as< fnAddStringFindString >()(this, symbol, lookup);
}

__int16* CUtlSymbolTable_t::add_string(__int16* symbol, const char* lookup)
{
	static auto add_string = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 0C 53 8B 5D 0C 57")).as<fnAddStringFindString>();

	return add_string(this, symbol, lookup);
}

uint32_t SequenceHash::find_hash(hash_value_type* value) {
	static auto find_hash = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 08 8D 45 FC C7 45 FC 00 00 00 00 50 8D"));
	return find_hash.as< uint32_t(__thiscall*)(void*, hash_value_type*) >()(this, value);
}

void CActivityToSequenceMapping::reinitialize(CStudioHdr* hdr)
{
	static auto reinitialize = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 56 8B F1 8B 06 85 C0"));
	reinitialize.as< void(__thiscall*)(void*, CStudioHdr*) >()(this, hdr);
}

bool CActivityToSequenceMapping::ValidateAgainst(const CStudioHdr* pstudiohdr)
{
	static auto emptyMapping = pattern::find(g_csgo.m_client_dll, XOR("8B 15 ? ? ? ? A1 ? ? ? ? 52")).add(0x2).as< CActivityToSequenceMapping* >();

	return (this == emptyMapping || (studio_hdr == pstudiohdr->m_pStudioHdr && v_model == pstudiohdr->GetVirtualModel()));
}

void AddActivityModifier(const char* szName)
{
	if (szName == NULL)
		return;

	char szLookup[32];
	std::strcpy(szLookup, szName);
	static CUtlSymbolTable_t* modifier = *pattern::find(g_csgo.m_client_dll, "B9 ? ? ? ? 50 E8 ? ? ? ? 8B 4F").add(0x1).as< CUtlSymbolTable_t** >();
	if (!modifier)
		return;

	auto sym = modifier->find_or_insert(szLookup);
	if (sym != 0xFFFF)
		g_server_animations.m_act_mods.push_back(sym);
}

void ServerAnimations::build_activity_modifiers( CCSGOPlayerAnimState* state ) 
{
	m_act_mods.clear();

	AddActivityModifier( state->GetWeaponPrefix() );

	if ( state->m_speed_as_portion_of_run_top_speed > 0.25f )
		AddActivityModifier( "moving" );

	if ( state->m_anim_duck_amount > 0.55000001f )
		AddActivityModifier( "crouch" );
}

void ServerAnimations::SetLayerSequence( CCSGOPlayerAnimState* state, C_AnimationLayer* layer, int sequence ) 
{
	if ( !state || !layer )
		return;

	if ( sequence < 2 )
		return;

	layer->m_cycle = 0.0f;
	layer->m_weight = 0.0f;
	layer->m_sequence = sequence;
	layer->m_playback_rate = state->m_player->GetLayerSequenceCycleRate( layer, sequence );
	state->UpdateLayerOrderPreset( sequence );
}

typedef CActivityToSequenceMapping* (__thiscall* fnFindMapping)(void*);

void VerifySequenceIndex( CStudioHdr* pHdr )
{
	//57 8B F9 85 FF 74 6E
	typedef void( __thiscall* HelooFn )( void* );
	static auto pat = pattern::find(g_csgo.m_client_dll, "57 8B F9 85 FF 74 6E").as<HelooFn>();

	return pat( pHdr );
}

int ServerAnimations::SelectSequenceFromActMods( int32_t activity )
{
	const auto hdr = g_ctx.m_local->GetModelPtr(); // only gonna use on local anyways...
	if (!hdr || !hdr->SequencesAvailable())
	{
		return -1;
	}

	VerifySequenceIndex(hdr); // https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/animation.cpp#L280
	// we dont really need verify tho tbh

	if (hdr->GetNumSeq() == 1)
	{
		return (g_ctx.m_local->GetSequenceActivity(0) == activity) ? 0 : -1;
	}

	static fnFindMapping findmapping = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 57 8B F9 8B 17").as< fnFindMapping >();

	const auto mapping = reinterpret_cast<fnFindMapping>(findmapping)(hdr);
	if (!hdr->m_ActivityToSequence) {
		hdr->m_ActivityToSequence = mapping;
	}

	if (!mapping->ValidateAgainst(hdr)) // https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/animation.cpp#L451
		mapping->reinitialize(hdr);

	if (!mapping->sequence_tuples)
		return -1;

	hash_value_type dummy{ activity, 0, 0, 0 };
	const uint32_t handle = mapping->act_to_sequence_hash.find_hash(&dummy);
	const int32_t data_index = handle & 0xFFFF;
	const int32_t bucket_index = (handle >> 16) & 0xFFFF;

	if (bucket_index >= mapping->act_to_sequence_hash.buckets.Count()
		|| data_index >= mapping->act_to_sequence_hash.buckets[bucket_index].Count()) // https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/public/tier1/utlhash.h#L129
		return -1;

	const hash_value_type* actData = &mapping->act_to_sequence_hash.buckets[bucket_index][data_index];

	int top_score = -1;
	std::vector<int> topScoring;
	topScoring.reserve(actData->count);
	for (int i = 0; i < actData->count; i++)
	{
		CActivityToSequenceMapping::SequenceTuple* sequenceInfo = &mapping->sequence_tuples[actData->starting_index + i];
		int score = 0;

		int num_seq_modifiers = sequenceInfo->count_activity_modifiers;
		for (int k = 0; k < num_seq_modifiers; k++)
		{
			bool bFoundMod = false;

			for (int m = 0; m < m_act_mods.size(); m++)
			{
				if (sequenceInfo->activity_modifiers[k] == m_act_mods[m])
				{
					bFoundMod = true;
				}
			}

			if (bFoundMod)
			{
				score += 2;
			}
			else
			{
				score--;
			}
		}

		if (score >= top_score)
		{
			if (score > top_score)
			{
				topScoring.clear();
			}

			for (int n = 0; n < sequenceInfo->weight; n++)
			{
				topScoring.push_back(sequenceInfo->seqnum);
			}

			top_score = score;
		}
	}

	if (topScoring.size())
	{
		return topScoring[g_csgo.RandomInt(0, topScoring.size() - 1)];
	}

	return -1;
}

void CCSGOPlayerAnimState::IncrementLayerCycle( int nLayerIndex, bool bAllowLoop )
{
	auto pLayer = &m_player->m_AnimOverlay()[ nLayerIndex ];
	if ( !pLayer )
		return;

	if ( abs( pLayer->m_playback_rate ) <= 0 )
		return;

	float flCurrentCycle = pLayer->m_cycle;
	flCurrentCycle += m_last_update_increment * pLayer->m_playback_rate;

	if ( !bAllowLoop && flCurrentCycle >= 1 )
		flCurrentCycle = 0.999f;

	pLayer->m_cycle = math::clamp_cycle( flCurrentCycle );
}

mstudioseqdesc_t* get_seq_desc( CStudioHdr* hdr, const int index )
{
	if ( !hdr->m_pVModel )
		return hdr->m_pStudioHdr->get_local_seqdesc( index );

	static auto get_seq = pattern::find( g_csgo.m_client_dll, "55 8B EC 83 79 04 00 75 25 8B 45 08 8B 09 85 C0 78 08 3B 81 ? ? ? ? 7C 02 33 C0 69 C0 ? ? ? ? 03" ).as<mstudioseqdesc_t * ( __thiscall* )( void*, int32_t )>();

	return get_seq( hdr, index );
}

float CCSGOPlayerAnimState::GetLayerIdealWeightFromSeqCycle( int nLayerIndex )
{
	//MDLCACHE_CRITICAL_SECTION();
	auto pLayer = &m_player->m_AnimOverlay()[ nLayerIndex ];
	if ( !pLayer )
		return 0.f;

	mstudioseqdesc_t& seqdesc = *get_seq_desc( m_player->GetModelPtr(), pLayer->m_sequence );

	float flCycle = pLayer->m_cycle;
	if ( flCycle >= 0.999f )
		flCycle = 1;
	float flEaseIn = seqdesc.fade_in_time;
	float flEaseOut = seqdesc.fade_out_time;
	float flIdealWeight = 1;

	if ( flEaseIn > 0 && flCycle < flEaseIn )
	{
		flIdealWeight = math::SmoothStepBounds( 0, flEaseIn, flCycle );
	}
	else if ( flEaseOut < 1 && flCycle > flEaseOut )
	{
		flIdealWeight = math::SmoothStepBounds( 1.0f, flEaseOut, flCycle );
	}

	if ( flIdealWeight < 0.0015f )
		return 0;

	return std::clamp( flIdealWeight, 0.f, 1.f );
}

void CCSGOPlayerAnimState::IncrementLayerCycleWeightRateGeneric( int nLayerIndex )
{
	float flWeightPrevious = m_player->m_AnimOverlay()[ nLayerIndex ].m_weight;
	IncrementLayerCycle( nLayerIndex, false );
	m_player->m_AnimOverlay()[ nLayerIndex ].m_weight = GetLayerIdealWeightFromSeqCycle( nLayerIndex );
	m_player->m_AnimOverlay()[ nLayerIndex ].m_weight_delta_rate = ( m_player->m_AnimOverlay()[ nLayerIndex ].m_weight - flWeightPrevious ) / m_last_update_increment;
}

bool CCSGOPlayerAnimState::IsLayerSequenceCompleted( int nLayerIndex )
{
	auto pLayer = &m_player->m_AnimOverlay()[ nLayerIndex ];
	if ( pLayer )
		return ( m_last_update_increment * pLayer->m_playback_rate ) + pLayer->m_cycle >= 1.0f;

	return false;
}

float GetAnySequenceAnimTag( CStudioHdr* pstudiohdr, int sequence, int nDesiredTag, float flDefault )
{
	if ( !pstudiohdr || sequence >= pstudiohdr->GetNumSeq() )
		return flDefault;

	mstudioseqdesc_t* seqdesc = get_seq_desc( pstudiohdr, sequence );
	if ( seqdesc->num_anim_tags == 0 )
		return flDefault;

	studioanimtag_t* panimtag = NULL;

	int index;
	for ( index = 0; index < (int)seqdesc->num_anim_tags; index++ )
	{
		panimtag = seqdesc->GetAnimTag( index );

		if ( panimtag->tag == ANIMTAG_INVALID )
			continue;

		if ( panimtag->tag == ANIMTAG_UNINITIALIZED )
		{
			panimtag->tag = panimtag->IndexFromAnimTagName( panimtag->TagName() );
		}

		if ( panimtag->tag == nDesiredTag )
		{
			return panimtag->cycle;
		}
	}

	return flDefault;
}

void ApplyLayerOrderPreset( CCSGOPlayerAnimState* state, int* nNewPreset, bool bForce = false )
{
	if ( !bForce && state->m_layer_order_preset == nNewPreset )
		return;

	state->m_layer_order_preset = nNewPreset;

	for ( int i = 0; i < ANIMATION_LAYER_COUNT; i++ )
	{
		C_AnimationLayer* pLayer = &state->m_player->m_AnimOverlay()[ state->m_layer_order_preset[ i ] ];
		if ( pLayer )
		{
			pLayer->m_order = i;

			// purge dispatch info too
			pLayer->m_flags = NULL;
			pLayer->m_activty = ACT_INVALID;
			pLayer->m_priority = ACT_INVALID;
		}
	}
}

void CCSGOPlayerAnimState::UpdateLayerOrderPreset( int nSequence )
{
	if ( !this->m_player || !g_server_animations.should_override_order ) // this fixes the weapon legs sliding stuff... 
																		 // in the game sourrce it compares the layer index to only run on WEAPON_ACTION, but im lazy so i just made a variable which gets set to true on weapon layers
																		 // so that it achieves the same functionality
		return;

	g_csgo.m_model_cache->BeginLock();

	if ( GetAnySequenceAnimTag( this->m_player->GetModelPtr(), nSequence, ANIMTAG_WEAPON_POSTLAYER, 0 ) != 0 )
	{
		ApplyLayerOrderPreset( this, GETANIMLAYERPRESET( WeaponPost ) );
	}
	else
	{
		ApplyLayerOrderPreset( this, GETANIMLAYERPRESET( Default ) );
	}
	g_csgo.m_model_cache->EndLock();
}

#define CSGO_ANIM_DEPLOY_RATELIMIT 0.15f
bool bDeployRatelimit = false;
bool bJumping = false;
bool m_bPlantAnimStarted = false;
void ServerAnimations::DoAnimationEvent( CCSGOPlayerAnimState* pState, int animEvent, C_AnimationLayer* pLayers )
{
	build_activity_modifiers( pState );
	WeaponInfo* data = pState->m_player && pState->m_player->GetActiveWeapon() ? pState->m_player->GetActiveWeapon()->GetWpnData() : nullptr;

	switch ( animEvent )
	{
	case PLAYERANIMEVENT_THROW_GRENADE_UNDERHAND:
	case PLAYERANIMEVENT_FIRE_GUN_PRIMARY:
	case PLAYERANIMEVENT_THROW_GRENADE:
	{

		if ( pState->m_weapon && animEvent == PLAYERANIMEVENT_FIRE_GUN_PRIMARY && pState->m_weapon->m_iItemDefinitionIndex() == Weapons_t::C4 )
		{
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WHOLE_BODY ], SelectSequenceFromActMods( ACT_CSGO_PLANT_BOMB ) );
			m_bPlantAnimStarted = true;
		}
		else if ( data && data->m_weapon_type == WEAPONTYPE_KNIFE )
		{
			// HACK: knives keep previous firing weight, if any
			should_override_order = true;
			float flSavedWeight = pLayers[ ANIMATION_LAYER_WEAPON_ACTION ].m_weight;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_PRIMARY ) );
			should_override_order = false;
			pLayers[ ANIMATION_LAYER_WEAPON_ACTION ].m_weight = flSavedWeight;
		}
		else
		{
			int seq = SelectSequenceFromActMods( ACT_CSGO_FIRE_PRIMARY );
			should_override_order = true;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], seq );
			should_override_order = false;
			//printf_s("setting seq: %i\n", seq);
		}
		break;
	}
	case PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT:
	{
		if ( data && data->m_weapon_type == WEAPONTYPE_KNIFE )
		{
			should_override_order = true;
			float flSavedWeight = pLayers[ ANIMATION_LAYER_WEAPON_ACTION ].m_weight;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_PRIMARY_OPT_1 ) );
			pLayers[ ANIMATION_LAYER_WEAPON_ACTION ].m_weight = flSavedWeight;
			should_override_order = false;
		}
		else
		{
			should_override_order = true;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_PRIMARY_OPT_1 ) );
			should_override_order = false;
		}
		break;
	}
	case PLAYERANIMEVENT_FIRE_GUN_PRIMARY_SPECIAL1:
	case PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT_SPECIAL1:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_PRIMARY_OPT_2 ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_FIRE_GUN_SECONDARY:
	{
		if ( data && data->m_weapon_type == WEAPONTYPE_SNIPER_RIFLE )
		{
			// hack: sniper rifles use primary fire anim when 'alt' firing, meaning scoped.
			should_override_order = true;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_PRIMARY ) );
			should_override_order = false;
		}
		else
		{
			should_override_order = true;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_SECONDARY ) );
			should_override_order = false;
		}
		break;
	}
	case PLAYERANIMEVENT_FIRE_GUN_SECONDARY_SPECIAL1:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_FIRE_SECONDARY_OPT_1 ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_GRENADE_PULL_PIN:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_OPERATE ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_SILENCER_ATTACH:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_SILENCER_ATTACH ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_SILENCER_DETACH:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_SILENCER_DETACH ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_RELOAD:
	{
		if ( data && data->m_weapon_type == WEAPONTYPE_SHOTGUN && pState->m_player->GetActiveWeapon()->m_iItemDefinitionIndex() != Weapons_t::MAG7 )
			break;
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_RELOAD ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_RELOAD_START:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_RELOAD_START ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_RELOAD_LOOP:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_RELOAD_LOOP ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_RELOAD_END:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_RELOAD_END ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_CATCH_WEAPON:
	{
		should_override_order = true;
		SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_CATCH ) );
		should_override_order = false;
		break;
	}
	case PLAYERANIMEVENT_CLEAR_FIRING:
	{
		m_bPlantAnimStarted = false;
		break;
	}
	case PLAYERANIMEVENT_DEPLOY:
	{
		if ( pState->m_weapon && pLayers[ ANIMATION_LAYER_WEAPON_ACTION ].m_sequence == ACT_CSGO_DEPLOY && pLayers[ ANIMATION_LAYER_WEAPON_ACTION ].m_cycle < CSGO_ANIM_DEPLOY_RATELIMIT )
			// we're already deploying
			bDeployRatelimit = true;
		else
		{
			should_override_order = true;
			SetLayerSequence( pState, &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ], SelectSequenceFromActMods( ACT_CSGO_DEPLOY ) );
			should_override_order = false;
		}
		break;
	}
	case PLAYERANIMEVENT_JUMP:
	{
		// note: this event means a jump is definitely happening, not just that a jump is desired
		bJumping = true;
		SetLayerSequence(pState, &pLayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL], SelectSequenceFromActMods(ACT_CSGO_JUMP));
		break;
	}

	default:
	{
		break;
	}
	}
}

void ServerAnimations::HandleWeapons( CCSGOPlayerAnimState* pAnimState, C_AnimationLayer* pLayers, int viewmodel_act )
{
	//c_weapon* wep = globals_i.local_player->active_weapon();
	bool swing_left = false;
	C_AnimationLayer* pLayer = nullptr;
	WeaponInfo* data = pAnimState->m_player && pAnimState->m_player->GetActiveWeapon() ? pAnimState->m_player->GetActiveWeapon()->GetWpnData() : nullptr;

	if ( data && data->m_weapon_type == WEAPONTYPE_KNIFE && pAnimState->m_weapon->m_flNextPrimaryAttack() + .4f < g_csgo.m_globals->m_curtime )
		swing_left = true;

	printf_s("Swing_left %i\n", swing_left);

	switch ( viewmodel_act )
	{
	case 0:
		//if (state->weapon != wep) {
		//	//printf_s("nig\n");
		//	state->do_animation_event(PLAYERANIMEVENT_DEPLOY, 0);
		//	globals_i.local_player->get_animation_layers(local_data.animlayers);
		//}
		break;
	case ACT_VM_IDLE:
		break;
	case ACT_VM_RELEASE:
		AddActivityModifier("underhand");
	case ACT_VM_THROW:
	case ACT_VM_PRIMARYATTACK:
	case ACT_VM_PRIMARYATTACK_SILENCED:
		DoAnimationEvent( pAnimState, PLAYERANIMEVENT_FIRE_GUN_PRIMARY, pLayers );
		break;
	case ACT_VM_HITCENTER:
	case ACT_VM_MISSCENTER:
		pLayer = &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ];
		if ( pLayer )
		{
			g_csgo.m_model_cache->BeginLock();
			should_override_order = true;
			int seq = SelectSequenceFromActMods( !swing_left ? 961 : 962 );
			pLayer->m_sequence = seq;
			float flPlaybackRate = pAnimState->m_player->GetLayerSequenceCycleRate( pLayer, seq );
			pLayer->m_playback_rate = flPlaybackRate;
			pLayer->m_cycle = pLayer->m_weight = 0.f;
			pAnimState->UpdateLayerOrderPreset(seq);
			should_override_order = false;
			g_csgo.m_model_cache->EndLock();
		}
		break;
	case ACT_VM_SWINGHIT:
		pLayer = &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ];
		if ( pLayer )
		{
			g_csgo.m_model_cache->BeginLock();
			should_override_order = true;
			int seq = SelectSequenceFromActMods( 963 );
			pLayer->m_sequence = seq;
			float flPlaybackRate = pAnimState->m_player->GetLayerSequenceCycleRate( pLayer, seq );
			pLayer->m_playback_rate = flPlaybackRate;
			pLayer->m_cycle = pLayer->m_weight = 0.f;
			g_csgo.m_model_cache->EndLock();
			pAnimState->UpdateLayerOrderPreset(seq);
			should_override_order = false;
		}
		swing_left = !swing_left;
		break;
	case ACT_VM_SECONDARYATTACK:
		DoAnimationEvent( pAnimState, PLAYERANIMEVENT_FIRE_GUN_SECONDARY, pLayers );
		break;
	case ACT_VM_HITCENTER2:
	case ACT_VM_MISSCENTER2:
		pLayer = &pLayers[ ANIMATION_LAYER_WEAPON_ACTION ];

		if ( pLayer )
		{
			g_csgo.m_model_cache->BeginLock();
			should_override_order = true;
			int seq = SelectSequenceFromActMods( 964 );
			pLayer->m_sequence = seq;
			float flPlaybackRate = pAnimState->m_player->GetLayerSequenceCycleRate( pLayer, seq );
			pLayer->m_playback_rate = flPlaybackRate;
			pLayer->m_cycle = pLayer->m_weight = 0.f;
			pAnimState->UpdateLayerOrderPreset( seq );
			should_override_order = false;
			g_csgo.m_model_cache->EndLock();
		}
		swing_left = true;
		break;
	case ACT_VM_SWINGHARD:
		DoAnimationEvent( pAnimState, PLAYERANIMEVENT_FIRE_GUN_SECONDARY, pLayers );
		break;
	case ACT_VM_RELOAD:
	case 831:
		DoAnimationEvent( pAnimState, PLAYERANIMEVENT_RELOAD, pLayers );
		break;
	case ACT_VM_PULLPIN:
		DoAnimationEvent( pAnimState, PLAYERANIMEVENT_GRENADE_PULL_PIN, pLayers );
		break;
	case ACT_VM_DRAW:
	case ACT_VM_EMPTY_DRAW:
	case ACT_VM_DRAW_SILENCED:
		DoAnimationEvent( pAnimState, PLAYERANIMEVENT_DEPLOY, pLayers );
		break;

		// todo: ACT_SHOTGUN_RELOAD_START, ACT_SHOTGUN_RELOAD_FINISH
	default:
		printf_s( "viewmodel_act: %i\n", viewmodel_act );
		break;
	}
}

void ServerAnimations::DoPreAnimation( CCSGOPlayerAnimState& pred_state, CUserCmd* cmd, int viewmodel_act, bool jumping, bool apply_data )
{
	const auto p = &pred_state;

	vec3_t forward{}, right{}, up{};
	math::AngleVectors( ang_t( 0.f, p->m_abs_yaw, 0.f ), &forward, &right, &up );
	right.normalize_in_place();
	const float to_forward_dot = p->m_velocity_normalized_non_zero.dot( forward );
	const float to_right_dot = p->m_velocity_normalized_non_zero.dot( right );

	const bool move_right = ( cmd->m_buttons & IN_MOVERIGHT ) != 0;
	const bool move_left = ( cmd->m_buttons & IN_MOVELEFT ) != 0;
	const bool move_forward = ( cmd->m_buttons & IN_FORWARD ) != 0;
	const bool move_backwards = ( cmd->m_buttons & IN_BACK ) != 0;
	const bool strafe_forward = p->m_speed_as_portion_of_run_top_speed >= .65f && move_forward && !move_backwards && to_forward_dot < -.55f;
	const bool strafe_backwards = p->m_speed_as_portion_of_run_top_speed >= .65f && move_backwards && !move_forward && to_forward_dot > .55f;
	const bool strafe_right = p->m_speed_as_portion_of_run_top_speed >= .73f && move_right && !move_left && to_right_dot < -.63f;
	const bool strafe_left = p->m_speed_as_portion_of_run_top_speed >= .73f && move_left && !move_right && to_right_dot > .63f;

	// client runs the code for this, we just need to set the netvar.
	g_ctx.m_local->m_bStrafing() = strafe_forward || strafe_backwards || strafe_right || strafe_left;

	CCSGOPlayerAnimState* unpred = g_ctx.m_local->m_PlayerAnimState();
	C_AnimationLayer* old_layers = g_ctx.m_local->m_AnimOverlay();

	HandleWeapons( unpred, old_layers, viewmodel_act ); // this will do our weapon animation events or whatever big G :minion:

	// https://gitlab.com/KittenPopo/csgo-2018-source/-/blob/main/game/shared/cstrike15/cs_gamemovement.cpp#L724
	if ( jumping )
	{
		DoAnimationEvent( unpred, PLAYERANIMEVENT_JUMP, old_layers );
	}

	bool bPreviouslyOnLadder = unpred->m_on_ladder;
	//m_bOnLadder = !m_bOnGround && m_pPlayer->GetMoveType() == MOVETYPE_LADDER;
	bool bStartedLadderingThisFrame = ( !bPreviouslyOnLadder && p->m_on_ladder );
	bool bStoppedLadderingThisFrame = ( bPreviouslyOnLadder && !p->m_on_ladder );

	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1615
	if ( p->m_ladder_weight > 0 || p->m_on_ladder )
	{
		if ( bStartedLadderingThisFrame )
		{
			SetLayerSequence( unpred, &old_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ], SelectSequenceFromActMods( ACT_CSGO_CLIMB_LADDER ) );
		}
	}

	if ( p->m_on_ground )
	{
		// unpred->m_landing is a (sort of) ghetto way to fix this check but it functionally is the same.
		if ( !unpred->m_landing && ( p->m_landed_on_ground_this_frame || bStoppedLadderingThisFrame ) )
		{
			// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1686
			SetLayerSequence( unpred, &old_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ], SelectSequenceFromActMods( unpred->m_duration_in_air > 1 ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT ) );
		}

		if ( p->m_landing && g_ctx.m_local->GetSequenceActivity( old_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_sequence ) != ACT_CSGO_CLIMB_LADDER )
		{
			// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1696
			bJumping = false;
		}

		// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1734
		if ( !p->m_landing && !bJumping && p->m_ladder_weight <= 0.f )
		{
			old_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_weight = 0.f;
		}
	}
	else if ( !p->m_on_ladder )
	{
		if ( p->m_left_the_ground_this_frame || bStoppedLadderingThisFrame )
		{
			// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1748
			if ( !bJumping )
			{
				SetLayerSequence( unpred, &old_layers[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ], SelectSequenceFromActMods( ACT_CSGO_FALL ) );
			}
		}
	}

	bool bStoppedMovingThisFrame = unpred->m_duration_still < 0.1f && p->m_duration_still > 0.f;
	if ( !unpred->m_adjust_started && bStoppedMovingThisFrame
		&& p->m_on_ground && !p->m_on_ladder && !p->m_landing && p->m_stutter_step < 50 )
	{
		SetLayerSequence( unpred, &g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ], SelectSequenceFromActMods( ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ) );
		unpred->m_adjust_started = true;
	}

	int act = g_ctx.m_local->GetSequenceActivity( g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ].m_sequence );
	if ( act == 980 || act == 979 )
	{
		if ( unpred->m_adjust_started && unpred->m_speed_as_portion_of_crouch_top_speed <= 0.25f )
		{
			unpred->IncrementLayerCycleWeightRateGeneric( ANIMATION_LAYER_ADJUST );
			unpred->m_adjust_started = !( unpred->IsLayerSequenceCompleted( ANIMATION_LAYER_ADJUST ) );
		}
		else
		{
			unpred->m_adjust_started = false;
			float flWeight = g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ].m_weight;
			g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ].m_weight = math::Approach( 0, flWeight, p->m_last_update_increment * 5 );
			g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ].m_weight_delta_rate = ( g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ].m_weight - flWeight ) / p->m_last_update_increment;
		}
	}

	if ( p->m_velocity_length_xy <= 1.0f && p->m_on_ground && !p->m_on_ladder && !p->m_landing && p->m_last_update_increment > 0
		&& abs( math::NormalizedAngle( p->m_abs_yaw - p->m_abs_yaw_last ) / p->m_last_update_increment > 120.0f ) )
	{
		SetLayerSequence( unpred, &g_ctx.m_local->m_AnimOverlay()[ ANIMATION_LAYER_ADJUST ], SelectSequenceFromActMods( ACT_CSGO_IDLE_TURN_BALANCEADJUST ) );
		unpred->m_adjust_started = true;
	}

	if ( apply_data )
	{
		if ( unpred->m_on_ground )
		{
			if ( p->m_velocity_length_xy > 0.1f )
			{
				g_ctx.m_local->m_flLowerBodyYawTarget() = p->m_eye_yaw;
				g_ctx.m_body_pred = g_csgo.m_globals->m_curtime + 0.22f;
			}
			else if ( g_csgo.m_globals->m_curtime > g_ctx.m_body_pred && abs( math::AngleDiff( p->m_abs_yaw, p->m_eye_yaw ) ) > 35.f )
			{
				g_ctx.m_local->m_flLowerBodyYawTarget() = p->m_eye_yaw;
				g_ctx.m_body_pred = g_csgo.m_globals->m_curtime + 1.1f;
			}

			SimulateFeetYaw( p, g_ctx.m_local->m_flLowerBodyYawTarget(), /*cmd->m_view_angles.y*/g_ctx.m_local->m_flLowerBodyYawTarget(), m_fake_feet_yaw );
		}
	}
}

std::pair<CCSGOPlayerAnimState, C_AnimationLayer*> ServerAnimations::PredictAnimationState( Player* player, ang_t viewangles )
{
	const CCSGOPlayerAnimState backup_state = *player->m_PlayerAnimState();
	C_AnimationLayer backup_layers[ 13 ]; player->GetAnimLayers( backup_layers );
	float backup_poses[ 24 ]; player->GetPoseParameters( backup_poses );

	if ( player->m_PlayerAnimState()->m_last_update_frame >= g_csgo.m_globals->m_frame )
	{
		player->m_PlayerAnimState()->m_last_update_frame = g_csgo.m_globals->m_frame - 1;
	}

	player->m_PlayerAnimState()->update( viewangles );

	const CCSGOPlayerAnimState pred = *player->m_PlayerAnimState();
	C_AnimationLayer layers[ 13 ];
	player->GetAnimLayers( layers );

	*player->m_PlayerAnimState() = backup_state;
	player->SetAnimLayers( backup_layers );
	player->SetPoseParameters( backup_poses );

	return { pred, layers };
}

float ServerAnimations::GetPredFeetRange()
{
	auto animstate = PredictAnimationState( g_ctx.m_local, ang_t( 0.f, 0.f, 0.f ) ).first; // viewangles dont matter for what we need here.

	float flAimMatrixWidthRange = math::Lerp( std::clamp( animstate.m_speed_as_portion_of_run_top_speed, 0.f, 1.f ), 1.0f, math::Lerp( animstate.m_walk_run_transition, 0.8f, 0.5f ) );
	if ( animstate.m_anim_duck_amount > 0 )
	{
		flAimMatrixWidthRange = math::Lerp( animstate.m_anim_duck_amount * std::clamp( animstate.m_speed_as_portion_of_crouch_top_speed, 0.f, 1.f ), flAimMatrixWidthRange, 0.5f );
	}

	return animstate.m_aim_yaw_max * flAimMatrixWidthRange;
}

void ServerAnimations::SimulateFeetYaw( CCSGOPlayerAnimState* animstate, float lby, float eye_yaw, float& custom_feet_yaw )
{
	// if the player is looking far enough to either side, turn the feet to keep them within the extent of the aim matrix
	custom_feet_yaw = std::clamp( custom_feet_yaw, -360.f, 360.f );

	float flEyeFootDelta = math::NormalizedAngle( eye_yaw - custom_feet_yaw );

	// narrow the available aim matrix width as speed increases
	float flAimMatrixWidthRange = math::Lerp( std::clamp( animstate->m_speed_as_portion_of_run_top_speed, 0.f, 1.f ), 1.0f, math::Lerp( animstate->m_walk_run_transition, 0.8f, 0.5f ) );

	if ( animstate->m_anim_duck_amount > 0 )
	{
		flAimMatrixWidthRange = math::Lerp( animstate->m_anim_duck_amount * std::clamp( animstate->m_speed_as_portion_of_crouch_top_speed, 0.f, 1.f ), flAimMatrixWidthRange, 0.5f );
	}

	float flTempYawMax = animstate->m_aim_yaw_max * flAimMatrixWidthRange;
	float flTempYawMin = animstate->m_aim_yaw_min * flAimMatrixWidthRange;

	if ( flEyeFootDelta > flTempYawMax )
	{
		custom_feet_yaw = eye_yaw - abs( flTempYawMax );
	}
	else if ( flEyeFootDelta < flTempYawMin )
	{
		custom_feet_yaw = eye_yaw + abs( flTempYawMin );
	}
	custom_feet_yaw = math::NormalizedAngle( custom_feet_yaw );

	if ( animstate->m_on_ground )
	{
		if ( animstate->m_velocity_length_xy > 0.1f )
		{
			custom_feet_yaw = math::ApproachAngle( eye_yaw, custom_feet_yaw, animstate->m_last_update_increment * ( 30.0f + 20.0f * animstate->m_walk_run_transition ) );
		}
		else
		{
			custom_feet_yaw = math::ApproachAngle( lby, custom_feet_yaw, animstate->m_last_update_increment * 100.0f );
		}
	}
}

bool LayerSequenceHasActMod(CCSGOPlayerAnimState* state, int nLayerIndex, const char* szActMod)
{
	C_AnimationLayer* pLayer = &state->m_player->m_AnimOverlay()[nLayerIndex];
	if (!pLayer)
		return false;

	mstudioseqdesc_t& seqdesc = *get_seq_desc(state->m_player->GetModelPtr(), pLayer->m_sequence);
	for (int i = 0; i < seqdesc.num_activity_modifiers; i++)
	{
		mstudioactivitymodifier_t* mod = seqdesc.pActivityModifier(i);

		if (!strcmp(mod->pszName(), szActMod))
			return true;
	}
	return false;
}

void ServerAnimations::OnSetupWeaponAction(CCSGOPlayerAnimState* pState) {
	bool bDoIncrement = true;
	C_AnimationLayer* pLayer = &pState->m_player->m_AnimOverlay()[ANIMATION_LAYER_WEAPON_ACTION];
	C_AnimationLayer* pLayer2 = &pState->m_player->m_AnimOverlay()[ANIMATION_LAYER_WEAPON_ACTION_RECROUCH];

	if (pState->m_weapon && bDeployRatelimit && pLayer->m_activty == ACT_CSGO_DEPLOY)
	{
		const auto world_model = pState->m_weapon ? reinterpret_cast<Weapon*>(g_csgo.m_entlist->GetClientEntityFromHandle(pState->m_weapon->m_hWeaponWorldModel())) : nullptr;
		if (world_model)
			world_model->m_fEffects() &= ~EF_NODRAW; // ShowWeaponWorldModel( false );

		if (pLayer->m_cycle >= CSGO_ANIM_DEPLOY_RATELIMIT)
		{
			bDeployRatelimit = false;
			SetLayerSequence(pState, pLayer, SelectSequenceFromActMods(ACT_CSGO_DEPLOY));
			pLayer->m_weight = 0.f;
			bDoIncrement = false;
		}
	}

	// fixme: this is a hack to fix all-body weapon actions that need to transition into crouch or stand poses they weren't built for.
	// This only matters for idle animation - the move layer is itself a kind of 're-crouch' and 're-stand' layer itself.

	if (pState->m_animstate_model_version < 2)
	{
		// old re-crouch behavior

		// fixme: this is a hack to fix the all-body weapon action that wants to crouch case. There's no fix for the crouching all-body action that wants to stand
		if (pState->m_anim_duck_amount > 0
			&& pLayer->m_weight > 0
			&& !LayerSequenceHasActMod(pState, ANIMATION_LAYER_WEAPON_ACTION, "crouch"))
		{
			if (pLayer2->m_sequence <= 0)
				SetLayerSequence(pState, pLayer2, pState->m_player->LookupSequence("recrouch_generic"));

			pLayer2->m_weight *= pState->m_anim_duck_amount;
			//SetLayerWeight(ANIMATION_LAYER_WEAPON_ACTION_RECROUCH, state, GetLayerWeight(ANIMATION_LAYER_WEAPON_ACTION_RECROUCH, state) * state->anim_duck_amount);
		}
		else
		{
			pLayer2->m_weight = 0;
		}
	}
	else
	{
		// newer version with "re-stand" blended into the re-crouch.

		float flTargetRecrouchWeight = 0;
		if ( pLayer->m_weight > 0)
		{
			if ( pLayer->m_sequence <= 0)
				SetLayerSequence(pState, pLayer2, pState->m_player->LookupSequence("recrouch_generic"));
		
			if (LayerSequenceHasActMod(pState, ANIMATION_LAYER_WEAPON_ACTION, "crouch"))
			{
				// this is a crouching anim. It might be the only anim available, or it's just lasted long enough that the 
				// player stood up after it started. If we're standing up at all, we need to force the stand pose artificially.
				if (pState->m_anim_duck_amount < 1)
					flTargetRecrouchWeight = pLayer->m_weight * (1.0f - pState->m_anim_duck_amount );
			}
			else
			{
				// this is NOT a crouching anim. Still if it's not a whole-body anim it might work fine when crouched though, 
				// and not actually need the re-crouch. How to detect this?
		
				// We can't trust this anim to crouch the player since it's not tagged as a crouch anim. So we need to force the
				// crouch pose artificially.
				if ( pState->m_anim_duck_amount > 0)
					flTargetRecrouchWeight = pLayer->m_weight * pState->m_anim_duck_amount;
			}
		}
		else
		{
			if (pLayer2->m_weight > 0)
				flTargetRecrouchWeight = math::Approach(0, pLayer2->m_weight, pState->m_last_update_increment * 4);
		}
		
		pLayer2->m_weight = flTargetRecrouchWeight;
	}

	if (bDoIncrement)
	{
		// increment the action
		pState->IncrementLayerCycle(ANIMATION_LAYER_WEAPON_ACTION, false);
		float flWeightPrev = pLayer->m_weight;
		float flDesiredWeight = pState->GetLayerIdealWeightFromSeqCycle(ANIMATION_LAYER_WEAPON_ACTION);
		pLayer->m_weight = flDesiredWeight;
		pLayer->m_weight_delta_rate = flWeightPrev;
	}

	//CAnimationLayer* pWeaponLayer = m_pPlayer->GetAnimOverlay( nLayer, USE_ANIMLAYER_RAW_INDEX );
	if ( pLayer && pState->m_weapon )
	{
		Weapon* pWeaponWorldModel = pState->m_weapon->GetWeaponWorldModel();
		if ( pWeaponWorldModel )
		{
			g_csgo.m_model_cache->BeginLock();
			if ( pLayer->m_activty > 0 && pLayer->m_weight > 0 ) // fixme: does the weight check make 0-frame events fail? Added a check below just in case.
			{
				pWeaponWorldModel->m_nSequence() = pLayer->m_activty;
				pWeaponWorldModel->m_flCycle() = pLayer->m_cycle;
				pWeaponWorldModel->m_flPlaybackRate() = pLayer->m_playback_rate;
				//pWeaponWorldModel->DispatchAnimEvents( pWeaponWorldModel );
			}
			else
			{
				//if ( pWeaponWorldModel->m_flCycle() != 0.f )
				//	pWeaponWorldModel->DispatchAnimEvents( pWeaponWorldModel );

				pWeaponWorldModel->m_nSequence() = 0;
				pWeaponWorldModel->m_flCycle() = 0.f;
				pWeaponWorldModel->m_flPlaybackRate() = 0.f;
			}
			g_csgo.m_model_cache->EndLock();
		}
	}
}