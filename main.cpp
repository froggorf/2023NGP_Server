#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"
#include "ProcessClientInput.h"
#include "array"

// Timer ����
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <mutex>

// TODO: �������� ��� ����(printf, cout) ��� ������ ����

// ========================== �Լ� ==========================
void CreateListenSockets(SOCKET&, SOCKET&, SOCKET&, SOCKET&, SOCKET&);			// ���� ���� ����
void InitGame();																// ���� ������ �ʱ�ȭ �κ�, ����� �� �ٽ� ȣ���Ͽ� ������ �� �ֵ��� ���� ����
void LoginPlayer(SOCKET&, SOCKET&, SOCKET&, SOCKET&, SOCKET&);					// listen_sock accept �� ������ ���� �Լ�
void ConnectAndAddPlayer(SOCKET&);												// ó�� ���ӽ� �÷��̾ ���� ���� ���� �� �ð� ���� ���� ����
void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock);					// �÷��̾� ���� Ű ��ǲ ���� ���� �޴� ���� �� ������ ����
DWORD WINAPI Send_Game_Time(LPVOID arg);										// �ð� ������Ʈ �Լ�
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);									// �÷��̾��� Ű �Է� ���� �۽Ź޾� Ű ���� ����
void CreateSendPlayerDataThread(SOCKET& senddata_listen_sock);					// �÷��̾� ����(��ġ,�躤��) ���� ���� ����
DWORD WINAPI SendPlayerDataToClient(LPVOID arg);								// ���� �۾� ������, �÷��̾� �̵� - �浹üũ - ��� Ŭ�󿡰� ��� �÷��̾� ��ġ ����
void CreateChatThread(SOCKET& chat_listen_sock);								// ä�� ���� �� ������ ����
DWORD WINAPI ProcessEchoChat(LPVOID arg);										// ä�� ���� ���� �޾� ��� Ŭ�󿡰� �����ϴ� �Լ�
void CreateCubeThread(SOCKET& Cube_listen_sock);								// �� �÷��̾� ���ӽ� ť�� ������ ����
DWORD WINAPI EchoClientRequestCube(LPVOID arg);									// ���� ť�� ���� ��ġ �� ���� �Ǻ� �� ��� Ŭ���̾�Ʈ���� �ٽ� ��������
bool Check_Add_Cube(Cube_Info cube);											// �÷��̾� �ֺ� �ٿ�� �ڽ��� �浹üũ�Ͽ� ť�� ��ġ �������� Ȯ���ϴ� �Լ�
bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b);	// XMFLOAT3 �� �Լ�
void ClearAllSocket();															// ���� �ʱ�ȭ �� ��� ���� ���� �ʱ�ȭ
bool PlayerLogout(int playerNumber);											// �÷��̾ ���� ������ ������ ������ �������� ������ �� �Լ�
void Set_Floor_Cube_Object();													// �ٴ� ������Ʈ ����
void Release_Floor_Cube_Object();												// ��ü ������Ʈ �ʱ�ȭ
void Delete_Cube(Cube_Info clientCubeInput);									// ť�� ���� ��û send
void Request_Delete_All_Cube();													// ť�� ��ü ���� ��û


// ========================== ���� ==========================

WSADATA wsa;																	// ����
int remainingSeconds = GAMETIME;												// ���� �ð� 
HWND timerHWND;																	// Ÿ�̸� �ڵ�
DWORD g_startTime;																// ElapsedTime ����
DWORD g_prevTime;
CRITICAL_SECTION cs_for_logout;													// PlayerLogout(int) ���� bool bPlayerLogout[] ����� ���� cs
CRITICAL_SECTION cs_Cube;														// cs_Cube ����� ���� cs
bool bPlayerLogout[MAXPLAYERCOUNT] = { false, };								// �α׾ƿ� �ߺ� ó�� ���� ����

std::mutex mtx;

int main(int argc, char *argv[])
{
	int retval;
	
	while(true)
	{
		// ť�� ������ �ʱ�ȭ
		Release_Floor_Cube_Object();
		// �ٴ� cube object ����
		Set_Floor_Cube_Object(); 

		// ���� ���� ����
		SOCKET login_listen, keyinput_listen, cube_listen, playerdata_listen, chat_listen;
		CreateListenSockets(login_listen, keyinput_listen, cube_listen, playerdata_listen, chat_listen);
		
		// ���� ������ �ʱ�ȭ
		InitGame();


		// �÷��̾� ������ �� �ο� ���ӽ�Ű��
		LoginPlayer(login_listen, keyinput_listen, cube_listen, playerdata_listen, chat_listen);

		// Ÿ�̸� �ʱ�ȭ
		std::cout << "Ÿ�̸� ���� - " << remainingSeconds << std::endl;
		HANDLE Timer_hThread = CreateThread(NULL, 0, Send_Game_Time,
			(LPVOID)0, 0, NULL);
		CloseHandle(Timer_hThread);

		bool bGame = true;
		while (bGame) {
			// ���� �÷��̾� �ο� üũ �� ���� ��� ����
			if (Current_Player_Count == 0)
			{
				printf("��� �÷��̾� ���Ḧ Ȯ����\n");
				bGame = false;
				break;
			}
		}

		printf("���� ���Ḧ Ȯ���� �ٽ� while ������ ��\n");

		// TODO: ����� InitGame()���� ���� �ʱ�ȭ ����, �ʿ�� EndGame() �����.
		InitGame();
		


	}
	
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

	// �ð� ��ſ� ���� ���Ϳ� ����
	if(socket_vector.size() > Current_Player_Count)		// ���� ���� �� �α׾ƿ� �� ����� �����ִ� ��
	{
		for(int i=0; i<socket_vector.size(); ++i)
		{
			if (socket_vector[i] == INVALID_SOCKET) {
				// PlayerNumber ����
				send(client_sock, (char*)&i, sizeof(Current_Player_Count), 0);
				socket_vector[i] = client_sock;
				Player_Info[i].fPosition_x = 0.0f; Player_Info[i].fPosition_y = 50.0f; Player_Info[i].fPosition_z = 0.0f;
				vPlayer[i].Set_Position(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));
				Player_Info[i].fLook_x = 0.0f; Player_Info[i].fLook_z = 1.0f;
				vPlayer[i].Set_Look_Vector(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
				// Ŭ���̾�Ʈ �ּ� ������ �߰� 
				clientAddr[i] = clientaddr;

				EnterCriticalSection(&cs_for_logout);
				bPlayerLogout[i] = false;
				LeaveCriticalSection(&cs_for_logout);
				break;
			}
		}
	}
	else
	{
		// PlayerNumber ����
		send(client_sock, (char*)&Current_Player_Count, sizeof(Current_Player_Count), 0);
		socket_vector.push_back(client_sock);
		Player_Info[Current_Player_Count].fPosition_x = 0.0f;
		Player_Info[Current_Player_Count].fPosition_y = 50.0f;
		Player_Info[Current_Player_Count].fPosition_z = 0.0f;
		vPlayer[Current_Player_Count].Set_Position(DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f));
		// Ŭ���̾�Ʈ �ּ� ������ �߰� 
		clientAddr[Current_Player_Count] = clientaddr;
	}

	// �α��� �� �Ѹ��� �÷��̾ �������� ���� ��� �����尡 ����Ǵµ�
	// �׿� ���� �߰� ���� 
	if (Current_Player_Count == 0) {
		//�÷��̾� ������ ���� ������ �̸� �����Ű��
		HANDLE hThread = CreateThread(NULL, 0, SendPlayerDataToClient,
			(LPVOID)0, 0, NULL);
		CloseHandle(hThread);
	}

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
	// �÷��̾� ��ġ ���� �ʱ�ȭ
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		Player_Info[i].fPosition_x = 0.0f;
		Player_Info[i].fPosition_y = -50.0f;
		Player_Info[i].fPosition_z = 0.0f;

		Player_Info[i].fLook_x = 0.0f;
		Player_Info[i].fLook_z = 1.0f;

		vPlayer[i].Set_Position(DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f));
		vPlayer[i].Set_Look_Vector(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
	}

	// ���� �ð� �ʱ�ȭ
	g_startTime = g_prevTime = timeGetTime();
	remainingSeconds = GAMETIME-1;

	// ��� ���� �ʱ�ȭ
	ClearAllSocket();

	// ���� �÷��̾� �� ����
	Current_Player_Count = 0;

	// Ŭ���̾�Ʈ �ּ� �ʱ�ȭ
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		clientAddr[i] = sockaddr_in{};
	}

	// ��� �÷��̾� Ű ���� �ʱ�ȭ
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		for(int j=0; j<256; ++j)	SetKeyBuffer(i, j, false);
	}

	// cs_for_logout 
	DeleteCriticalSection(&cs_for_logout);
	InitializeCriticalSection(&cs_for_logout);
	// cs_for_Cube
	DeleteCriticalSection(&cs_Cube);
	InitializeCriticalSection(&cs_Cube);

	// �α׾ƿ� ���� �ʱ�ȭ
	for(int i=0; i<MAXPLAYERCOUNT; ++i)
	{
		bPlayerLogout[i] = false;
	}

}

void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(KeyInput_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateClientKeyInputThread() - accept()");
		return;
	}

	printf("123123\n");

	HANDLE hThread = CreateThread(NULL, 0, ProcessClientKeyInput,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) {  closesocket(client_sock); }
	else { CloseHandle(hThread); }
}
DWORD WINAPI ProcessClientKeyInput(LPVOID arg)
{
	SOCKET ClientKeyInputSocket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	struct KeyInput clientKeyInput{};
	int retval;
	while(1)
	{
		if (remainingSeconds <= 0) {
			break;
		}
		retval = recv(ClientKeyInputSocket, (char*)&clientKeyInput, sizeof(clientKeyInput), 0);
		if (retval == SOCKET_ERROR||(clientKeyInput.Key<0 || clientKeyInput.Key>256)) {
			break;
		}
		if(!clientKeyInput.KeyDown && (clientKeyInput.Key==27||clientKeyInput.Key==0))
		{
			break;
		}
		if(clientKeyInput.KeyDown)
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, true);

		}else
		{
			SetKeyBuffer(clientKeyInput.PlayerNumber, clientKeyInput.Key, false);
		}
	}
	closesocket(ClientKeyInputSocket);
	return 0;
}

void CreateChatThread(SOCKET& chat_listen_sock)
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	client_sock = accept(chat_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("CreateClientKeyInputThread() - accept()");
		return;
	}
	if (socket_chat_vector.size() >= Current_Player_Count)		// ���� ���� �� �α׾ƿ� �� ����� �����ִ� ��
	{
		for (int i = 0; i < socket_chat_vector.size(); ++i)
		{
			if (socket_chat_vector[i] == INVALID_SOCKET) {
				socket_chat_vector[i] = client_sock;
				break;
			}
		}
	}
	else
	{
		socket_chat_vector.push_back(client_sock);
	}

	HANDLE hThread = CreateThread(NULL, 0, ProcessEchoChat,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}

DWORD WINAPI ProcessEchoChat(LPVOID arg)
{
	printf("ä�� ������ ����\n");
	SOCKET echo_chat_socket = (SOCKET)arg;
	struct sockaddr_in clientaddr;

	ChatString chat_string;
	
	int retval;
	while (1)
	{
		if(echo_chat_socket != INVALID_SOCKET)
		{
			retval = recv(echo_chat_socket, (char*)&chat_string, sizeof(ChatString), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				for (int i = 0; i < MAXPLAYERCOUNT; ++i)
				{
					if (socket_chat_vector[i] == echo_chat_socket) {
						if (PlayerLogout(i)) return 0;
						
						break;
					}
				}
				break;
			}
		}
		
		
		int size = socket_chat_vector.size();
		for(int i =0; i<size; ++i)
		{
			if (socket_chat_vector[i] != INVALID_SOCKET)
			{
				retval = send(socket_chat_vector[i], (char*)&chat_string, sizeof(ChatString), 0);
				if(retval == SOCKET_ERROR)
				{
					if(PlayerLogout(i)) return -1;
				}
				
			}
				
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
	if (socket_SendPlayerData_vector.size() >= Current_Player_Count)		// ���� ���� �� �α׾ƿ� �� ����� �����ִ� ��
	{
		for (int i = 0; i < socket_SendPlayerData_vector.size(); ++i)
		{
			if (socket_SendPlayerData_vector[i] == INVALID_SOCKET) {
				socket_SendPlayerData_vector[i] = client_sock;
				break;
			}
		}
	}
	else
	{
		socket_SendPlayerData_vector.push_back(client_sock);
	}

}

DWORD WINAPI SendPlayerDataToClient(LPVOID arg)
{
	printf("�÷��̾� ���� ���� �� �躤�� �ޱ� ����\n");
	//SOCKET SendPlayerDataSocket = (SOCKET)arg;
	//struct sockaddr_in clientaddr;

	int retval;
	struct Look_Data data;
	while (1) 
	{
		Sleep(16);
		int size = socket_SendPlayerData_vector.size();
		for (int i = 0; i < size; ++i) {
			if(socket_SendPlayerData_vector[i]!=INVALID_SOCKET)
			{
				retval = recv(socket_SendPlayerData_vector[i], (char*)&data, sizeof(struct Look_Data), MSG_WAITALL);
				if (retval == SOCKET_ERROR)
				{
					if(PlayerLogout(i)) return -1;
					continue;
				}
				Player_Info[data.PlayerNumber].fLook_x = data.fLook_x;
				Player_Info[data.PlayerNumber].fLook_z = data.fLook_z;
			}
			
		}

		// ElapsedTime ���
		DWORD ElapsedTime = 0;
		DWORD CurrentTime = timeGetTime();
		if (g_prevTime == 0) ElapsedTime = CurrentTime - g_startTime;
		else ElapsedTime = CurrentTime - g_prevTime;
		g_prevTime = CurrentTime;
		float ElapsedTimeInSec = (float)ElapsedTime / 1000.0f;
		if (ElapsedTimeInSec > 0.1f)	ElapsedTimeInSec = 0.1f;

		//�÷��̾� �̵����� 
		// �÷��̾� �浹üũ �� ������ ����
		if(remainingSeconds <= GAMETIME)
		{
			for (int i = 0; i < MAXPLAYERCOUNT; ++i)
			{
				vPlayer[i].Set_Look_Vector(DirectX::XMFLOAT3(Player_Info[i].fLook_x, 0, Player_Info[i].fLook_z));
				vPlayer[i].Set_Right_Vector(vPlayer[i].Get_Look_Vector());

				vPlayer[i].Move(i, PLAYER_MOVE_DISTANCE * ElapsedTimeInSec, true);
				vPlayer[i].Update(i, ElapsedTimeInSec);
				vPlayer[i].Udt_N_Prcs_Collision(ppObjects, nObjects, i);

				Player_Info[i].fPosition_x = vPlayer[i].Get_Position().x;
				Player_Info[i].fPosition_y = vPlayer[i].Get_Position().y;
				Player_Info[i].fPosition_z = vPlayer[i].Get_Position().z;
			}
		}
		

		//�÷��̾� ���� ��� ����
		for(int i =0; i<socket_SendPlayerData_vector.size(); ++i)
		{
			if(socket_SendPlayerData_vector[i]!=INVALID_SOCKET)
			{
				retval = send(socket_SendPlayerData_vector[i], (char*)&Player_Info, sizeof(struct Player_Info) * MAXPLAYERCOUNT, 0);
				if (retval == SOCKET_ERROR) {
					if (PlayerLogout(i)) return -1;
					continue;
				}
			}
			
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

	if (socket_Cube_vector.size() >= Current_Player_Count)		// ���� ���� �� �α׾ƿ� �� ����� �����ִ� ��
	{
		for (int i = 0; i < socket_Cube_vector.size(); ++i)
		{
			if (socket_Cube_vector[i] == INVALID_SOCKET) {
				socket_Cube_vector[i] = client_sock;
				break;
			}
		}
	}
	else
	{
		socket_Cube_vector.push_back(client_sock);
	}

	// ���� �ʿ� �����ִ� ��� ť�� ���� ����
	EnterCriticalSection(&cs_Cube);
	for(const auto cube : Total_Cube)
	{
		send(client_sock, (char*)&cube, sizeof(cube), 0);
	}
	LeaveCriticalSection(&cs_Cube);

	// �� �÷��̾� ���� ť�� ������ ����
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
		if (CubeSocket != INVALID_SOCKET)
		{
			retval = recv(CubeSocket, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
			if (retval == SOCKET_ERROR) {
				for (int i = 0; i < socket_Cube_vector.size(); ++i)
				{
					if (socket_Cube_vector[i] == CubeSocket)
					{
						if (PlayerLogout(i)) return -1;
						break;
					}
				}
				break;
			}
		}

		if (remainingSeconds <= GAMETIME)
		{


			// ���� ť�� ���� ���
			printf("[Cube] -[%.2f, %.2f, %.2f] ��ġ�� [%.2f, %.2f, %.2f] �� %s\n",
				clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z,
				clientCubeInput.fColor_r, clientCubeInput.fColor_g, clientCubeInput.fColor_b,
				clientCubeInput.AddorDelete ? "Add" : "Delete");

			// Cube Add
			if (clientCubeInput.AddorDelete)
			{
				// ���⼭ ť��� ��� �浹üũ
				if (Check_Add_Cube(clientCubeInput))
				{
					//printf("�ش� ��ġ�� ť�� ��ġ �Ұ���\n");
				}
				// ���ɽÿ��� �� Ŭ�󿡰� ť�� ���� send
				else
				{
					// add to Total_Cube
					EnterCriticalSection(&cs_Cube);
					Total_Cube.push_back(clientCubeInput);
					LeaveCriticalSection(&cs_Cube);

					// add to Cube Object
					CObject* pObject = new CObject();
					pObject->Set_Position(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
					ppObjects[nObjects++] = pObject;

					// ť�� send to every client
					for (int i = 0; i < socket_Cube_vector.size(); ++i)
					{
						if (socket_Cube_vector[i] != INVALID_SOCKET)
						{
							int retval = send(socket_Cube_vector[i], (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
							if (retval == SOCKET_ERROR) {
								if (PlayerLogout(i)) return -1;
								break;
							}
						}
					}

				}
			}
			// Cube Delete
			else
			{
				Delete_Cube(clientCubeInput);
			}
		}
	}
	return 0;

}

DWORD WINAPI Send_Game_Time(LPVOID arg) {
	remainingSeconds = GAMETIME + 3;
	std::cout << "�ð� ������ ����" << std::endl;
	// �ð� ������ ������
	while (true)
	{
		if (Current_Player_Count == 0) {
			std::cout << "�ð� ������ ����" << std::endl;
			return 0;
		}
		// ���� �ð��� 0���� ũ�ų� ������ Ŭ���̾�Ʈ�� �ð� ������Ʈ �� ����
		if (remainingSeconds > 0) 
		{
			for (int i = 0; i < socket_vector.size(); ++i)
			{
				if (socket_vector[i])
				{
					int retval = send(socket_vector[i], (char*)&remainingSeconds, sizeof(int), 0);
					if (retval == SOCKET_ERROR) {
						if (PlayerLogout(i)) return 0;
						continue;
					}
				}
			}
			std::cout << "���� ���� �ð�: " << remainingSeconds << " seconds" << std::endl;
		}
		
		// ���� ���� �̺�Ʈ 
		else
		{
			std::array<int, MAXPLAYERCOUNT> player_cube_count {};
			EnterCriticalSection(&cs_Cube);
			for (const auto cube : Total_Cube)
			{
				if (fabs(cube.fColor_r - 1.0f) < FLT_EPSILON) ++player_cube_count[0];
				if (fabs(cube.fColor_g - 1.0f) < FLT_EPSILON) ++player_cube_count[1];
				if (fabs(cube.fColor_b - 1.0f) < FLT_EPSILON) ++player_cube_count[2];
			}
			LeaveCriticalSection(&cs_Cube);
			int max = 0;
			int number_max = 0;
			for (int i = 0; i < 3; i++) {
				if (player_cube_count[i] > max)
				{
					max = player_cube_count[i];
					number_max = i;
				}
			}
			switch (number_max) {
			case 0:
				number_max = -1;
				break;
			case 1:
				number_max = -2;
				break;
			case 2:
				number_max = -3;
				break;
			}
			for (int i = 0; i < socket_vector.size(); ++i)
			{
				printf("[%d] �÷��̾� ť�� ��ġ ���� - %d\n", i, player_cube_count[i]);
				if (socket_vector[i] != INVALID_SOCKET)
				{
					int retval = send(socket_vector[i], (char*)&number_max, sizeof(int), 0);
					if (retval == SOCKET_ERROR) {
						if (PlayerLogout(i)) return 0;
						continue;
					}
					std::cout << i << "ť�� ��ġ ���ۉ�" << std::endl;
				}
			}
			break;

		}
		
		Sleep(1000);	// 1�ʿ� �ѹ��� �������� �����带 sleep
		--remainingSeconds;
	}
	std::cout << "�ð� ������ ����" << std::endl;
	Sleep(1000);
	for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
		PlayerLogout(i);
	}
	return 0;
}

// ��� �÷��̾�� ��ġ�Ϸ��� ť��� �̸� �浹üũ�Ͽ� return
bool Check_Add_Cube(Cube_Info cube)
{
	CObject* CheckCollsion_Cube = NULL;
	CheckCollsion_Cube = new CObject();
	CheckCollsion_Cube->Set_Position(cube.fPosition_x, cube.fPosition_y, cube.fPosition_z);
	for (int i = 0; i < MAXPLAYERCOUNT; i++)
	{
		bool checkCube = vPlayer[i].Check_Collision_Add_Cube(CheckCollsion_Cube, i);
		if (checkCube)
		{
			return true;
		}
	}
	return false;

}


bool CompareXMFLOAT3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return (a.x == b.x && a.y == b.y && a.z == b.z);
}


void ClearAllSocket()
{
	/* Ű ��ǲ ������ ��� �ٸ� Ŭ��鿡�� ������ ������ �ʿ䰡 ���� �ش� �����忡�� ���������� ����Ǿ� �ʱ�ȭ �� �ʿ䰡 ����. (CloseSocket�� ���� ����) */
	for(auto p = socket_vector.begin(); p!=socket_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);
	for (auto p = socket_Cube_vector.begin(); p != socket_Cube_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);
	for (auto p = socket_SendPlayerData_vector.begin(); p != socket_SendPlayerData_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);
	for (auto p = socket_chat_vector.begin(); p != socket_chat_vector.end(); ++p)	if (*p != INVALID_SOCKET) closesocket(*p);

	socket_vector.clear();
	socket_Cube_vector.clear();
	socket_SendPlayerData_vector.clear();
	socket_chat_vector.clear();
}



bool PlayerLogout(int playerNumber)
{
	
	// ���� ����
	EnterCriticalSection(&cs_for_logout);
	if (socket_vector[playerNumber] != INVALID_SOCKET && !bPlayerLogout[playerNumber]) {
		mtx.lock();
		bPlayerLogout[playerNumber] = true;
		printf(" %d ��° �÷��̾ �α׾ƿ� �Ͽ����ϴ�.\n ", playerNumber);
		closesocket(socket_vector[playerNumber]);
		//if (socket_Cube_vector[playerNumber] != INVALID_SOCKET)	
		closesocket(socket_Cube_vector[playerNumber]);
		//if (socket_SendPlayerData_vector[playerNumber] != INVALID_SOCKET)	
		closesocket(socket_SendPlayerData_vector[playerNumber]);
		//if (socket_chat_vector[playerNumber] != INVALID_SOCKET)	
		closesocket(socket_chat_vector[playerNumber]);
		socket_vector[playerNumber] = INVALID_SOCKET;
		socket_Cube_vector[playerNumber] = INVALID_SOCKET;
		socket_SendPlayerData_vector[playerNumber] = INVALID_SOCKET;
		socket_chat_vector[playerNumber] = INVALID_SOCKET;
		mtx.unlock();


		// ���� �÷��̾� �� ���̱�
		Current_Player_Count -= 1;
		printf("%d ���� �÷��̾ ����\n",Current_Player_Count);

		Player_Info[playerNumber].fPosition_x = 0.0f; Player_Info[playerNumber].fPosition_y = -50.0f; Player_Info[playerNumber].fPosition_z = 0.0f;
		vPlayer[playerNumber].Set_Position(DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f));
		Player_Info[playerNumber].fLook_x = 0.0f; Player_Info[playerNumber].fLook_z = 1.0f;
		vPlayer[playerNumber].Set_Look_Vector(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
		
	}
	LeaveCriticalSection(&cs_for_logout);
	if (Current_Player_Count == 0) return true;
	else return false;
	
}

void CreateListenSockets(SOCKET& login_listen_sock, SOCKET& KeyInput_listen_sock, SOCKET& Cube_listen_sock, SOCKET& send_playerdata_listen_sock, SOCKET& echo_chat_listen_sock)
{
	//----------------�α��� ���� ����� ����----------------   //TODO: �Ʒ����� ���ļ� �Լ�ȭ ó�� ����
	int retval;

	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	// ���� ����(�÷��̾� ���� üũ ����)
	login_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (login_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(login_listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(login_listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//---------------- ���� ����� ����(Ű ��ǲ ����)----------------
	// ���� ����(Ű ��ǲ ���� ����)
	KeyInput_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
	Cube_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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
	send_playerdata_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
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

	//---------------- ���� ����� ����(ä�� ����)----------------
	// ���� ����(ä�� ����)
	echo_chat_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (echo_chat_listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr_chat;
	memset(&serveraddr_chat, 0, sizeof(serveraddr_chat));
	serveraddr_chat.sin_family = AF_INET;
	serveraddr_chat.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr_chat.sin_port = htons(ECHOCHATDATAPORT);

	if (bind(echo_chat_listen_sock, (struct sockaddr*)&serveraddr_chat, sizeof(serveraddr_chat))
		== SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(echo_chat_listen_sock, SOMAXCONN)
		== SOCKET_ERROR) err_quit("listen()");

}

void LoginPlayer(SOCKET& login_listen, SOCKET& keyinput_listen, SOCKET& cube_listen, SOCKET& playerdata_listen, SOCKET& chat_listen)
{
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen);
		CreateClientKeyInputThread(keyinput_listen);
		CreateCubeThread(cube_listen);
		CreateSendPlayerDataThread(playerdata_listen);
		CreateChatThread(chat_listen);
	}

	// ���� ���۽� ���� ť�� ��� ���� �� �⺻ �ٴ� ť�� �缳ġ
	Request_Delete_All_Cube();
	Release_Floor_Cube_Object();
	Set_Floor_Cube_Object();

	Player_Info[0].fPosition_x = 75.0f; Player_Info[0].fPosition_y = 50.0f; Player_Info[0].fPosition_z = 0.0f;
	Player_Info[1].fPosition_x = 0.0f; Player_Info[1].fPosition_y = 50.0f; Player_Info[1].fPosition_z = 0.0f;
	Player_Info[2].fPosition_x = 0.0f; Player_Info[2].fPosition_y = 50.0f; Player_Info[2].fPosition_z = -75.0f;
	vPlayer[0].Set_Position(DirectX::XMFLOAT3(75.0f, 50.0f, 0.0f));
	vPlayer[1].Set_Position(DirectX::XMFLOAT3(0.0f,50.0f,0.0f));
	vPlayer[2].Set_Position(DirectX::XMFLOAT3(0.0f,50.0f,-75.0f));

	// ���� �ݱ�
	closesocket(login_listen);
	closesocket(keyinput_listen);
	closesocket(cube_listen);
	closesocket(playerdata_listen);
	closesocket(chat_listen);
}

// �ٴ� ť�� set
void Set_Floor_Cube_Object()
{
	nObjects = (CUBE_INIT_RING_NUMBER * 2 + 1) * (CUBE_INIT_RING_NUMBER * 2 + 1);
	ppObjects = new CObject * [CUBE_MAX_NUMBER] { NULL };

	int c_i = 0;
	CObject* pObject = NULL;

	for (int x = -CUBE_INIT_RING_NUMBER; x <= CUBE_INIT_RING_NUMBER; ++x) {
		for (int z = -CUBE_INIT_RING_NUMBER; z <= CUBE_INIT_RING_NUMBER; ++z) {
			pObject = new CObject();
			pObject->Set_Position(CUBE_WIDTH * x, 0.0f, CUBE_WIDTH * z);
			ppObjects[c_i++] = pObject;
		}
	}
}

// �ٴ� �� ��� ť�� ���� ���� �ʱ�ȭ
void Release_Floor_Cube_Object()
{
	// ť�� ������ �ʱ�ȭ
	Total_Cube.clear();

	if (nObjects)	nObjects = 0;
	if (ppObjects != NULL)
	{
		ppObjects = NULL;
	}
}

// ť�긦 �����ϱ� ���� �Լ�
void Delete_Cube(Cube_Info clientCubeInput)
{
	EnterCriticalSection(&cs_Cube);
	auto it = std::find_if(Total_Cube.begin(), Total_Cube.end(), [&](const Cube_Info& value) {
		return CompareXMFLOAT3(DirectX::XMFLOAT3(value.fPosition_x, value.fPosition_y, value.fPosition_z),
		DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z));
		});

	if (it != Total_Cube.end())
	{
		// ť�� ���� ã�� ���
		//printf("ť�� ���� ����\n");
		Total_Cube.erase(it);

		// delete to Cube Object
		CObject* pSelected_Object = NULL;
		int nSelected_Index = 0;
		for (int i = 0; i < nObjects; ++i) {
			if (CompareXMFLOAT3(ppObjects[i]->Get_Position(), DirectX::XMFLOAT3(clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z)))
			{
				pSelected_Object = ppObjects[i];
				nSelected_Index = i;
			}
		}
		if (pSelected_Object)
		{
			ppObjects[nSelected_Index] = NULL;

			if (nSelected_Index != nObjects - 1) {
				ppObjects[nSelected_Index] = ppObjects[nObjects - 1];
			}
			--nObjects;
		}

		for (int i = 0; i < socket_Cube_vector.size(); ++i)
		{
			if (socket_Cube_vector[i] != INVALID_SOCKET)
			{
				int retval = send(socket_Cube_vector[i], (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
				if (retval == SOCKET_ERROR) {
					break;
				}
			}
		}
	}
	else {
		// ť�� ���� ã�� ���� ���
		std::cout << "Delete Cube Value not found!" << std::endl;
	}
	LeaveCriticalSection(&cs_Cube);
}

// ��� ť�� �����ϴ� �ڵ�
void Request_Delete_All_Cube() 
{
	// ���� �ʿ� �����ִ� ��� ť�� ���� ������ �ٲ㼭 ����
	/*EnterCriticalSection(&cs_Cube);
	for (auto socket : socket_Cube_vector) {
		for (auto cube : Total_Cube)
		{
			cube.AddorDelete = false;
			send(socket, (char*)&cube, sizeof(cube), 0);
		}
	}
	LeaveCriticalSection(&cs_Cube);*/

	// ť�� ������ �ʱ�ȭ
}