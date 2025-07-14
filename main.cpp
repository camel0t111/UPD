#include <iostream>
#include <string>

void runClient(const std::string& nickname, int color);
int mainServer();

int main() {
    std::cout << "=====================\n";
    std::cout << " UDP Чат (Windows)\n";
    std::cout << "=====================\n";
    std::cout << "1 - Запустити сервер\n";
    std::cout << "2 - Запустити клієнт\n";
    std::cout << "Ваш вибір: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    if (choice == 1) {
        return mainServer();
    } else if (choice == 2) {
        std::string nickname;
        int color;

        std::cout << "Введіть нікнейм: ";
        std::getline(std::cin, nickname);

        std::cout << "Введіть номер кольору (1-15): ";
        std::cin >> color;
        std::cin.ignore();

        runClient(nickname, color);
    }

    return 0;
}
