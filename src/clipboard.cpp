#include "clipboard.h"
#include "dimensions.h"
#include <ShlObj.h>
#include <filesystem>
#include <numeric>
#include <shellapi.h>
#include <vector>
#include <wingdi.h>
#include <winuser.h>

namespace fs = std::filesystem;

static void truncate(HWND hwndLabel, std::wstring &text) {
  RECT rc;
  GetClientRect(hwndLabel, &rc);
  int labelWidth = rc.right - rc.left;

  HDC hdc = GetDC(hwndLabel);

  TEXTMETRICW tm;
  GetTextMetricsW(hdc, &tm);
  int avgCharWidth = tm.tmAveCharWidth;

  ReleaseDC(hwndLabel, hdc);

  int maxChars = labelWidth / avgCharWidth;
  if ((int)text.length() > maxChars) {
    text = text.substr(0, maxChars - 3) + L"...";
  }
}

std::wstring ClipboardLabel::getClipboardContent(HANDLE hData) {
  LPCWSTR pszText = (LPCWSTR)GlobalLock(hData);
  if (pszText) {
    std::wstring text(pszText);
    truncate(_hwndLabel, text);
    GlobalUnlock(hData);
    return text;
  }

  return L"<invalid content>";
}

void ClipboardLabel::UpdateText() {
  if (!OpenClipboard(NULL))
    return;

  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  std::wstring text = L"<no text>";
  if (hData) {
    text = getClipboardContent(hData);
  } else {
    hData = GetClipboardData(CF_HDROP);
    if (hData != nullptr) {
      HDROP hDropHandle = (HDROP)hData;
      std::vector<std::wstring> fileNames;

      UINT fileCount = DragQueryFileW(hDropHandle, 0xFFFFFFFF, NULL, 0);
      if (fileCount > 6) {
        fileCount = 7;
      }
      for (UINT i = 0; i < fileCount; i++) {
        UINT nameLen = DragQueryFileW(hDropHandle, i, NULL, 0);
        std::wstring fileName(nameLen + 1, L'\0');
        DragQueryFileW(hDropHandle, i, &fileName[0], nameLen + 1);
        fileName.resize(nameLen);
        fileName = fs::path(fileName).filename().wstring();
        fileNames.push_back(fileName);
      }

      auto joined = std::accumulate(
          std::next(fileNames.begin()), fileNames.end(), fileNames.front(),
          [](std::wstring acc, const std::wstring &next) {
            return std::move(acc) + L", " + next;
          });
      text = L"files: " + joined;
      truncate(_hwndLabel, text);
    }
  }
  SetWindowTextW(_hwndLabel, text.c_str());
  CloseClipboard();
}

void ClipboardLabel::Create(HWND parent, int x, int y, int width, int height) {
  _hwndLabel = CreateWindowW(
      L"STATIC", L"",
      WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | SS_NOPREFIX, x, y,
      width, height, parent, NULL,
      (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);

  _hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
  SendMessageW(_hwndLabel, WM_SETFONT, (WPARAM)_hFont, TRUE);
}

void ClipboardLabel::Resize(int clientWidth, int clientHeight) {
  int newLabelX = (LABEL_X * clientWidth) / ui::MIN_WINDOW_WIDTH;
  int newLabelY = (LABEL_Y * clientHeight) / ui::MIN_WINDOW_HEIGHT;
  int newLabelW = (LABEL_WIDTH * clientWidth) / ui::MIN_WINDOW_WIDTH;
  int newLabelH = (LABEL_HEIGHT * clientHeight) / ui::MIN_WINDOW_HEIGHT;

  MoveWindow(_hwndLabel, newLabelX, newLabelY, newLabelW, newLabelH, TRUE);
  int scaledFontHeight = (16 * clientHeight) / ui::MIN_WINDOW_HEIGHT;

  _hFont =
      CreateFontW(scaledFontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                  DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

  SendMessageW(_hwndLabel, WM_SETFONT, (WPARAM)_hFont, TRUE);
}
