#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <functional>
#include <thread>
#include <windows.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

extern SOCKET g_listenSocket;
extern bool g_serverRunning;
extern std::thread g_serverThread;

void serverThread(std::function<void()> action);

void pickFolder();
void uploadNextFile();
