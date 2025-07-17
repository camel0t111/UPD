#include <iostream>          // для cin, cout — базове введення/виведення
#include <string>            // щоб юзати string (рядки), getline і т.д.

// оголошуємо функції, які реалізовані в інших файлах
void runClient(const std::string& nickname, int color);  // клієнтська частина
int mainServer();                                        // серверна частина

int main() {
    std::cout << "=====================\n";              // просто рамка для краси
    std::cout << " UDP Чат (Windows)\n";                 // заголовок
    std::cout << "=====================\n";
    std::cout << "1 - Запустити сервер\n";               // вибір режиму
    std::cout << "2 - Запустити клієнт\n";
    std::cout << "Ваш вибір: ";                          // підказка для юзера

    int choice;                                          // зберігаємо вибір користувача
    std::cin >> choice;                                  // зчитуємо число
    std::cin.ignore();                                   // чистимо буфер, бо потім буде getline

    if (choice == 1) {
        return mainServer();                             // запускаємо сервер
    } else if (choice == 2) {
        std::string nickname;                            // нік юзера
        int color;                                       // номер кольору (може бути для прикрас)

        std::cout << "Введіть нікнейм: ";                // просимо нік
        std::getline(std::cin, nickname);                // читаємо нік з пробілами

        std::cout << "Введіть номер кольору (1-15): ";   // просимо вибрати колір
        std::cin >> color;                               // читаємо колір
        std::cin.ignore();                               // знову чистимо буфер

        runClient(nickname, color);                      // запускаємо клієнта з нікнеймом і кольором
    }

    return 0;                                            // завершення програми
}
