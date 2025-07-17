#define _WINSOCK_DEPRECATED_NO_WARNINGS          // щоб дозволити старі ф-ції типу inet_addr
#include <winsock2.h>                            // основна бібліотека для сокетів (Windows)
#include <windows.h>                             // для кольорів у консолі (SetConsoleTextAttribute)
#include <iostream>                              // для cin, cout
#include <string>                                // string, getline і т.д.
#include <thread>                                // для багатопотоковості (слухати сервер паралельно)
#include <ctime>                                 // щоб отримати поточний час

#pragma comment(lib, "ws2_32.lib")               // лінкуємо бібліотеку сокетів (обов'язково)

using namespace std;

const int SERVER_PORT = 12345;                   // порт, на якому слухає сервер (має співпадати)
const int BUFFER_SIZE = 1024;                    // буфер для прийому повідомлень

// змінюємо колір тексту в консолі (Windows)
void setTextColor(int colorCode) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
}

// повертає поточний час у форматі "год:хв:сек"
string currentTime() {
    time_t now = time(0);                        // беремо поточний час
    tm* localtm = localtime(&now);               // переводимо в локальний формат
    char buffer[9];                              // для HH:MM:SS
    strftime(buffer, sizeof(buffer), "%H:%M:%S", localtm);  // форматування
    return string(buffer);                       // повертаємо строку
}

// окремий потік для прийому повідомлень з сервера
void receiveMessages(SOCKET& sock) {
    sockaddr_in from;                            // адреса відправника
    int fromLen = sizeof(from);
    char buffer[BUFFER_SIZE];                    // куди будемо приймати

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);          // чистимо буфер
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE, 0, (sockaddr*)&from, &fromLen);  // приймаємо

        if (bytes > 0) {
            string msg(buffer);                  // переводимо в string
            cout << msg << endl;                 // просто виводимо в консоль
        }
    }
}

// головна функція клієнта (після вводу ніку і кольору)
void runClient(const string& nickname, int color) {
    WSADATA wsa;                                 // структура для ініціалізації Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);            // стартуємо сокети

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);  // створюємо UDP-сокет

    sockaddr_in serverAddr{};                    // адреса сервера
    serverAddr.sin_family = AF_INET;             // IPv4
    serverAddr.sin_port = htons(SERVER_PORT);    // порт (в мережевому порядку)
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP локальний (127.0.0.1)

    // формуємо JOIN повідомлення, типу: "JOIN:Dan:3"
    string joinMsg = "JOIN:" + nickname + ":" + to_string(color);
    sendto(clientSocket, joinMsg.c_str(), joinMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    thread receiver(receiveMessages, ref(clientSocket));  // створюємо потік прийому
    receiver.detach();                          // від’єднуємо — він працює сам по собі

    string input;                                // сюди вводимо повідомлення юзера
    while (true) {
        getline(cin, input);                     // читаємо з клави
        if (input == "exit") break;              // якщо ввів "exit", то виходимо

        string timeStr = currentTime();          // поточний час
        string fullMsg = "[" + timeStr + "] " + nickname + ": " + input;  // формат повідомлення

        // відправляємо на сервер
        sendto(clientSocket, fullMsg.c_str(), fullMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    }

    // відправляємо LEAVE повідомлення, коли юзер виходить
    string leaveMsg = "LEAVE:" + nickname;
    sendto(clientSocket, leaveMsg.c_str(), leaveMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    closesocket(clientSocket);                   // закриваємо сокет
    WSACleanup();                                // чистимо після себе
}
