#pragma once

#define END_OF_FREE_LIST	-1
#define ENTRY_IN_USE		-2

struct GlowObjectDefinition_t
{
	Entity* m_pEntity;
	vec3_t m_vGlowColor;
	float m_flGlowAlpha;

	bool m_bGlowAlphaCappedByRenderAlpha;
	float m_flGlowAlphaFunctionOfMaxVelocity;
	float m_flGlowAlphaMax;
	float m_flGlowPulseOverdrive;
	bool m_bRenderWhenOccluded;
	bool m_bRenderWhenUnoccluded;
	bool m_bFullBloomRender;
	int m_nFullBloomStencilTestValue; // only render full bloom objects if stencil is equal to this value (value of -1 implies no stencil test)
	int m_nRenderStyle;
	int m_nSplitScreenSlot;

	// Linked list of free slots
	int m_nNextFreeSlot;

	bool unused() const { return m_nNextFreeSlot != ENTRY_IN_USE; }
};

class CGlowObjectManager
{
public:
	CUtlVector< GlowObjectDefinition_t >	m_object_definitions;
	int										m_first_free_slot;
};

enum PostProcessParameterNames_t {
	PPPN_FADE_TIME = 0,
	PPPN_LOCAL_CONTRAST_STRENGTH,
	PPPN_LOCAL_CONTRAST_EDGE_STRENGTH,
	PPPN_VIGNETTE_START,
	PPPN_VIGNETTE_END,
	PPPN_VIGNETTE_BLUR_STRENGTH,
	PPPN_FADE_TO_BLACK_STRENGTH,
	PPPN_DEPTH_BLUR_FOCAL_DISTANCE,
	PPPN_DEPTH_BLUR_STRENGTH,
	PPPN_SCREEN_BLUR_STRENGTH,
	PPPN_FILM_GRAIN_STRENGTH,

	POST_PROCESS_PARAMETER_COUNT
};

struct PostProcessParameters_t {
	PostProcessParameters_t() {
		memset(m_parameters, 0, sizeof(m_parameters));
		m_parameters[PPPN_VIGNETTE_START] = 0.8f;
		m_parameters[PPPN_VIGNETTE_END] = 1.1f;
	}

	float m_parameters[POST_PROCESS_PARAMETER_COUNT];
};

enum ParticleAttachment_t
{
	PATTACH_ABSORIGIN = 0,			// Create at absorigin, but don't follow
	PATTACH_ABSORIGIN_FOLLOW,		// Create at absorigin, and update to follow the entity
	PATTACH_CUSTOMORIGIN,			// Create at a custom origin, but don't follow
	PATTACH_CUSTOMORIGIN_FOLLOW,	// Create at a custom origin, follow relative position to specified entity
	PATTACH_POINT,					// Create on attachment point, but don't follow
	PATTACH_POINT_FOLLOW,			// Create on attachment point, and update to follow the entity
	PATTACH_EYES_FOLLOW,			// Create on eyes of the attached entity, and update to follow the entity
	PATTACH_OVERHEAD_FOLLOW,		// Create at the top of the entity's bbox
	PATTACH_WORLDORIGIN,			// Used for control points that don't attach to an entity
	PATTACH_ROOTBONE_FOLLOW,		// Create at the root bone of the entity, and update to follow

	MAX_PATTACH_TYPES,
};

struct te_tf_particle_effects_colors_t
{
	vec3_t m_vecColor1;
	vec3_t m_vecColor2;
};

struct te_tf_particle_effects_control_point_t
{
	ParticleAttachment_t m_eParticleAttachment;
	vec3_t m_vecOffset;
};

class CEffectData
{
public:
	vec3_t m_vOrigin;
	vec3_t m_vStart;
	vec3_t m_vNormal;
	ang_t m_vAngles;
	int		m_fFlags;
	EHANDLE m_hEntity;
	float	m_flScale;
	float	m_flMagnitude;
	float	m_flRadius;
	int		m_nAttachmentIndex;
	short	m_nSurfaceProp;

	// Some TF2 specific things
	int		m_nMaterial;
	int		m_nDamageType;
	int		m_nHitBox;

	unsigned char	m_nColor;

	// Color customizability
	bool							m_bCustomColors;
	te_tf_particle_effects_colors_t	m_CustomColors;

	bool									m_bControlPoint1;
	te_tf_particle_effects_control_point_t	m_ControlPoint1;
	int m_iEffectName;
};

class C_TEEffectDispatch
{
public:
	CEffectData m_EffectData;
};