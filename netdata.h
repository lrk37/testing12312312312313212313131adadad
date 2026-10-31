#pragma once

class NetData
{
private:
	class PredictionData_t
	{
	public:
		int m_cmd_num;
		C_AnimationLayer m_layers[ 13 ];
		CCSGOPlayerAnimState m_state;
		float m_fake_feet_yaw;

	public:
		float m_timebase;
		vec3_t m_shoot_pos;
		float m_postpone_fire_ready_time;
		float m_inaccuracy;
	};

	std::array< PredictionData_t, MULTIPLAYER_BACKUP > m_prediction_data;

public:
	void StoreAnims( int cmd_num, bool store_state = true );
	void RestoreAnims( int cmd_num );

	void MaintainClientLayers( C_AnimationLayer* layers );

public:
	float& GetTimebase( int cmd_num );
	vec3_t& GetShootPos( int cmd_num );
	float& GetInaccuracy( int cmd_num );

public:
	void StoreViewmodel( int cmd_num );
	void SetViewmodel( int cmd_num );

	std::array< std::pair<float, float>, MULTIPLAYER_BACKUP > m_viewmodel_data;
};

extern NetData g_netdata;