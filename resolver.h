#pragma once

class ShotRecord;

class Resolver
{
public:
	enum Modes : size_t
	{
		RESOLVE_AIR,
		RESOLVE_STAND,
		RESOLVE_UPDATE, RESOLVE_THRESHOLD = RESOLVE_UPDATE,
		RESOLVE_MOVE,
		RESOLVE_NONE
	};

public:
	void OnBodyUpdate(Player* player, float value);
	void MatchShot(AimPlayer* data, LagRecord* record);

public:
	float SnapToNearest(float angle, std::vector<float> angles);
	std::pair<float, bool> FreestandYaw(AimPlayer* data, Player* from_player, std::vector<AdaptiveAngle> angles, bool get_worst = false); // freestand angle & whether that angle is actually hidden or not.

public:
	void CheckAnimations(LagRecord* record, AimPlayer* data);
	int GetTimingOffset(AimPlayer* data);

	void SetConfidence(AimPlayer* data, LagRecord* record, bool balance_adjust, int clamp_preference);
	void Resolve(AimPlayer* data, LagRecord* record);

public:
	void SetFacingPlayer(AimPlayer* data, LagRecord* record);

public:
	enum ClampPreference_t
	{
		unsure = 0,
		bias_clamped,
		bias_unclamped,
		neutral
	};

	void ResolveStand(AimPlayer* data, LagRecord* record);
	void ResolveWalk(AimPlayer* data, LagRecord* record);
	void ResolveAir(AimPlayer* data, LagRecord* record);

	void CheckUnclamped(AimPlayer* data, float angle);

	std::vector<float> m_test_angles;

	void UpdateTransition(AimPlayer* data, LagRecord* record, LagRecord* previous, CCSGOPlayerAnimState* state);
	void ResolveTwist(AimPlayer* data, LagRecord* record, LagRecord* previous, CCSGOPlayerAnimState* state);

public:
	void process_override();

public:
	LagRecord m_resolver_record;
};

extern Resolver g_resolver;