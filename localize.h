#pragma once

class C_EconItemView
{
public:
};

struct string_helper {
	char* buf;
	int capacity;
	int unk;
	int len;
};

class CPaintKit {
public:
	int id;
	string_helper name;
	string_helper desc_str;
	string_helper des_tag;
	string_helper pattern;
	string_helper unk;
	string_helper logo_material;
	bool diff_override;
	int rarity;
	int style;
	Color col[4];
	Color logocol[4];
	float flWearDefault;
	float flWearRemapMin;
	float flWearRemapMax;
	unsigned char nFixedSeed;
	unsigned char uchPhongExponent;
	unsigned char uchPhongAlbedoBoost;
	unsigned char uchPhongIntensity;
};

template < typename Key, typename Value >
struct Node_t
{
	int previous_id;    
	int next_id;        
	void* _unknown_ptr; 
	int _unknown;       
	Key key;            
	Value value;        
};

template < typename Key, typename Value >
struct Head_t
{
	Node_t< Key, Value >* memory;
	int allocation_count;        
	int grow_size;               
	int start_element;           
	int next_available;          
	int _unknown;                
	int last_element;            
};

using fnGetPaintkitDefinition = CPaintKit * (__thiscall*)(void*, int); // returns CPaintKit, vftable is CEconItemSchema

class CEconItemSchema {
public:
	CPaintKit* GetPaintkitDefinition(int id);
};

class CEconItemDefinition
{
public:
	__forceinline const char* GetItemBaseName()
	{
		return util::get_method< const char* ( __thiscall* )( void* ) >( this, 2 )( this );
	}
};

class ILocalize
{
public:
	__forceinline const wchar_t* Find( const char* token )
	{
		return util::get_method< const wchar_t* ( __thiscall* )( void*, const char* ) >( this, 12 )( this, token );
	}
};