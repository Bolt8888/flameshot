// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "widgets/capture/windowdetector.h"

#include <QDateTime>
#include <QPoint>
#include <QVector>

#ifdef Q_OS_WIN

#include <dwmapi.h>
#include <oleacc.h>
#include <qt_windows.h>

namespace {

struct WindowCollection
{
    QVector<QRect> rects;
    QVector<void*> hwnds;
};

BOOL CALLBACK collectWindow(HWND hwnd, LPARAM lParam)
{
    auto* result = reinterpret_cast<WindowCollection*>(lParam);
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
    result->rects.append(QRect(
      rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));
    result->hwnds.append(hwnd);
    return TRUE;
}

// Walks the MSAA tree of the given window down to the leaf element at the
// physical screen point and returns its location. Everything is expressed in
// physical screen pixels, matching the caller's coordinate space.
QRect elementRectAt(HWND hwnd, const QPoint& physicalPos)
{
    IAccessible* node = nullptr;
    // OBJID_CLIENT is what most applications expose their UI tree through;
    // OBJID_WINDOW is the fallback for the ones that only implement the
    // window object itself.
    if (FAILED(AccessibleObjectFromWindow(hwnd,
                                          OBJID_CLIENT,
                                          __uuidof(IAccessible),
                                          reinterpret_cast<void**>(&node)))) {
        node = nullptr;
        if (FAILED(
              AccessibleObjectFromWindow(hwnd,
                                         OBJID_WINDOW,
                                         __uuidof(IAccessible),
                                         reinterpret_cast<void**>(&node)))) {
            return {};
        }
    }
    if (!node) {
        return {};
    }

    // accHitTest answers with the object itself (CHILDID_SELF) when it is
    // the leaf at the point, or a child object to descend into. Every node
    // is normalized to (IAccessible*, CHILDID_SELF) so accLocation can use
    // the same child id along the way.
    QRect element;
    for (int depth = 0; depth < 20 && node; ++depth) {
        VARIANT kid{};
        VariantInit(&kid);
        const HRESULT hit =
          node->accHitTest(physicalPos.x(), physicalPos.y(), &kid);
        if (FAILED(hit) || kid.vt == VT_EMPTY) {
            VariantClear(&kid);
            break;
        }
        if (kid.vt == VT_I4 && kid.lVal == CHILDID_SELF) {
            VariantClear(&kid);
            break;
        }
        IAccessible* kidNode = nullptr;
        if (kid.vt == VT_DISPATCH && V_DISPATCH(&kid) &&
            SUCCEEDED(V_DISPATCH(&kid)->QueryInterface(
              __uuidof(IAccessible), reinterpret_cast<void**>(&kidNode))) &&
            kidNode) {
            VariantClear(&kid);
            node->Release();
            node = kidNode;
            continue;
        }
        VariantClear(&kid);
        break;
    }

    if (node) {
        long left = 0, top = 0, width = 0, height = 0;
        VARIANT self{};
        self.vt = VT_I4;
        self.lVal = CHILDID_SELF;
        if (SUCCEEDED(node->accLocation(&left, &top, &width, &height, self)) &&
            width > 1 && height > 1) {
            const QRect rect(left, top, width, height);
            // The element must own the queried point and stay inside the
            // window it belongs to; anything else is an unreliable answer.
            if (rect.contains(physicalPos)) {
                element = rect;
            }
        }
        node->Release();
    }
    return element;
}

} // namespace

QRect WindowDetector::hitRectAt(const QPoint& physicalPos)
{
    ensureEnumerated();

    // Repeated mouse-move events over the same pixel reuse the previous
    // answer; MSAA walks into other processes and must not run per event.
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastQueryPos == physicalPos && now - m_lastQueryMs < 50) {
        return m_lastQueryResult;
    }
    m_lastQueryPos = physicalPos;
    m_lastQueryMs = now;
    m_lastQueryResult = QRect();

    if (!m_comInitialized) {
        // The GUI thread usually already runs in apartment threading, in
        // which case this only takes a reference; a mode conflict simply
        // disables the element refinement.
        if (SUCCEEDED(CoInitializeEx(
              nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
            m_comInitialized = true;
        }
    }

    QRect windowRect;
    for (int i = 0; i < m_windows.size(); ++i) {
        if (m_windows.at(i).contains(physicalPos)) {
            windowRect = m_windows.at(i);
            const HWND hwnd = static_cast<HWND>(m_hwnds.at(i));
            if (m_comInitialized) {
                const QRect element = elementRectAt(hwnd, physicalPos);
                // Only trust elements that sit inside their window; the
                // fallback keeps the phase-one behavior intact.
                if (!element.isNull() && windowRect.contains(element)) {
                    m_lastQueryResult = element;
                    return element;
                }
            }
            break;
        }
    }
    m_lastQueryResult = windowRect;
    return windowRect;
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
    WindowCollection collected;
    EnumWindows(collectWindow, reinterpret_cast<LPARAM>(&collected));
    m_windows = collected.rects;
    m_hwnds = collected.hwnds;
}

#else // not Windows

QRect WindowDetector::hitRectAt(const QPoint& physicalPos)
{
    Q_UNUSED(physicalPos)
    return {};
}

void WindowDetector::ensureEnumerated() {}

#endif
