#define _WINSOCK_DEPRECATED_NO_WARNINGS     // щоб не сварився на старі ф-ції типу inet_addr
#include <winsock2.h>                       // головна бібліотека для сокетів на Windows
#include <ws2tcpip.h>                       // ще для адрес, типу inet_pton
#include <iostream>                         // для cout / cin
#include <vector>                           // щоб тримати список клієнтів
#include <string>                           // щоб працювати з std::string
#include <ctime>                            // для часу (можна використовувати, але не обов’язково)
#include <algorithm>                        // для remove_if (видалення з вектора)

#pragma comment(lib, "ws2_32.lib")          // підключаємо бібліотеку сокетів

using namespace std;

const int SERVER_PORT = 12345;              // порт, на якому буде слухати сервер
const int BUFFER_SIZE = 1024;               // буфер для отримання повідомлень
const int MAX_CLIENTS = 10;                 // максимум підключених клієнтів

// структура, яка зберігає дані про кожного клієнта
struct Client {
    string nickname;                        // нік клієнта
    sockaddr_in address;                    // адреса (IP + порт)
    int color;                              // колір (типу для виводу, але тут не юзається)
};

vector<Client> clients;                     // список всіх клієнтів, що зараз в чаті
vector<string> chat_history;                // зберігаємо історію чату

// розсилає повідомлення всім клієнтам
void broadcastMessage(const string& message, SOCKET& serverSocket) {
    for (const auto& client : clients) {
        sendto(serverSocket, message.c_str(), message.size(), 0,
               (sockaddr*)&client.address, sizeof(client.address));
    }
}

// перевіряє, чи вже є такий клієнт
bool clientExists(const sockaddr_in& addr) {
    for (const auto& client : clients) {
        if (client.address.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
            client.address.sin_port == addr.sin_port) {
            return true;
        }
    }
    return false;
}

// видаляє клієнта по ніку
void removeClient(const string& nickname) {
    clients.erase(remove_if(clients.begin(), clients.end(),
        [&](const Client& c) { return c.nickname == nickname; }), clients.end());
}

// основна функція сервера
int mainServer() {
    WSADATA wsaData;                                // структура для ініціалізації сокетів

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // запускаємо Winsock
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0); // створюємо UDP-сокет
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{}, clientAddr{};         // адреси сервера і клієнта
    int clientAddrLen = sizeof(clientAddr);

    serverAddr.sin_family = AF_INET;                // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;        // приймаємо з будь-якої адреси
    serverAddr.sin_port = htons(SERVER_PORT);       // задаємо порт

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSocket);                  // якщо не вдалось прив'язати — закриваємо сокет
        WSACleanup();
        return 1;
    }

    char buffer[BUFFER_SIZE];                       // буфер для прийому повідомлень

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);             // чистимо буфер
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0,
                                     (sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) continue; // помилка при прийомі — ігноруємо

        string msg(buffer);                         // отримуємо повідомлення як string

        if (msg.rfind("JOIN:", 0) == 0) {           // якщо клієнт підключився
            size_t nickEnd = msg.find(':', 5);      // шукаємо розділювач між ніком і кольором
            string nickname = msg.substr(5, nickEnd - 5);  // витягуємо нік
            int color = stoi(msg.substr(nickEnd + 1));     // витягуємо колір (поки не юзається)

            if (!clientExists(clientAddr) && clients.size() < MAX_CLIENTS) {
                clients.push_back({ nickname, clientAddr, color });  // додаємо клієнта

                string joinMsg = nickname + " приєднався до чату.";  // повідомлення для всіх
                broadcastMessage(joinMsg, serverSocket);             // розсилаємо його

                for (const auto& past : chat_history) {              // надсилаємо історію новачку
                    sendto(serverSocket, past.c_str(), past.size(), 0,
                           (sockaddr*)&clientAddr, sizeof(clientAddr));
                }
            }

        } else if (msg.rfind("LEAVE:", 0) == 0) {     // якщо клієнт вийшов
            string nickname = msg.substr(6);          // витягуємо його нік
            removeClient(nickname);                   // видаляємо зі списку
            string leaveMsg = nickname + " вийшов із чату.";
            broadcastMessage(leaveMsg, serverSocket); // розсилаємо про вихід
        }

        else { // звичайне повідомлення
            chat_history.push_back(msg);              // додаємо в історію
            broadcastMessage(msg, serverSocket);      // шлемо всім
        }
    }

    closesocket(serverSocket);                        // закриваємо сокет
    WSACleanup();                                     // чистимо Winsock
    return 0;
}
