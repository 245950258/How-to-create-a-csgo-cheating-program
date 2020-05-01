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

const int g_players_count = 64;
struct player_list
{
	bool effective;//是否有效
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
	float squat;//是否下蹲
	float jump;//是否跳跃
};

//获取下蹲状态
bool get_squat_state(player_list* players)
{
	for (int i = 0; i < g_players_count; i++)
		if (players[i].effective && players[i].self) return players[i].squat < 50.0f;
	return false;
}

//获取跳跃状态
bool get_jump_state(player_list* players)
{
	for (int i = 0; i < g_players_count; i++)
		if (players[i].effective && players[i].self) return (int)players[i].jump;

	return false;
}

//获取鼠标左键按下状态
bool get_mouse_left_down()
{
	return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
}

//获取开镜状态
bool get_open_mirror_state(player_list* players)
{
	for (int i = 0; i < g_players_count; i++)
		if (players[i].effective && players[i].self) return players[i].mirror;
	return false;
}

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

	float to_width = window_width + (matrix[0][0] * location[0]
		+ matrix[0][1] * location[1]
		+ matrix[0][2] * location[2]
		+ matrix[0][3]) * to_target * window_width;

	float to_height_h = window_heigt - (matrix[1][0] * location[0]
		+ matrix[1][1] * location[1]
		+ matrix[1][2] * (location[2] + 75.0f)
		+ matrix[1][3]) * to_target * window_heigt;

	float to_height_w = window_heigt - (matrix[1][0] * location[0]
		+ matrix[1][1] * location[1]
		+ matrix[1][2] * (location[2] - 5.0f)
		+ matrix[1][3]) * to_target * window_heigt;

	x = (int)(to_width - (to_height_w - to_height_h) / 4.0f);
	y = (int)(to_height_h);
	w = (int)((to_height_w - to_height_h) / 2.0f);
	h = (int)(to_height_w - to_height_h);
	return true;
}

//获取玩家列表
void get_player_list(struct player_list* players)
{
	handle process = g_process_handle;

	for (int i = 0; i < g_players_count; i++)
	{
		int player_base_address;
		read_memory(process, (g_players_address + i * 0x10), &player_base_address, sizeof(int));
		if(player_base_address == 0) continue;

		read_memory(process, player_base_address + 0x100, &players[i].blood, sizeof(int));
		if(players[i].blood <= 0) continue;

		players[i].effective = true;
		players[i].aimbot_len = 9999;

		int bone_base_address;
		if (read_memory(process, (player_base_address + 0x26A8), &bone_base_address, sizeof(int)))
		{
			read_memory(process, (bone_base_address + 99 * sizeof(float)), &players[i].head_bone[0], sizeof(float));
			read_memory(process, (bone_base_address + 103 * sizeof(float)), &players[i].head_bone[1], sizeof(float));
			read_memory(process, (bone_base_address + 107 * sizeof(float)), &players[i].head_bone[2], sizeof(float));
		}

		read_memory(process, player_base_address + 0x138, players[i].location, sizeof(players[i].location));
		read_memory(process, player_base_address + 0xF4, &players[i].camp, sizeof(int));
		read_memory(process, player_base_address + 0xB368, &players[i].armor, sizeof(int));
		read_memory(process, player_base_address + 0xB35C, &players[i].helmet, sizeof(bool));
		read_memory(process, player_base_address + 0x3914, &players[i].mirror, sizeof(bool));
		read_memory(process, player_base_address + 0xB358, &players[i].money, sizeof(int));
		read_memory(process, player_base_address + 0x302C, &players[i].recoil, sizeof(float));
		read_memory(process, player_base_address + 0x3930, &players[i].immunity, sizeof(bool));
		read_memory(process, player_base_address + 0xA380, &players[i].shot, sizeof(int));
		read_memory(process, player_base_address + 0xA40C, &players[i].flash, sizeof(float));
		read_memory(process, player_base_address + 0x110, &players[i].squat, sizeof(float));
		read_memory(process, player_base_address + 0x11C, &players[i].jump, sizeof(float));
	}
}

//释放玩家列表内存
void free_player_list(player_list* players)
{
	ZeroMemory(players, sizeof(player_list) * g_players_count);
}

//获取自己的位置
void get_self_location(float* location)
{
	int location_base_address;
	read_memory(g_process_handle, g_self_address, &location_base_address, sizeof(int));
	if (location_base_address)
		read_memory(g_process_handle, location_base_address + 0x35A8, location, sizeof(float) * 3);
}

//设置自己阵营
void set_self_camp(player_list* players)
{
	float self_location[3];
	get_self_location(self_location);

	for (int i = 0; i < g_players_count; i++)
	{
		if (players[i].effective)
		{
			int temp_x = abs((int)self_location[0] - (int)players[i].location[0]);
			int temp_y = abs((int)self_location[1] - (int)players[i].location[1]);
			if (temp_x < 10.0f && temp_y < 10.f)
			{
				players[i].self = true;
				break;
			}
		}
	}
}

//获取自己的阵营
int get_self_camp(player_list* players)
{
	for (int i = 0; i < g_players_count; i++)
		if (players[i].effective && players[i].self) return players[i].camp;
	return 2;
}

//获取自己开枪状态
bool get_shot_state(player_list* players)
{
	for (int i = 0; i < g_players_count; i++)
		if (players[i].effective && players[i].self) return players[i].shot;
	return false;
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

	int matrix_address = find_pattern(process_handle, client_panorama_module, "\x80\xBF??\xE0\xC0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xBF", 1) + 0x16;
	int angle_address = find_pattern(process_handle, engine_module, "\x70\x9D??\x70\x37??\x50\x0B??\x01\x00\x00\x00\x70\x9D??\x70\x9D", 0) + 0x40;
	int self_address = find_pattern(process_handle, client_panorama_module, "\x20\xE3??\x30\x91??\x08\x20\x08\x00\xE8\x45??\x28\xFB??\x4C\xE3??\xC8\xA4?\x04\x04", 0) + 0x50;
	int players_address = find_pattern(process_handle, client_panorama_module, "\x40??\x80???\x10\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00?\x00\x00\x00\xFF\xFF\xFF\xFF", 6) + 0x9B;

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
	return (int)sqrt((temp_x * temp_x) + (temp_y * temp_y));
}

//绘制血量
void render_player_blood(float blood, int x, int y, int h)
{
	float value = blood / 100.0f * h;
	render_line(D3DCOLOR_XRGB(250, 0, 255), x, y, x, y + value);
}

//绘制指示线
void render_underline(D3DCOLOR color, int window_w, int window_h, int x, int y)
{
	render_line(color, x, y, window_w, window_h * 2);
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

	int self_camp = get_self_camp(players);

	for (int i = 0; i < g_players_count; i++)
	{
		int x, y, w, h;
		if (players[i].effective  && players[i].self == false && to_rect_info(matrix, players[i].location, window_w, window_h, x, y, w, h))
		{
			D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 0);
			if (self_camp != players[i].camp)
			{
				color = D3DCOLOR_XRGB(255, 0, 0);
				players[i].aimbot_len = get_aimbot_len(window_w, window_h, x + (w / 2), y + (h / 2));
			}
			render_rect(color, x, y, w, h);
			render_player_blood(players[i].blood, x - 5, y, h);
			render_underline(color,window_w, window_h, x + (w / 2), y + h);
		}
	}
}

//获取后座力
float get_recoil(player_list* player)
{
	for (int i = 0; i < g_players_count; i++)
		if (player[i].effective && player[i].self) player[i].recoil *= 1.9f;
	return 0.0f;
}

//获取当前角度
void get_current_angle(float* angle)
{
	int angle_base_address;
	if (read_memory(g_process_handle, g_angle_address, &angle_base_address, sizeof(int)))
		read_memory(g_process_handle, angle_base_address + 0x4D88, angle, sizeof(float) * 2);
}

//设置当前角度
void set_current_angle(float* angle)
{
	int angle_base_address;
	if (read_memory(g_process_handle, g_angle_address, &angle_base_address, sizeof(int)))
		write_memory(g_process_handle, angle_base_address + 0x4D88, angle, sizeof(float) * 2);
}

//获取自瞄角度
void get_aimbot_angle(float* self_location, float* player_location, float* aim_angle, bool squat,float recoil)
{
	float x = self_location[0] - player_location[0];
	float y = self_location[1] - player_location[1];
	float z = self_location[2] - player_location[2] + 65.0f + 15.0f;
	if (squat) z -= 15.0f;
	z += recoil;

	const float pi = 3.1415f;
	aim_angle[0] = (float)atan(z / sqrt(x * x + y * y)) / pi * 180.f;
	aim_angle[1] = (float)atan(y / x);
	
	if (x >= 0.0f && y >= 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.0f - 180.0f;
	else if (x < 0.0f && y >= 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.0f;
	else if (x < 0.0f && y < 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.0f;
	else if (x >= 0.0f && y < 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.f + 180.0f;
}

//获取最近的任务骨骼位置
int get_recent_head_location(player_list* players, float* self_location)
{
	int index = -1;
	int camp = get_self_camp(players);
	for (int i = 0; i < g_players_count; i++)
	{
		if (players[i].effective && camp != players[i].camp)
		{
			if (index == -1) index = i;
			else if (players[index].aimbot_len > players[i].aimbot_len) index = i;
		}
	}
	return index;
}

//自瞄开启
void aimbot_players(player_list* player, float max_fov = 30)
{
	if (get_jump_state(player) == true) return;

	float self_location[3];
	get_self_location(self_location);

	int aim_index = get_recent_head_location(player, self_location);
	if (aim_index == -1) return;

	float current_angle[2];
	get_current_angle(current_angle);

	bool squat = get_squat_state(player);

	float aim_angle[2];
	get_aimbot_angle(self_location, player[aim_index].head_bone, aim_angle, squat, 0.0f);

	if (abs((int)aim_angle[0] - (int)current_angle[0]) > max_fov 
		|| abs((int)aim_angle[1] - (int)current_angle[1]) > max_fov)
		return;

	set_current_angle(aim_angle);
}

//工作开始
void cheats_doing()
{
	player_list players[g_players_count]{ 0 };

	get_player_list(players);

	set_self_camp(players);

	render_player_box(players);

	if(get_mouse_left_down() || get_shot_state(players)) aimbot_players(players);
	//else if(get_open_mirror_state(players)) aimbot_players(players);
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


