#pragma once

struct SndInfo_t
{
	// Sound Guid
	int			m_nGuid;
	void* m_filenameHandle;		// filesystem filename handle - call IFilesystem to conver this to a string
	int			m_nSoundSource;
	int			m_nChannel;
	// If a sound is being played through a speaker entity (e.g., on a monitor,), this is the
	//  entity upon which to show the lips moving, if the sound has sentence data
	int			m_nSpeakerEntity;
	float		m_flVolume;
	float		m_flLastSpatializedVolume;
	// Radius of this sound effect (spatialization is different within the radius)
	float		m_flRadius;
	int			m_nPitch;
	vec3_t* m_pOrigin;
	vec3_t* m_pDirection;

	// if true, assume sound source can move and update according to entity
	bool		m_bUpdatePositions;
	// true if playing linked sentence
	bool		m_bIsSentence;
	// if true, bypass all dsp processing for this sound (ie: music)	
	bool		m_bDryMix;
	// true if sound is playing through in-game speaker entity.
	bool		m_bSpeaker;
	// for snd_show, networked sounds get colored differently than local sounds
	bool		m_bFromServer;
};

class IRecipientFilter {
public:
	virtual			~IRecipientFilter() {}

	virtual bool	IsReliable(void) const = 0;
	virtual bool	IsInitMessage(void) const = 0;

	virtual int		GetRecipientCount(void) const = 0;
	virtual int		GetRecipientIndex(int slot) const = 0;
};

enum soundlevel_t
{
	SNDLVL_NONE = 0,
	SNDLVL_20dB = 20,		// rustling leaves
	SNDLVL_25dB = 25,		// whispering
	SNDLVL_30dB = 30,		// library
	SNDLVL_35dB = 35,
	SNDLVL_40dB = 40,
	SNDLVL_45dB = 45,		// refrigerator
	SNDLVL_50dB = 50,	    // 3.9    // average home
	SNDLVL_55dB = 55,	    // 3.0
	SNDLVL_IDLE = 60,	    // 2.0	
	SNDLVL_60dB = 60,	    // 2.0	// normal conversation, clothes dryer
	SNDLVL_65dB = 65,	    // 1.5	// washing machine, dishwasher
	SNDLVL_STATIC = 66,	    // 1.25
	SNDLVL_70dB = 70,	    // 1.0	// car, vacuum cleaner, mixer, electric sewing machine
	SNDLVL_NORM = 75,
	SNDLVL_75dB = 75,	    // 0.8	// busy traffic
	SNDLVL_80dB = 80,	    // 0.7	// mini-bike, alarm clock, noisy restaurant, office tabulator, outboard motor, passing snowmobile
	SNDLVL_TALKING = 80,    // 0.7
	SNDLVL_85dB = 85,	    // 0.6	// average factory, electric shaver
	SNDLVL_90dB = 90,	    // 0.5	// screaming child, passing motorcycle, convertible ride on frw
	SNDLVL_95dB = 95,
	SNDLVL_100dB = 100,	    // 0.4	// subway train, diesel truck, woodworking shop, pneumatic drill, boiler shop, jackhammer
	SNDLVL_105dB = 105,     // helicopter, power mower
	SNDLVL_110dB = 110,     // snowmobile drvrs seat, inboard motorboat, sandblasting
	SNDLVL_120dB = 120,     // auto horn, propeller aircraft
	SNDLVL_130dB = 130,     // air raid siren
	SNDLVL_GUNFIRE = 140,	// 0.27	// THRESHOLD OF PAIN, gunshot, jet engine
	SNDLVL_140dB = 140,	    // 0.2
	SNDLVL_150dB = 150,	    // 0.2
	SNDLVL_180dB = 180,		// rocket launching

	// NOTE: Valid soundlevel_t values are 0-255.
	//       256-511 are reserved for sounds using goldsrc compatibility attenuation.
};

enum
{
	PITCH_NORM = 100,	// non-pitch shifted
	PITCH_LOW = 95,		// other values are possible - 0-255, where 255 is very high
	PITCH_HIGH = 120
};


class IEngineSound
{
public:
	// Precache a particular sample
	virtual bool PrecacheSound(const char* pSample, bool bPreload = false, bool bIsUISound = false) = 0;
	virtual bool IsSoundPrecached(const char* pSample) = 0;
	virtual void PrefetchSound(const char* pSample) = 0;
	virtual bool IsLoopingSound(const char* pSample) = 0;

	// Just loads the file header and checks for duration (not hooked up for .mp3's yet)
	// Is accessible to server and client though
	virtual float GetSoundDuration(const char* pSample) = 0;

	// Pitch of 100 is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
	// down to 1 is a lower pitch.   150 to 70 is the realistic range.
	// EmitSound with pitch != 100 should be used sparingly, as it's not quite as
	// fast (the pitchshift mixer is not native coded).

	// NOTE: setting iEntIndex to -1 will cause the sound to be emitted from the local
	// player (client-side only)
	virtual int EmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample,
		float flVolume, float flAttenuation, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const vec3_t* pOrigin = NULL, const vec3_t* pDirection = NULL, CUtlVector< vec3_t >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual int EmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample,
		float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const vec3_t* pOrigin = NULL, const vec3_t* pDirection = NULL, CUtlVector< vec3_t >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void EmitSentenceByIndex(IRecipientFilter& filter, int iEntIndex, int iChannel, int iSentenceIndex,
		float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
		const vec3_t* pOrigin = NULL, const vec3_t* pDirection = NULL, CUtlVector< vec3_t >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void    StopSound(int iEntIndex, int iChannel, const char* pSample, unsigned int nSoundEntryHash) = 0;
	virtual void    StopAllSounds(bool bClearBuffers) = 0;
	virtual void    SetRoomType(IRecipientFilter& filter, int roomType) = 0;
	virtual void    SetPlayerDSP(IRecipientFilter& filter, int dspType, bool fastReset) = 0;
	virtual int     EmitAmbientSound(const char* pSample, float flVolume, int iPitch = PITCH_NORM, int flags = 0, float soundtime = 0.0f) = 0;
	virtual float   GetDistGainFromSoundLevel(soundlevel_t soundlevel, float dist) = 0;
	virtual int		GetGuidForLastSoundEmitted() = 0;
	virtual bool	IsSoundStillPlaying(int guid) = 0;
	virtual void	StopSoundByGuid(int guid, bool bForceSync) = 0;
	virtual void	SetVolumeByGuid(int guid, float fvol) = 0;
	virtual void	GetActiveSounds(CUtlVector<SndInfo_t>& sndlist) = 0;
	virtual void	PrecacheSentenceGroup(const char* pGroupName) = 0;
	virtual void	NotifyBeginMoviePlayback() = 0;
	virtual void	NotifyEndMoviePlayback() = 0;
	virtual bool	GetSoundChannelVolume(const char* sound, float& flVolumeLeft, float& flVolumeRight) = 0;
	virtual float	GetElapsedTimeByGuid(int guid) = 0;
};

#define SOUNDEMITTER_INVALID_HASH	(unsigned int)-1
enum
{
	CHAN_REPLACE = -1,
	CHAN_AUTO = 0,
	CHAN_WEAPON = 1,
	CHAN_VOICE = 2,
	CHAN_ITEM = 3,
	CHAN_BODY = 4,
	CHAN_STREAM = 5,		// allocate stream channel from the static or dynamic area
	CHAN_STATIC = 6,		// allocate channel from the static area 
	CHAN_VOICE_BASE = 7,		// allocate channel for network voice data
};

enum SoundFlags_t
{
	SND_NOFLAGS = 0,			// to keep the compiler happy
	SND_CHANGE_VOL = ( 1 << 0 ),		// change sound vol
	SND_CHANGE_PITCH = ( 1 << 1 ),		// change sound pitch
	SND_STOP = ( 1 << 2 ),		// stop the sound
	SND_SPAWNING = ( 1 << 3 ),		// we're spawning, used in some cases for ambients
	// not sent over net, only a param between dll and server.
	SND_DELAY = ( 1 << 4 ),		// sound has an initial delay
	SND_STOP_LOOPING = ( 1 << 5 ),		// stop all looping sounds on the entity.
	SND_SPEAKER = ( 1 << 6 ),		// being played again by a microphone through a speaker

	SND_SHOULDPAUSE = ( 1 << 7 ),		// this sound should be paused if the game is paused
	SND_IGNORE_PHONEMES = ( 1 << 8 ),
	SND_IGNORE_NAME = ( 1 << 9 ),		// used to change all sounds emitted by an entity, regardless of scriptname
	SND_IS_SCRIPTHANDLE = ( 1 << 10 ),		// server has passed the actual SoundEntry instead of wave filename

	SND_UPDATE_DELAY_FOR_CHOREO = ( 1 << 11 ),		// True if we have to update snd_delay_for_choreo with the IO latency.
	SND_GENERATE_GUID = ( 1 << 12 ),		// True if we generate the GUID when we send the sound.

	SND_OVERRIDE_PITCH = ( 1 << 13 ),		// The pitch given in code overrides what is present in the sound entry.
};

class CSFXTable
{
public:
	// gets sound name, possible decorated with prefixes
	virtual const char* getname( char* pBuf, size_t bufLen ) = 0;

	int					m_namePoolIndex;
	void* pSource;

	bool				m_bUseErrorFilename : 1;
	bool				m_bIsUISound : 1;
	bool				m_bIsLateLoad : 1;
	bool				m_bMixGroupsCached : 1;
	bool				m_bIsMusic : 1;
	bool				m_bIsCreatedByQueuedLoader : 1;

	byte				m_mixGroupCount;
	// UNDONE: Use a fixed bit vec here?
	byte				m_mixGroupList[ 8 ];

public:
	// Only set in debug mode so you can see the name.
	const char* m_pDebugName;
};

struct StartSoundParams_t
{
	StartSoundParams_t() :
		m_nSoundScriptHash( SOUNDEMITTER_INVALID_HASH ),
		m_pSoundEntryName( NULL ),
		staticsound( false ),
		userdata( 0 ),
		soundsource( 0 ),
		entchannel( CHAN_AUTO ),
		pSfx( 0 ),
		bUpdatePositions( true ),
		fvol( 1.0f ),
		soundlevel( SNDLVL_NORM ),
		flags( SND_NOFLAGS ),
		pitch( PITCH_NORM ),
		fromserver( false ),
		delay( 0.0f ),
		speakerentity( -1 ),
		bToolSound( false ),
		initialStreamPosition( 0 ),
		skipInitialSamples( 0 ),
		m_nQueuedGUID( UNINT_GUID ),
		m_bIsScriptHandle( false ),
		m_pOperatorsKV( NULL ),
		opStackElapsedTime( 0.0f ),
		opStackElapsedStopTime( 0.0f ),
		m_bDelayedStart( false ),
		m_bInEyeSound( false ),
		m_bHRTFFollowEntity( false ),
		m_bHRTFBilinear( false ),
		m_bHRTFLock( false )
	{
		origin.init();
		direction.init();
	}
	void Copy( StartSoundParams_t& destParams )
	{
		destParams.userdata = userdata;
		destParams.soundsource = soundsource;
		destParams.entchannel = entchannel;
		destParams.pSfx = pSfx;
		destParams.origin = origin;//( origin, destParams.origin );
		destParams.direction = direction; // = VectorCopy( direction, destParams.direction );
		destParams.fvol = fvol;
		destParams.soundlevel = soundlevel;
		destParams.flags = flags;
		destParams.pitch = pitch;
		destParams.delay = delay;
		destParams.speakerentity = speakerentity;
		destParams.initialStreamPosition = initialStreamPosition;
		destParams.skipInitialSamples = skipInitialSamples;
		destParams.m_nQueuedGUID = m_nQueuedGUID;
		destParams.m_nSoundScriptHash = m_nSoundScriptHash;
		destParams.m_pSoundEntryName = m_pSoundEntryName;
		destParams.m_pOperatorsKV = m_pOperatorsKV;
		destParams.opStackElapsedTime = opStackElapsedTime;
		destParams.opStackElapsedStopTime = opStackElapsedStopTime;
		destParams.staticsound = staticsound;
		destParams.bUpdatePositions = bUpdatePositions;
		destParams.fromserver = fromserver;
		destParams.bToolSound = bToolSound;
		destParams.m_bIsScriptHandle = m_bIsScriptHandle;
		destParams.m_bDelayedStart = m_bDelayedStart;
		destParams.m_bInEyeSound = m_bInEyeSound;
		destParams.m_bHRTFFollowEntity = m_bHRTFFollowEntity;
		destParams.m_bHRTFBilinear = m_bHRTFBilinear;
		destParams.m_bHRTFLock = m_bHRTFLock;
	}
	void CopyNewFromParams( StartSoundParams_t& destParams )
	{
		destParams.userdata = userdata;
		// 		destParams.soundsource = soundsource;
		// 		destParams.entchannel = entchannel;
		destParams.pSfx = pSfx;
		destParams.origin = origin; //VectorCopy( origin, destParams.origin );
		destParams.direction = direction; //VectorCopy( direction, destParams.direction );
		destParams.fvol = fvol;
		destParams.soundlevel = soundlevel;
		destParams.flags = flags;
		destParams.pitch = pitch;
		destParams.delay = delay;
		destParams.speakerentity = speakerentity;
		// 		destParams.initialStreamPosition = initialStreamPosition;
		// 		destParams.skipInitialSamples = skipInitialSamples;
		// 		destParams.m_nQueuedGUID = m_nQueuedGUID;
		// 		destParams.m_nSoundScriptHash = m_nSoundScriptHash;
		// 		destParams.m_pSoundEntryName = m_pSoundEntryName;
		// 		destParams.m_pOperatorsKV = m_pOperatorsKV;
		// 		destParams.opStackElapsedTime = opStackElapsedTime;
		// 		destParams.opStackElapsedStopTime = opStackElapsedStopTime;
		destParams.staticsound = staticsound;
		destParams.bUpdatePositions = bUpdatePositions;
		destParams.fromserver = fromserver;
		destParams.bToolSound = bToolSound;
		destParams.m_bIsScriptHandle = m_bIsScriptHandle;
		destParams.m_bInEyeSound = m_bInEyeSound;
		destParams.m_bHRTFFollowEntity = m_bHRTFFollowEntity;
		destParams.m_bHRTFBilinear = m_bHRTFBilinear;
		destParams.m_bHRTFLock = m_bHRTFLock;

		/*		destParams.m_bDelayedStart = m_bDelayedStart;*/
	}
	int				userdata;
	int				soundsource;
	int				entchannel;
	CSFXTable* pSfx;
	vec3_t			origin;
	vec3_t			direction;
	float			fvol;
	soundlevel_t	soundlevel;
	int				flags;
	int				pitch;
	float			delay;
	int				speakerentity;
	int				initialStreamPosition;
	int				skipInitialSamples;
	int				m_nQueuedGUID;
	unsigned int m_nSoundScriptHash;
	const char* m_pSoundEntryName;
	KeyValues* m_pOperatorsKV;
	float			opStackElapsedTime;
	float			opStackElapsedStopTime;

	bool			staticsound : 1;
	bool			bUpdatePositions : 1;
	bool			fromserver : 1;
	bool			bToolSound : 1;
	bool			m_bIsScriptHandle : 1;
	bool			m_bDelayedStart : 1;
	bool			m_bInEyeSound : 1;
	bool			m_bHRTFFollowEntity : 1;
	bool			m_bHRTFBilinear : 1;
	bool			m_bHRTFLock : 1;

	static const int	UNINT_GUID = -1;
	static const int	GENERATE_GUID = -2;		// Generate GUID regardless of the other vol and pitch flags.
};

struct SoundInfo_t
{
	vec3_t			m_origin;
	vec3_t			m_direction;
	vec3_t			m_listener_origin;
	const char* m_name;		// UNDONE: Make this a FilenameHandle_t to avoid bugs with arrays of these
	float			m_volume;
	float			m_delay;
	float			m_tick_time;			// delay is encoded relative to this tick, fix up if packet is delayed
	int				m_sequence_number;
	int				m_entity_index;
	int				m_channel;
	int				m_pitch;
	int				m_flags;
	unsigned int 	m_sound_num;
	int				m_speaker_entity;
	int				m_random_seed;
	int				m_sound_level;
	bool			m_is_sentence;
	bool			m_is_ambient;
	bool			m_looping;
};