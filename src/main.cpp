#include "button.h"
#include "clipboard.h"
#include "dimensions.h"
#include "keypress.h"

// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>

#pragma comment(linker, "/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' \
    publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32.lib")

static bool s_isBackspaceHeld = false;
static bool s_repeatStarted = false;
static HWND s_hNextClipboardViewer = nullptr;

ClipboardLabel g_clipboardLabel;

const unsigned long long ID_TIMER_BACKSPACE_DELAY = 1001;
const unsigned long long ID_TIMER_BACKSPACE_REPEAT = 1002;
const float ASPECT_RATIO =
    (float)ui::MIN_WINDOW_WIDTH / (float)ui::MIN_WINDOW_HEIGHT;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
  RECT rc = {0, 0, ui::MIN_WINDOW_WIDTH, ui::MIN_WINDOW_HEIGHT};
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX, FALSE);

  HWND hwnd = CreateWindowExW(
      WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, CLASS_NAME,
      L"On-Top Numpad", WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX | WS_THICKFRAME,
      CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
      NULL, NULL, hInstance, NULL);

  if (hwnd == NULL)
    return 0;

  CreateButtons(hwnd);
  g_clipboardLabel.Create(hwnd, LABEL_X, LABEL_Y, LABEL_WIDTH, LABEL_HEIGHT);
  g_clipboardLabel.UpdateText();
  // CreateLabel(hwnd, ui::MIN_WINDOW_WIDTH);
  // UpdateClipboardLabel(s_hClipboardLabel);
  ShowWindow(hwnd, nCmdShow);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE:
    s_hNextClipboardViewer = SetClipboardViewer(hwnd);
    break;
  case WM_DRAWCLIPBOARD:
    // UpdateClipboardLabel(s_hClipboardLabel); // Refresh your label
    g_clipboardLabel.UpdateText();

    if (s_hNextClipboardViewer != NULL) {
      SendMessage(s_hNextClipboardViewer, msg, wParam, lParam);
    }
    break;

  case WM_CHANGECBCHAIN:
    if ((HWND)wParam == s_hNextClipboardViewer) {
      s_hNextClipboardViewer = (HWND)lParam;
    } else if (s_hNextClipboardViewer != NULL) {
      SendMessage(s_hNextClipboardViewer, msg, wParam, lParam);
    }
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
    mmi->ptMinTrackSize.x = ui::MIN_WINDOW_WIDTH;
    mmi->ptMinTrackSize.y = ui::MIN_WINDOW_HEIGHT;
    return 0;
  }
  case WM_COMMAND:
    SendKeyPress(wParam);
    break;
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

    g_clipboardLabel.Resize(clientWidth, clientHeight);
    for (const auto &btn : buttons) {
      HWND hButton = btn.hWnd;
      if (hButton == nullptr)
        continue;

      int newX = (btn.x * clientWidth) / ui::MIN_WINDOW_WIDTH;
      int newY = (btn.y * clientHeight) / ui::MIN_WINDOW_HEIGHT;
      int newW = (btn.width * clientWidth) / ui::MIN_WINDOW_WIDTH;
      int newH = (btn.height * clientHeight) / ui::MIN_WINDOW_HEIGHT;

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
    ChangeClipboardChain(hwnd, s_hNextClipboardViewer);
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}
