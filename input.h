#pragma once

constexpr int MULTIPLAYER_BACKUP = 150;

#define BigShort(val)    WordSwap(val)
#define BigWord(val)     WordSwap(val)
#define BigLong(val)     DWordSwap(val)
#define BigDWord(val)    DWordSwap(val)
#define LittleShort(val) (val)
#define LittleWord(val)  (val)
#define LittleLong(val)  (val)
#define LittleDWord(val) (val)
#define SwapShort(val)   BigShort(val)
#define SwapWord(val)    BigWord(val)
#define SwapLong(val)    BigLong(val)
#define SwapDWord(val)   BigDWord(val)

#define CRC32_INIT_VALUE 0xFFFFFFFFUL
#define CRC32_XOR_VALUE  0xFFFFFFFFUL

#define NUM_BYTES 256

typedef unsigned long crc32_t;

static const crc32_t util_crc_table[NUM_BYTES] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d };

inline void crc32_init(crc32_t* pulCRC) {
	*pulCRC = CRC32_INIT_VALUE;
}

inline void crc32_final(crc32_t* pulCRC) {
	*pulCRC ^= CRC32_XOR_VALUE;
}

inline crc32_t crc32_get_table_entry(unsigned int slot) {
	return util_crc_table[(unsigned char)slot];
}

static void crc32_process_buffer(crc32_t* pulCRC, const void* pBuffer, int nBuffer) {
	crc32_t ulCrc = *pulCRC;
	unsigned char* pb = (unsigned char*)pBuffer;
	unsigned int nFront;
	int nMain;

JustAfew:

	switch (nBuffer) {
	case 7:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 6:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 5:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 4:
		ulCrc ^= LittleLong(*(crc32_t*)pb);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		*pulCRC = ulCrc;
		return;

	case 3:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 2:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 1:
		ulCrc = util_crc_table[*pb ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

	case 0:
		*pulCRC = ulCrc;
		return;
	}

	// We may need to do some alignment work up front, and at the end, so that
	// the main loop is aligned and only has to worry about 8 uint8_t at a time.
	//
	// The low-order two bits of pb and nBuffer in total control the
	// upfront work.
	//
	nFront = ((unsigned int)pb) & 3;
	nBuffer -= nFront;
	switch (nFront) {
	case 3:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
	case 2:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
	case 1:
		ulCrc = util_crc_table[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
	}

	nMain = nBuffer >> 3;
	while (nMain--) {
		ulCrc ^= LittleLong(*(crc32_t*)pb);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc ^= LittleLong(*(crc32_t*)(pb + 4));
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		ulCrc = util_crc_table[(unsigned char)ulCrc] ^ (ulCrc >> 8);
		pb += 8;
	}

	nBuffer &= 7;
	goto JustAfew;
}

enum button_flags_t : int
{
	IN_ATTACK = ( 1 << 0 ),
	IN_JUMP = ( 1 << 1 ),
	IN_DUCK = ( 1 << 2 ),
	IN_FORWARD = ( 1 << 3 ),
	IN_BACK = ( 1 << 4 ),
	IN_USE = ( 1 << 5 ),
	IN_CANCEL = ( 1 << 6 ),
	IN_LEFT = ( 1 << 7 ),
	IN_RIGHT = ( 1 << 8 ),
	IN_MOVELEFT = ( 1 << 9 ),
	IN_MOVERIGHT = ( 1 << 10 ),
	IN_ATTACK2 = ( 1 << 11 ),
	IN_RUN = ( 1 << 12 ),
	IN_RELOAD = ( 1 << 13 ),
	IN_ALT1 = ( 1 << 14 ),
	IN_ALT2 = ( 1 << 15 ),
	IN_SCORE = ( 1 << 16 ),
	IN_SPEED = ( 1 << 17 ),
	IN_WALK = ( 1 << 18 ),
	IN_ZOOM = ( 1 << 19 ),
	IN_WEAPON1 = ( 1 << 20 ),
	IN_WEAPON2 = ( 1 << 21 ),
	IN_BULLRUSH = ( 1 << 22 ),
	IN_GRENADE1 = ( 1 << 23 ),
	IN_GRENADE2 = ( 1 << 24 ),
	IN_ATTACK3 = ( 1 << 25 ),
};

class CCSUsrMsg_ServerRankRevealAll
{
public:
	__forceinline CCSUsrMsg_ServerRankRevealAll() : seconds_till_shutdown{} { }
public:
	PAD( 0x8 );
	int seconds_till_shutdown;
};

class CUserCmd
{
public:
	__forceinline CUserCmd() { reset(); }

	__forceinline void reset()
	{
		m_command_number = 0;
		m_tick = 0;
		m_view_angles.clear();
		m_aimdirection.clear();
		m_forward_move = 0.f;
		m_side_move = 0.f;
		m_up_move = 0.f;
		m_buttons = 0;
		m_impulse = 0;
		m_weapon_select = 0;
		m_weapon_subtype = 0;
		m_random_seed = 0;
		m_mousedx = 0;
		m_mousedy = 0;
		m_predicted = false;
		m_head_angles.clear();
		m_head_offset.clear();
	}

	__forceinline CUserCmd& operator =( const CUserCmd& src )
	{
		if ( this == &src )
			return *this;

		m_command_number = src.m_command_number;
		m_tick = src.m_tick;
		m_view_angles = src.m_view_angles;
		m_aimdirection = src.m_aimdirection;
		m_forward_move = src.m_forward_move;
		m_side_move = src.m_side_move;
		m_up_move = src.m_up_move;
		m_buttons = src.m_buttons;
		m_impulse = src.m_impulse;
		m_weapon_select = src.m_weapon_select;
		m_weapon_subtype = src.m_weapon_subtype;
		m_random_seed = src.m_random_seed;
		m_mousedx = src.m_mousedx;
		m_mousedy = src.m_mousedy;
		m_predicted = src.m_predicted;
		m_head_angles = src.m_head_angles;
		m_head_offset = src.m_head_offset;

		return *this;
	}

	__forceinline CUserCmd( const CUserCmd& src )
	{
		*this = src;
	}

	// virtual dtor.
	PAD( 0x4 )

public:
	int     m_command_number;
	int     m_tick;
	ang_t   m_view_angles;
	vec3_t  m_aimdirection;
	float   m_forward_move;
	float   m_side_move;
	float   m_up_move;
	int     m_buttons;
	uint8_t m_impulse;
	int     m_weapon_select;
	int     m_weapon_subtype;
	int     m_random_seed;
	short   m_mousedx;
	short   m_mousedy;
	bool    m_predicted;
	vec3_t  m_head_angles;
	vec3_t  m_head_offset;

	crc32_t get_checksum(void) const {
		crc32_t crc;
		crc32_init(&crc);

		crc32_process_buffer(&crc, &m_command_number, sizeof(m_command_number));
		crc32_process_buffer(&crc, &m_tick, sizeof(m_tick));
		crc32_process_buffer(&crc, &m_view_angles, sizeof(m_view_angles));
		crc32_process_buffer(&crc, &m_aimdirection, sizeof(m_aimdirection));
		crc32_process_buffer(&crc, &m_forward_move, sizeof(m_forward_move));
		crc32_process_buffer(&crc, &m_side_move, sizeof(m_side_move));
		crc32_process_buffer(&crc, &m_up_move, sizeof(m_up_move));
		crc32_process_buffer(&crc, &m_buttons, sizeof(m_buttons));
		crc32_process_buffer(&crc, &m_impulse, sizeof(m_impulse));
		crc32_process_buffer(&crc, &m_weapon_select, sizeof(m_weapon_select));
		crc32_process_buffer(&crc, &m_weapon_subtype, sizeof(m_weapon_subtype));
		crc32_process_buffer(&crc, &m_random_seed, sizeof(m_random_seed));
		crc32_process_buffer(&crc, &m_mousedx, sizeof(m_mousedx));
		crc32_process_buffer(&crc, &m_mousedy, sizeof(m_mousedy));

		crc32_final(&crc);
		return crc;
	}
};

class CVerifiedUserCmd
{
public:
	CUserCmd   m_cmd;
	crc32_t    m_crc;
};

class C_CommandContext
{
public:
	bool			needsprocessing;
	CUserCmd		cmd;
	int				command_number;
};

class IInputSystem
{
public:
	enum indices : size_t
	{
		ENABLEINPUT = 11,
		ISBUTTONDOWN = 15,
		GETBUTTONPRESSEDTICK = 16,
		GETCURSORPOSITION = 56,
	};

public:
	__forceinline void EnableInput( bool enable )
	{
		return util::get_method< void( __thiscall* )( decltype( this ), bool ) >( this, ENABLEINPUT )( this, enable );
	}

	__forceinline void GetCursorPosition( int* x, int* y )
	{
		return util::get_method< void( __thiscall* )( decltype( this ), int*, int* ) >( this, GETCURSORPOSITION )( this, x, y );
	}
};

class CInput
{
public:
	enum indices : size_t
	{
		CREATEMOVE = 3,
		WRITEUSERCMDDELTATOBUFFER = 5,
		ENCODEUSERCMDTOBUFFER = 6,
		GETUSERCMD = 8,
		CAMISTHIRDPERSON = 32,
		CAMTOTHIRDPERSON = 35,
		CAMTOFIRSTPERSON = 36,
	};

public:
	void* vtable;									// 0x00
	bool              m_trackir;					// 0x04
	bool              m_mouse_init;					// 0x05
	bool              m_mouse_active;				// 0x06
	bool              m_joystick_adv_init;			// 0x07
	PAD( 0x2C );									// 0x08
	void* m_keys;						// 0x34
	PAD( 0x6C );									// 0x38
	bool              m_camera_intercepting_mouse;	// 0x9C
	bool              m_camera_in_third_person;		// 0x9D
	bool              m_camera_moving_with_mouse;	// 0x9E
	vec3_t            m_camera_offset;				// 0xA0
	bool              m_camera_distance_move;		// 0xAC
	int               m_camera_old_x;				// 0xB0
	int               m_camera_old_y;				// 0xB4
	int               m_camera_x;					// 0xB8
	int               m_camera_y;					// 0xBC
	bool              m_camera_is_orthographic;		// 0xC0
	ang_t             m_previous_view_angles;		// 0xC4
	ang_t             m_previous_view_angles_tilt;	// 0xD0
	float             m_last_forward_move;			// 0xDC
	int               m_clear_input_state;			// 0xE0
	CUserCmd* m_commands;					// 0xEC
	CVerifiedUserCmd* m_verified_commands;					// 0xF0

public:
	__forceinline int CAM_IsThirdPerson( int slot = -1 )
	{
		return util::get_method< int( __thiscall* )( decltype( this ), int ) >( this, CAMISTHIRDPERSON )( this, slot );
	}

	__forceinline void CAM_ToThirdPerson()
	{
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, CAMTOTHIRDPERSON )( this );
	}

	__forceinline void CAM_ToFirstPerson()
	{
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, CAMTOFIRSTPERSON )( this );
	}

	__forceinline CUserCmd* GetUserCmd( int sequence_number )
	{
		return util::get_method< CUserCmd * ( __thiscall* )( decltype( this ), int, int ) >( this, GETUSERCMD )( this, -1, sequence_number );
	}

	CVerifiedUserCmd* GetVerifiedCmd(const int sequence_number) {
		return &m_verified_commands[sequence_number % MULTIPLAYER_BACKUP];
	}
};