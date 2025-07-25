#include <winsock2.h>
#include <thread>
#include <iostream>
#pragma comment(lib, "ws2_32.lib") 

// 클라이언트 소켓 처리 함수
void ClientHandler(SOCKET clientSock) {
    char buffer[512];
    while (true) {
        int received = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            std::cout << "클라이언트 연결 종료\n";
            break; // 연결 종료 또는 오류 발생 시 루프 종료
        }
        buffer[received] = '\0'; // 문자열 끝 표시

        std::cout << "클라이언트 메시지: " << buffer << std::endl;

        // 에코(받은 메시지를 다시 클라이언트에 보냄)
        send(clientSock, buffer, received, 0);
    }
    closesocket(clientSock); // 클라이언트 소켓 닫기
}

int main() {
    WSADATA wsaData;

    // 윈속 초기화 (윈속 버전 2.2 사용)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock 초기화 실패\n";
        return 1;
    }

    // TCP 소켓 생성 (IPv4, 스트림 소켓)
    SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock == INVALID_SOCKET) {
        std::cerr << "소켓 생성 실패\n";
        WSACleanup();
        return 1;
    }

    // 서버 주소 구조체 설정
    sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;             // IPv4
    servAddr.sin_port = htons(9000);           // 포트 9000 (네트워크 바이트 순서)
    servAddr.sin_addr.s_addr = INADDR_ANY;     // 모든 IP에서 접속 허용

    // 소켓에 주소 할당
    if (bind(servSock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        std::cerr << "bind 실패\n";
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    // 연결 대기 상태로 전환 (최대 대기 클라이언트 수 SOMAXCONN)
    if (listen(servSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen 실패\n";
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    std::cout << "서버 대기 중... 포트 9000\n";

    // 무한 루프로 클라이언트 접속 대기 및 처리
    while (true) {
        SOCKET clientSock = accept(servSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "accept 실패\n";
            break;
        }

        std::cout << "클라이언트 연결됨\n";

        // 새 스레드 생성, 클라이언트 처리 함수 실행
        // detach()를 호출해 스레드를 분리하여 메인 스레드와 독립적으로 동작하게 함
        std::thread(ClientHandler, clientSock).detach();
    }

    // 서버 소켓 닫기 및 윈속 종료
    closesocket(servSock);
    WSACleanup();

    return 0;
}
