#pragma once

class AutoWall
{
private:
	void TraceLine( vec3_t& absStart, vec3_t& absEnd, unsigned int mask, Entity* ignore, CGameTrace* ptr );

public:
	void GetBulletTypeParameters( float& maxRange, float& maxDistance, const char* bulletType, bool sv_penetration_type );
	float ScaleDamage( Player* player, float damage, float armor_ratio, int hitgroup );
	bool TraceToExit( CGameTrace& enterTrace, CGameTrace& exitTrace, vec3_t startPosition, vec3_t direction );

	bool HandleBulletPenetration( int from_team, 
		WeaponInfo* weaponData, 
		CGameTrace& enterTrace, 
		vec3_t& eyePosition, 
		vec3_t direction, 
		int& possibleHitsRemaining, 
		float& currentDamage, 
		float penetrationPower, 
		bool sv_penetration_type, 
		float ff_damage_reduction_bullets, 
		float ff_damage_bullet_penetration );

	float FireBullet( Player* from_player, vec3_t& from_pos, vec3_t& direction, int &hits_remaining );

public:
	float run( Player* from_player, vec3_t start_pos, vec3_t end_pos );
};

extern AutoWall g_autowall;