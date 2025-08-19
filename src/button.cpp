#include "button.h"
#include "color.h"
#include "dimensions.h"
#include <CommCtrl.h>
#include <errhandlingapi.h>
#include <format>
#include <libloaderapi.h>
#include <stdexcept>
#include <stdlib.h>
#include <utilapiset.h>
#include <winuser.h>

Button::Button(HWND parent, int id, LPCWSTR text, int x, int y, int w, int h,
               COLORREF baseColor, bool isLightText)
    : m_id(id), m_text(text), m_x(x), m_y(y), m_w(w), m_h(h),
      m_baseColor(baseColor), m_isLightText(isLightText), m_isPressed(false) {
  m_hWnd =
      CreateWindowW(L"BUTTON", m_text,
                    BS_OWNERDRAW | WS_TABSTOP | WS_VISIBLE | WS_CHILD |
                        BS_PUSHBUTTON | BS_MULTILINE,
                    m_x, m_y, m_w, m_h, parent, reinterpret_cast<HMENU>(m_id),
                    GetModuleHandleW(nullptr), nullptr);

  auto [dark, light] = ComputeShades(baseColor, 40);
  m_darkColor = dark, m_lightColor = light;

  if (!m_hWnd) {
    DWORD err = GetLastError();
    throw std::runtime_error(
        std::format("Failed to create button window. Erorr code: {}", err));
  }

  SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  SetWindowSubclass(m_hWnd, &Button::SubClassProc, 0, 0);
}

void Button::changePressState(bool pressState) {
  m_isPressed = pressState;
  InvalidateRect(m_hWnd, NULL, TRUE);
  UpdateWindow(m_hWnd);
}

LRESULT CALLBACK Button::SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, UINT_PTR uIdSubclass,
                                      [[maybe_unused]] DWORD_PTR dwRefData) {
  auto btnId = GetDlgCtrlID(hWnd);
  Button *pThis =
      reinterpret_cast<Button *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
  if (pThis) {
    switch (uMsg) {
    case WM_LBUTTONDBLCLK: {
    case WM_LBUTTONDOWN:
      pThis->changePressState(true);

      if (btnId == IDC_BTN_BS) {
        PostMessageW(GetParent(hWnd), WM_BACKSPACE_DOWN, 0, 0);
      }
      break;
    }
    case WM_LBUTTONUP: {
    case WM_MOUSELEAVE:
      pThis->changePressState(false);

      if (btnId == IDC_BTN_BS) {
        PostMessageW(GetParent(hWnd), WM_BACKSPACE_UP, 0, 0);
      } else if (btnId == IDC_BTN_INPUT_OPTIONS) {
        PostMessageW(GetParent(hWnd), WM_OPEN_INPUT_OPTIONS, 0, 0);
      }
      break;
    }
    case WM_NCDESTROY:
      RemoveWindowSubclass(hWnd, &Button::SubClassProc, uIdSubclass);
      break;
    }
  }
  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void Button::Draw(LPDRAWITEMSTRUCT pdis) const {
  RECT fillRect = pdis->rcItem;
  InflateRect(&fillRect, -2, -2);

  HBRUSH bgBrush = CreateSolidBrush(m_baseColor);
  FillRect(pdis->hDC, &fillRect, bgBrush);
  DeleteObject(bgBrush);

  drawBorder(pdis->hDC, pdis->rcItem);
  drawText(pdis->hDC, pdis->rcItem);
  if (m_isPressed) {
    InflateRect(&pdis->rcItem, -1, -1);
    DrawEdge(pdis->hDC, &pdis->rcItem, EDGE_SUNKEN, BF_RECT);
  }
}

void Button::Resize(int clientWidth, int clientHeight) {

  int newX = (m_x * clientWidth) / ui::MIN_WINDOW_WIDTH;
  int newY = (m_y * clientHeight) / ui::MIN_WINDOW_HEIGHT;
  int newW = (m_w * clientWidth) / ui::MIN_WINDOW_WIDTH;
  int newH = (m_h * clientHeight) / ui::MIN_WINDOW_HEIGHT;

  MoveWindow(m_hWnd, newX, newY, newW, newH, TRUE);
}

constexpr COLORREF NUMBER_BASE = MakeRGB(36, 36, 40);
constexpr COLORREF OPERATOR_BASE = MakeRGB(56, 56, 60);
constexpr COLORREF SPECIAL_BASE = MakeRGB(131, 179, 244);
constexpr COLORREF BS_BASE = MakeRGB(235, 102, 19);
constexpr COLORREF ENTR_BASE = MakeRGB(52, 235, 103);

void createButtons(HWND parent) {
  g_buttons = createAllButtons(
      parent,
      {{IDC_BTN_FORWARDSLASH, L"/", MARGIN, MARGIN + LABEL_OFFSET, BUTTON_SIZE,
        BUTTON_SIZE, SPECIAL_BASE, false},
       {IDC_BTN_TAB, L"TAB", MARGIN, MARGIN + LABEL_OFFSET + BUTTON_SIZE,
        BUTTON_SIZE, BUTTON_SIZE * 4, SPECIAL_BASE, false},
       {IDC_BTN_7, L"7", MARGIN + BUTTON_SIZE,
        MARGIN + BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
        NUMBER_BASE, true},
       {IDC_BTN_8, L"8", MARGIN + BUTTON_SIZE * 2,
        MARGIN + BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
        NUMBER_BASE, true},
       {IDC_BTN_9, L"9", MARGIN + BUTTON_SIZE * 3,
        MARGIN + BUTTON_SIZE + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE,
        NUMBER_BASE, true},
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
       {IDC_BTN_PASTE, L"Ctrl\nV", MARGIN + BUTTON_SIZE * 3,
        MARGIN + LABEL_OFFSET, BUTTON_SIZE * 2, BUTTON_SIZE, SPECIAL_BASE,
        false},
       {IDC_BTN_BS, L"BS", MARGIN + BUTTON_SIZE * 5, MARGIN + LABEL_OFFSET,
        BUTTON_SIZE, BUTTON_SIZE * 3, BS_BASE, false},
       {IDC_BTN_ENTER, L"Entr", MARGIN + BUTTON_SIZE * 5,
        BUTTON_SIZE * 3 + MARGIN + LABEL_OFFSET, BUTTON_SIZE, BUTTON_SIZE * 2,
        ENTR_BASE, false},
       {IDC_BTN_INPUT_OPTIONS, L"\\/", MARGIN,
        BUTTON_SIZE * 5 + MARGIN + LABEL_OFFSET, BUTTON_SIZE * 6, BUTTON_SIZE,
        SPECIAL_BASE, false}}

  );
}

void Button::drawBorder(HDC hdc, const RECT &rect) const {
  HPEN hLightPen = CreatePen(PS_SOLID, 2, m_lightColor);
  HPEN hDarkPen = CreatePen(PS_SOLID, 2, m_darkColor);

  HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hLightPen));

  MoveToEx(hdc, rect.left, rect.bottom - 2, nullptr);
  LineTo(hdc, rect.left, rect.top);
  LineTo(hdc, rect.right - 2, rect.top);

  SelectObject(hdc, hDarkPen);
  MoveToEx(hdc, rect.right - 2, rect.top, NULL);
  LineTo(hdc, rect.right - 2, rect.bottom - 2);
  LineTo(hdc, rect.left - 2, rect.bottom - 2);

  SelectObject(hdc, hOldPen);

  DeleteObject(hLightPen);
  DeleteObject(hDarkPen);
}

void Button::drawText(HDC hdc, const RECT &rect) const {
  COLORREF textColor = m_isLightText ? RGB(255, 255, 255) : RGB(0, 0, 0);
  SetTextColor(hdc, textColor);
  SetBkMode(hdc, TRANSPARENT);

  DrawTextW(hdc, m_text, -1, const_cast<RECT *>(&rect),
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
Button::Button(Button &&other) noexcept
    : m_hWnd(other.m_hWnd), m_id(other.m_id), m_text(other.m_text),
      m_x(other.m_x), m_y(other.m_y), m_w(other.m_w), m_h(other.m_h),
      m_baseColor(other.m_baseColor), m_isLightText(other.m_isLightText),
      m_isPressed(other.m_isPressed) {
  other.m_hWnd = nullptr;

  if (m_hWnd) {
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }
}

Button &Button::operator=(Button &&other) noexcept {
  if (this != &other) {
    if (m_hWnd) {
      DestroyWindow(m_hWnd);
    }

    m_id = other.m_id;
    m_text = other.m_text;
    m_x = other.m_x;
    m_y = other.m_y;
    m_w = other.m_w;
    m_h = other.m_h;
    m_baseColor = other.m_baseColor;
    m_isLightText = other.m_isLightText;
    m_hWnd = other.m_hWnd;
    m_isPressed = other.m_isPressed;

    other.m_hWnd = nullptr;

    if (m_hWnd) {
      SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }
  }
  return *this;
}

Button::~Button() {
  if (m_hWnd) {
    DestroyWindow(m_hWnd);
  }
}

std::vector<Button>
createAllButtons(HWND parent,
                 const std::initializer_list<ButtonParams> &params) {
  std::vector<Button> buttons;
  buttons.reserve(params.size());
  for (const auto &p : params) {
    buttons.emplace_back(parent, p.id, p.text, p.x, p.y, p.w, p.h, p.color,
                         p.light);
  }
  return buttons;
}

std::vector<Button> g_buttons;
