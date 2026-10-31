#include "includes.h"

int ScaleStabDamage( Player* player, bool backStab, bool secondary )
{
	const auto firstSwing{ ( g_ctx.m_weapon->m_flNextPrimaryAttack() + 0.4 ) < g_csgo.m_globals->m_curtime };
	const auto armoured{ player->m_bHasHeavyArmor() || player->m_ArmorValue() };

	if ( !armoured )
	{
		if ( secondary )
		{
			if ( backStab )
				return 180;
			else
				return 65;
		}
		else
		{
			if ( backStab )
				return 90;
			else if ( firstSwing ) 
				// first swing does full damage
				return 40;
			else
				// subsequent swings do less	
				return 25;
		}
	}
	else
	{
		if ( secondary )
		{
			if ( backStab )
				return 153;
			else
				return 55;
		}
		else
		{
			if ( backStab )
				return 76;
			else if ( firstSwing )
				// first swing does full damage
				return 34;
			else
				// subsequent swings do less	
				return 21;
		}
	}
}

bool KnifeBotCanHitTarget( Player* target, vec3_t dir, bool secondary )
{
	const auto minDistance{ secondary ? 32.f : 48.f };

	const auto end{ g_ctx.m_shoot_pos + ( dir * minDistance ) };

	CTraceFilter traceFilter;
	traceFilter.skip_entity = g_ctx.m_local;
	CGameTrace trace;

	g_csgo.m_engine_trace->TraceRay(
		{ g_ctx.m_shoot_pos, end }, MASK_SOLID,
		&traceFilter, &trace
	);

	if ( trace.m_fraction >= 1.f )
		return false;

	if ( trace.m_entity != target )
		return false;

	return true;
}

inline float Vector2DNormalize( vec2_t& v )
{
	if ( !v.valid() )
		return 0.f;

	float l = v.length();

	if ( l != 0.f )
		v /= l;
	else
		v.x = v.y = 0.0f;

	return l;
}

bool IsBackStab( Player* player )
{
	vec3_t fwd;

	math::AngleVectors( player->GetAbsAngles(), &fwd );// firebullet is ran before animstate->update

	vec2_t vecLOS = ( player->GetAbsOrigin() - g_ctx.m_local->GetAbsOrigin() ).to_vec2();
	Vector2DNormalize( vecLOS );

	float flDot = vecLOS.dot( fwd.to_vec2() );

	return flDot > 0.475f;
}

void KnifeBotTargetPlayer( AimPlayer* target, LagRecord* record )
{
	record->cache();

	const auto hitboxSet = target->m_player->m_studioHdr()->m_pStudioHdr->GetHitboxSet( target->m_player->m_nHitboxSet() );
	const auto hitbox = hitboxSet->GetHitbox( HITBOX_CHEST );

	vec3_t point;
	math::VectorTransform( ( hitbox->m_mins + hitbox->m_maxs ) / 2.f, target->m_player->m_BoneCache().m_pCachedBones[ hitbox->m_bone ], point );

	const auto dir{ ( point - g_ctx.m_shoot_pos ).normalized() };

	const bool canhit = KnifeBotCanHitTarget( target->m_player, dir, false );
	if ( canhit )
		target->knife_info.m_can_hit = true;

	target->knife_info.m_backstab = IsBackStab( target->m_player );

	if ( canhit )
	{
		if ( KnifeBotCanHitTarget( target->m_player, dir, true ) )
			target->knife_info.can_hit_secondary_swing = true;
	}
}

struct KnifeTarget
{
	AimPlayer* target;
	LagRecord* record;
	float damage;

	KnifeTarget()
	{
		target = nullptr;
		record = nullptr;
		damage = 0.f;
	}

	KnifeTarget( AimPlayer* t, LagRecord* r )
	{
		target = t;
		record = r;
		damage = 0.f;
	}

	void clear()
	{
		record = nullptr;
		target = nullptr;
	}
};

void Aimbot::DoKnifeBot()
{
	static std::vector<KnifeTarget> knife_targs;
	static KnifeTarget best_targ;

	best_targ.clear();
	knife_targs.clear();

	for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ )
	{
		Player* player = g_csgo.m_entlist->GetClientEntity<Player*>( i );
		if ( !player || !IsValidTarget( player ) || player->dormant() )
			continue;

		AimPlayer* data = &m_players[ i - 1 ];
		if ( !data || data->m_records.empty() || data->m_ignore )
			continue;

		if ( player->GetAbsOrigin().dist_to( g_ctx.m_local->GetAbsOrigin() ) > 200.f )
			continue;

		LagRecord* best = nullptr;

		bool broke_lc = false;
		for ( int i = 0; i < data->m_records.size(); i++ )
		{
			auto record = data->m_records.at( i ).get();

			LagRecord* previous = i > 0 ? data->m_records[ i - 1 ].get() : nullptr;
			if ( previous && ( record->m_origin - previous->m_origin ).length_sqr() > 4096.0f )
			{
				broke_lc = i == 1; // we wont have a valid record to knif oh yeaaaaaa

				if ( broke_lc )
				{
					KnifeBotTargetPlayer( data, data->m_records[ 0 ].get() );
					if ( data->knife_info.m_can_hit )
						best = data->m_records[ 0 ].get();
				}

				break;
			}

			if ( !record->valid() || record->immune() )
				continue;

			auto prev_can_hit = data->knife_info.m_can_hit;
			auto prev_backstab = data->knife_info.m_backstab;

			KnifeBotTargetPlayer( data, record );
			if ( ( data->knife_info.m_can_hit && !prev_can_hit ) || ( data->knife_info.m_backstab && !prev_backstab ) )
			{
				best = record;
			}

			if ( data->knife_info.m_can_hit && data->knife_info.m_backstab && data->knife_info.can_hit_secondary_swing )
			{
				best = record;
				break;
			}

		}

		if ( best && data->knife_info.m_can_hit )
			knife_targs.push_back( KnifeTarget( data, best ) );
	}

	for ( auto& targ : knife_targs )
	{
		targ.damage = ScaleStabDamage( targ.target->m_player, targ.target->knife_info.m_backstab, targ.target->knife_info.can_hit_secondary_swing );

		if ( !best_targ.target )
		{
			best_targ = targ;
			continue;
		}

		if ( ( best_targ.target && targ.damage >= best_targ.damage ) || targ.damage >= targ.target->m_player->m_iHealth() )
			best_targ = targ;
	}

	if ( best_targ.target )
	{


		best_targ.record->cache();

		const auto hitboxSet{ best_targ.target->m_player->m_studioHdr()->m_pStudioHdr->GetHitboxSet( best_targ.target->m_player->m_nHitboxSet() ) };
		const auto hitbox{ hitboxSet->GetHitbox( HITBOX_CHEST ) };

		math::VectorTransform( ( hitbox->m_mins + hitbox->m_maxs ) / 2.f, best_targ.record->m_bones[ hitbox->m_bone ], m_aim );
		math::VectorAngles( m_aim - g_ctx.m_shoot_pos, m_angle );

		const auto dmgSwing{ ScaleStabDamage( best_targ.target->m_player, best_targ.target->knife_info.m_backstab, false ) };
		const auto dmgStab{ ScaleStabDamage( best_targ.target->m_player, best_targ.target->knife_info.m_backstab, true ) };

		if ( ( dmgSwing >= best_targ.target->m_player->m_iHealth()
			|| dmgStab < best_targ.target->m_player->m_iHealth()
			/* || !bestTarget->m_bCanHitSecondary*/ )// TODO: this fucks up back stabbing at full hp
			&& g_ctx.m_weapon_fire )
		{

	
			g_ctx.m_cmd->m_buttons |= IN_ATTACK;
			g_ctx.m_cmd->m_buttons &= ~IN_ATTACK2;
			g_ctx.m_cmd->m_buttons &= ~IN_USE;

			//best_targ.target->m_player->m_iHealth( ) -= ScaleStabDamage(best_targ.target->m_player, best_targ.target->knife_info.m_backstab, false );

			g_ctx.m_cmd->m_view_angles = m_angle;
			g_ctx.m_cmd->m_tick = game::TIME_TO_TICKS( best_targ.record->m_sim_time + g_ctx.m_lerp );
			*g_ctx.m_packet = true; // i dont think theres a point to choke on shot?
		}
		else
		{
			// delay if we have to
			if ( g_ctx.m_weapon_fire && best_targ.target->knife_info.can_hit_secondary_swing )
			{
				g_ctx.m_cmd->m_buttons |= IN_ATTACK2;
				g_ctx.m_cmd->m_buttons &= ~IN_ATTACK;
				g_ctx.m_cmd->m_buttons &= ~IN_USE;

				//best_targ.target->m_player->m_iHealth() -= ScaleStabDamage(best_targ.target->m_player, best_targ.target->knife_info.m_backstab, true);

				g_ctx.m_cmd->m_view_angles = m_angle;
				g_ctx.m_cmd->m_tick = game::TIME_TO_TICKS( best_targ.record->m_sim_time + g_ctx.m_lerp );
				*g_ctx.m_packet = true;
			}
		}
	}
}