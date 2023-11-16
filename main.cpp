#include "Common.h"
#include "Global.h"
#include "ServerData.h"
#include "ClientKeyInput.h"



// �Լ�
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// ���� ������ �ʱ�ȭ �κ�, ����� �� �ٽ� ȣ���Ͽ� ������ �� �ֵ��� ���� ����
void CreateClientKeyInputThread(SOCKET&);
DWORD WINAPI ProcessClient(LPVOID arg); // Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClientKeyInput(LPVOID arg);

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

	// �÷��̾� ���� üũ ����
	
	HANDLE hThread;


	// ���� ������ �ʱ�ȭ
	InitGame();

	// �÷��̾� ������ �� �ο� ���ӽ�Ű��
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(login_listen_sock);
		CreateClientKeyInputThread(KeyInput_listen_sock);
	}

	// ���� �ݱ�
	closesocket(login_listen_sock);


	// TODO: ���� ���� �ǹǷ� ���� ���� ���� ������ ���� ����
	//// ������ ���� - 
	//hThread = CreateThread(NULL, 0, ProcessClient,
	//	(LPVOID)client_sock, 0, NULL);
	//if (hThread == NULL) { closesocket(client_sock); }
	//else { CloseHandle(hThread); }

	// �÷��̾��� Ű ��ǲ ������ �޴� ������ ����

	// TODO: while������ main �����忡���� �߷�, �浹üũ �� �ð� ���� ���� ����ǵ��� ���� ����


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
}

void CreateClientKeyInputThread(SOCKET& KeyInput_listen_sock)
{
	SOCKET* client_sock = new SOCKET();
	struct sockaddr_in clientaddr;

	// accept()
	int addrlen = sizeof(clientaddr);
	*client_sock = accept(KeyInput_listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (*client_sock == INVALID_SOCKET) {
		printf("�̰� �ι븮���?\n");
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

	struct KeyInput* clientKeyInput{};
	char clientdata[5];
	int retval;
	while(1)
	{
		retval = recv(*ClientKeyInputSocket, clientdata, sizeof(clientdata), 0);
		clientKeyInput = (KeyInput*)clientdata;
		if (retval == SOCKET_ERROR) {
			break;
		}
		if(clientKeyInput->KeyDown)
		{
			printf("%d ����\n", clientKeyInput->Key);
		}else
		{
			printf("%d ��\n", clientKeyInput->Key);
		}
	}
	return 0;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	printf("aaa\n");
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[512 + 1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// ������ �ޱ�
		retval = recv(client_sock, buf, 512, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

		// ������ ������
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// ���� �ݱ�
	closesocket(client_sock);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}
