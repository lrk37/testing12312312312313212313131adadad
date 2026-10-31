#pragma once

struct mstudiobbox_t
{
	int     m_bone;                 // 0x0000
	int     m_group;                // 0x0004
	vec3_t  m_mins;                 // 0x0008
	vec3_t  m_maxs;                 // 0x0014
	int     m_name_id;				// 0x0020
	ang_t   m_angle;                // 0x0024
	float   m_radius;               // 0x0030
	PAD( 0x10 );                    // 0x0034
};

struct mstudiohitboxset_t
{
	int	m_name_id;
	int	m_hitboxes;
	int	m_hitbox_id;

	__forceinline mstudiobbox_t* GetHitbox( int index ) const
	{
		return (mstudiobbox_t*)( ( (byte*)this ) + m_hitbox_id ) + index;
	};
};

struct mstudiobone_t
{
	int m_bone_name_index;
	int m_parent;
	PAD( 0x98 );
	int m_flags;
	PAD( 0x34 );
};

class mstudioposeparamdesc_t
{
public:
	int					sznameindex;
	__forceinline char* const name( void ) const { return ( (char*)this ) + sznameindex; }
	int					flags;	// ????
	float				start;	// starting value
	float				end;	// ending value
	float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
};

struct mstudioseqdesc_t;
class studiohdr_t
{
public:
	int    m_id;                    // 0x0000
	int    m_version;                // 0x0004
	int    m_checksum;                // 0x0008
	char   m_name[ 64 ];            // 0x000C
	int    m_length;                // 0x004C
	vec3_t m_eye_pos;                // 0x0050
	vec3_t m_illum_pos;                // 0x005C
	vec3_t m_hull_mins;                // 0x0068
	vec3_t m_hull_maxs;             // 0x0074
	vec3_t m_view_mins;             // 0x0080
	vec3_t m_view_maxs;             // 0x008C
	int    m_flags;                    // 0x0098
	int    m_num_bones;                // 0x009C
	int    m_bone_id;                // 0x00A0
	int    m_num_controllers;        // 0x00A4
	int    m_controller_id;            // 0x00A8
	int    m_num_sets;                // 0x00AC
	int    m_set_id;                // 0x00B0
	int					numlocalanim;			// animations/poses
	int					localanimindex;		// animation descriptions
	int					numlocalseq;				// sequences
	int					localseqindex;
	mutable int			activitylistversion;	// initialization flag - have the sequences been indexed?
	mutable int			eventsindexed;
	int					numtextures;
	int					textureindex;
	int					numcdtextures;
	int					cdtextureindex;
	int					numskinref;
	int					numskinfamilies;
	int					skinindex;
	int					numbodyparts;
	int					bodypartindex;
	int					numlocalattachments;
	int					localattachmentindex;
	int					numlocalnodes;
	int					localnodeindex;
	int					localnodenameindex;
	int					numflexdesc;
	int					flexdescindex;
	int					numflexcontrollers;
	int					flexcontrollerindex;
	int					numflexrules;
	int					flexruleindex;
	int					numikchains;
	int					ikchainindex;
	int					nummouths;
	int					mouthindex;
	int					numlocalposeparameters;
	int					localposeparamindex;

	mstudioseqdesc_t* get_local_seqdesc( int i ) const
	{
		const auto hdr = reinterpret_cast<unsigned char*>( const_cast<studiohdr_t*>( this ) );
		if ( i < 0 || i >= numlocalseq ) i = 0;
		return (mstudioseqdesc_t*)( static_cast<unsigned char*>( hdr + localseqindex ) + i );
	}

	__forceinline mstudioposeparamdesc_t* pLocalPoseParameter( int i ) const
	{
		assert( i >= 0 && i < numlocalposeparameters );
		return (mstudioposeparamdesc_t*)( ( (byte*)this ) + localposeparamindex ) + i;
	};

	__forceinline mstudiobone_t* GetBone( int index ) const
	{
		return (mstudiobone_t*)( ( (byte*)this ) + m_bone_id ) + index;
	}

	__forceinline mstudiohitboxset_t* GetHitboxSet( int index ) const
	{
		return (mstudiohitboxset_t*)( ( (byte*)this ) + m_set_id ) + index;
	}

	__forceinline mstudiobbox_t* GetHitbox( int hitbox, int set ) const
	{
		const mstudiohitboxset_t* hitboxSet = GetHitboxSet( set );

		if ( hitboxSet == nullptr )
			return nullptr;

		return hitboxSet->GetHitbox( hitbox );
	}
};

class model_t
{
public:
	void* m_handle;
	char   m_name[ 260 ];
	int    m_load_flags;
	int    m_server_count;
	int    m_type;
	int    m_flags;
	vec3_t m_mins;
	vec3_t m_maxs;
	float  m_radius;

private:
	PAD( 0x1C );
};

class IVModelInfo
{
public:
	// indexes for virtuals and hooks.
	enum indices : size_t
	{
		GETMODEL = 1,
		GETMODELINDEX = 2,
		GETMODELFRAMECOUNT = 8,
		GETSTUDIOMODEL = 30,
		FINDORLOADMODEL = 43
	};

public:
	__forceinline const model_t* GetModel( int modelindex )
	{
		return util::get_method< const model_t * ( __thiscall* )( void*, int ) >( this, GETMODEL )( this, modelindex );
	}

	__forceinline int GetModelIndex( const char* model )
	{
		return util::get_method< int( __thiscall* )( void*, const char* ) >( this, GETMODELINDEX )( this, model );
	}

	__forceinline int GetModelFrameCount( const model_t* model )
	{
		return util::get_method< int( __thiscall* )( void*, const model_t* ) >( this, GETMODELFRAMECOUNT )( this, model );
	}

	__forceinline studiohdr_t* GetStudioModel( const model_t* model )
	{
		return util::get_method< studiohdr_t * ( __thiscall* )( void*, const model_t* ) >( this, GETSTUDIOMODEL )( this, model );
	}

	__forceinline const model_t* FindOrLoadModel( const char* name )
	{
		return util::get_method< const model_t * ( __thiscall* )( void*, const char* ) >( this, FINDORLOADMODEL )( this, name );
	}
};