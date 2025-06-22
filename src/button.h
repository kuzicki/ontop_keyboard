#pragma once
#include <functional>
#include <unordered_map>
#include <utility>
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

const int BUTTON_SIZE = 40;
const int MARGIN = 5;

constexpr BYTE ExtractR(COLORREF color) noexcept { return color & 0xFF; }
constexpr BYTE ExtractG(COLORREF color) noexcept { return (color >> 8) & 0xFF; }
constexpr BYTE ExtractB(COLORREF color) noexcept {
  return (color >> 16) & 0xFF;
}

constexpr COLORREF MakeRGB(BYTE r, BYTE g, BYTE b) noexcept {
  return r | (g << 8) | (b << 16);
}

constexpr auto ComputeShades(COLORREF base, int delta = 40) noexcept {
  auto clamp = [](int value) -> BYTE {
    return static_cast<BYTE>(value < 0 ? 0 : (value > 255 ? 255 : value));
  };

  return std::make_pair(
      MakeRGB(clamp(GetRValue(base) - delta), clamp(GetGValue(base) - delta),
              clamp(GetBValue(base) - delta)),
      MakeRGB(clamp(GetRValue(base) + delta), clamp(GetGValue(base) + delta),
              clamp(GetBValue(base) + delta)));
}

struct Button {
  int id;
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

  // Constructor that computes dark/light shades from baseColor
  constexpr Button(int id, LPCWSTR text, int x, int y, int width, int height,
                   COLORREF baseColor)
      : id(id), hWnd(nullptr), text(text), x(x), y(y), width(width),
        height(height), baseColor(baseColor),
        darkColor(ComputeShades(baseColor).first),
        lightColor(ComputeShades(baseColor).second), isPressed(false) {}

  // Constructor that uses explicitly provided colors
  constexpr Button(int id, LPCWSTR text, int x, int y, int width, int height,
                   COLORREF baseColor, COLORREF darkColor, COLORREF lightColor)
      : id(id), hWnd(nullptr), text(text), x(x), y(y), width(width),
        height(height), baseColor(baseColor), darkColor(darkColor),
        lightColor(lightColor), isPressed(false) {}
};

constexpr COLORREF NUMBER_BASE = MakeRGB(240, 240, 240);
constexpr COLORREF OPERATOR_BASE = MakeRGB(200, 230, 200);
constexpr COLORREF SPECIAL_BASE = MakeRGB(180, 200, 240);

inline std::unordered_map<int, std::reference_wrapper<Button>> buttonMap;

inline Button buttons[] = {
    {IDC_BTN_7, L"7", MARGIN, MARGIN, BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_8, L"8", MARGIN + BUTTON_SIZE, MARGIN, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE},
    {IDC_BTN_9, L"9", MARGIN + 2 * BUTTON_SIZE, MARGIN, BUTTON_SIZE,
     BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_P, L"P", MARGIN + 3 * BUTTON_SIZE, MARGIN, BUTTON_SIZE,
     BUTTON_SIZE, SPECIAL_BASE},
    {IDC_BTN_4, L"4", MARGIN, MARGIN + BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE,
     NUMBER_BASE},
    {IDC_BTN_5, L"5", MARGIN + BUTTON_SIZE, MARGIN + BUTTON_SIZE, BUTTON_SIZE,
     BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_6, L"6", MARGIN + 2 * BUTTON_SIZE, MARGIN + BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_MINUS, L"-", MARGIN + 3 * BUTTON_SIZE, MARGIN + BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, OPERATOR_BASE},
    {IDC_BTN_1, L"1", MARGIN, MARGIN + 2 * BUTTON_SIZE, BUTTON_SIZE,
     BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_2, L"2", MARGIN + BUTTON_SIZE, MARGIN + 2 * BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_3, L"3", MARGIN + 2 * BUTTON_SIZE, MARGIN + 2 * BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_PLUS, L"+", MARGIN + 3 * BUTTON_SIZE, MARGIN + 2 * BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, SPECIAL_BASE},
    {IDC_BTN_0, L"0", MARGIN, MARGIN + 3 * BUTTON_SIZE, BUTTON_SIZE * 2,
     BUTTON_SIZE, NUMBER_BASE},
    {IDC_BTN_COMMA, L",", MARGIN + 2 * BUTTON_SIZE, MARGIN + 3 * BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, OPERATOR_BASE},
    {IDC_BTN_DOT, L".", MARGIN + 3 * BUTTON_SIZE, MARGIN + 3 * BUTTON_SIZE,
     BUTTON_SIZE, BUTTON_SIZE, OPERATOR_BASE},
    {IDC_BTN_COPY, L"Ctrl\nC", MARGIN + 4 * BUTTON_SIZE, MARGIN, BUTTON_SIZE,
     BUTTON_SIZE * 2, SPECIAL_BASE},
    {IDC_BTN_PASTE, L"Ctrl\nV", MARGIN + 4 * BUTTON_SIZE,
     BUTTON_SIZE * 2 + MARGIN, BUTTON_SIZE, BUTTON_SIZE * 2, SPECIAL_BASE},
    {IDC_BTN_BS, L"BS", MARGIN + 5 * BUTTON_SIZE, MARGIN, BUTTON_SIZE,
     BUTTON_SIZE * 2, SPECIAL_BASE},
    {IDC_BTN_ENTER, L"Entr", MARGIN + 5 * BUTTON_SIZE, BUTTON_SIZE * 2 + MARGIN,
     BUTTON_SIZE, BUTTON_SIZE * 2, SPECIAL_BASE}};
