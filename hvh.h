#pragma once

enum ManualAAMode : int
{
	NONE = -1,
	BACK,
	RIGHT,
	LEFT,
	FORWARD
};

class AdaptiveAngle
{
public:
	float m_yaw;
	float m_dist;
	float m_range;

public:
	// ctor.
	__forceinline AdaptiveAngle( float yaw, float range = 32.f )
	{
		m_yaw = math::NormalizedAngle( yaw );
		m_dist = 0.f;
		m_range = range;
	}
};

enum AntiAimMode : size_t
{
	STAND = 0,
	WALK,
	AIR,
};

enum PeekResult_t : char
{
	BRO = -1,
	UNCHOKE = 0,
	FORCE
};

class HVH
{
public:
	size_t m_mode;

	Player* m_best_target;

	float m_view;
	float m_base_yaw;

	int m_twist_lag;
	int m_updates_cycle;

	float m_lastmove_yaw;
	ang_t m_real_angles;

	bool m_lby_timing_broken;

	float m_last_press_time;

	int m_queued_dir;

	int m_manual_direction;
	int m_manual_lastmove;

	bool m_fresh_jump;

public:
	void ManualAntiaim();
	float DynamicFreestand( Player* target, std::vector<AdaptiveAngle> angles, bool get_worst = false );
	void SetFreestandInfo( Player* target );

	float  m_left_dist;
	float  m_right_dist;

	float  m_left_close_dist;
	float  m_right_close_dist;

public:
	Player* GetFOVTarget();
	bool IsYawSideways( vec3_t from, vec3_t to, float yaw );

	void ApplySupression();
	void ApplyTwist();
	void ApplyFlick();
	void AntiAim();

public:
	PeekResult_t m_peek_result;
	PeekResult_t DoPeek( Player* player );
	void SendPacket();
};

extern HVH g_hvh;