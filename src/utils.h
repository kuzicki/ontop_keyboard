#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <consoleapi2.h>

template <typename T> inline constexpr T zero_init() { return T{}; }


#ifdef DEBUG
inline void spawn_debug_console() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
    SetConsoleTitle(L"Debug Console");
}
#else
inline void spawn_debug_console() {}
#endif
