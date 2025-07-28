#pragma once
#include <Windows.h>
#include <functional>
#include <vector>
#include <optional>

inline const UINT POPUP_MENU_BASE_ID = 1000;

class PopupMenu {
public:
  struct MenuItem {
    const wchar_t *displayName;
    const wchar_t *outputString;
  };

  PopupMenu();
  void AddItem(const wchar_t *displayName, const wchar_t *outputString);
  void Show(HWND parent, POINT position);
  void HandleCommand(UINT commandId,
                     std::function<void(const wchar_t *)> callback);

  std::optional<MenuItem> GetItem(UINT id) const;

private:
  std::vector<MenuItem> _items;
  UINT _baseId = POPUP_MENU_BASE_ID;
};
