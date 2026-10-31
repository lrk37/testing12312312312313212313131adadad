#pragma once

class TickbaseManager
{
public:
	int m_extra_process_ticks;
	int m_shift_ticks;
	bool m_shifting;
	float m_last_shift_time;

public:
	int m_pre_shift_ticks;

	int GetShiftAmount();
	void UpdateTicks();
	void FinishShift();

public:
	void UpdateTickbase( int command );
	void GetHighLowTickbase( int* low, int* high, int* ideal );

	struct TickbaseLog_t
	{
		int command;
		int low;
		int high;
		int ideal;
		int queued;
	};

	std::deque<TickbaseLog_t> m_tickbase_log;
	void AddTickShift( int command, int low, int high, int ideal, int queued );
};

extern TickbaseManager g_tickbase;