#pragma once
#include <utility>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

constexpr BYTE ExtractR(COLORREF color) noexcept { return color & 0xFF; }
constexpr BYTE ExtractG(COLORREF color) noexcept { return (color >> 8) & 0xFF; }
constexpr BYTE ExtractB(COLORREF color) noexcept {
  return (color >> 16) & 0xFF;
}

constexpr COLORREF MakeRGB(BYTE r, BYTE g, BYTE b) noexcept {
  return r | (g << 8) | (b << 16);
}

constexpr auto ComputeShades(COLORREF base, int delta = 20) noexcept {
  auto clamp = [](int value) -> BYTE {
    return static_cast<BYTE>(value < 0 ? 0 : (value > 255 ? 255 : value));
  };

  return std::make_pair(
      MakeRGB(clamp(ExtractR(base) - delta), clamp(ExtractG(base) - delta),
              clamp(ExtractB(base) - delta)),
      MakeRGB(clamp(ExtractR(base) + delta), clamp(ExtractG(base) + delta),
              clamp(ExtractB(base) + delta)));
}
