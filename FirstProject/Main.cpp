#include <winsock2.h>
#include <thread>
#include <iostream>
#pragma comment(lib, "ws2_32.lib") 

// Ŭ���̾�Ʈ ���� ó�� �Լ�
void ClientHandler(SOCKET clientSock) {
    char buffer[512];
    while (true) {
        int received = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            std::cout << "Ŭ���̾�Ʈ ���� ����\n";
            break; // ���� ���� �Ǵ� ���� �߻� �� ���� ����
        }
        buffer[received] = '\0'; // ���ڿ� �� ǥ��

        std::cout << "Ŭ���̾�Ʈ �޽���: " << buffer << std::endl;

        // ����(���� �޽����� �ٽ� Ŭ���̾�Ʈ�� ����)
        send(clientSock, buffer, received, 0);
    }
    closesocket(clientSock); // Ŭ���̾�Ʈ ���� �ݱ�
}

int main() {
    WSADATA wsaData;

    // ���� �ʱ�ȭ (���� ���� 2.2 ���)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock �ʱ�ȭ ����\n";
        return 1;
    }

    // TCP ���� ���� (IPv4, ��Ʈ�� ����)
    SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock == INVALID_SOCKET) {
        std::cerr << "���� ���� ����\n";
        WSACleanup();
        return 1;
    }

    // ���� �ּ� ����ü ����
    sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;             // IPv4
    servAddr.sin_port = htons(9000);           // ��Ʈ 9000 (��Ʈ��ũ ����Ʈ ����)
    servAddr.sin_addr.s_addr = INADDR_ANY;     // ��� IP���� ���� ���

    // ���Ͽ� �ּ� �Ҵ�
    if (bind(servSock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        std::cerr << "bind ����\n";
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    // ���� ��� ���·� ��ȯ (�ִ� ��� Ŭ���̾�Ʈ �� SOMAXCONN)
    if (listen(servSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen ����\n";
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    std::cout << "���� ��� ��... ��Ʈ 9000\n";

    // ���� ������ Ŭ���̾�Ʈ ���� ��� �� ó��
    while (true) {
        SOCKET clientSock = accept(servSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "accept ����\n";
            break;
        }

        std::cout << "Ŭ���̾�Ʈ �����\n";

        // �� ������ ����, Ŭ���̾�Ʈ ó�� �Լ� ����
        // detach()�� ȣ���� �����带 �и��Ͽ� ���� ������� ���������� �����ϰ� ��
        std::thread(ClientHandler, clientSock).detach();
    }

    // ���� ���� �ݱ� �� ���� ����
    closesocket(servSock);
    WSACleanup();

    return 0;
}
