#pragma once

class Chams
{
public:
	enum model_type_t : uint32_t
	{
		invalid = 0,
		player,
		weapon,
		arms,
		view_weapon
	};

	enum materialtype_t {
		textured = 0,
		flat, 
		shine,
		shine2,
		glow,
		material_max
	};

	enum drawside_t {
		local = 0,
		enemy,
		friendly
	};

public:
	void SetColor( int material, Color col );
	void SetAlpha( int material, float alpha );
	void SetupMaterial( int material, Color col, bool z_flag );

	void init();
	bool GenerateLerpedMatrix( int index, BoneArray* out, float& alphamod );

	//void RenderPlayer( Player* player );

	bool OnDrawModelExecute( Player* entity, void*, IMatRenderContext*, DrawModelState_t&, ModelRenderInfo_t&, matrix3x4_t* );
public:
	//std::vector< Player* > m_players;

	IMaterial* m_materials[material_max];
	drawside_t m_draw_side;

	BoneArray* local_hidden;
	float last_hidden_time;
	//struct {
	//	float min, max;
	//} m_original_exposure;
};

extern Chams g_chams;