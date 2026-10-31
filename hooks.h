#pragma once

//using CLMove_t = void( __vectorcall* )( float, bool );
//inline CLMove_t oCLMove;

using PhysicsSimulate_t = void( __thiscall* )( void* );
inline PhysicsSimulate_t oPhysicsSimulate;

using GetExposureRange_t = void( __fastcall* )( float*, float* );
inline GetExposureRange_t oGetExposureRange;

using DrawModelExecuteFn = void( __thiscall* )( void*, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld );
inline DrawModelExecuteFn oDrawModelExecute;

class Hooks
{
public:
	void init();

public:
	// forward declarations
	class IRecipientFilter;

	// prototypes.
	using PaintTraverse_t = void( __thiscall* )( void*, VPANEL, bool, bool );
	using DoPostScreenSpaceEffects_t = bool( __thiscall* )( void*, CViewSetup* );
	using CreateMove_t = bool( __thiscall* )( void*, float, CUserCmd* );
	using LevelInitPostEntity_t = void( __thiscall* )( void* );
	using LevelShutdown_t = void( __thiscall* )( void* );
	using LevelInitPreEntity_t = void( __thiscall* )( void*, const char* );
	using IN_KeyEvent_t = int( __thiscall* )( void*, int, int, const char* );  
	using FrameStageNotify_t = void( __thiscall* )( void*, Stage_t );
	using GetPredDescMap_t = datamap_t * ( __thiscall* )( void* );
	using UpdateClientSideAnimation_t = void( __thiscall* )( void* );
	using GetActiveWeapon_t = Weapon * ( __thiscall* )( void* );
	using DoExtraBoneProcessing_t = void( __thiscall* )( void*, int, int, int, int, int, int );
	using BuildTransformations_t = void( __thiscall* )( void*, CStudioHdr*, int, int, int, int, int );
	using CalcViewModelView_t = void( __thiscall* )( void*, vec3_t&, ang_t& );
	using InPrediction_t = bool( __thiscall* )( void* );
	using OverrideView_t = void( __thiscall* )( void*, CViewSetup* );
	using LockCursor_t = void( __thiscall* )( void* );
	using PacketStart_t = void(__thiscall*)(void*, int, int);

	using PostEntityPacketReceived_t = void( __thiscall* )( void* );
	using PostNetworkDataReceived_t = void( __thiscall* )( void*, int );
	using RunCommand_t = void( __thiscall* )( void*, Entity*, CUserCmd*, IMoveHelper* );

	using ProcessPacket_t = void( __thiscall* )( void*, void*, bool );
	using SendDatagram_t = int( __thiscall* )( void*, void* );
	using SendNetMsg_t = bool(__thiscall*)(void*, INetMessage&, bool, bool);

	// using CanPacket_t                = bool( __thiscall* )( void* );
	using GetScreenSize_t = void( __thiscall* )( void*, int&, int& );
	using Push3DView_t = void( __thiscall* )( void*, CViewSetup&, int, void*, void* );
	using SceneEnd_t = void( __thiscall* )( void* );
	using ComputeShadowDepthTextures_t = void( __thiscall* )( void*, const CViewSetup&, bool );
	using GetInt_t = int( __thiscall* )( void* );
	using GetBool_t = bool( __thiscall* )( void* );
	using IsConnected_t = bool( __thiscall* )( void* );
	using IsHLTV_t = bool( __thiscall* )( void* );
	using OnEntityCreated_t = void( __thiscall* )( void*, Entity* );
	using OnEntityDeleted_t = void( __thiscall* )( void*, Entity* );
	using RenderSmokeOverlay_t = void( __thiscall* )( void*, bool );
	using ShouldDrawFog_t = bool( __thiscall* )( void* );
	using ShouldDrawParticles_t = bool( __thiscall* )( void* );
	using Render2DEffectsPostHUD_t = void( __thiscall* )( void*, const CViewSetup& );
	using OnRenderStart_t = void( __thiscall* )( void* );
	using RenderView_t = void( __thiscall* )( void*, const CViewSetup&, const CViewSetup&, int, int );
	using OnScreenSizeChanged_t = void( __thiscall* )( void*, int, int );
	using OverrideConfig_t = bool( __thiscall* )( void*, MaterialSystem_Config_t*, bool );
	using PostDataUpdate_t = void( __thiscall* )( void*, DataUpdateType_t );
	using TempEntities_t = bool( __thiscall* )( void*, void* );
	using EmitSound_t = void( __thiscall* )( void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, float, int, int, int, const vec3_t*, const vec3_t*, void*, bool, float, int );
	// using PreDataUpdate_t            = void( __thiscall* )( void*, DataUpdateType_t );
	using StandardBlendingRules_t = void( __thiscall* )( void*, int, int, int, int, int );
	using DrawSetColor_t = void( __thiscall* )( void*, int, int, int, int );

public:
	bool                     TempEntities( void* msg );
	void					 PacketStart( int incoming_sequence, int outgoing_acknowledged );
	bool                     DoPostScreenSpaceEffects( CViewSetup* setup );
	void                     Shutdown();
	bool                     CreateMove( float input_sample_time, CUserCmd* cmd );
	void                     LevelInitPostEntity();
	void                     LevelShutdown();
	//int                      IN_KeyEvent( int event, int key, const char* bind );
	void                     LevelInitPreEntity( const char* map );
	void                     FrameStageNotify( Stage_t stage );
	void                     UpdateClientSideAnimation();
	datamap_t* GetPredDescMap();
	bool                     InPrediction();
	bool                     ShouldDrawParticles();
	bool                     ShouldDrawFog();
	void                     OverrideView( CViewSetup* view );
	void                     LockCursor();
	void                     OnScreenSizeChanged( int oldwidth, int oldheight );

	void                     PostEntityPacketReceived();
	void                     PostNetworkDataReceived( int commands_acknowledged );
	void                     RunCommand( Entity* ent, CUserCmd* cmd, IMoveHelper* movehelper );

	int                      SendDatagram( void* data );
	void                     ProcessPacket( void* packet, bool header );
	bool                     SendNetMsg(INetMessage& msg, bool bForceReliable, bool bVoice);

	//void                     GetScreenSize( int& w, int& h );
	void                     ComputeShadowDepthTextures( const CViewSetup& view, bool unk );
	int                      DebugSpreadGetInt();
	void                     DoExtraBoneProcessing( int a2, int a3, int a4, int a5, int a6, int a7 );
	void                     BuildTransformations( CStudioHdr* a2, int a3, int a4, int a5, int a6, int a7 );
	bool                     IsConnected();
	bool                     IsHLTV();
	void                     EmitSound( IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, float flAttenuation, int nSeed, int iFlags, int iPitch, const vec3_t* pOrigin, const vec3_t* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity );
	void                     RenderSmokeOverlay( bool unk );
	void                     RenderView( const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw );
	void                     Render2DEffectsPostHUD( const CViewSetup& setup );
	bool                     OverrideConfig( MaterialSystem_Config_t* config, bool update );
	void                     PostDataUpdate( DataUpdateType_t type );
	void                     StandardBlendingRules( int a2, int a3, int a4, int a5, int a6 );
	void original_cl_move( float accumulated_extra_sample, bool final_tick );
	static void __fastcall cl_move_naked();
	static HRESULT WINAPI Reset( IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParameters );
	static long WINAPI Present( IDirect3DDevice9* pDevice, void* a1, void* a2, HWND a3, void* a4 );

	static LRESULT WINAPI WndProc( HWND wnd, uint32_t msg, WPARAM wp, LPARAM lp );
	float GetViewmodelFOV();

public:
	// vmts.
	VMT m_client_mode;
	VMT m_client;
	VMT m_client_state;
	VMT m_engine;
	VMT m_engine_sound;
	VMT m_prediction;
	VMT m_surface;
	VMT m_render;
	VMT m_net_channel;
	VMT m_shadow_mgr;
	VMT m_view_render;
	VMT m_material_system;
	VMT m_device;

	// player shit.
	std::array< VMT, 64 > m_player;
	std::array< VMT, 64 > m_player_networkable;

	// cvars
	VMT m_debug_spread;
	VMT cl_pred_doresetlatch;

	// wndproc old ptr.
	WNDPROC m_old_wndproc;

	// old player create fn.
	DoExtraBoneProcessing_t     m_DoExtraBoneProcessing;
	UpdateClientSideAnimation_t m_UpdateClientSideAnimation;
	GetPredDescMap_t            m_GetPredDescMap;
	GetActiveWeapon_t           m_GetActiveWeapon;
	BuildTransformations_t      m_BuildTransformations;
	StandardBlendingRules_t     m_StandardBlendingRules;
	PostDataUpdate_t            m_PostDataUpdate;

	// netvar proxies.
	RecvVarProxy_t m_Pitch_original;
	RecvVarProxy_t m_Body_original;
	RecvVarProxy_t m_Force_original;
	RecvVarProxy_t m_AbsYaw_original;
	RecvVarProxy_t m_SimTime_original;
};

// note - dex; these are defined in player.cpp.
class CustomEntityListener : public IEntityListener
{
public:
	virtual void OnEntityCreated( Entity* ent );
	virtual void OnEntityDeleted( Entity* ent );

	__forceinline void init()
	{
		g_csgo.AddListenerEntity( this );
	}
};

extern Hooks                g_hooks;
extern CustomEntityListener g_custom_entity_listener;