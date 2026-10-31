#pragma once
#define MAX_PARTICLE_ATTRIBUTES 24

#define CREATE_IF_NULL( x, cond ) \
if ( !x ) \
x = cond \

// predeclares.
class CCSGOPlayerAnimState;
class C_CSGameRules;

class CModelCache {
private:
	enum indices : size_t {
		BEGINLOCK = 32,
		ENDLOCK = 33
	};
public:
	__forceinline void BeginLock() {
		util::get_method < void(__thiscall*)(void*) >(this, BEGINLOCK)(this);
	}

	__forceinline void EndLock() {
		util::get_method < void(__thiscall*)(void*) >(this, ENDLOCK)(this);
	}
};

class CCenterPrint
{
public:
	virtual void* Create(unsigned long long unknown) = 0;
	virtual int Destroy() = 0;
	virtual int    SetTextColor(int, int, int, int) = 0;
	virtual void Print(wchar_t*) = 0;
	virtual int    PrintW(wchar_t*) = 0;
	virtual void ColorPrint(int, int, int, int, char16_t*) = 0;
	virtual int     ColorPrintW(int, int, int, int, wchar_t*) = 0;
	virtual int    Clear() = 0;
};

class C_INIT_RandomColor {
	BYTE pad_0[92];
public:
	vec3_t m_normal_color_min;
	vec3_t m_normal_color_max;
};

struct CParticleAttributeAddressTable {
	float* m_pAttributes[MAX_PARTICLE_ATTRIBUTES];
	size_t m_nFloatStrides[MAX_PARTICLE_ATTRIBUTES];

	__forceinline float* FloatAttributePtr(int nAttribute, int nParticleNumber) const {
		int block_ofs = nParticleNumber / 4;
		return m_pAttributes[nAttribute] +
			m_nFloatStrides[nAttribute] * block_ofs +
			(nParticleNumber & 3);
	}
};

struct CUtlString_simple{
	char* buffer;
	int capacity;
	int grow_size;
	int length;
};

template<class T> struct CUtlReference {
	CUtlReference* m_pNext;
	CUtlReference* m_pPrev;
	T* m_pObject;
};

template<class T> struct CUtlIntrusiveList {
	T* m_pHead;
};
template<class T> struct CUtlIntrusiveDList : public CUtlIntrusiveList<T> {};
template<class T> struct CUtlReferenceList : public CUtlIntrusiveDList< CUtlReference<T> > {};

class CParticleSystemDefinition {
	BYTE pad_0[308];
public:
	CUtlString_simple m_Name;
};

class CParticleCollection {
	BYTE pad_0[48];//0
public:
	int m_nActiveParticles;//48
private:
	BYTE pad_1[12];//52
public:
	CUtlReference<CParticleSystemDefinition> m_pDef;//64
private:
	BYTE pad_2[60];//80
public:
	CParticleCollection* m_pParent;//136
private:
	BYTE pad_3[84];//140
public:
	CParticleAttributeAddressTable m_ParticleAttributes;//224
};

class CSGO
{
public:
	using MD5_PseudoRandom_t = uint32_t( __thiscall* )( uint32_t );
	using GetGlowObjectManager_t = CGlowObjectManager * ( __cdecl* )( );
	using RandomSeed_t = void( __cdecl* )( int );
	using RandomInt_t = int( __cdecl* )( int, int );
	using RandomFloat_t = float( __cdecl* )( float, float );
	using IsBreakableEntity_t = bool( __thiscall* )( Entity* );
	using AngleMatrix_t = void( __fastcall* )( const ang_t&, matrix3x4_t& );
	using LoadFromBuffer_t = bool( __thiscall* )( KeyValues*, const char*, const char*, void*, void*, void* );
	using ServerRankRevealAll_t = bool( __cdecl* )( CCSUsrMsg_ServerRankRevealAll* );
	using IsReady_t = void( __cdecl* )( );
	using ShowAndUpdateSelection_t = void( __thiscall* )( CHudElement*, int, Weapon*, bool );
	using GetEconItemView_t = C_EconItemView * ( __thiscall* )( Weapon* );
	using GetStaticData_t = CEconItemDefinition * ( __thiscall* )( C_EconItemView* );
	using ConcatTransforms_t = void( __fastcall* )( );
	using BeamAlloc_t = Beam_t * ( __thiscall* )( IViewRenderBeams*, bool );
	using SetupBeam_t = void( __stdcall* )( Beam_t*, const BeamInfo_t& );
	using ClearNotices_t = void( __thiscall* )( KillFeed_t* );
	using AddListenerEntity_t = void( __stdcall* )( IEntityListener* );
	using GetShotgunSpread_t = void( __stdcall* )( int, int, int, float*, float* );
	using CL_ReadPackets_t = void( __cdecl* )( bool );
	using SetCollisionBounds_t = void( __thiscall* )( void*, const vec3_t&, const vec3_t& );

public:
	bool m_done;

	// module objects.
	PE::Module m_kernel32_dll;
	PE::Module m_user32_dll;
	PE::Module m_shell32_dll;
	PE::Module m_shlwapi_dll;
	PE::Module m_client_dll;
	PE::Module m_server_dll;
	PE::Module m_engine_dll;
	PE::Module m_vstdlib_dll;
	PE::Module m_tier0_dll;
	PE::Module m_serverbrowser_dll;
	PE::Module m_vguimatsurface_dll;
public:
	// interface ptrs.
	CHLClient* m_client;
	ICvar* m_cvar;
	IVEngineClient* m_engine;
	IClientEntityList* m_entlist;
	IInputSystem* m_input_system;
	ISurface* m_surface;
	IPanel* m_panel;
	IEngineVGui* m_engine_vgui;
	CPrediction* m_prediction;
	IEngineTrace* m_engine_trace;
	CGameMovement* m_game_movement;
	IVRenderView* m_render_view;
	CViewRender* m_view_render;
	IVModelRender* m_model_render;
	IMaterialSystem* m_material_system;
	CStudioRenderContext* m_studio_render;
	IVModelInfo* m_model_info;
	IVDebugOverlay* m_debug_overlay;
	IPhysicsSurfaceProps* m_phys_props;
	IGameEventManager2* m_game_events;
	ILocalize* m_localize;
	INetworkStringTableContainer* m_networkstringtable;
	IEngineSound* m_sound;
	IClientLeafSystem* m_leaf_system;
	CEconItemSchema* m_item_schema;

	IClientMode* m_client_mode;
	CGlobalVarsBase* m_globals;
	CInput* m_input;
	IMoveHelper* m_move_helper;
	INetChannel* m_net;

	CGlowObjectManager* m_glow;
	CClientState* m_cl;
	CGame* m_game;
	CRender* m_render;
	IMemAlloc* m_mem_alloc;
	IClientShadowMgr* m_shadow_mgr;
	// IClientEntityListener** m_entity_listeners;
	CHud* m_hud;
	C_CSGameRules* m_gamerules;
	IViewRenderBeams* m_beams;
	CModelCache* m_model_cache;
	CCenterPrint* m_center_print;
	void* m_radar;
	void* m_hookable_cl;

public:
	// convars.
	ConVar* clear;
	ConVar* toggleconsole;
	ConVar* name;
	ConVar* sv_maxunlag;
	ConVar* sv_gravity;
	ConVar* sv_jump_impulse;
	ConVar* sv_enablebunnyhopping;
	ConVar* sv_airaccelerate;
	ConVar* sv_friction;
	ConVar* sv_stopspeed;
	ConVar* cl_interp;
	ConVar* cl_interp_ratio;
	ConVar* cl_updaterate;
	ConVar* cl_cmdrate;
	ConVar* cl_lagcompensation;
	ConVar* mp_teammates_are_enemies;
	ConVar* weapon_debug_spread_show;
	ConVar* net_showfragments;
	ConVar* molotov_throw_detonate_time;
	ConVar* weapon_molotov_maxdetonateslope;
	ConVar* weapon_recoil_scale;
	ConVar* view_recoil_tracking;
	ConVar* cl_fullupdate;
	ConVar* r_DrawSpecificStaticProp;
	ConVar* cl_crosshair_sniper_width;
	ConVar* hud_scaling;
	ConVar* sv_clip_penetration_traces_to_players;
	ConVar* weapon_accuracy_shotgun_spread_patterns;
	ConVar* sv_maxusrcmdprocessticks;
	ConVar* cl_pred_doresetlatch;
	ConVar* cl_forwardspeed;
	ConVar* cl_backspeed;
	ConVar* cl_sidespeed;
	ConVar* sv_penetration_type;
	ConVar* ff_damage_reduction_bullets;
	ConVar* ff_damage_bullet_penetration;
	ConVar* sv_cheats;
	ConVar* r_skin;
	ConVar* r_drawallrenderables;
	ConVar* mp_maxrounds;
	ConVar* cl_pred_optimize;
public:
	// functions.
	GetGlowObjectManager_t   GetGlowObjectManager;
	MD5_PseudoRandom_t	     MD5_PseudoRandom;
	Address                  SetAbsAngles;
	Address				     SetAbsOrigin;
	Address                  InvalidateBoneCache;
	Address				     LockStudioHdr;
	RandomSeed_t             RandomSeed;
	RandomInt_t              RandomInt;
	RandomFloat_t            RandomFloat;
	IsBreakableEntity_t      IsBreakableEntity;
	Address	                 SetAbsVelocity;
	AngleMatrix_t            AngleMatrix;
	Address					 DoProceduralFootPlant;
	Address					 ComputeHitboxSurroundingBox;
	Address					 GetSequenceActivity;
	LoadFromBuffer_t		 LoadFromBuffer;
	ServerRankRevealAll_t    ServerRankRevealAll;
	Address					 HasC4;
	Address					 InvalidatePhysicsRecursive;
	IsReady_t				 IsReady;
	ShowAndUpdateSelection_t ShowAndUpdateSelection;
	GetEconItemView_t        GetEconItemView;
	GetStaticData_t          GetStaticData;
	BeamAlloc_t              BeamAlloc;
	SetupBeam_t              SetupBeam;
	ClearNotices_t           ClearNotices;
	AddListenerEntity_t      AddListenerEntity;
	GetShotgunSpread_t       GetShotgunSpread;
	CL_ReadPackets_t         CL_ReadPackets;
	SetCollisionBounds_t     SetCollisionBounds;
	IDirect3DDevice9* m_device;

	size_t BoneAccessor;
	size_t AnimOverlay;
	size_t SpawnTime;
	size_t MostRecentModelBoneCounter;
	size_t LastBoneSetupTime;
	size_t IsLocalPlayer;
	size_t PlayerAnimState;
	size_t studioHdr;
	Address GetFirstSequenceAnimTag;

	Address UTIL_TraceLine;
	Address CTraceFilterSimple_vmt;
	Address CTraceFilterSkipTwoEntities_vmt;

	int* m_nPredictionRandomSeed;
	Player* m_pPredictionPlayer;

public:
	// initialize class.
	bool init();
	void patch();
};

extern CSGO g_csgo;

namespace game
{
	__forceinline int HitboxToHitgroup( int hitbox )
	{
		switch ( hitbox )
		{
		case 0:  // HITBOX_HEAD
		case 1:  // HITBOX_NECK
		case 2:  // HITBOX_LOWER_NECK
			return 1;  // HITGROUP_HEAD
		case 7:  // HITBOX_UPPER_CHEST
		case 6:  // HITBOX_CHEST
		case 5:  // HITBOX_THORAX
		case 18: // HITBOX_L_UPPER_ARM
		case 16: // HITBOX_R_UPPER_ARM
			return 2;  // HITGROUP_CHEST
		case 3:  // HITBOX_PELVIS
		case 9:  // HITBOX_L_THIGH
		case 8:  // HITBOX_R_THIGH
		case 4:  // HITBOX_BODY
			return 3;  // HITGROUP_STOMACH
		case 11: // HITBOX_L_CALF
		case 12: // HITBOX_L_FOOT
			return 6;  // HITGROUP_LEFTLEG
		case 10: // HITBOX_R_CALF
		case 13: // HITBOX_R_FOOT
			return 7;  // HITGROUP_RIGHTLEG
		case 19: // HITBOX_L_FOREARM
		case 15: // HITBOX_L_HAND
			return 4;  // HITGROUP_LEFTARM
		case 17: // HITBOX_R_FOREARM
		case 14: // HITBOX_R_HAND
			return 5;  // HITGROUP_RIGHTARM
		default:
			return 0;  // HITGROUP_GENERIC
		}
	}

	__forceinline float GetClientInterpAmount()
	{
		static auto* sv_minupdaterate = g_csgo.m_cvar->FindVar( HASH( "sv_minupdaterate" ) );
		static auto* sv_maxupdaterate = g_csgo.m_cvar->FindVar( HASH( "sv_maxupdaterate" ) );

		static auto* sv_client_min_interp_ratio = g_csgo.m_cvar->FindVar( HASH( "sv_client_min_interp_ratio" ) );
		static auto* sv_client_max_interp_ratio = g_csgo.m_cvar->FindVar( HASH( "sv_client_max_interp_ratio" ) );

		float minupdaterate = sv_minupdaterate->GetFloat();
		float maxupdaterate = sv_maxupdaterate->GetFloat();

		float min_interp = sv_client_min_interp_ratio->GetFloat();
		float max_interp = sv_client_max_interp_ratio->GetFloat();

		float updaterate = g_csgo.cl_updaterate->GetFloat();

		float flLerpAmount = g_csgo.cl_interp->GetFloat();
		float flLerpRatio = g_csgo.cl_interp_ratio->GetFloat();

		flLerpRatio = std::clamp<float>( flLerpRatio, max_interp, min_interp );
		if ( flLerpRatio == 0.0f )
			flLerpRatio = 1.0f;

		float updateRate = std::clamp<float>( updaterate, minupdaterate, maxupdaterate );
		return std::fminf( flLerpAmount, flLerpRatio / updateRate );
	}

	__forceinline int TIME_TO_TICKS( float time )
	{
		return (int)( 0.5f + time / g_csgo.m_globals->m_interval );
	}

	__forceinline float TICKS_TO_TIME( int ticks )
	{
		return g_csgo.m_globals->m_interval * (float)( ticks );
	}

	__forceinline bool IsFakePlayer( int index )
	{
		player_info_t info;
		if ( g_csgo.m_engine->GetPlayerInfo( index, &info ) )
			return info.m_fake_player;

		return false;
	}

	__forceinline bool IsValidHitgroup( int index )
	{
		if ( ( index >= HITGROUP_HEAD && index <= HITGROUP_RIGHTLEG ) || index == HITGROUP_GEAR )
			return true;

		return false;
	}

	// note - dex; all of the static sigscans here should be moved to CSGO class... funcs that rely on these do 2 test statements to make sure the data is initialized
	//             also, nitro, is there a reason these are not __forceinline? i forget if you told me they must be inline.
	inline void CreateAnimationState( Player* holder, CCSGOPlayerAnimState* state )
	{
		using CreateAnimState_t = void( __thiscall* )( CCSGOPlayerAnimState*, Player* );

		static auto func = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46" ) ).as< CreateAnimState_t >();
		if ( !func )
			return;

		func( state, holder );
	}

	inline void UpdateAnimationState( CCSGOPlayerAnimState* state, ang_t ang )
	{
		static auto func = pattern::find( g_csgo.m_client_dll, XOR( "E8 ? ? ? ? 0F 57 C0 0F 11 86" ) ).rel32< uintptr_t >( 0x1 );
		if ( !func )
			return;

		__asm {
			push  0
			mov	  ecx, state
			movss xmm1, dword ptr[ ang + 4 ]
			movss xmm2, dword ptr[ ang ]
			call  func
		}
	}

	inline void ResetAnimationState( CCSGOPlayerAnimState* state )
	{
		using ResetAnimState_t = void( __thiscall* )( CCSGOPlayerAnimState* );

		static auto func = pattern::find( g_csgo.m_client_dll, XOR( "56 6A 01 68 ? ? ? ? 8B F1" ) ).as< ResetAnimState_t >();
		if ( !func )
			return;

		func( state );
	}

	__forceinline void UTIL_ClipTraceToPlayers( const vec3_t& start, const vec3_t& end, uint32_t mask, ITraceFilter* filter, CGameTrace* tr, float range )
	{
		static auto func = pattern::find( g_csgo.m_client_dll, XOR( "E8 ? ? ? ? 83 C4 14 8A 56 37" ) ).rel32< uintptr_t >( 0x1 );
		if ( !func )
			return;

		__asm {
			mov  ecx, start
			mov	 edx, end
			push range
			push tr
			push filter
			push mask
			call func
			add	 esp, 16
		}
	}

	__forceinline void ConcatTransforms( const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& output )
	{
		static auto func = pattern::find( g_csgo.m_client_dll, XOR( "E8 ? ? ? ? 0F 28 44 24" ) ).rel32< uintptr_t >( 0x1 );
		if ( !func )
			return;

		__asm {
			push output
			lea  edx, in2
			lea  ecx, in1
			call func
			add  esp, 4
		}
	}

	template<class T = DWORD>
	__forceinline T* FindHudElement( const char* name )
	{
		static auto pThis = *pattern::find( g_csgo.m_client_dll, XOR( "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08" ) ).add( 0x1 ).as< DWORD**>();

		static auto find_hud_element = pattern::find( g_csgo.m_client_dll, ( "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39" ) ).as<DWORD( __thiscall* )( void*, const char* )>();
		return (T*)find_hud_element( pThis, name );
	}

	__forceinline void LoadNamedSky( const char* txt )
	{
		using func = void( __fastcall* )( const char* );
		static auto LoadBro1337 = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45" ) ).as< func >();
		return LoadBro1337( txt );
	}

	bool   IsBreakable( Entity* ent );
	Beam_t* CreateGenericBeam( const BeamInfo_t& beam_info );
}