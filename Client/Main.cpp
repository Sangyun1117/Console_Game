#include <winsock2.h>
#include <ws2tcpip.h>  // inet_pton 사용 시 필요
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main() {
    WSADATA wsaData;

    // 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock 초기화 실패\n";
        return 1;
    }

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "소켓 생성 실패\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);  // 서버 포트

    // 문자열 IP를 네트워크 주소로 변환 (inet_pton 사용)
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "IP 주소 변환 실패\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 서버에 연결 시도
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "서버 연결 실패\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "서버에 연결되었습니다.\n";

    // 메시지 송신
    const char* msg = "Hello, Server!";
    send(sock, msg, strlen(msg), 0);

    // 서버로부터 응답 수신
    char buffer[512];
    int received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (received > 0) {
        buffer[received] = '\0';
        std::cout << "서버 응답: " << buffer << std::endl;
    }
    
    string line = "adf";
    while (true) {
        std::cin >> line;
        send(sock, line.c_str(),line.length() + 1, 0);
    }
    std::cin.get();
    // 종료
    closesocket(sock);
    WSACleanup();
    return 0;
}
