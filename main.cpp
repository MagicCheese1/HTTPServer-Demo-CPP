#include <iostream>
#include <winsock.h>
#include <ws2tcpip.h>
#include <string>
#include<fstream>
#include<sstream>

int main(int argc, char * argv[]) {
    const char *port = "8080";
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = INVALID_SOCKET;

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        SOCKET ClientSocket = INVALID_SOCKET;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        char recvbuf[512];
        int iSendResult;
        int recvbuflen = 512;
//        const char *sendbuf = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 12\n\nHello World!";
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n\n", iResult);
            printf(recvbuf);
            printf("\n\n");
            std::string msg = std::string(recvbuf);
            int firstSpace = msg.find("/");
            std::string execpath = std::string(argv[0]);
            std::string requestpath = execpath.substr(0, execpath.find_last_of('\\')).append("\\").append(
                    msg.substr(firstSpace +1, msg.find(" ", firstSpace + 1)-firstSpace));
            std::cout << requestpath << "\n";
            std::ifstream f(requestpath); //taking file as inputstream
            //"L:\\HTTPServer-Demo-CPP\\index.html");//
            std::string str;
            if (f) {
                std::ostringstream ss;
                ss << f.rdbuf(); // reading data
                str = ss.str();
            }
            std::string sendbufpp = std::string("HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ").append(
                    std::to_string(str.length())).append("\n\n").append(str);
            const char *sendbuf = sendbufpp.c_str();
            printf("sendbuf: %s", sendbuf);
            iSendResult = send(ClientSocket, sendbuf, sendbufpp.length(), 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        } else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        closesocket(ClientSocket);
    }
    WSACleanup();

    return 0;
}


