#pragma once

enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY = 0,
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT, // an optional primary attack for variation in animation. For example, the knife toggles between left AND right slash animations.
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY_SPECIAL1,
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT_SPECIAL1, // an optional primary special attack for variation in animation.
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY,
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY_SPECIAL1,
	PLAYERANIMEVENT_GRENADE_PULL_PIN,
	PLAYERANIMEVENT_THROW_GRENADE,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_RELOAD,
	PLAYERANIMEVENT_RELOAD_START,	///< w_model partial reload for shotguns
	PLAYERANIMEVENT_RELOAD_LOOP,	///< w_model partial reload for shotguns
	PLAYERANIMEVENT_RELOAD_END,		///< w_model partial reload for shotguns
	PLAYERANIMEVENT_CLEAR_FIRING,	///< clear animations on the firing layer
	PLAYERANIMEVENT_DEPLOY,			///< Used to play deploy animations on third person models.
	PLAYERANIMEVENT_SILENCER_ATTACH,
	PLAYERANIMEVENT_SILENCER_DETACH,

	// new events
	PLAYERANIMEVENT_THROW_GRENADE_UNDERHAND,
	PLAYERANIMEVENT_CATCH_WEAPON,
	PLAYERANIMEVENT_COUNT
};

class ServerAnimations
{
public:
	void build_activity_modifiers( CCSGOPlayerAnimState* state );
	void SetLayerSequence( CCSGOPlayerAnimState* state, C_AnimationLayer* layer, int sequence );
	int SelectSequenceFromActMods( int32_t activity );

public:
	void HandleWeapons( CCSGOPlayerAnimState* pAnimState, C_AnimationLayer* pLayers, int viewmodel_act );
	void DoAnimationEvent( CCSGOPlayerAnimState* pState, int animEvent, C_AnimationLayer* pLayers );
	void DoPreAnimation( CCSGOPlayerAnimState& pred_state, CUserCmd* cmd, int viewmodel_act, bool jumping, bool apply_data );
	void OnSetupWeaponAction( CCSGOPlayerAnimState* pState );
public:
	std::pair<CCSGOPlayerAnimState, C_AnimationLayer*> PredictAnimationState( Player* player, ang_t viewangles );

public:
	float GetPredFeetRange();
	void SimulateFeetYaw( CCSGOPlayerAnimState* animstate, float lby, float eye_yaw, float& custom_feet_yaw );
	float m_fake_feet_yaw;
	int m_viewmodel_act;
	std::vector<uint16_t> m_act_mods;
	BoneArray* local_matrix;
	BoneArray* local_render_matrix;
	vec3_t last_render_setup;
	bool should_override_order;
};

extern ServerAnimations g_server_animations;