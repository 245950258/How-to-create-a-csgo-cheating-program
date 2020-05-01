#include "csgo_cheats.hpp"

/*
2020.05.01
CSGO简单外挂
实现功能: 方框透视 + 开枪自瞄 + 开镜自瞄
高级功能内挂才好实现一点，外挂的话可能要使用远程线程，没测试过不确定
进入游戏对战再打开程序,不然矩阵内存签名会查找不到!!!当然你也可以到csgo_cheats.hpp的initialize_address()函数里修改你自己找到的内存签名
*/

int main(int argc, char* argv[])
{
	start_cheats_csgo();
	return 0;
}


