#include "popup_menu.h"
#include <optional>
#include "utils.h"

PopupMenu::PopupMenu() { _baseId = POPUP_MENU_BASE_ID; }
static HWND CreateMenuHostWindow(HINSTANCE hInst) {
  auto wc = zero_init<WNDCLASS>();
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = hInst;
  wc.lpszClassName = L"MenuHostWindow";

  RegisterClass(&wc);

  return CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"MenuHostWindow",
                         L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, hInst,
                         nullptr);
}

void PopupMenu::Show(HWND parent, POINT position) {
  HMENU hMenu = CreatePopupMenu();
  static HWND menuHost = CreateMenuHostWindow(GetModuleHandle(nullptr));
  if (!hMenu)
    return;

  for (size_t i = 0; i < _items.size(); i++) {
    AppendMenuW(hMenu, MF_OWNERDRAW, _baseId + static_cast<UINT>(i),
                _items[i].displayName);
  }
  UINT selectedId = TrackPopupMenuEx(
      hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY,
      position.x, position.y, parent, nullptr);

  if (selectedId != 0) {
    SendMessage(parent, WM_COMMAND, selectedId, 0);
  }

  DestroyMenu(hMenu);
}
void PopupMenu::HandleCommand(UINT commandId,
                              std::function<void(const wchar_t *)> callback) {
  UINT index = commandId - _baseId;
  if (index < _items.size()) {
    callback(_items[index].outputString);
  }
}

void PopupMenu::AddItem(const wchar_t *displayName,
                        const wchar_t *outputString) {
  _items.push_back({displayName, outputString});
}

std::optional<PopupMenu::MenuItem> PopupMenu::GetItem(UINT id) const {
  size_t index = id - _baseId;
  if (index < _items.size()) {
    return _items[index];
  }
  return std::nullopt;
}
