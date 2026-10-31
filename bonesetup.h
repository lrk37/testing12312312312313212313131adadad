#pragma once

class Bones
{
public:
	bool m_running;

public:
	bool setup( Player* player, BoneArray* out, LagRecord* record );
	bool BuildBones( Player* target, int mask, BoneArray* out, LagRecord* record );
	bool Bro( Player* target, int mask, BoneArray* out );

public:
	bool update_interp( LagRecord* record );
};

extern Bones g_bones;