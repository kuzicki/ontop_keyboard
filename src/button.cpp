#include "button.h"
#include <CommCtrl.h>
// #include <gdiplus.h>

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

// HBITMAP LoadResizeAsBitmap(const wchar_t *filename, int width, int height) {
//   using namespace Gdiplus;
//
//   Bitmap *original = Bitmap::FromFile(filename);
//   if (!original || original->GetLastStatus() != Ok) {
//     delete original;
//     return NULL;
//   }
//
//   Bitmap resized(width, height, PixelFormat32bppARGB);
//   Graphics graphics(&resized);
//
//   // Optional: set high quality interpolation for better resizing
//   graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
//
//   // Draw the original bitmap scaled to fit the new size
//   graphics.DrawImage(original, 0, 0, width, height);
//
//   HBITMAP hBitmap = NULL;
//   resized.GetHBITMAP(Color(0, 0, 0, 0), &hBitmap);
//
//   delete original;
//   return hBitmap;
// }
