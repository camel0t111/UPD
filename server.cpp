#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int SERVER_PORT = 12345;
const int BUFFER_SIZE = 1024;
const int MAX_CLIENTS = 10;

struct Client {
    string nickname;
    sockaddr_in address;
    int color;
};

vector<Client> clients;
vector<string> chat_history;

void broadcastMessage(const string& message, SOCKET& serverSocket) {
    for (const auto& client : clients) {
        sendto(serverSocket, message.c_str(), message.size(), 0, (sockaddr*)&client.address, sizeof(client.address));
    }
}

bool clientExists(const sockaddr_in& addr) {
    for (const auto& client : clients) {
        if (client.address.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
            client.address.sin_port == addr.sin_port) {
            return true;
        }
    }
    return false;
}

void removeClient(const string& nickname) {
    clients.erase(remove_if(clients.begin(), clients.end(),
        [&](const Client& c) { return c.nickname == nickname; }), clients.end());
}

int mainServer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{}, clientAddr{};
    int clientAddrLen = sizeof(clientAddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) continue;

        string msg(buffer);

        if (msg.rfind("JOIN:", 0) == 0) {
            size_t nickEnd = msg.find(':', 5);
            string nickname = msg.substr(5, nickEnd - 5);
            int color = stoi(msg.substr(nickEnd + 1));

            if (!clientExists(clientAddr) && clients.size() < MAX_CLIENTS) {
                clients.push_back({ nickname, clientAddr, color });

                string joinMsg = nickname + " приєднався до чату.";
                broadcastMessage(joinMsg, serverSocket);

                for (const auto& past : chat_history) {
                    sendto(serverSocket, past.c_str(), past.size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                }
            }
        }
        else if (msg.rfind("LEAVE:", 0) == 0) {
            string nickname = msg.substr(6);
            removeClient(nickname);
            string leaveMsg = nickname + " вийшов із чату.";
            broadcastMessage(leaveMsg, serverSocket);
        }
        else {
            chat_history.push_back(msg);
            broadcastMessage(msg, serverSocket);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
