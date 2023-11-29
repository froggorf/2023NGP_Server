#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"
#include "ProcessClientInput.h"

// Timer ����
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")




// �Լ�
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// ���� ������ �ʱ�ȭ �κ�, ����� �� �ٽ� ȣ���Ͽ� ������ �� �ֵ��� ���� ����
void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime); 	// �ð� ������Ʈ �Լ�
void Send_Game_Time();
void SendPlayerLocationToAllClient();
int remainingSeconds = MAX_MIN * 60; // 5���� �ʷ� ȯ��

// Ű ��ǲ �޾� ó���ϴ� ������
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);

// �÷��̾� ���� �����ϴ� ������
void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock);
DWORD WINAPI SendPlayerDataToClient(LPVOID arg);

// �÷��̾� �躤�� ���۹޴� ������
void CreateRecvLookVectorThread(SOCKET& recv_lookvector_listen_sock);
DWORD WINAPI RecvLookVectorFromClient(LPVOID arg);


void CreateCubeThread(SOCKET& Cube_listen_sock);
DWORD WINAPI EchoClientRequestCube(LPVOID arg);
bool Check_Add_Cube(Cube_Info cube);

bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);

// ���� ����
WSADATA wsa;

// ElapsedTime ����
DWORD g_startTime;
DWORD g_prevTime;

int main(int argc, char *argv[])
{
	int retval;

	//----------------�α��� ���� ����� ����----------------   //TODO: �Ʒ����� ���ļ� �Լ�ȭ ó�� ����
	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����(�÷��̾� ���� üũ ����)
	SOCKET login_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (login_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(login_listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(login_listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//---------------- ���� ����� ����(Ű ��ǲ ����)----------------
	// ���� ����(Ű ��ǲ ���� ����)
	SOCKET KeyInput_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (KeyInput_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind() - KEYINPUTSERVERPORT �� ���ε�,
	struct sockaddr_in serveraddr_keyinput;
	memset(&serveraddr_keyinput, 0, sizeof(serveraddr_keyinput));
	serveraddr_keyinput.sin_family = AF_INET;
	serveraddr_keyinput.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_keyinput.sin_port = htons(KEYINPUTSERVERPORT);

	if (bind(KeyInput_listen_sock, (struct sockaddr*)&serveraddr_keyinput, sizeof(serveraddr_keyinput))
		== SOCKET_ERROR) err_quit("CreateClientKeyInputThread() - bind()");

	// listen()
	if (listen(KeyInput_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("CreateClientKeyInputThread() - listen()");
	

	//---------------- ���� ����� ����(ť�� ����)----------------
	// ���� ����(ť�� ���� ����)
	SOCKET Cube_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Cube_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_Cube;
	memset(&serveraddr_Cube, 0, sizeof(serveraddr_Cube));
	serveraddr_Cube.sin_family = AF_INET;
	serveraddr_Cube.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_Cube.sin_port = htons(CUBESERVERPORT);

	if (bind(Cube_listen_sock, (struct sockaddr*)&serveraddr_Cube, sizeof(serveraddr_Cube))
		== SOCKET_ERROR) err_quit("CreateCubeThread() - bind()");

	// listen()
	if (listen(Cube_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("CreateCubeThread() - listen()");

	//---------------- ���� ����� ����(�÷��̾� ������ ���� ����)----------------
	// ���� ����(�÷��̾� ������ ���� ����)
	SOCKET send_playerdata_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (send_playerdata_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_sendPlayerData;
	memset(&serveraddr_sendPlayerData, 0, sizeof(serveraddr_sendPlayerData));
	serveraddr_sendPlayerData.sin_family = AF_INET;
	serveraddr_sendPlayerData.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_sendPlayerData.sin_port = htons(SENDPLAYERDATAPORT);

	if (bind(send_playerdata_listen_sock, (struct sockaddr*)&serveraddr_sendPlayerData, sizeof(serveraddr_sendPlayerData))
		== SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(send_playerdata_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("listen()");

	//---------------- ���� ����� ����(�÷��̾� ������ ���� ����)----------------
	// ���� ����(�÷��̾� ������ ���� ����)
	SOCKET recv_LookVector_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (recv_LookVector_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_recvLookVector;
	memset(&serveraddr_recvLookVector, 0, sizeof(serveraddr_recvLookVector));
	serveraddr_recvLookVector.sin_family = AF_INET;
	serveraddr_recvLookVector.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_recvLookVector.sin_port = htons(RECVLOOKVECTORPORT);

	if (bind(recv_LookVector_listen_sock, (struct sockaddr*)&serveraddr_recvLookVector, sizeof(serveraddr_recvLookVector))
		== SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(recv_LookVector_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("listen()");

	//-----------------
	// ���� ������ �ʱ�ȭ
	InitGame();
	
	//�÷��̾� ������ ���� ������ �� �躤�� ���۹޴� ������ �̸� �����Ű��
	HANDLE hThread = CreateThread(NULL, 0, SendPlayerDataToClient,
		(LPVOID)0, 0, NULL);
	CloseHandle(hThread);
	hThread = CreateThread(NULL, 0, RecvLookVectorFromClient,
		(LPVOID)0, 0, NULL);
	CloseHandle(hThread);

	// �÷��̾� ������ �� �ο� ���ӽ�Ű��
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen_sock);
		CreateClientKeyInputThread(KeyInput_listen_sock);
		CreateCubeThread(Cube_listen_sock);
		CreateSendPlayerDataThread(send_playerdata_listen_sock);
		CreateRecvLookVectorThread(recv_LookVector_listen_sock);
	}

	// ���� �ݱ�
	closesocket(login_listen_sock);



	// TODO: ���� ���� �ǹǷ� ���� ���� ���� ������ ���� ����

	// Ÿ�̸� �ʱ�ȭ
	SetTimer(NULL, TIMER_ID, 1000, TimerProc); // 1000ms(1��)���� Ÿ�̸� ȣ��
	std::cout << "Ÿ�̸� ���� - " << remainingSeconds << std::endl;

	// �÷��̾��� Ű ��ǲ ������ �޴� ������ ����

	// TODO: while������ main �����忡���� �߷�, �浹üũ �� �ð� ���� ���� ����ǵ��� ���� ����
	while (true) {


		// �ð� ó���� ���� �޼��� ����
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			
		}

		SendPlayerLocationToAllClient();
	}

	// ���� �ݱ�
	//closesocket(listen_sock);


	// ���� ����
	WSACleanup();
	return 0;
}

void ConnectAndAddPlayer(SOCKET& listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("accept()");
		return;
	}

	// PlayerNumber ����
	send(client_sock, (char*)&Current_Player_Count, sizeof(Current_Player_Count), 0);
	// �ð� ��ſ� ���� ���Ϳ� ����
	socket_vector.push_back(client_sock);
	// Ŭ���̾�Ʈ �ּ� ������ �߰� 
	clientAddr[Current_Player_Count] = clientaddr;

	Player_Info[Current_Player_Count].fPosition_x = 0.0f;
	Player_Info[Current_Player_Count].fPosition_y = 50.0f;
	Player_Info[Current_Player_Count].fPosition_z = 0.0f;

	// �÷��̾� �� ����
	Current_Player_Count += 1;

	// ������ Ŭ���̾�Ʈ ���� ���
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));

	printf("�� �÷��̾� �� : %d\n", Current_Player_Count);
}

void InitGame()
{
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		Player_Info[i].fPosition_x = 0.0f;
		Player_Info[i].fPosition_y = -50.0f;
		Player_Info[i].fPosition_z = 0.0f;

		Player_Info[i].fLook_x = 0.0f;
		Player_Info[i].fLook_z = 1.0f;
	}

	

	g_startTime = g_prevTime = 0;



	Total_Cube.clear();
}

void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock)
{
	SOCKET* client_sock = new SOCKET();
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	*client_sock = accept(KeyInput_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (*client_sock == INVALID_SOCKET) {
		err_display("CreateClientKeyInputThread() - accept()");
		return;
	}

	HANDLE hThread = CreateThread(NULL, 0, ProcessClientKeyInput,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) {  closesocket(*client_sock); }
	else { CloseHandle(hThread); }
}
DWORD WINAPI ProcessClientKeyInput(LPVOID arg)
{
	printf("Ű ��ǲ ������ ����\n");
	SOCKET* ClientKeyInputSocket = (SOCKET*)arg;
	struct sockaddr_in clientaddr;

	struct KeyInput clientKeyInput{};
	int retval;
	while(1)
	{
		retval = recv(*ClientKeyInputSocket, (char*)&clientKeyInput, sizeof(clientKeyInput), 0);
		if (retval == SOCKET_ERROR||(clientKeyInput.Key<0 || clientKeyInput.Key>256)) {
			printf("?\n");
			break;
		}
		if(!clientKeyInput.KeyDown && (clientKeyInput.Key==27||clientKeyInput.Key==0))
		{
			//������ ��Ȳ���� Ȯ�ε�
			closesocket(*ClientKeyInputSocket);
			break;
		}
		printf("%d �÷��̾ %d ��ư ", clientKeyInput.PlayerNumber, clientKeyInput.Key);
		if(clientKeyInput.KeyDown)
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, true);
			printf("����\n");

		}else
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, false);
			printf("��\n");
		}
	}
	return 0;
}

void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(senddata_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateSendPlayerDataThread() - accept()");
		return;
	}
	socket_SendPlayerData_vector.push_back(client_sock);


}

DWORD WINAPI SendPlayerDataToClient(LPVOID arg)
{
	printf("�÷��̾� ���� ���� ����\n");
	//SOCKET SendPlayerDataSocket = (SOCKET)arg;
	//struct sockaddr_in clientaddr;

	int retval;
	while (1)
	{
		Sleep(30);
		// ElapsedTime ���
		DWORD ElapsedTime = 0;
		DWORD CurrentTime = timeGetTime();
		if (g_prevTime == 0) ElapsedTime = CurrentTime - g_startTime;
		else ElapsedTime = CurrentTime - g_prevTime;
		g_prevTime = CurrentTime;
		float ElapsedTimeInSec = (float)ElapsedTime / 1000.0f;

		//�÷��̾� �̵����� 
		ProcessClientInput(ElapsedTimeInSec);

		//�÷��̾� ���� ��� ����
		for (auto p = socket_SendPlayerData_vector.begin(); p != socket_SendPlayerData_vector.end(); ++p) {
			retval = send(*p, (char*)&Player_Info, sizeof(struct Player_Info) * MAXPLAYERCOUNT, 0);
			if (retval == SOCKET_ERROR) {
				printf("����� ������ Ȯ�ε�\n");
				closesocket(*p);
				socket_SendPlayerData_vector.erase(p);
				break;
			}
		}
	}
	return 0;
}

void CreateRecvLookVectorThread(SOCKET& recv_lookvector_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(recv_lookvector_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateRecvLookVectorThread() - accept()");
		return;
	}
	socket_RecvLookVector.push_back(client_sock);
}

DWORD WINAPI RecvLookVectorFromClient(LPVOID arg)
{
	printf("�躤�� ���۹ޱ� ����\n");

	int retval;
	struct Look_Data data;
	while (1)
	{
		//�÷��̾� ���� ��� ����
		int size = socket_RecvLookVector.size();
		for (int i = 0; i < size; ++i) {
			
			retval = recv(socket_RecvLookVector[i], (char*)&data, sizeof(struct Look_Data), MSG_WAITALL);
			if(retval == SOCKET_ERROR)
			{
				printf("����� ������ Ȯ�ε�\n");
				closesocket(socket_RecvLookVector[i]);
				socket_RecvLookVector.erase(socket_RecvLookVector.begin()+i);
				break;
			}
			Player_Info[data.PlayerNumber].fLook_x = data.fLook_x;
			Player_Info[data.PlayerNumber].fLook_z = data.fLook_z;
		}
	}
	return 0;
}



void CreateCubeThread(SOCKET& Cube_listen_sock)
{
	printf("ť�� ������ ����\n");
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(Cube_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateCubeThread() - accept()");
		return;
	}
	socket_Cube_vector.push_back(client_sock);

	// ���� �ʿ� �����ִ� ��� ť�� ���� ����
	for(const auto cube : Total_Cube)
	{
		send(client_sock, (char*)&cube, sizeof(cube), 0);
	}

	HANDLE hThread = CreateThread(NULL, 0, EchoClientRequestCube,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}


DWORD WINAPI EchoClientRequestCube(LPVOID arg)
{
	SOCKET CubeSocket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	struct Cube_Info clientCubeInput;
	int retval;
	while (1)
	{
		// ť�� ���ú�
		retval = recv(CubeSocket, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
		if (retval == SOCKET_ERROR) {
			printf("?\n");
			break;
		}
		printf("Cube Position - %.2f, %.2f, %.2f\n", clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
		printf("Cube Color - %.2f, %.2f, %.2f\n", clientCubeInput.fColor_r, clientCubeInput.fColor_g, clientCubeInput.fColor_b);
		printf("Cube Add or Delete - %s\n", clientCubeInput.AddorDelete ? "Add" : "Delete");
		
		// Cube Add
		if (clientCubeInput.AddorDelete)	
		{
			// ���⼭ ť��� ��� �浹üũ
			if (Check_Add_Cube(clientCubeInput))
			{
				printf("ť�� ��ġ �Ұ���\n");
			}
			// ���ɽÿ��� �� Ŭ�󿡰� ť�� ���� send
			else
			{
				printf("ť�� ��ġ ����\n");
				// add to Total_Cube
				Total_Cube.push_back(clientCubeInput);
				// ť�� send to every client
				for (auto i : socket_Cube_vector) {
					int retval = send(i, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}
					std::cout << "Sending Add cube_info to the client" << std::endl;
				}
			}
		}
		// Cube Delete
		else
		{
			auto it = std::find_if(Total_Cube.begin(), Total_Cube.end(), [&](const Cube_Info& value) {
				return CompareXMFLOAT3(DirectX::XMFLOAT3(value.fPosition_x, value.fPosition_y, value.fPosition_z),
				DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z));
				});

			if (it != Total_Cube.end()) 
			{
				// ť�� ���� ã�� ���
				printf("ť�� ���� ����\n");
				Total_Cube.erase(it);

				for (auto i : socket_Cube_vector) {
					int retval = send(i, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}
					std::cout << "Sending Delete cube_info to the client" << std::endl;
				}
			}
			else {
				// ť�� ���� ã�� ���� ���
				std::cout << "Delete Cube Value not found!" << std::endl;
			}
		}
	}
	return 0;
}


// Ÿ�̸� �ݹ� �Լ�
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	// �ð� ������Ʈ
	remainingSeconds--;
	// ���� �ð��� 0���� ũ�ų� ������ Ŭ���̾�Ʈ�� �ð� ������Ʈ �� ����
	if (remainingSeconds >= 0) {
		Send_Game_Time();
	}
	else {
		// ���� ���� �̺�Ʈ ���⿡ �߰�
		
		// Ÿ�̸� ����
		KillTimer(hwnd, TIMER_ID);
		for (auto i : socket_vector)	closesocket(i);		// �ð� ���� close
	}
}


void Send_Game_Time() {
	// �ð� ������ ������
	for (auto i : socket_vector) {
		int retval = send(i, (char*)&remainingSeconds, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}
	std::cout << "Sending time to the client: " << remainingSeconds << " seconds" << std::endl;
}

void SendPlayerLocationToAllClient()
{

}

bool Check_Add_Cube(Cube_Info cube)
{
	for (int i = 0; i < MAXPLAYERCOUNT; i++)
	{
		// X, Z �࿡ ���� ��ġ���� Ȯ��
		float xDiff = Player_Info[i].fPosition_x - cube.fPosition_x;
		float zDiff = Player_Info[i].fPosition_z - cube.fPosition_z;

		bool isXInRange = (xDiff >= -5.0f) && (xDiff <= 5.0f);
		bool isZInRange = (zDiff >= -5.0f) && (zDiff <= 5.0f);

		// Y �࿡ ���� ���� Ȯ��
		bool isYInRange = (Player_Info[i].fPosition_y >= cube.fPosition_y) && (Player_Info[i].fPosition_y <= cube.fPosition_y + 10.0f);

		printf("xDiff : %.2f, zDiff : %.2f, bool Y : %d\n", xDiff, zDiff, isYInRange);
		// ��� �࿡�� ������ �����ϴ��� Ȯ���Ͽ� ��� �ȿ� �÷��̾ �ִ��� ��ȯ
		return isXInRange && isZInRange && isYInRange;
		
	}
}


bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}

