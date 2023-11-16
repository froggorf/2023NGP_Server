#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"



// �Լ�
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// ���� ������ �ʱ�ȭ �κ�, ����� �� �ٽ� ȣ���Ͽ� ������ �� �ֵ��� ���� ����
void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime); 	// �ð� ������Ʈ �Լ�
void Send_Game_Time();
void SendPlayerLocationToAllClient();
int remainingSeconds = MAX_MIN * 60; // 5���� �ʷ� ȯ��

DWORD WINAPI ProcessClientKeyInput(LPVOID arg);

void CreateCubeThread(SOCKET& Cube_listen_sock);
DWORD WINAPI EchoClientRequestCube(LPVOID arg);

// ���� ����
WSADATA wsa;


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


	//-----------------
	// ���� ������ �ʱ�ȭ
	InitGame();


	// �÷��̾� ������ �� �ο� ���ӽ�Ű��
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen_sock);
		CreateClientKeyInputThread(KeyInput_listen_sock);
		CreateCubeThread(Cube_listen_sock);
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
	// Ŭ���̾�Ʈ �ּ� ������ �߰� �� �÷��̾� �� ����
	clientAddr[Current_Player_Count] = clientaddr;
	Current_Player_Count += 1;

	// ���ο� �÷��̾� �߰�
	struct Player_Info* newPlayer = new struct Player_Info();
	Player_Info.push_back(newPlayer);

	// ������ Ŭ���̾�Ʈ ���� ���
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));

	printf("�� �÷��̾� �� : %d\n", Current_Player_Count);


}

void InitGame()
{
	Player_Info.clear();
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
		printf("%d �÷��̾ %d ��ư ", clientKeyInput.PlayerNumber, clientKeyInput.Key);
		if(clientKeyInput.KeyDown)
		{
			printf("����\n");
		}else
		{
			printf("��\n");
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
	HANDLE hThread = CreateThread(NULL, 0, EchoClientRequestCube,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}


DWORD WINAPI EchoClientRequestCube(LPVOID arg)
{
	printf("Ű ��ǲ ������ ����\n");
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
		printf("Cube Position - %.2f, %.2f, %.2f", clientCubeInput.fPosition_x, clientCubeInput.fPosition_y, clientCubeInput.fPosition_z);
		printf("Cube Color - %.2f, %.2f, %.2f", clientCubeInput.fColor_r, clientCubeInput.fColor_g, clientCubeInput.fColor_b);

		// ���⼭ ť��� ��� �浹üũ
		//..

		// ť�� send to every cube
		for (auto i : socket_Cube_vector) {
			int retval = send(i, (char*)&clientCubeInput, sizeof(clientCubeInput), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
			std::cout << "Sending cube_info to the client" << std::endl;
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