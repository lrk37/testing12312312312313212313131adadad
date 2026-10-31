#include "includes.h"

AutoWall g_autowall {};

// Nice
bool CGameTrace::did_hit_world( )
{
	return m_entity != nullptr && m_entity == g_csgo.m_entlist->GetClientEntity( 0 );
}

void AutoWall::TraceLine( vec3_t& absStart, vec3_t& absEnd, unsigned int mask, Entity* ignore, CGameTrace* ptr )
{
	Ray ray( absStart, absEnd );
	CTraceFilter filter;
	filter.skip_entity = ignore;

	g_csgo.m_engine_trace->TraceRay( ray, mask, &filter, ptr );
}

void AutoWall::GetBulletTypeParameters( float& maxRange, float& maxDistance, const char* bulletType, bool sv_penetration_type )
{
	if ( sv_penetration_type )
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if ( !strcmp( bulletType, "BULLET_PLAYER_338MAG" ) )
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_762MM" ) )
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_556MM" ) || !strcmp( bulletType, "BULLET_PLAYER_556MM_SMALL" ) || !strcmp( bulletType, "BULLET_PLAYER_556MM_BOX" ) )
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_57MM" ) )
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_50AE" ) )
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_357SIG" ) || !strcmp( bulletType, "BULLET_PLAYER_357SIG_SMALL" ) || !strcmp( bulletType, "BULLET_PLAYER_357SIG_P250" ) || !strcmp( bulletType, "BULLET_PLAYER_357SIG_MIN" ) )
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_9MM" ) )
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_45ACP" ) )
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if ( !strcmp( bulletType, "BULLET_PLAYER_BUCKSHOT" ) )
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}

float AutoWall::ScaleDamage( Player* player, float damage, float armor_ratio, int hitgroup )
{
	//Cred. to N0xius for reversing this.
	//TODO: _xAE^; look into reversing this yourself sometime

	bool hasHeavyArmor = player->m_bHasHeavyArmor( );
	int armorValue = player->m_ArmorValue( );

	//Does the person have armor on for the hitbox checked?
	auto IsArmored = [ &hitgroup, &player ]( )->bool
		{
			switch ( hitgroup )
			{
			case HITGROUP_HEAD:
				return !!player->m_bHasHelmet( );
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return true;
			default:
				return false;
			}
		};

	switch ( hitgroup )
	{
	case HITGROUP_HEAD:
		damage *= hasHeavyArmor ? 2.f : 4.f; //Heavy Armor does 1/2 damage
		break;
	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= 0.75f;
		break;
	default:
		break;
	}

	if ( armorValue > 0 && IsArmored( ) )
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = armor_ratio / 2.f;

		//Damage gets modified for heavy armor users
		if ( hasHeavyArmor )
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = damage * armorRatio;

		if ( hasHeavyArmor )
			NewDamage *= 0.85f;

		if ( ( ( damage - ( damage * armorRatio ) ) * ( bonusValue * armorBonusRatio ) ) > armorValue )
			NewDamage = damage - ( armorValue / armorBonusRatio );

		damage = NewDamage;
	}

	return damage;
}

bool AutoWall::TraceToExit( CGameTrace& enterTrace, CGameTrace& exitTrace, vec3_t startPosition, vec3_t direction )
{
	vec3_t start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while ( currentDistance <= maxDistance )
	{
		//Add extra distance to our ray
		currentDistance += rayExtension;

		//Multiply the direction vector to the distance so we go outwards, add our position to it.
		start = startPosition + direction * currentDistance;

		if ( !firstContents )
			firstContents = g_csgo.m_engine_trace->GetPointContents( start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr ); /*0x4600400B*/

		int pointContents = g_csgo.m_engine_trace->GetPointContents( start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr );

		if ( !( pointContents & MASK_SHOT_HULL ) || pointContents & CONTENTS_HITBOX && pointContents != firstContents ) /*0x600400B, *0x40000000*/
		{
			//Let's setup our end position by deducting the direction by the extra added distance
			end = start - ( direction * rayExtension );

			//Let's cast a ray from our start pos to the end pos
			TraceLine( start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace );

			//Let's check if a hitbox is in-front of our enemy and if they are behind of a solid wall
			if ( exitTrace.m_startsolid && exitTrace.m_surface.m_flags & SURF_HITBOX )
			{
				TraceLine( start, startPosition, MASK_SHOT_HULL, exitTrace.m_entity, &exitTrace );

				if ( exitTrace.hit( ) && !exitTrace.m_startsolid )
				{
					start = exitTrace.m_endpos;
					return true;
				}

				continue;
			}

			//Can we hit? Is the wall solid?
			if ( exitTrace.hit( ) && !exitTrace.m_startsolid )
			{
				//Is the wall a breakable? If so, let's shoot through it.
				if ( game::IsBreakable( enterTrace.m_entity ) && game::IsBreakable( exitTrace.m_entity ) )
					return true;

				if ( enterTrace.m_surface.m_flags & SURF_NODRAW || !( exitTrace.m_surface.m_flags & SURF_NODRAW ) && ( exitTrace.m_plane.m_normal.dot( direction ) <= 1.f ) )
				{
					float multAmount = exitTrace.m_fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if ( !exitTrace.hit( ) || exitTrace.m_startsolid )
			{
				if ( enterTrace.did_hit_non_world_entity( ) && game::IsBreakable( enterTrace.m_entity ) )
				{
					exitTrace = enterTrace;
					exitTrace.m_endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}

	return false;
}

bool AutoWall::HandleBulletPenetration( int from_team, WeaponInfo* weaponData, CGameTrace& enterTrace, vec3_t& eyePosition, vec3_t direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration )
{
	CGameTrace exitTrace;
	Entity* pEnemy = enterTrace.m_entity;
	surfacedata_t* enterSurfaceData = g_csgo.m_phys_props->GetSurfaceData( enterTrace.m_surface.m_surface_props );
	int enterMaterial = enterSurfaceData->m_game.m_material;

	float enterSurfPenetrationModifier = enterSurfaceData->m_game.m_penetration_modifier;
	float enterDamageModifier = enterSurfaceData->m_game.m_damage_modifier;
	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ( ( enterTrace.m_contents >> 3 ) & CONTENTS_SOLID );
	bool isLightSurf = ( ( enterTrace.m_surface.m_flags >> 7 ) & SURF_LIGHT );

	if ( possibleHitsRemaining <= 0
		//Test for "DE_CACHE/DE_CACHE_TELA_03" as the entering surface and "CS_ITALY/CR_MISCWOOD2B" as the exiting surface.
		//Fixes a wall in de_cache which seems to be broken in some way. Although bullet penetration is not recorded to go through this wall
		//Decals can be seen of bullets within the glass behind of the enemy. Hacky method, but works.
		//You might want to have a check for this to only be activated on de_cache.
		|| ( enterTrace.m_surface.m_name == ( const char* )0x2227c261 && exitTrace.m_surface.m_name == ( const char* )0x2227c868 )
		|| ( !possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS )
		|| weaponData->m_penetration <= 0.f
		|| !TraceToExit( enterTrace, exitTrace, enterTrace.m_endpos, direction )
		&& !( g_csgo.m_engine_trace->GetPointContents( enterTrace.m_endpos, MASK_SHOT_HULL, nullptr ) & MASK_SHOT_HULL ) )
		return false;

	surfacedata_t* exitSurfaceData = g_csgo.m_phys_props->GetSurfaceData( exitTrace.m_surface.m_surface_props );
	int exitMaterial = exitSurfaceData->m_game.m_material;
	float exitSurfPenetrationModifier = exitSurfaceData->m_game.m_penetration_modifier;
	float exitDamageModifier = exitSurfaceData->m_game.m_damage_modifier;

	//Are we using the newer penetration system?
	if ( enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS )
	{
		combinedPenetrationModifier = 3.f;
		finalDamageModifier = 0.05f;
	}
	else if ( isSolidSurf || isLightSurf )
	{
		combinedPenetrationModifier = 1.f;
		finalDamageModifier = 0.16f;
	}
	else if ( enterMaterial == CHAR_TEX_FLESH && ( from_team == pEnemy->m_iTeamNum( ) && ff_damage_reduction_bullets == 0.f ) ) //TODO: Team check config
	{
		//Look's like you aren't shooting through your teammate today
		if ( ff_damage_bullet_penetration == 0.f )
			return false;

		//Let's shoot through teammates and get kicked for teamdmg! Whatever, atleast we did damage to the enemy. I call that a win.
		combinedPenetrationModifier = ff_damage_bullet_penetration;
		finalDamageModifier = 0.16f;
	}
	else
	{
		combinedPenetrationModifier = ( enterSurfPenetrationModifier + exitSurfPenetrationModifier ) / 2.f;
		finalDamageModifier = 0.16f;
	}

	//Do our materials line up?
	if ( enterMaterial == exitMaterial )
	{
		if ( exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD )
			combinedPenetrationModifier = 3.f;
		else if ( exitMaterial == CHAR_TEX_PLASTIC )
			combinedPenetrationModifier = 2.f;
	}

	//Calculate thickness of the wall by getting the length of the range of the trace and squaring
	thickness = ( exitTrace.m_endpos - enterTrace.m_endpos ).length_sqr( );
	modifier = fmaxf( 1.f / combinedPenetrationModifier, 0.f );

	//This calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
	lostDamage = fmaxf(
		( ( modifier * thickness ) / 24.f )
		+ ( ( currentDamage * finalDamageModifier )
			+ ( fmaxf( 3.75f / penetrationPower, 0.f ) * 3.f * modifier ) ), 0.f );

	//Did we loose too much damage?
	if ( lostDamage > currentDamage )
		return false;

	//We can't use any of the damage that we've lost
	if ( lostDamage > 0.f )
		currentDamage -= lostDamage;

	//Do we still have enough damage to deal?
	if ( currentDamage < 1.f )
		return false;

	eyePosition = exitTrace.m_endpos;
	--possibleHitsRemaining;

	return true;
}

float AutoWall::FireBullet( Player* from_player, vec3_t& from_pos, vec3_t& direction, int& hits_remaining )
{
	Weapon* weapon = from_player->GetActiveWeapon( );
	if ( !weapon )
		return 0.f;

	WeaponInfo* weaponData = weapon->GetWpnData( );
	if ( !weaponData )
		return 0.f;

	CGameTrace enterTrace;

	//We should be skipping localplayer when casting a ray to players.
	CTraceFilter filter;
	filter.skip_entity = from_player;

	float maxRange = weaponData->m_range;

	//	  Current bullet travel Power to penetrate Distance to penetrate Range               Player bullet reduction convars			  Amount to extend ray by
	float currentDistance = 0.f, penetrationPower, penetrationDistance, rayExtension = 40.f;

	bool sv_penetration_type = g_csgo.sv_penetration_type->GetInt( );
	float ff_damage_reduction_bullets = g_csgo.ff_damage_reduction_bullets->GetFloat( );
	float ff_damage_bullet_penetration = g_csgo.ff_damage_bullet_penetration->GetFloat( );

	GetBulletTypeParameters( penetrationPower, penetrationDistance, weaponData->m_ammo_type, sv_penetration_type );

	if ( sv_penetration_type )
		penetrationPower = weaponData->m_penetration;

	//Set our current damage to what our gun's initial damage reports it will do
	float currentDamage = weaponData->m_damage;

	// handlebulletpenetration modify this and fk it up.
	vec3_t eye_pos_copy = from_pos;

	//If our damage is greater than (or equal to) 1, and we can shoot, let's shoot.
	while ( hits_remaining > 0 && currentDamage >= 1.f )
	{
		//Calculate max bullet range
		maxRange -= currentDistance;

		//Create endpoint of bullet
		vec3_t end = eye_pos_copy + direction * maxRange;

		TraceLine( eye_pos_copy, end, MASK_SHOT_HULL | CONTENTS_HITBOX, from_player, &enterTrace );

		// i might just be fucking schizophrenic but this only works against the local player if we do -60.f??? instead of positive 60.f
		game::UTIL_ClipTraceToPlayers( eye_pos_copy, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace, -60.f );

		//We have to do this *after* tracing to the player.
		surfacedata_t* enterSurfaceData = g_csgo.m_phys_props->GetSurfaceData( enterTrace.m_surface.m_surface_props );
		float enterSurfPenetrationModifier = enterSurfaceData->m_game.m_penetration_modifier;
		int enterMaterial = enterSurfaceData->m_game.m_material;

		//"Fraction == 1" means that we didn't hit anything. We don't want that- so let's break on it.
		if ( enterTrace.m_fraction == 1.f )
			break;

		//calculate the damage based on the distance the bullet traveled.
		currentDistance += enterTrace.m_fraction * maxRange;

		//Let's make our damage drops off the further away the bullet is.
		currentDamage *= pow( weaponData->m_range_modifier, ( currentDistance / 500.f ) );

		//Sanity checking / Can we actually shoot through?
		if ( currentDistance > penetrationDistance && weaponData->m_penetration > 0.f || enterSurfPenetrationModifier < 0.1f )
			break;

		bool canDoDamage = ( enterTrace.m_hitgroup != HITGROUP_GEAR && enterTrace.m_hitgroup != HITGROUP_GENERIC );
		bool isPlayer = ( ( enterTrace.m_entity )->GetClientClass( )->m_ClassID == _CCSPlayer );
		bool isEnemy = from_player->m_iTeamNum( ) != enterTrace.m_entity->m_iTeamNum( );
		bool onTeam = enterTrace.m_entity->m_iTeamNum( ) == TEAM_COUNTERTERRORISTS || enterTrace.m_entity->m_iTeamNum( ) == TEAM_TERRORISTS;

		//g_ctx.print( tfm::format( "%i %i %i %i\n", canDoDamage, isPlayer, isEnemy, onTeam ) );

		//TODO: Team check config
		if ( ( canDoDamage && isPlayer && isEnemy ) && onTeam )
			return ScaleDamage( ( Player* )enterTrace.m_entity, currentDamage, weaponData->m_armor_ratio, enterTrace.m_hitgroup );

		//Calling HandleBulletPenetration here reduces our penetrationCounter, and if it returns true, we can't shoot through it.
		if ( !HandleBulletPenetration( from_player->m_iTeamNum( ), weaponData, enterTrace, eye_pos_copy, direction, hits_remaining, currentDamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration ) )
			break;
	}

	return 0.f;
}

float AutoWall::run( Player* from_player, vec3_t start_pos, vec3_t end_pos )
{
	//This gets set in FX_Firebullets to 4 as a pass-through value.
	//CS:GO has a maximum of 4 surfaces a bullet can pass-through before it 100% stops.
	//Excerpt from Valve: https://steamcommunity.com/sharedfiles/filedetails/?id=275573090
	//"The total number of surfaces any bullet can penetrate in a single flight is capped at 4." -CS:GO Official
	int possibleHitsRemaining = 4;

	return FireBullet( from_player, start_pos, ( end_pos - start_pos ).normalized( ), possibleHitsRemaining );
}