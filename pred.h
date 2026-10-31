#pragma once

class InputPrediction
{
public:
	float m_curtime;
	float m_frametime;
	bool m_in_prediction;
	bool m_first_time_predicted;

public:
	datamap_t* cm_start;
	datamap_t* og_predicted;

	void GetDatamap( datamap_t*& map );
	void SetDatamap( datamap_t*& map );

public:
	CUserCmd m_dummy_cmd;

	void update();
	void store();
	void predict();
	void restore();

public:
	void predict_postpone_time( CUserCmd* cmd );
};

extern InputPrediction g_inputpred;