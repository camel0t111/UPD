#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int SERVER_PORT = 12345;
const int BUFFER_SIZE = 1024;

void setTextColor(int colorCode) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
}

string currentTime() {
    time_t now = time(0);
    tm* localtm = localtime(&now);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", localtm);
    return string(buffer);
}

void receiveMessages(SOCKET& sock) {
    sockaddr_in from;
    int fromLen = sizeof(from);
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&from, &fromLen);
        if (bytes > 0) {
            string msg(buffer);
            cout << msg << endl;
        }
    }
}

void runClient(const string& nickname, int color) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    string joinMsg = "JOIN:" + nickname + ":" + to_string(color);
    sendto(clientSocket, joinMsg.c_str(), joinMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    thread receiver(receiveMessages, ref(clientSocket));
    receiver.detach();

    string input;
    while (true) {
        getline(cin, input);
        if (input == "exit") break;

        string timeStr = currentTime();
        string fullMsg = "[" + timeStr + "] " + nickname + ": " + input;

        sendto(clientSocket, fullMsg.c_str(), fullMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    }

    string leaveMsg = "LEAVE:" + nickname;
    sendto(clientSocket, leaveMsg.c_str(), leaveMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    closesocket(clientSocket);
    WSACleanup();
}
