#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class ClipboardLabel {
public:
  ClipboardLabel() = default;

  void Create(HWND parent, int x, int y, int width, int height);
  void UpdateText();
  void Resize(int clientWidth, int clientHeight);

private:
  std::wstring getClipboardContent(HANDLE hData);
  std::wstring getClipboardFiles(HANDLE hData);

  HWND _hwndLabel = nullptr;
  HFONT _hFont = nullptr;
};

// extern HWND s_hClipboardLabel;
//
// void UpdateClipboardLabel(HWND hwndLabel);
// void CreateLabel(HWND hwnd, int min_window_width);

constexpr int LABEL_X = 5;
constexpr int LABEL_Y = 5;
constexpr int LABEL_WIDTH = 240;
constexpr int LABEL_HEIGHT = 25;
