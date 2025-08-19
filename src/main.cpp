#include "button.h"
#include "clipboard.h"
#include "color.h"
#include "dimensions.h"
#include "keypress.h"
#include "popup_menu.h"
#include "server.h"
#include <thread>
#include <winuser.h>

#include "resources/resource.h"
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 10001
#define ID_TRAY_RESTORE 10002

HMENU hTrayMenu = nullptr;
NOTIFYICONDATA nid = {};

void AddTrayIcon(HWND hWnd) {
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hWnd;
  nid.uID = 1;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYICON;
  HINSTANCE hInstance = GetModuleHandle(nullptr);
  if (!hInstance) {
    MessageBoxW(nullptr, L"Failed to get module handle!", L"Error",
                MB_OK | MB_ICONERROR);
    return;
  }
  nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_TRAYICON),
                               IMAGE_ICON, 32, 32, LR_SHARED);
  if (!nid.hIcon) {
    DWORD error = GetLastError();
    wchar_t errorMsg[256];
    swprintf_s(errorMsg, L"Failed to load tray icon! Error code: %lu", error);
    MessageBoxW(nullptr, errorMsg, L"Error", MB_OK | MB_ICONERROR);
    return;
  }
  wcscpy_s(nid.szTip, L"On-Top Numpad");

  if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
    MessageBoxW(nullptr, L"Failed to add tray icon!", L"Error",
                MB_OK | MB_ICONERROR);
    DestroyIcon(nid.hIcon);
    return;
  }

  if (!hTrayMenu) {
    hTrayMenu = CreatePopupMenu();
    AppendMenu(hTrayMenu, MF_STRING, ID_TRAY_RESTORE, L"Open");
    AppendMenu(hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
  }
}

#pragma comment(linker, "/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' \
    publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32.lib")

static bool s_isBackspaceHeld = false;
static bool s_repeatStarted = false;
static HWND s_hNextClipboardViewer = nullptr;

static ClipboardLabel g_clipboardLabel;
static PopupMenu g_inputOptionsMenu;

const unsigned long long ID_TIMER_BACKSPACE_DELAY = 1001;
const unsigned long long ID_TIMER_BACKSPACE_REPEAT = 1002;
const float ASPECT_RATIO =
    (float)ui::MIN_WINDOW_WIDTH / (float)ui::MIN_WINDOW_HEIGHT;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                   [[maybe_unused]] HINSTANCE hPrevInstance,
                   [[maybe_unused]] LPSTR lpCmdLine, int nCmdShow) {
  g_serverThread = std::thread(serverThread);
  // AllocConsole();
  //
  // FILE *fp;
  // freopen_s(&fp, "CONOUT$", "w", stdout);
  // freopen_s(&fp, "CONOUT$", "w", stderr);
  // freopen_s(&fp, "CONIN$", "r", stdin);
  //
  // SetConsoleTitle(L"Debug Console");

  const wchar_t CLASS_NAME[] = L"OnTopKeyboardClass";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  HBRUSH backgroungBrush = CreateSolidBrush(MakeRGB(26, 26, 30));
  wc.hbrBackground = backgroungBrush;

  RegisterClass(&wc);

  RECT rc = {0, 0, ui::MIN_WINDOW_WIDTH, ui::MIN_WINDOW_HEIGHT};
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX, FALSE);

  HWND hwnd = CreateWindowExW(
      WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, CLASS_NAME,
      L"On-Top Numpad", WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX | WS_THICKFRAME,
      CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
      nullptr, nullptr, hInstance, nullptr);

  if (hwnd == nullptr)
    return 0;

  createButtons(hwnd);
  g_inputOptionsMenu.AddItem(L"24мм(4/16/4)", L"4/16/4");
  g_inputOptionsMenu.AddItem(L"32мм(4/10/4/10/4)", L"4/10/4/10/4");
  g_inputOptionsMenu.AddItem(L"42мм(4И/14AR/4М1/16AR/И4)",
                             L"4И/14AR/4М1/16AR/И4");
  g_inputOptionsMenu.AddItem(L"PVC 24мм(PVC.24.WSWS)", L"PVC.24.WSWS");
  g_inputOptionsMenu.AddItem(L"PVS 32мм(PVS.32.WSWS)", L"PVS.32.WSWS");
  g_inputOptionsMenu.AddItem(L"PVC 40мм(PVC.40.WSWS)", L"PVC.40.WSWS");
  g_clipboardLabel.Create(hwnd, LABEL_X, LABEL_Y, LABEL_WIDTH, LABEL_HEIGHT);
  g_clipboardLabel.UpdateText();

  ShowWindow(hwnd, nCmdShow);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE:
    s_hNextClipboardViewer = SetClipboardViewer(hWnd);
    break;
  case WM_DRAWCLIPBOARD:
    g_clipboardLabel.UpdateText();

    if (s_hNextClipboardViewer != nullptr) {
      SendMessage(s_hNextClipboardViewer, msg, wParam, lParam);
    }
    break;

  case WM_CHANGECBCHAIN:
    if ((HWND)wParam == s_hNextClipboardViewer) {
      s_hNextClipboardViewer = (HWND)lParam;
    } else if (s_hNextClipboardViewer != nullptr) {
      SendMessage(s_hNextClipboardViewer, msg, wParam, lParam);
    }
    break;
  case WM_DRAWITEM: {
    LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
    Button *pBtn = reinterpret_cast<Button *>(
        GetWindowLongPtrW(pdis->hwndItem, GWLP_USERDATA));

    if (pBtn) {
      pBtn->Draw(pdis);
      return TRUE;
    }
    if (pdis->CtlType == ODT_MENU) {
      auto item = g_inputOptionsMenu.GetItem(pdis->itemID);
      if (item) {
        // const UINT dpi = 96;
        // Was used for dpi scaling
        const int padding = GetSystemMetrics(SM_CXEDGE);

        HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 44));
        FillRect(pdis->hDC, &pdis->rcItem, bgBrush);
        DeleteObject(bgBrush);

        RECT textRect = pdis->rcItem;
        InflateRect(&textRect, -padding, 0);

        SetTextColor(pdis->hDC, RGB(255, 255, 255));
        SetBkMode(pdis->hDC, TRANSPARENT);

        DrawTextW(pdis->hDC, item->displayName, -1, &textRect,
                  DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);

        if (pdis->itemState & ODS_SELECTED) {
          HBRUSH hilite = CreateSolidBrush(RGB(70, 130, 200));
          FrameRect(pdis->hDC, &pdis->rcItem, hilite);
          DeleteObject(hilite);
        }
        return TRUE;
      }
    }

    break;
  }
  case WM_GETMINMAXINFO: {
    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
    mmi->ptMinTrackSize.x = ui::MIN_WINDOW_WIDTH;
    mmi->ptMinTrackSize.y = ui::MIN_WINDOW_HEIGHT;
    return 0;
  }
  case WM_MEASUREITEM: {
    LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT)lParam;
    if (pmis->CtlType == ODT_MENU) {
      HWND hButton = GetDlgItem(hWnd, IDC_BTN_INPUT_OPTIONS);

      HDC hdc = GetDC(hWnd);
      const int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
      ReleaseDC(hWnd, hdc);
      const float scale = dpi / 96.0f;

      RECT btnRect;
      GetWindowRect(hButton, &btnRect);
      const int buttonWidth =
          static_cast<int>((btnRect.right - btnRect.left - 19) / scale);

      pmis->itemWidth = buttonWidth;
      pmis->itemHeight =
          static_cast<int>(GetSystemMetrics(SM_CYMENUSIZE) * 1.2f * scale);

      return TRUE;
    }
    break;
  }
  case WM_TRAYICON:
    switch (lParam) {
    case WM_LBUTTONUP:
      ShowWindow(hWnd, SW_SHOW);
      Shell_NotifyIcon(NIM_DELETE, &nid);
      break;
    case WM_RBUTTONUP: {
      POINT pt;
      GetCursorPos(&pt);
      SetForegroundWindow(hWnd);
      TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
      break;
    }
    }
    return 0;
  case WM_COMMAND: {
    UINT id = LOWORD(wParam);

    if (id == ID_TRAY_EXIT) {
      Shell_NotifyIcon(NIM_DELETE, &nid);
      DestroyWindow(hWnd);
      return 0;
    } else if (id == ID_TRAY_RESTORE) {
      ShowWindow(hWnd, SW_SHOW);
      Shell_NotifyIcon(NIM_DELETE, &nid);
      return 0;
    }

    if (id < POPUP_MENU_BASE_ID) {
      sendKeyPress(wParam);
    }
    g_inputOptionsMenu.HandleCommand(
        id, [](const wchar_t *output) { setClipboardText(output); });

    break;
  }
  case WM_OPEN_INPUT_OPTIONS: {
    HWND hButton = GetDlgItem(hWnd, IDC_BTN_INPUT_OPTIONS);

    RECT rect;
    GetWindowRect(hButton, &rect);

    POINT pt = {rect.left, rect.bottom};

    g_inputOptionsMenu.Show(hWnd, pt);
    break;
  }
  case WM_BACKSPACE_DOWN:
    if (!s_isBackspaceHeld) {
      s_isBackspaceHeld = true;
      s_repeatStarted = false;
      sendBackspacePress();
      SetTimer(hWnd, ID_TIMER_BACKSPACE_DELAY, 500, nullptr);
    }
    break;
  case WM_BACKSPACE_UP:
    if (s_isBackspaceHeld) {
      s_isBackspaceHeld = false;
      s_repeatStarted = false;
      KillTimer(hWnd, ID_TIMER_BACKSPACE_DELAY);
      KillTimer(hWnd, ID_TIMER_BACKSPACE_REPEAT);
    }
    break;
  case WM_TIMER:
    if (wParam == ID_TIMER_BACKSPACE_DELAY && s_isBackspaceHeld) {
      s_repeatStarted = true;

      sendBackspacePress();

      KillTimer(hWnd, ID_TIMER_BACKSPACE_DELAY);
      SetTimer(hWnd, ID_TIMER_BACKSPACE_REPEAT, 40, nullptr);
    } else if (wParam == ID_TIMER_BACKSPACE_REPEAT && s_isBackspaceHeld) {
      sendBackspacePress();
    }
    break;
  case WM_LBUTTONUP:
  case WM_NCLBUTTONUP:
    if (s_isBackspaceHeld) {
      s_isBackspaceHeld = false;
      s_repeatStarted = false;
      KillTimer(hWnd, ID_TIMER_BACKSPACE_DELAY);
      KillTimer(hWnd, ID_TIMER_BACKSPACE_REPEAT);
    }
    break;
  case WM_SIZE: {
    int clientWidth = LOWORD(lParam);
    int clientHeight = HIWORD(lParam);

    g_clipboardLabel.Resize(clientWidth, clientHeight);
    for (auto &btn : g_buttons) {
      btn.Resize(clientWidth, clientHeight);
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
  case WM_CLOSE:
    ShowWindow(hWnd, SW_HIDE);
    AddTrayIcon(hWnd);
    break;

  case WM_DESTROY:
    g_serverRunning = false;
    if (g_listenSocket != INVALID_SOCKET) {
      closesocket(g_listenSocket);
    }
    if (g_serverThread.joinable()) {
      g_serverThread.join();
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hTrayMenu) {
      DestroyMenu(hTrayMenu);
    }

    ChangeClipboardChain(hWnd, s_hNextClipboardViewer);
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  return 0;
}
