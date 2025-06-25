#include "button.h"
#include "keypress.h"
#include <CommCtrl.h>
#include <gdiplus.h>
#include <utilapiset.h>
#include <windef.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#define WM_BACKSPACE_DOWN (WM_APP + 1)
#define WM_BACKSPACE_UP (WM_APP + 2)

#pragma comment(linker, "/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' \
    publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32.lib")

// Window dimensions
const int MIN_WINDOW_WIDTH = 210;
const int MIN_WINDOW_HEIGHT = 210;

// static void sendUnicodeKey(WORD keyCode, INPUT &ip, WPARAM wParam);

static bool s_isBackspaceHeld = false;
static bool s_repeatStarted = false;

const unsigned long long ID_TIMER_BACKSPACE_DELAY = 1001;
const unsigned long long ID_TIMER_BACKSPACE_REPEAT = 1002;
const float ASPECT_RATIO = (float)MIN_WINDOW_WIDTH / (float)MIN_WINDOW_HEIGHT;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateButtons(HWND);
HBITMAP LoadResizeAsBitmap(const wchar_t *filename, int width, int height);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  const wchar_t CLASS_NAME[] = L"OnTopKeyboardClass";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  HBRUSH backgroungBrush = CreateSolidBrush(MakeRGB(26, 26, 30));
  wc.hbrBackground = backgroungBrush;

  RegisterClass(&wc);

  // Calculate window size
  RECT rc = {0, 0, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT};
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX, FALSE);

  HWND hwnd = CreateWindowExW(
      WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, CLASS_NAME,
      L"On-Top Numpad", WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX | WS_THICKFRAME,
      CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
      NULL, NULL, hInstance, NULL);

  if (hwnd == NULL)
    return 0;

  CreateButtons(hwnd);
  ShowWindow(hwnd, nCmdShow);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

void drawButton(LPDRAWITEMSTRUCT pdis) {
  auto it = buttonMap.find(pdis->CtlID);
  if (it == buttonMap.end()) {
    return;
  }
  const Button &btn = it->second.get();
  COLORREF baseColor = btn.baseColor, borderLight = btn.lightColor,
           borderDark = btn.darkColor;

  RECT fillRect = pdis->rcItem;
  InflateRect(&fillRect, -2, -2); // Shrinks the rect by 2px on each side

  HBRUSH bgBrush = CreateSolidBrush(baseColor);
  FillRect(pdis->hDC, &fillRect, bgBrush);
  DeleteObject(bgBrush);

  // Draw custom border - 1 pixel wide
  HPEN hOldPen =
      (HPEN)SelectObject(pdis->hDC, CreatePen(PS_SOLID, 2, borderLight));

  // Light edges (top and left)
  MoveToEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.bottom - 2, NULL);
  LineTo(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top);
  LineTo(pdis->hDC, pdis->rcItem.right - 2, pdis->rcItem.top);

  // Dark edges (right and bottom)
  SelectObject(pdis->hDC, CreatePen(PS_SOLID, 2, borderDark));
  MoveToEx(pdis->hDC, pdis->rcItem.right - 2, pdis->rcItem.top, NULL);
  LineTo(pdis->hDC, pdis->rcItem.right - 2, pdis->rcItem.bottom - 2);
  LineTo(pdis->hDC, pdis->rcItem.left - 2, pdis->rcItem.bottom - 2);

  // Cleanup pens
  DeleteObject(SelectObject(pdis->hDC, hOldPen));

  // Draw text
  COLORREF textColor = btn.isLightText ? RGB(255, 255, 255) : RGB(0, 0, 0);
  SetTextColor(pdis->hDC, textColor);
  SetBkMode(pdis->hDC, TRANSPARENT);
  DrawText(pdis->hDC, btn.text, -1, &pdis->rcItem,
           DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Add pressed effect when selected
  if (btn.isPressed) {
    InflateRect(&pdis->rcItem, -1, -1); // Shrink rectangle
    DrawEdge(pdis->hDC, &pdis->rcItem, EDGE_SUNKEN, BF_RECT);
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE:
    break;
  case WM_DRAWITEM: {
    LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;

    if (pdis->CtlType == ODT_BUTTON) {
      drawButton(pdis);
      return TRUE;
    }
    break;
  }
  case WM_GETMINMAXINFO: {
    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
    mmi->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
    mmi->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
    return 0;
  }
  case WM_COMMAND: {
    WORD id = LOWORD(wParam);
    // if (id == IDC_BTN_BS) {
    //   if (!s_isBackspaceHeld) {
    //     s_isBackspaceHeld = true;
    //     s_repeatStarted = false;
    //
    //     sendBackspacePress();
    //
    //     // Start initial delay timer
    //     SetTimer(hwnd, ID_TIMER_BACKSPACE_DELAY, 500, NULL);
    //   }
    // } else {
    SendKeyPress(wParam);
    // }
    break;
  }
  case WM_BACKSPACE_DOWN:
    if (!s_isBackspaceHeld) {
      s_isBackspaceHeld = true;
      s_repeatStarted = false;
      sendBackspacePress();
      SetTimer(hwnd, ID_TIMER_BACKSPACE_DELAY, 500, NULL);
    }
    break;
  // case WM_NCLBUTTONDOWN:
  case WM_BACKSPACE_UP:
    if (s_isBackspaceHeld) {
      s_isBackspaceHeld = false;
      s_repeatStarted = false;
      KillTimer(hwnd, ID_TIMER_BACKSPACE_DELAY);
      KillTimer(hwnd, ID_TIMER_BACKSPACE_REPEAT);
    }
    break;
  case WM_TIMER:
    if (wParam == ID_TIMER_BACKSPACE_DELAY && s_isBackspaceHeld) {
      s_repeatStarted = true;

      sendBackspacePress();

      KillTimer(hwnd, ID_TIMER_BACKSPACE_DELAY);
      SetTimer(hwnd, ID_TIMER_BACKSPACE_REPEAT, 40, NULL);
    } else if (wParam == ID_TIMER_BACKSPACE_REPEAT && s_isBackspaceHeld) {
      sendBackspacePress();
    }
    break;
  case WM_LBUTTONUP:
  case WM_NCLBUTTONUP:
    if (s_isBackspaceHeld) {
      s_isBackspaceHeld = false;
      s_repeatStarted = false;
      KillTimer(hwnd, ID_TIMER_BACKSPACE_DELAY);
      KillTimer(hwnd, ID_TIMER_BACKSPACE_REPEAT);
      Beep(100, 100);
    }
    break;
  case WM_SIZE: {
    int clientWidth = LOWORD(lParam);
    int clientHeight = HIWORD(lParam);

    for (const auto &btn : buttons) {
      HWND hButton = btn.hWnd;
      if (hButton == nullptr)
        continue;

      int newX = (btn.x * clientWidth) / MIN_WINDOW_WIDTH;
      int newY = (btn.y * clientHeight) / MIN_WINDOW_HEIGHT;
      int newW = (btn.width * clientWidth) / MIN_WINDOW_WIDTH;
      int newH = (btn.height * clientHeight) / MIN_WINDOW_HEIGHT;

      MoveWindow(hButton, newX, newY, newW, newH, TRUE);
    }
    break;
  }

  case WM_SIZING: {
    RECT *pRect = (RECT *)lParam;
    int width = pRect->right - pRect->left;
    int height = pRect->bottom - pRect->top;

    int expectedHeight = (int)(width / ASPECT_RATIO + 0.5f);
    int expectedWidth = (int)(height * ASPECT_RATIO + 0.5f);

    switch (wParam) {
    case WMSZ_LEFT:
    case WMSZ_RIGHT:
    case WMSZ_TOPLEFT:
    case WMSZ_TOPRIGHT:
    case WMSZ_BOTTOMLEFT:
    case WMSZ_BOTTOMRIGHT:
      pRect->bottom = pRect->top + expectedHeight;
      break;

    case WMSZ_TOP:
    case WMSZ_BOTTOM:
      pRect->right = pRect->left + expectedWidth;
      break;

    default:
      break;
    }
    return TRUE;
  }
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

LRESULT CALLBACK ButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, UINT_PTR uIdSubclass,
                                    DWORD_PTR dwRefData) {
  int btnId = GetDlgCtrlID(hWnd);
  switch (uMsg) {
  case WM_LBUTTONDBLCLK: {
  case WM_LBUTTONDOWN:
    auto it = buttonMap.find(btnId);
    if (it == buttonMap.end())
      break;
    Button &btn = it->second.get();
    btn.isPressed = true;
    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);

    if (btnId == IDC_BTN_BS) {
      PostMessageW(GetParent(hWnd), WM_BACKSPACE_DOWN, 0, 0);
    }
    break;
  }
  case WM_LBUTTONUP: {
  case WM_MOUSELEAVE:
    auto it = buttonMap.find(btnId);
    if (it == buttonMap.end())
      break;
    it->second.get().isPressed = false;
    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);

    if (btnId == IDC_BTN_BS) {
      PostMessageW(GetParent(hWnd), WM_BACKSPACE_UP, 0, 0);
    }
    break;
  }
  case WM_NCDESTROY:
    RemoveWindowSubclass(hWnd, ButtonSubclassProc, uIdSubclass);
    break;
  }
  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void CreateButtons(HWND hwnd) {
  // Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  // ULONG_PTR gdiplusToken;
  // GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  // Load and set image on button
  for (auto &btn : buttons) {
    // HBITMAP hBmp =
    //     LoadResizeAsBitmap(L"resources\\alan.png", btn.width, btn.height);
    buttonMap.emplace(btn.id, std::ref(btn));
    HWND button =
        CreateWindowW(L"BUTTON", btn.text,
                      BS_OWNERDRAW | WS_TABSTOP | WS_VISIBLE | WS_CHILD |
                          BS_PUSHBUTTON | BS_MULTILINE,
                      btn.x, btn.y, btn.width, btn.height, hwnd, (HMENU)btn.id,
                      (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    btn.hWnd = button;
    SetWindowSubclass(button, ButtonSubclassProc, 0, 0);
    // SendMessageW(button, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
  }
  // Gdiplus::GdiplusShutdown(gdiplusToken);
}

HBITMAP LoadResizeAsBitmap(const wchar_t *filename, int width, int height) {
  using namespace Gdiplus;

  Bitmap *original = Bitmap::FromFile(filename);
  if (!original || original->GetLastStatus() != Ok) {
    delete original;
    return NULL;
  }

  Bitmap resized(width, height, PixelFormat32bppARGB);
  Graphics graphics(&resized);

  // Optional: set high quality interpolation for better resizing
  graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

  // Draw the original bitmap scaled to fit the new size
  graphics.DrawImage(original, 0, 0, width, height);

  HBITMAP hBitmap = NULL;
  resized.GetHBITMAP(Color(0, 0, 0, 0), &hBitmap);

  delete original;
  return hBitmap;
}
