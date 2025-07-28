#pragma once
#include <windows.h>

void sendKeyPress(WPARAM wParam);
void sendBackspacePress();
void sendString(const wchar_t* output);
void setClipboardText(const wchar_t* text);
