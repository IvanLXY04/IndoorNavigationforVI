#include "ui_vi.h"
#include "audio_feedback.h"
#include <iostream>

void showMainMenu() {
    std::cout << "\n===== Indoor Navigation System =====\n";
    std::cout << "1. Start Navigation (Scan for QR code)\n";
    std::cout << "2. Where am I? (Rescan for current location)\n";
    std::cout << "3. Set Destination by Voice\n";
    std::cout << "4. Help (Listen to instructions)\n";
    std::cout << "5. Exit\n";
    std::cout << "======================================\n";
    Speak("Main menu. Option 1: Start Navigation. Option 2: Where am I. Option 3: Set Destination by Voice. Option 4: Help. Option 5: Exit.");
}

int getUserChoice() {
    int option = 0;
    std::cout << "Enter your choice (1-5): "; // Reverted to 5 options
    std::cin >> option;
    if (std::cin.fail() || option < 1 || option > 5) { // Reverted to 5 options
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        Speak("Invalid input. Please enter a number between 1 and 5.");
        return -1;
    }
    return option;
}