#pragma once
#include "color.h"
#include <functional>
#include <unordered_map>
#include <windows.h>
// #include <windef.h>

// Button IDs
#define IDC_BTN_7 100
#define IDC_BTN_8 101
#define IDC_BTN_9 102
#define IDC_BTN_MULT 103
#define IDC_BTN_BS 104
#define IDC_BTN_4 105
#define IDC_BTN_5 106
#define IDC_BTN_6 107
#define IDC_BTN_MINUS 108
#define IDC_BTN_1 109
#define IDC_BTN_2 110
#define IDC_BTN_3 111
#define IDC_BTN_PLUS 112
#define IDC_BTN_0 113
#define IDC_BTN_DOT 114
#define IDC_BTN_DIV 115
#define IDC_BTN_ENTER 116
#define IDC_BTN_COMMA 117
#define IDC_BTN_P 118
#define IDC_BTN_COPY 119
#define IDC_BTN_PASTE 120
#define IDC_BTN_TAB 121
#define IDC_BTN_FORWARDSLASH 122

#define WM_BACKSPACE_DOWN (WM_APP + 1)
#define WM_BACKSPACE_UP (WM_APP + 2)

const int LABEL_OFFSET = 25;
const int BUTTON_SIZE = 40;
const int MARGIN = 5;

void CreateButtons(HWND);
void drawButton(LPDRAWITEMSTRUCT pdis);
// HBITMAP LoadResizeAsBitmap(const wchar_t *filename, int width, int height);
LRESULT CALLBACK ButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, UINT_PTR uIdSubclass,
                                    DWORD_PTR dwRefData);

struct Button {
  UINT_PTR id;
  HWND hWnd;
  LPCWSTR text;
  int x;
  int y;
  int width;
  int height;
  COLORREF baseColor;
  COLORREF darkColor;
  COLORREF lightColor;
  bool isPressed;
  bool isLightText;

  constexpr Button(int id, LPCWSTR text, int x, int y, int width, int height,
                   COLORREF baseColor, bool isLightText)
      : id(id), hWnd(nullptr), text(text), x(x), y(y), width(width),
        height(height), baseColor(baseColor),
        darkColor(ComputeShades(baseColor).first),
        lightColor(ComputeShades(baseColor).second), isPressed(false),
        isLightText(isLightText) {}

  constexpr Button(int id, LPCWSTR text, int x, int y, int width, int height,
                   COLORREF baseColor, COLORREF darkColor, COLORREF lightColor,
                   bool isLightText)
      : id(id), hWnd(nullptr), text(text), x(x), y(y), width(width),
        height(height), baseColor(baseColor), darkColor(darkColor),
        lightColor(lightColor), isPressed(false), isLightText(isLightText) {}
};

constexpr COLORREF NUMBER_BASE = MakeRGB(36, 36, 40);
constexpr COLORREF OPERATOR_BASE = MakeRGB(56, 56, 60);
constexpr COLORREF SPECIAL_BASE = MakeRGB(131, 179, 244);
constexpr COLORREF BS_BASE = MakeRGB(235, 102, 19);
constexpr COLORREF ENTR_BASE = MakeRGB(52, 235, 103);

inline std::unordered_map<int, std::reference_wrapper<Button>> buttonMap;

inline Button buttons[] = {
    {IDC_BTN_FORWARDSLASH, L"/", MARGIN, MARGIN + LABEL_OFFSET, BUTTON_SIZE,
     BUTTON_SIZE, SPECIAL_BASE, false},
    {IDC_BTN_TAB, L"TAB", MARGIN, MARGIN + LABEL_OFFSET + BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE * 4, SPECIAL_BASE, false},
    {IDC_BTN_7, L"7", MARGIN + BUTTON_SIZE, MARGIN + BUTTON_SIZE + LABEL_OFFSET,
     BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE, true},
    {IDC_BTN_8, L"8", MARGIN + BUTTON_SIZE * 2,
     MARGIN + BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE,
     true},
    {IDC_BTN_9, L"9", MARGIN + BUTTON_SIZE * 3,
     MARGIN + BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE,
     true},
    {IDC_BTN_P, L"P", MARGIN + BUTTON_SIZE * 4,
     MARGIN + BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     OPERATOR_BASE, true},
    {IDC_BTN_4, L"4", MARGIN + BUTTON_SIZE,
     MARGIN + 2 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_5, L"5", MARGIN + BUTTON_SIZE * 2,
     MARGIN + 2 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_6, L"6", MARGIN + BUTTON_SIZE * 3,
     MARGIN + 2 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_MINUS, L"-", MARGIN + BUTTON_SIZE * 4,
     MARGIN + 2 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     OPERATOR_BASE, true},
    {IDC_BTN_1, L"1", MARGIN + BUTTON_SIZE,
     MARGIN + 3 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_2, L"2", MARGIN + BUTTON_SIZE * 2,
     MARGIN + 3 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_3, L"3", MARGIN + BUTTON_SIZE * 3,
     MARGIN + 3 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_PLUS, L"+", MARGIN + BUTTON_SIZE * 4,
     MARGIN + 3 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     OPERATOR_BASE, true},
    {IDC_BTN_0, L"0", MARGIN + BUTTON_SIZE,
     MARGIN + 4 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE * 2, BUTTON_SIZE,
     NUMBER_BASE, true},
    {IDC_BTN_COMMA, L",", MARGIN + BUTTON_SIZE * 3,
     MARGIN + 4 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     OPERATOR_BASE, true},
    {IDC_BTN_DOT, L".", MARGIN + BUTTON_SIZE * 4,
     MARGIN + 4 * BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
     OPERATOR_BASE, true},
    {IDC_BTN_COPY, L"Ctrl\nC", MARGIN + BUTTON_SIZE, MARGIN + LABEL_OFFSET,
     BUTTON_SIZE * 2, BUTTON_SIZE, SPECIAL_BASE, false},
    {IDC_BTN_PASTE, L"Ctrl\nV", MARGIN + BUTTON_SIZE * 3, MARGIN + LABEL_OFFSET,
     BUTTON_SIZE * 2, BUTTON_SIZE, SPECIAL_BASE, false},
    {IDC_BTN_BS, L"BS", MARGIN + BUTTON_SIZE * 5, MARGIN + LABEL_OFFSET,
     BUTTON_SIZE, BUTTON_SIZE * 3, BS_BASE, false},
    {IDC_BTN_ENTER, L"Entr", MARGIN + BUTTON_SIZE * 5,
     BUTTON_SIZE * 3 + MARGIN + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE * 2,
     ENTR_BASE, false}};
