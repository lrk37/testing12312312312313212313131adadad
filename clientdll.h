#pragma once

enum Stage_t
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

enum Hitgroups_t
{
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD = 1,
	HITGROUP_CHEST = 2,
	HITGROUP_STOMACH = 3,
	HITGROUP_LEFTARM = 4,
	HITGROUP_RIGHTARM = 5,
	HITGROUP_LEFTLEG = 6,
	HITGROUP_RIGHTLEG = 7,
	HITGROUP_NECK = 8,
	HITGROUP_GEAR = 10
};

class CHLClient
{
public:
	enum indices : size_t
	{
		INIT = 0,
		SHUTDOWN = 4,
		LEVELINITPREENTITY = 5,
		LEVELINITPOSTENTITY = 6,
		LEVELSHUTDOWN = 7,
		GETALLCLASSES = 8,
		HUDPROCESSINPUT = 10,
		HUDUPDATE = 11,
		INACTIVATEMOUSE = 15,
		INKEYEVENT = 20,
		CREATEMOVE = 21, // 21
		RENDERVIEW = 27,
		FRAMESTAGENOTIFY = 36, // 36
		DISPATCHUSERMESSAGE = 37,
		ISCHATRAISED = 89,
	};

public:
	__forceinline ClientClass* GetAllClasses()
	{
		return util::get_method< ClientClass * ( __thiscall* )( decltype( this ) )>( this, GETALLCLASSES )( this );
	}

	__forceinline bool IsChatRaised()
	{
		return util::get_method< bool ( __thiscall* )( decltype( this ) )>( this, ISCHATRAISED )( this );
	}
};

class IClientRenderable;

class RenderableInfo_t
{
public:
	IClientRenderable* m_pRenderable;
	void* m_pAlphaProperty;
	int m_EnumCount;
	int m_nRenderFrame;
	unsigned short m_FirstShadow;
	unsigned short m_LeafList;
	short m_Area;
	uint16_t m_Flags;   // 0x0016
	uint16_t m_Flags2; // 0x0018
	vec3_t m_vecBloatedAbsMins;
	vec3_t m_vecBloatedAbsMaxs;
	vec3_t m_vecAbsMins;
	vec3_t m_vecAbsMaxs;
	int pad;
};
