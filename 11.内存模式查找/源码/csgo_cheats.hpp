#pragma once

#include "direct3d9.hpp"

dword g_process_id;
handle g_process_handle;

hwnd g_game_hwnd;
hwnd g_transparent_hwnd;

int g_matrix_address;
int g_angle_address;
int g_self_address;
int g_players_address;

struct player_list
{
	int aimbot_len;//自瞄长度
	bool self;//是自己
	float location[3];//身体位置
	float head_bone[3];//头骨位置
	int camp;//阵营
	int blood;//血量
	int armor;//铠甲
	bool helmet;//头盔
	bool mirror;//是否开镜
	int money;//金钱
	float recoil;//后座力
	bool immunity;//买枪状态
	int shot;//是否开枪
	float flash;//闪光度
	struct player_list* next;
};

//转化为矩阵信息
bool to_rect_info(float matrix[][4], float* location, int window_width, int window_heigt, int& x, int& y, int& w, int& h)
{
	float to_target = matrix[2][0] * location[0]
		+ matrix[2][1] * location[1]
		+ matrix[2][2] * location[2]
		+ matrix[2][3];
	if (to_target < 0.01f)
	{
		x = y = w = h = 0;
		return false;
	}
	to_target = 1.0f / to_target;

	int to_width = window_width + (matrix[0][0] * location[0]
		+ matrix[0][1] * location[1]
		+ matrix[0][2] * location[2]
		+ matrix[0][3]) * to_target * window_width;

	int to_height_h = window_heigt - (matrix[1][0] * location[0]
		+ matrix[1][1] * location[1]
		+ matrix[1][2] * (location[2] + 75.0f)
		+ matrix[1][3]) * to_target * window_heigt;

	int to_height_w = window_heigt - (matrix[1][0] * location[0]
		+ matrix[1][1] * location[1]
		+ matrix[1][2] * (location[2] - 5.0f)
		+ matrix[1][3]) * to_target * window_heigt;

	x = to_width - (to_height_w - to_height_h) / 4;
	y = to_height_h;
	w = (to_height_w - to_height_h) / 2;
	h = to_height_w - to_height_h;
	return true;
}

//获取玩家列表
player_list* get_player_list()
{
	player_list *head = nullptr, *help = nullptr;
	handle process = g_process_handle;

	for (int i = 0; i < 64; i++)
	{
		int player_base_address;
		read_memory(process, (g_players_address + i * 0x10), &player_base_address, sizeof(int));
		if(player_base_address == 0) break;

		int player_blood;
		read_memory(process, player_base_address + 0x100, &player_blood, sizeof(int));
		if(player_blood <= 0) continue;

		player_list* temp = (player_list*)alloc_memory(sizeof(player_list));
		temp->blood = player_blood;
		temp->aimbot_len = 999;

		int bone_base_address;
		if (read_memory(process, (player_base_address + 0x26A8), &bone_base_address, sizeof(int)))
		{
			read_memory(process, (bone_base_address + 99 * sizeof(float)), &temp->head_bone[0], sizeof(float));
			read_memory(process, (bone_base_address + 103 * sizeof(float)), &temp->head_bone[1], sizeof(float));
			read_memory(process, (bone_base_address + 107 * sizeof(float)), &temp->head_bone[2], sizeof(float));
		}

		read_memory(process, player_base_address + 0xA0, temp->location, sizeof(temp->location));
		read_memory(process, player_base_address + 0xF4, &temp->camp, sizeof(int));
		read_memory(process, player_base_address + 0xB368, &temp->armor, sizeof(int));
		read_memory(process, player_base_address + 0xB35C, &temp->helmet, sizeof(bool));
		read_memory(process, player_base_address + 0x3914, &temp->mirror, sizeof(bool));
		read_memory(process, player_base_address + 0xB358, &temp->money, sizeof(int));
		read_memory(process, player_base_address + 0x302C, &temp->recoil, sizeof(float));
		read_memory(process, player_base_address + 0x3930, &temp->immunity, sizeof(bool));
		read_memory(process, player_base_address + 0xA380, &temp->shot, sizeof(int));
		read_memory(process, player_base_address + 0xA40C, &temp->flash, sizeof(float));

		if (head == nullptr)
		{
			head = temp;
			help = head;
		}
		else
		{
			help->next = temp;
			help = temp;
		}
	}
	return head;
}

//释放玩家列表内存
void free_player_list(player_list* data)
{
	while (data)
	{
		player_list* temp = data;
		data = data->next;
		free_memory(temp);
	}
}

//获取自己的位置
void get_self_location(float* location)
{
	int location_base_address;
	read_memory(g_process_handle, g_self_address, &location_base_address, sizeof(int));
	if (location_base_address)
		read_memory(g_process_handle, location_base_address + 0x35A8, location, sizeof(float) * 3);
}

//获取自己的阵营
int get_self_camp(float* self_location, player_list* players)
{
	int value = 999, camp = 2;
	while (players)
	{
		int temp_x = abs((int)self_location[0] - (int)players->head_bone[0]);
		int temp_y = abs((int)self_location[1] - (int)players->head_bone[1]);
		if (temp_x < 50.0f && temp_y < 50.0f)
		{
			players->self = true;
			return players->camp;
		}
		players = players->next;
	}
	return camp;
}

//初始化地址信息
void initialize_address(const char* process_name)
{
	dword process_id = get_process_id(process_name);
	handle process_handle = get_process_handle(process_id);
	g_process_id = process_id;
	g_process_handle = process_handle;

	struct module_information engine_module;
	struct module_information client_panorama_module;
	get_module_info(process_handle, process_id, "engine.dll", engine_module);
	get_module_info(process_handle, process_id, "client_panorama.dll", client_panorama_module);

	int matrix_address = find_pattern(process_handle, client_panorama_module, "\x16\x08\x80\xBF\x26\x0E\xE0\xC0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xBF\x00\x00\x00\x00", 1) + 0x18;
	int angle_address = find_pattern(process_handle, engine_module, "\x70\x9D??\x70\x37??\x50\x0B?\x03\x01\x00\x00\x00\x70\x9D??\x70\x9D", 0) + 0x40;
	int self_address = find_pattern(process_handle, client_panorama_module, "\x20\xE3??\x30\x91??\x08\x20\x08\x00\xE8\x45??\x28\xFB??\x4C\xE3??\xC8\xA4?\x04\x04", 0) + 0x50;
	int players_address = find_pattern(process_handle, client_panorama_module, "\xC0\x2F??\x10\xE2?\x13?\x01\x00\x00\x00\x00\x00\x00\xB4\x3A", 0) + 0x14;

#ifdef DEBUG_STRING
	printf("自己矩阵基地址 : %8x \n", matrix_address);
	printf("自己角度基地址 : %8x \n", angle_address);
	printf("自己位置基地址 : %8x \n", self_address);
	printf("玩家信息基地址 : %8x \n", players_address);
	printf("\n");
#endif // DEBUG_STRING

	g_matrix_address = matrix_address;
	g_angle_address = angle_address;
	g_self_address = self_address;
	g_players_address = players_address;
}

//计算自瞄距离
int get_aimbot_len(int window_w, int window_h, int x, int y)
{
	int temp_x = abs(window_w - x);
	int temp_y = abs(window_h - y);
	return sqrt((temp_x * temp_x) + (temp_y * temp_y));
}

//绘制玩家方框
void render_player_box(player_list* players)
{
	int window_x, window_y, window_w, window_h;
	get_window_size(g_game_hwnd, window_x, window_y, window_w, window_h);
	window_w /= 2;
	window_h /= 2;

	float matrix[4][4];
	read_memory(g_process_handle, g_matrix_address, matrix, sizeof(float) * 4 * 4);

	float self_location[3];
	get_self_location(self_location);

	int self_camp = get_self_camp(self_location, players);

	D3DCOLOR color;

	while (players)
	{
		int x, y, w, h;
		if (players->self == false && to_rect_info(matrix, players->location, window_w, window_h, x, y, w, h))
		{
			if (self_camp == players->camp)
			{
				color = D3DCOLOR_XRGB(255, 0, 0);
			}
			else
			{
				color = D3DCOLOR_XRGB(255, 255, 0);
				players->aimbot_len = get_aimbot_len(window_w, window_h, x, y);
			}
			render_rect(color, x, y, w, h);
		}
		players = players->next;
	}
}

//自瞄开启
void aimbot_players(player_list* player)
{


}

//工作开始
void cheats_doing()
{
	player_list* players = get_player_list();
	player_list* help = players;

	render_player_box(help);



	free_player_list(players);
}

//开始操作
void start_cheats_csgo()
{
	initialize_address("csgo.exe");

	g_cheating = cheats_doing;

	hwnd game_hwnd = FindWindowA(nullptr, "Counter-Strike: Global Offensive");
	hwnd transparent_hwnd = create_transparent_window(game_hwnd);
	g_game_hwnd = game_hwnd;
	g_transparent_hwnd = transparent_hwnd;

	initialize_direct3d9(transparent_hwnd);
	message_handle(game_hwnd, transparent_hwnd);
}


