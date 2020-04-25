#pragma once
#include <windows.h>
#include <winsock.h>

#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"Ws2_32.lib")


struct state{
	union {
		char stt[36];
		struct {
			float x;
			float y;
			float z;
			float xs;
			float ys;
			float zs;
			float xang;
			float yang;
			float zang;
		};
	};
};

struct message {
	union {
		char mes[44];
		struct {
			int tMes;
			int id;
			state st;
		};
	};
};

class server
{
public:
	server(HRESULT& hr);
	~server();
	bool recive(state &mes);
	void send();
private:
	WSADATA WsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	sockaddr_in addresses[10];
};

