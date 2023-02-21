#include "AppServer.h"


void AppServer::ThreadTestfuntion()
{
	while (1)
	{
		std::cout << "타 클라스 함수 호출\n";
		Sleep(500);
	}
}
