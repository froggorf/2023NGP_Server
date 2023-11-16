#include "Common.h"
#include "Global.h"
#include "ServerData.h"

#define SERVERPORT 9000
#define BUFSIZE    512

// 함수
void ConnectAndAddPlayer(SOCKET&);
void InitGame();						// 게임 데이터 초기화 부분, 재시작 시 다시 호출하여 실행할 수 있도록 구현 예정
DWORD WINAPI ProcessClient(LPVOID arg); // 클라이언트와 데이터 통신
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);	// 시간 업데이트 함수
void Send_Game_Time();				// 현재 남은 시간 모든 서버로 전송
int remainingSeconds = MAX_MIN * 60; // 5분을 초로 환산

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성(플레이어 접속 체크 소켓)
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 플레이어 접속 체크 변수
	
	HANDLE hThread;


	// 플레이어 지정한 수 인원 접속시키기
	while (Current_Player_Count != MAXPLAYERCOUNT) {
		ConnectAndAddPlayer(listen_sock);
	}

	// 게임 데이터 초기화
	InitGame();

	

	// TODO: 게임 시작 되므로 게임 시작 관련 스레드 제작 예정
	// 스레드 생성 - 
	/*hThread = CreateThread(NULL, 0, ProcessClient,
		(LPVOID)client_sock, 0, NULL);*/
	// 타이머 초기화
	SetTimer(NULL, TIMER_ID, 1000, TimerProc); // 1000ms(1초)마다 타이머 호출
	std::cout << "타이머 시작 - " << remainingSeconds << std::endl;

	// 타이머 쓰레드는 ProcessClient 안에서 지웠으므로 해당 코드는 지워도 될듯
	/*if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }*/

	// TODO: while문으로 main 쓰레드에서는 중력, 충돌체크 및 시간 전송 등이 진행되도록 구현 예정
	// 메인 루프
	while (true) {

		// 시간 처리를 위한 메세지 루프
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
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

	// 클라이언트 주소 변수에 추가 및 플레이어 수 증가
	clientAddr[Current_Player_Count] = clientaddr;
	Current_Player_Count += 1;

	// 새로운 플레이어 추가
	struct Player_Info* newPlayer = new struct Player_Info();
	Player_Info.push_back(newPlayer);

	// 접속한 클라이언트 정보 출력
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

	printf("총 플레이어 수 : %d\n", Current_Player_Count);


	// 각 클라이언트 쓰레드 생성
	HANDLE hThread;

	hThread = CreateThread(NULL, 0, ProcessClient,
		(LPVOID)client_sock, 0, NULL);
	if (hThread == NULL) { closesocket(client_sock); }
	else { CloseHandle(hThread); }
}

void InitGame()
{
	Player_Info.clear();
}


DWORD WINAPI ProcessClient(LPVOID arg)
{
	printf("aaa\n");
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE + 1];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// 클라이언트 소켓 추가
	socket_vector.push_back(client_sock);

	while (1) {
		// 데이터 받기
		retval = recv(client_sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}


// 타이머 콜백 함수
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	// 시간 업데이트
	remainingSeconds--;
	// 남은 시간이 0보다 크거나 같으면 클라이언트로 시간 업데이트 및 전송
	if (remainingSeconds >= 0) {
		Send_Game_Time();
		std::cout << remainingSeconds << std::endl;
	}
	else {
		// 게임 종료 이벤트 여기에 추가
		
		// 타이머 종료
		KillTimer(hwnd, TIMER_ID);
	}
}


void Send_Game_Time() {
	// 시간 데이터 보내기
	for (auto i : socket_vector) {
		int retval = send(i, (char*)&remainingSeconds, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}
	std::cout << "Sending time to the client: " << remainingSeconds << " seconds remaining" << std::endl;
}