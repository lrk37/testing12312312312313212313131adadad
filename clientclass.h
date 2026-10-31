#pragma once

// pre-declares.
class CRecvProxyData;
class RecvProp;
class RecvTable;

// prototypes.
// these are networkable entity ptrs.
using CreateClientClass_t = void* ( __cdecl* )( int index, int serial );
using CreateEvent_t = void* ( __cdecl* )( );

// other prototypes.
using ArrayLengthRecvProxy_t = void( __cdecl* )( void* ptr, int id, int len );
using RecvVarProxy_t = void( __cdecl* )( CRecvProxyData* data, void* struct_ptr, void* out );
using DataTableRecvVarProxy_t = void( __cdecl* )( RecvProp* prop, void** out, void* data, int id );

// types of props.
enum SendPropType
{
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_String,
	DPT_Array,
	DPT_DataTable
};

class RecvProp
{
public:
	char* m_pVarName;
	int						 m_RecvType;
	int						 m_Flags;
	int						 m_StringBufferSize;
	bool					 m_bInsideArray;
	const void* m_pExtraData;
	RecvProp* m_pArrayProp;
	ArrayLengthRecvProxy_t	 m_ArrayLengthProxy;
	RecvVarProxy_t			 m_ProxyFn;
	DataTableRecvVarProxy_t	 m_DataTableProxyFn;
	RecvTable* m_pDataTable;
	int						 m_Offset;
	int						 m_ElementStride;
	int						 m_nElements;
	const char* m_pParentArrayPropName;
};

class RecvTable
{
public:
	RecvProp* m_pProps;
	int				 m_nProps;
	void* m_pDecoder;
	char* m_pNetTableName;
	bool			 m_bInitialized;
	bool			 m_bInMainList;
};

class ClientClass
{
public:
	CreateClientClass_t m_pCreate;
	CreateEvent_t	    m_pCreateEvent;
	char* m_pNetworkName;
	RecvTable* m_pRecvTable;
	ClientClass* m_pNext;
	int					m_ClassID;
};

class DVariant
{
public:
	union
	{
		float	     m_Float;
		long	     m_Int;
		char* m_pString;
		void* m_pData;
		float	     m_Vector[ 3 ];
		long long	 m_Int64;
	};

	SendPropType     m_Type;
};

class CRecvProxyData
{
public:
	const RecvProp* m_pRecvProp;
	DVariant		 m_Value;
	int				 m_iElement;
	int				 m_ObjectID;
};

enum ClassId_t {
	_CAI_BaseNPC = 0,
	_CAK47,
	_CBaseAnimating,
	_CBaseAnimatingOverlay,
	_CBaseAttributableItem,
	_CBaseButton,
	_CBaseCombatCharacter,
	_CBaseCombatWeapon,
	_CBaseCSGrenade,
	_CBaseCSGrenadeProjectile,
	_CBaseDoor,
	_CBaseEntity,
	_CBaseFlex,
	_CBaseGrenade,
	_CBaseParticleEntity,
	_CBasePlayer,
	_CBasePropDoor,
	_CBaseTeamObjectiveResource,
	_CBaseTempEntity,
	_CBaseToggle,
	_CBaseTrigger,
	_CBaseViewModel,
	_CBaseVPhysicsTrigger,
	_CBaseWeaponWorldModel,
	_CBeam,
	_CBeamSpotlight,
	_CBoneFollower,
	_CBreakableProp,
	_CBreakableSurface,
	_CC4,
	_CCascadeLight,
	_CChicken,
	_CColorCorrection,
	_CColorCorrectionVolume,
	_CCSGameRulesProxy,
	_CCSPlayer,
	_CCSPlayerResource,
	_CCSRagdoll,
	_CCSTeam,
	_CDEagle,
	_CDecoyGrenade,
	_CDecoyProjectile,
	_CDynamicLight,
	_CDynamicProp,
	_CEconEntity,
	_CEconWearable,
	_CEmbers,
	_CEntityDissolve,
	_CEntityFlame,
	_CEntityFreezing,
	_CEntityParticleTrail,
	_CEnvAmbientLight,
	_CEnvDetailController,
	_CEnvDOFController,
	_CEnvParticleScript,
	_CEnvProjectedTexture,
	_CEnvQuadraticBeam,
	_CEnvScreenEffect,
	_CEnvScreenOverlay,
	_CEnvTonemapController,
	_CEnvWind,
	_CFEPlayerDecal,
	_CFireCrackerBlast,
	_CFireSmoke,
	_CFireTrail,
	_CFish,
	_CFlashbang,
	_CFogController,
	_CFootstepControl,
	_CFunc_Dust,
	_CFunc_LOD,
	_CFuncAreaPortalWindow,
	_CFuncBrush,
	_CFuncConveyor,
	_CFuncLadder,
	_CFuncMonitor,
	_CFuncMoveLinear,
	_CFuncOccluder,
	_CFuncReflectiveGlass,
	_CFuncRotating,
	_CFuncSmokeVolume,
	_CFuncTrackTrain,
	_CGameRulesProxy,
	_CHandleTest,
	_CHEGrenade,
	_CHostage,
	_CHostageCarriableProp,
	_CIncendiaryGrenade,
	_CInferno,
	_CInfoLadderDismount,
	_CInfoOverlayAccessor,
	_CItem_Healthshot,
	_CItemDogTags,
	_CKnife,
	_CKnifeGG,
	_CLightGlow,
	_CMaterialModifyControl,
	_CMolotovGrenade,
	_CMolotovProjectile,
	_CMovieDisplay,
	_CParticleFire,
	_CParticlePerformanceMonitor,
	_CParticleSystem,
	_CPhysBox,
	_CPhysBoxMultiplayer,
	_CPhysicsProp,
	_CPhysicsPropMultiplayer,
	_CPhysMagnet,
	_CPlantedC4,
	_CPlasma,
	_CPlayerResource,
	_CPointCamera,
	_CPointCommentaryNode,
	_CPointWorldText,
	_CPoseController,
	_CPostProcessController,
	_CPrecipitation,
	_CPrecipitationBlocker,
	_CPredictedViewModel,
	_CProp_Hallucination,
	_CPropDoorRotating,
	_CPropJeep,
	_CPropVehicleDriveable,
	_CRagdollManager,
	_CRagdollProp,
	_CRagdollPropAttached,
	_CRopeKeyframe,
	_CSCAR17,
	_CSceneEntity,
	_CSensorGrenade,
	_CSensorGrenadeProjectile,
	_CShadowControl,
	_CSlideshowDisplay,
	_CSmokeGrenade,
	_CSmokeGrenadeProjectile,
	_CSmokeStack,
	_CSpatialEntity,
	_CSpotlightEnd,
	_CSprite,
	_CSpriteOriented,
	_CSpriteTrail,
	_CStatueProp,
	_CSteamJet,
	_CSun,
	_CSunlightShadowControl,
	_CTeam,
	_CTeamplayRoundBasedRulesProxy,
	_CTEArmorRicochet,
	_CTEBaseBeam,
	_CTEBeamEntPoint,
	_CTEBeamEnts,
	_CTEBeamFollow,
	_CTEBeamLaser,
	_CTEBeamPoints,
	_CTEBeamRing,
	_CTEBeamRingPoint,
	_CTEBeamSpline,
	_CTEBloodSprite,
	_CTEBloodStream,
	_CTEBreakModel,
	_CTEBSPDecal,
	_CTEBubbles,
	_CTEBubbleTrail,
	_CTEClientProjectile,
	_CTEDecal,
	_CTEDust,
	_CTEDynamicLight,
	_CTEEffectDispatch,
	_CTEEnergySplash,
	_CTEExplosion,
	_CTEFireBullets,
	_CTEFizz,
	_CTEFootprintDecal,
	_CTEFoundryHelpers,
	_CTEGaussExplosion,
	_CTEGlowSprite,
	_CTEImpact,
	_CTEKillPlayerAttachments,
	_CTELargeFunnel,
	_CTEMetalSparks,
	_CTEMuzzleFlash,
	_CTEParticleSystem,
	_CTEPhysicsProp,
	_CTEPlantBomb,
	_CTEPlayerAnimEvent,
	_CTEPlayerDecal,
	_CTEProjectedDecal,
	_CTERadioIcon,
	_CTEShatterSurface,
	_CTEShowLine,
	_CTesla,
	_CTESmoke,
	_CTESparks,
	_CTESprite,
	_CTESpriteSpray,
	_CTest_ProxyToggle_Networkable,
	_CTestTraceline,
	_CTEWorldDecal,
	_CTriggerPlayerMovement,
	_CTriggerSoundOperator,
	_CVGuiScreen,
	_CVoteController,
	_CWaterBullet,
	_CWaterLODControl,
	_CWeaponAug,
	_CWeaponAWP,
	_CWeaponBaseItem,
	_CWeaponBizon,
	_CWeaponCSBase,
	_CWeaponCSBaseGun,
	_CWeaponCycler,
	_CWeaponElite,
	_CWeaponFamas,
	_CWeaponFiveSeven,
	_CWeaponG3SG1,
	_CWeaponGalil,
	_CWeaponGalilAR,
	_CWeaponGlock,
	_CWeaponHKP2000,
	_CWeaponM249,
	_CWeaponM3,
	_CWeaponM4A1,
	_CWeaponMAC10,
	_CWeaponMag7,
	_CWeaponMP5Navy,
	_CWeaponMP7,
	_CWeaponMP9,
	_CWeaponNegev,
	_CWeaponNOVA,
	_CWeaponP228,
	_CWeaponP250,
	_CWeaponP90,
	_CWeaponSawedoff,
	_CWeaponSCAR20,
	_CWeaponScout,
	_CWeaponSG550,
	_CWeaponSG552,
	_CWeaponSG556,
	_CWeaponSSG08,
	_CWeaponTaser,
	_CWeaponTec9,
	_CWeaponTMP,
	_CWeaponUMP45,
	_CWeaponUSP,
	_CWeaponXM1014,
	_CWorld,
	_DustTrail,
	_MovieExplosion,
	_ParticleSmokeGrenade,
	_RocketTrail,
	_SmokeTrail,
	_SporeExplosion,
	_SporeTrail,
	_MAX_CLASSID
};