#include "keypress.h"
#include "button.h"
#include <winuser.h>

static void sendCopyOrPaste(INPUT &ip, WPARAM wParam);
static void sendUnicodeKey(WORD keyCode, INPUT &ip, WPARAM wParam);

void SendKeyPress(WPARAM wParam) {
  INPUT ip = {0};
  ip.type = INPUT_KEYBOARD;

  switch (LOWORD(wParam)) {
  case IDC_BTN_0:
    ip.ki.wVk = '0';
    break;
  case IDC_BTN_1:
    ip.ki.wVk = '1';
    break;
  case IDC_BTN_2:
    ip.ki.wVk = '2';
    break;
  case IDC_BTN_3:
    ip.ki.wVk = '3';
    break;
  case IDC_BTN_4:
    ip.ki.wVk = '4';
    break;
  case IDC_BTN_5:
    ip.ki.wVk = '5';
    break;
  case IDC_BTN_6:
    ip.ki.wVk = '6';
    break;
  case IDC_BTN_7:
    ip.ki.wVk = '7';
    break;
  case IDC_BTN_8:
    ip.ki.wVk = '8';
    break;
  case IDC_BTN_9:
    ip.ki.wVk = '9';
    break;
  case IDC_BTN_DOT:
    sendUnicodeKey(0x002E, ip, wParam);
    return;
  case IDC_BTN_PLUS:
    ip.ki.wVk = VK_ADD;
    break;
  case IDC_BTN_MINUS:
    ip.ki.wVk = VK_SUBTRACT;
    break;
  case IDC_BTN_P:
    sendUnicodeKey(0x0440, ip, wParam);
    return;
  case IDC_BTN_COMMA:
    sendUnicodeKey(0x002C, ip, wParam);
    return;
  case IDC_BTN_ENTER:
    ip.ki.wVk = VK_RETURN;
    break;
  case IDC_BTN_TAB:
    ip.ki.wVk = VK_TAB;
    break;
  case IDC_BTN_FORWARDSLASH:
    sendUnicodeKey(0x002F, ip, wParam);
    return;
  case IDC_BTN_BS:
    // ip.ki.wVk = VK_BACK;
    return;
  case IDC_BTN_COPY:
  case IDC_BTN_PASTE:
    sendCopyOrPaste(ip, wParam);
    return;
  default:
    return;
  }

  SendInput(1, &ip, sizeof(INPUT));
  ip.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(INPUT));
}

static void sendCopyOrPaste(INPUT &ip, WPARAM wParam) {
  ip.ki.wVk = VK_CONTROL;
  ip.ki.dwFlags = 0;
  SendInput(1, &ip, sizeof(INPUT));

  ip.ki.wVk = (LOWORD(wParam) == IDC_BTN_COPY) ? 'C' : 'V';
  ip.ki.dwFlags = 0;
  SendInput(1, &ip, sizeof(INPUT));

  ip.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(INPUT));

  ip.ki.wVk = VK_CONTROL;
  ip.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(INPUT));
}

static void sendUnicodeKey(WORD keyCode, INPUT &ip, WPARAM wParam) {
  ip.ki.wScan = keyCode;
  ip.ki.dwFlags = KEYEVENTF_UNICODE;
  SendInput(1, &ip, sizeof(INPUT));
  ip.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(INPUT));
}

static void sendBackspacePress() {
  INPUT inputs[2] = {};

  // Key down
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VK_BACK;
  inputs[0].ki.dwFlags = 0;

  // Key up
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = VK_BACK;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

  SendInput(2, inputs, sizeof(INPUT));
}
