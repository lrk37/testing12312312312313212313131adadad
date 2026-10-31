#pragma once

class Movement
{
public:
	bool m_fakewalking;

	int m_fake_walk_ticks_left;
	bool m_fakewalk_lby_stop;

	bool m_stop_the_fakewalk;

	vec3_t m_autopeek_pos;
	bool m_autopeek_retreat;
	bool m_should_quickswitch;

public:
	bool m_transition_flick;
	std::pair<bool, bool> m_transition_stage;
	bool m_transition_reset;

public:
	void EdgeJump();
	void AutoStrafe();
	void JumpRelated();
	void MoonWalk( CUserCmd* cmd );
	void FixMove( CUserCmd* cmd, const ang_t& old_angles, bool local = true );
	void AutoPeek();
	void NullVelocity( CUserCmd* cmd );
	bool QuickStop();
	void ClampMovementSpeed( CUserCmd* cmd, float speed );
	void FakeWalk();
};

extern Movement g_movement;