#pragma once

// pre-declare.
class datamap_t;

// prototype.
using inputfunc_t = void( __cdecl* )( void* data );

enum fieldtype_t
{
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)
	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)
	FIELD_VECTOR2D,			// 2 floats
	FIELD_TYPECOUNT,		// MUST BE LAST
};

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,
	TD_OFFSET_COUNT,
};

class typedescription_t
{
public:
	int							field_type;
	const char* field_name;
	int							field_offset;
	unsigned short				field_size;
	short						flags;

	const char* external_name;

	void* restore_ops;
	void* input_func;

	datamap_t* td;

	int							field_size_in_butes;

	class c_type_description* override_field;

	int							override_count;

	float						field_tolerance;

	int							flat_offset[ TD_OFFSET_COUNT ];
	unsigned short				flat_group;
};

struct datarun_t
{
	/*datarun_t() : m_nStartFlatField(0), m_nEndFlatField(0), m_nLength(0)
	{
		for (int i = 0; i < TD_OFFSET_COUNT; ++i)
		{
			m_nStartOffset[i] = 0;
#ifdef _GAMECONSOLE
			// These are the offsets of the next run, for priming the L1 cache
			m_nPrefetchOffset[i] = 0;
#endif
		}
	}*/

	// Indices of start/end fields in the flattened typedescription_t list
	int m_nStartFlatField;
	int m_nEndFlatField;

	// Offsets for run in the packed/unpacked data (I think the run starts need to be properly aligned)
	int m_nStartOffset[ TD_OFFSET_COUNT ];

	// we not on the xbox nigga
/*#ifdef _GAMECONSOLE
	// These are the offsets of the next run, for priming the L1 cache
	int m_nPrefetchOffset[TD_OFFSET_COUNT];
#endif*/
	int m_nLength;
};

struct datacopyruns_t
{
public:
	CUtlVector< datarun_t > m_vecRuns;
};

struct flattenedoffsets_t
{
	CUtlVector< typedescription_t >	m_Flattened;
	int								m_nPackedSize; // Contiguous memory to pack all of these together for TD_OFFSET_PACKED
	int								m_nPackedStartOffset;
};

struct datamapinfo_t
{
	// Flattened list, with FIELD_EMBEDDED, FTYPEDESC_PRIVATE,
	//  and FTYPEDESC_OVERRIDE (overridden) fields removed
	flattenedoffsets_t	m_Flat;
	datacopyruns_t		m_CopyRuns;
};

struct optimized_datamap_t
{
	// Optimized info for PC_NON_NETWORKED and PC_NETWORKED data
	datamapinfo_t	m_Info[ 2/*PC_COPYTYPE_COUNT*/ ];
};

class datamap_t
{
public:
	typedescription_t* dataDesc;
	int					dataNumFields;
	char const* dataClassName;
	datamap_t* baseMap;

	int					m_nPackedSize;
	optimized_datamap_t* m_pOptimizedDataMap;
};

typedef void ( *FN_FIELD_COMPARE )( const char* classname, const char* fieldname, const char* fieldtype, bool networked, bool noterrorchecked, bool differs, bool withintolerance, const char* value );

class CPredictionCopy
{
public:
	typedef enum
	{
		DIFFERS = 0,
		IDENTICAL,
		WITHINTOLERANCE,
	} difftype_t;

	typedef enum
	{
		TRANSFERDATA_COPYONLY = 0,  // Data copying only (uses runs)
		TRANSFERDATA_ERRORCHECK_NOSPEW, // Checks for errors, returns after first error found
		TRANSFERDATA_ERRORCHECK_SPEW,   // checks for errors, reports all errors to console
		TRANSFERDATA_ERRORCHECK_DESCRIBE, // used by hud_pdump, dumps values, etc, for all fields
	} optype_t;

	CPredictionCopy( int type, byte* dest, bool dest_packed, const byte* src, bool src_packed,
		optype_t opType, FN_FIELD_COMPARE func = nullptr );

	int		TransferData( const char* operation, int entindex, datamap_t* dmap );

private:

	optype_t		m_OpType;
	int				m_nType;
	byte* m_pDest;
	const byte* m_pSrc;
	int				m_nDestOffsetIndex;
	int				m_nSrcOffsetIndex;
	int				m_nErrorCount;
	int				m_nEntIndex;

	FN_FIELD_COMPARE	m_FieldCompareFunc;

	const typedescription_t* m_pWatchField;
	char const* m_pOperation;

	const typedescription_t* m_FieldStack;
};