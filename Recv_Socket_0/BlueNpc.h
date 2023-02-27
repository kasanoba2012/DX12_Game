#pragma once
class BlueNpc
{
	struct minion
	{
		int my_index = 0;
		int npc_pos_[3];
		int npc_pos_dir_ = 5;
		int team_color = 0;
		int npc_speed = 1;
	};
public:
	minion npc_info_;
};

