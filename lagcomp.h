#pragma once

class AimPlayer;

class LagCompensation
{
public:
	struct PreviousData_t
	{
		float m_duck;
		int m_flags;
	};

public:
	void SimulatePlayer( Player* player, float time, float speed, float direction, PreviousData_t& old_data );

public:
	struct LagCompData_t
	{
		int m_extrap_ticks;
		bool m_needs_extrap;
		bool m_can_fire;
	};

	LagCompData_t GetLagcompData( AimPlayer* data );

public:
	bool valid_interp( AimPlayer* data, LagRecord* record, bool allow_interp = true );
};

extern LagCompensation g_lagcomp;