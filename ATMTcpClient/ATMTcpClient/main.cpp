//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <thread>
//#include <vector>
//#include <iomanip>
//#include <sstream>
//#include <chrono>
//#include <atomic>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//std::atomic<bool> running(true);
//
//void receiveThread(SOCKET sock, const std::string& hexFile, const std::string& asciiFile) {
//    std::ofstream hexOut(hexFile, std::ios::app);
//    std::ofstream asciiOut(asciiFile, std::ios::app);
//    if (!hexOut || !asciiOut) {
//        std::cerr << "Failed to open log files." << std::endl;
//        return;
//    }
//
//    char buffer[1024];
//    while (running) {
//        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
//        if (bytesReceived > 0) {
//            // Log to hex file
//            hexOut << std::hex << std::setfill('0');
//            for (int i = 0; i < bytesReceived; ++i) {
//                hexOut << std::setw(2) << (static_cast<unsigned char>(buffer[i]) & 0xFF) << " ";
//            }
//            hexOut << std::dec << std::endl;
//
//            // Log to ascii file
//            for (int i = 0; i < bytesReceived; ++i) {
//                char c = buffer[i];
//                asciiOut << (std::isprint(static_cast<unsigned char>(c)) ? c : '.');
//            }
//            asciiOut << std::endl;
//        }
//        else if (bytesReceived == 0) {
//            std::cout << "Connection closed by server." << std::endl;
//            running = false;
//            break;
//        }
//        else {
//            std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
//            running = false;
//            break;
//        }
//    }
//    hexOut.close();
//    asciiOut.close();
//}
//
//std::vector<char> hexStringToBytes(const std::string& hex) {
//    std::vector<char> bytes;
//    std::stringstream ss(hex);
//    std::string byteStr;
//    while (ss >> byteStr) {
//        if (byteStr.length() == 2) {
//            char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
//            bytes.push_back(byte);
//        }
//    }
//    return bytes;
//}
//
//int main() {
//    // Read configuration
//    std::string ip, portStr;
//    std::ifstream config("cfg.txt");
//    if (!config) {
//        std::cerr << "Failed to open cfg.txt : " << std::endl;
//        // Additional debugging: Check current working directory
//        char cwd[1024];
//        if (GetCurrentDirectoryA(sizeof(cwd), cwd)) {
//            std::cerr << "Current working directory: " << cwd << std::endl;
//        }
//        else {
//            std::cerr << "Failed to get current working directory: " << WSAGetLastError() << std::endl;
//        }
//        return 1;
//    }
//    std::string line;
//    while (std::getline(config, line)) {
//        std::cerr << line;
//        if (line.find("IP=") == 0) {
//            ip = line.substr(3);
//        }
//        else if (line.find("Port=") == 0) {
//            portStr = line.substr(5);
//        }
//    }
//    config.close();
//    if (ip.empty() || portStr.empty()) {
//        std::cerr << "Missing IP or Port in config.txt" << std::endl;
//        return 1;
//    }
//    int port = std::stoi(portStr);
//
//    // Initialize Winsock
//    WSADATA wsaData;
//    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (result != 0) {
//        std::cerr << "WSAStartup failed: " << result << std::endl;
//        return 1;
//    }
//
//    // Create socket
//    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (connectSocket == INVALID_SOCKET) {
//        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // Set keep-alive
//    BOOL keepAlive = TRUE;
//    setsockopt(connectSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(BOOL));
//
//    // Resolve address
//    struct addrinfo* resultAddr = nullptr;
//    struct addrinfo hints;
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    result = getaddrinfo(ip.c_str(), portStr.c_str(), &hints, &resultAddr);
//    if (result != 0) {
//        std::cerr << "getaddrinfo failed: " << result << std::endl;
//        closesocket(connectSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    // Connect
//    result = connect(connectSocket, resultAddr->ai_addr, (int)resultAddr->ai_addrlen);
//    freeaddrinfo(resultAddr);
//    if (result == SOCKET_ERROR) {
//        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
//        closesocket(connectSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    std::cout << "Connected to " << ip << ":" << port << std::endl;
//
//    // Log files
//    std::string hexFile = "recv_hex.txt";
//    std::string asciiFile = "recv_ascii.txt";
//
//    // Start receive thread
//    std::thread recvThread(receiveThread, connectSocket, hexFile, asciiFile);
//
//    // Main loop for sending
//    std::string input;
//    while (running) {
//        std::cout << "Enter hex bytes to send (e.g., 'DE AD BE EF') or 'disconnect' to quit: ";
//        std::getline(std::cin, input);
//        if (input == "disconnect") {
//            running = false;
//            break;
//        }
//        auto bytes = hexStringToBytes(input);
//        if (!bytes.empty()) {
//            int bytesSent = send(connectSocket, bytes.data(), bytes.size(), 0);
//            if (bytesSent == SOCKET_ERROR) {
//                std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
//                running = false;
//            }
//            else {
//                std::cout << "Sent " << bytesSent << " bytes." << std::endl;
//            }
//        }
//    }
//
//    // Cleanup
//    shutdown(connectSocket, SD_SEND);
//    recvThread.join();
//    closesocket(connectSocket);
//    WSACleanup();
//
//    return 0;
//}
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <atomic>
#pragma comment(lib, "Ws2_32.lib")

std::atomic<bool> running(true);

void receiveThread(SOCKET sock, const std::string& hexFile, const std::string& asciiFile) {
    std::ofstream hexOut(hexFile, std::ios::app);
    std::ofstream asciiOut(asciiFile, std::ios::app);
    if (!hexOut || !asciiOut) {
        std::cerr << "Failed to open log files." << std::endl;
        return;
    }
    char buffer[1024];
    while (running) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            hexOut << std::hex << std::setfill('0');
            for (int i = 0; i < bytesReceived; ++i) {
                hexOut << std::setw(2) << (static_cast<unsigned char>(buffer[i]) & 0xFF) << " ";
            }
            hexOut << std::dec << std::endl;
            for (int i = 0; i < bytesReceived; ++i) {
                char c = buffer[i];
                asciiOut << (std::isprint(static_cast<unsigned char>(c)) ? c : '.');
            }
            asciiOut << std::endl;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by server." << std::endl;
            running = false;
            break;
        }
        else {
            std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
            running = false;
            break;
        }
    }
    hexOut.close();
    asciiOut.close();
}

std::vector<char> hexStringToBytes(const std::string& hex) {
    std::vector<char> bytes;
    std::stringstream ss(hex);
    std::string byteStr;
    while (ss >> byteStr) {
        if (byteStr.length() == 2) {
            try {
                char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
                bytes.push_back(byte);
            }
            catch (const std::exception& e) {
                std::cerr << "Invalid hex byte: " << byteStr << std::endl;
            }
        }
        else {
            std::cerr << "Invalid hex format: " << byteStr << std::endl;
        }
    }
    return bytes;
}

// Function to send all data, handling partial sends
int sendAll(SOCKET sock, const char* data, int length) {
    int totalSent = 0;
    while (totalSent < length) {
        int bytesSent = send(sock, data + totalSent, length - totalSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            return -1;
        }
        totalSent += bytesSent;
    }
    return totalSent;
}

int main() {
    // Read configuration
    std::string ip, portStr;
    std::ifstream config("cfg.txt");
    if (!config) {
        std::cerr << "Failed to open cfg.txt" << std::endl;
        char cwd[1024];
        if (GetCurrentDirectoryA(sizeof(cwd), cwd)) {
            std::cerr << "Current working directory: " << cwd << std::endl;
        }
        else {
            std::cerr << "Failed to get current working directory: " << WSAGetLastError() << std::endl;
        }
        return 1;
    }
    std::string line;
    while (std::getline(config, line)) {
        if (line.find("IP=") == 0) {
            ip = line.substr(3);
        }
        else if (line.find("Port=") == 0) {
            portStr = line.substr(5);
        }
    }
    config.close();
    if (ip.empty() || portStr.empty()) {
        std::cerr << "Missing IP or Port in cfg.txt" << std::endl;
        return 1;
    }
    int port = std::stoi(portStr);

    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Create socket
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Set keep-alive
    BOOL keepAlive = TRUE;
    setsockopt(connectSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(BOOL));

    // Increase send buffer size (optional, adjust as needed)
    int sndBufSize = 65536; // 64 KB
    setsockopt(connectSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sndBufSize, sizeof(sndBufSize));

    // Resolve address
    struct addrinfo* resultAddr = nullptr;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    result = getaddrinfo(ip.c_str(), portStr.c_str(), &hints, &resultAddr);
    if (result != 0) {
        std::cerr << "getaddrinfo failed: " << result << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Connect
    result = connect(connectSocket, resultAddr->ai_addr, (int)resultAddr->ai_addrlen);
    freeaddrinfo(resultAddr);
    if (result == SOCKET_ERROR) {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Connected to " << ip << ":" << port << std::endl;

    // Log files
    std::string hexFile = "recv_hex.txt";
    std::string asciiFile = "recv_ascii.txt";

    // Start receive thread
    std::thread recvThread(receiveThread, connectSocket, hexFile, asciiFile);

    // Main loop for sending
    std::string input;
    while (running) {
        std::cout << "Enter hex bytes to send (e.g., 'DE AD BE EF') or 'disconnect' to quit: ";
        std::getline(std::cin, input);
        if (input == "disconnect") {
            running = false;
            break;
        }
        auto bytes = hexStringToBytes(input);
        if (!bytes.empty()) {
            int bytesSent = sendAll(connectSocket, bytes.data(), bytes.size());
            if (bytesSent == -1) {
                running = false;
            }
            else {
                std::cout << "Sent " << bytesSent << " bytes." << std::endl;
            }
        }
        else {
            std::cout << "No valid bytes to send." << std::endl;
        }
    }

    // Cleanup
    shutdown(connectSocket, SD_SEND);
    recvThread.join();
    closesocket(connectSocket);
    WSACleanup();
    return 0;
}