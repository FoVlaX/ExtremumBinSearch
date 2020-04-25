#include "server.h"

server::server(HRESULT &hr)
{
	 hr = S_OK;
	if (FAILED(WSAStartup(MAKEWORD(2, 2), &WsaData)))
	{
		hr = E_FAIL;
	}
	ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ListenSocket <= 0) return;
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(27015);
	if (bind(ListenSocket,(const sockaddr*)&address,sizeof(sockaddr_in)) < 0)
	{
		hr = E_FAIL;
	}
	DWORD nonBlocking = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &nonBlocking) != 0)
	{
		hr = E_FAIL;
	}
}
server::~server()
{

}

bool server::recive(state& mes)
{
	int count_b;
	sockaddr_in from;
	int fromlength = sizeof(from);
	count_b = recvfrom(ListenSocket, (char*)mes.stt, 36, 0, (sockaddr *)&from, &fromlength);
	if (count_b!=36) return false;
	return true;
}
