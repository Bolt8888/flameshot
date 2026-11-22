// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "widgets/capture/windowdetector.h"

#include <QPoint>
#include <QVector>

#ifdef Q_OS_WIN

#include <dwmapi.h>
#include <qt_windows.h>

namespace {

BOOL CALLBACK collectWindow(HWND hwnd, LPARAM lParam)
{
    auto* windows = reinterpret_cast<QVector<QRect>*>(lParam);
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) {
        return TRUE;
    }
    // Our own capture overlay and pinned images must never match.
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId()) {
        return TRUE;
    }
    // Click-through layers would otherwise shadow the real windows below.
    if (GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT) {
        return TRUE;
    }
    // Cloaked UWP windows are invisible to the user.
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(
          hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) &&
        cloaked != 0) {
        return TRUE;
    }
    RECT rect{};
    // The extended frame bounds exclude the invisible drop shadow margins,
    // so the offered rectangle hugs the visible window edges.
    if (FAILED(DwmGetWindowAttribute(
          hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect)))) {
        if (!GetWindowRect(hwnd, &rect)) {
            return TRUE;
        }
    }
    if (rect.right - rect.left < 1 || rect.bottom - rect.top < 1) {
        return TRUE;
    }
    windows->append(QRect(
      rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));
    return TRUE;
}

} // namespace

QRect WindowDetector::windowRectAt(const QPoint& physicalPos)
{
    ensureEnumerated();
    for (const QRect& rect : m_windows) {
        if (rect.contains(physicalPos)) {
            return rect;
        }
    }
    return {};
}

void WindowDetector::ensureEnumerated()
{
    if (m_enumerated) {
        return;
    }
    m_enumerated = true;
    // EnumWindows walks the windows in z-order, topmost first, which is
    // exactly the order the hit test needs. The capture is a momentary
    // state, so a single enumeration is enough.
    QVector<QRect> windows;
    EnumWindows(collectWindow, reinterpret_cast<LPARAM>(&windows));
    m_windows = windows;
}

#else // not Windows

QRect WindowDetector::windowRectAt(const QPoint& physicalPos)
{
    Q_UNUSED(physicalPos)
    return {};
}

void WindowDetector::ensureEnumerated() {}

#endif
