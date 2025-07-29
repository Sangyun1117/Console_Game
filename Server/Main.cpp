#include <iostream>
#include <thread>
#include <WinSock2.h> //소켓관련 헤더
#pragma comment(lib, "ws2_32.lib") //Windows에서 Winsock 2.0 기능을 제공하는 라이브러리, 없으면 링크에러남. 프로젝트 속성에서 추가 종속성에 넣어도됨

#include <string>
#include <fcntl.h>
#include <io.h>
void ClientHandler(SOCKET clientSocket) {
	char buffer[512];
	while (true) {
		int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (received <= 0) {
			std::cout << "클라이언트 연결 종료\n";
			break; // 연결 종료 또는 오류 발생 시 루프 종료
		}
		buffer[received] = '\0'; // 문자열 끝 표시

		std::cout << "클라이언트 메시지: " << buffer << std::endl;

		// 에코(받은 메시지를 다시 클라이언트에 보냄)
		send(clientSocket, buffer, received, 0);
	}
	closesocket(clientSocket); // 클라이언트 소켓 닫기
}

void ConsoleSetting() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// 1. 먼저 버퍼 크기 설정 (창 크기보다 커야 함)
	COORD bufferSize = { 120, 40 };
	SetConsoleScreenBufferSize(hOut, bufferSize);

	// 2. 콘솔 창 크기 설정
	SMALL_RECT windowSize = { 0, 0, 50, 39 }; // width = 120, height = 40
	SetConsoleWindowInfo(hOut, TRUE, &windowSize);

	std::cout << "콘솔 창 크기 조절 완료\n";
	//HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	// 배경: 파랑(1), 글자: 흰색(15)
	SetConsoleTextAttribute(hOut, (1 << 4) | 15);
	std::cout << "배경색이 바뀌었습니다!!!!!!" << std::endl;

	// 콘솔 인코딩을 UTF-8로 설정
	SetConsoleOutputCP(CP_UTF8);
	_setmode(_fileno(stdout), _O_U8TEXT); // <iostream> 기반일 땐 필요없을 수도 있음

	// 이모지 출력
	std::wcout << L"출력 테스트 😎🔥🚀 ffdd" << std::endl;
	//std::cout << u8"Hello, emoji 😎🔥🚀" << std::endl;
}
int main() {
	ConsoleSetting();

	//윈도우 소켓을 사용하기 위한 초기화
	WSADATA wsaData; //윈속API(윈도우에서 소켓 네트워크 통신을 할 수 있게 해주는 API)를 사용할 때 필요한 구조체
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {//윈속 2.2버전 요청
		std::cerr << "윈속 초기화 실패\n";
		return 1;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); //AF_INET는 IPv4 사용 선언, SOCK_STREAM은 TCP 방식 사용
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "소켓 생성 실패\n";
		WSACleanup(); //윈속 정리
		return 1;
	}

	//서버 주소 구조체 설정
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; //IPv4
	serverAddr.sin_port = htons(9000); //포트 9000번 
	serverAddr.sin_addr.s_addr = INADDR_ANY; //모든 로컬 IP에서 접속 허용

	//소켓에 주소 할당
	//sockaddr_in과 sockaddr의 첫 멤버 family가 같은 위치에 있어서 형변환 해도 된다고 함. c 소켓 API는 모든 주소 구조체를 sockaddr로 취급하는게 관례라니까 넘어가자
	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "소켓 주소 할당 실패\n";
		closesocket(serverSocket);
		WSACleanup(); //윈속 정리
		return 1;
	}

	//소켓을 연결 대기상태로 변환
	//SOMAXCONN는 서버가 동시에 처리 가능한 최대 대기 클라이언트 수
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "연결 대기 실패\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "서버 대기 중...\n";

	while (true) {
		SOCKET clientSocket = accept(serverSocket, NULL, NULL); //클라이언트 접속 전까지 대기, 접속되면 클라이언트소켓에 저장
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "클라이언트 접속 실패\n";
			break;
		}

		std::cout << "클라이언트 연결됨\n";

		// 연결된 클라이언트를 다른 스레드에서 처리
		// 이 스레드는 ClientHandler를 실행하고 인자로 clientSocket 전달함
	    // detach()를 호출해 스레드를 분리 메인 스레드와 독립적으로 동작하게 함
		std::thread(ClientHandler, clientSocket).detach();
	}

	// 서버 소켓 닫기, 윈속 종료
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
