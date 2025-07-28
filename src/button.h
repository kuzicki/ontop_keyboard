#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

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
#define IDC_BTN_INPUT_OPTIONS 123

#define WM_BACKSPACE_DOWN (WM_APP + 1)
#define WM_BACKSPACE_UP (WM_APP + 2)
#define WM_OPEN_INPUT_OPTIONS (WM_APP + 3)

#define ID_MENU_FIRST 200
#define ID_MENU_SECOND 201

const int LABEL_OFFSET = 25;
const int BUTTON_SIZE = 40;
const int MARGIN = 5;

struct ButtonParams;
class Button;

extern std::vector<Button> g_buttons;

void createButtons(HWND parent);
std::vector<Button> createAllButtons(HWND parent,
                      const std::initializer_list<ButtonParams> &params);

class Button {
public:
  Button(HWND parent, int id, LPCWSTR text, int x, int y, int w, int h,
         COLORREF baseColor, bool isLightText);
  ~Button();

  Button(Button &&other) noexcept;
  Button &operator=(Button &&other) noexcept;

  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;

  void Draw(LPDRAWITEMSTRUCT pdis) const;
  void HandlePress(bool pressed);
  void Resize(int clientWidth, int clientHeight);

private:
  LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  void drawBorder(HDC hdc, const RECT &rect) const;
  void drawText(HDC hdc, const RECT &rect) const;
  void changePressState(bool pressState);
  COLORREF adjustColor(COLORREF color, int delta) const;

  static LRESULT CALLBACK SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam, UINT_PTR uIdSubclass,
                                       DWORD_PTR dwRefData);

private:
  HWND m_hWnd;
  UINT_PTR m_id;
  LPCWSTR m_text;
  int m_x, m_y;
  int m_w, m_h;
  COLORREF m_baseColor;
  COLORREF m_darkColor;
  COLORREF m_lightColor;
  bool m_isLightText;
  bool m_isPressed;
};


struct ButtonParams {
  int id;
  LPCWSTR text;
  int x;
  int y;
  int w;
  int h;
  COLORREF color;
  bool light;
};


