// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QPoint>
#include <QRect>
#include <QVector>

/**
 * @brief Offers the rectangle of the topmost regular window under a screen
 *        position and, when possible, the rectangle of the concrete UI
 *        element (button, edit box, tree item, ...) under the cursor inside
 *        that window, so the capture widget can pre-select it on hover and
 *        select it with a plain click.
 *
 * Only implemented on Windows: the window list is taken once from the
 * Win32 API in z-order (EnumWindows) and hit tested per query; the element
 * refinement walks the MSAA accessibility tree of the hit window with
 * accHitTest. On every other platform every query simply returns a null
 * rect, so callers need no platform guards.
 */
class WindowDetector
{
public:
    /**
     * Physical-pixel rectangle (global screen coordinates) of the most
     * specific UI region under the given physical point: the leaf element
     * when MSAA can resolve one inside the hit window, otherwise the
     * window rectangle itself. Returns a null QRect when nothing matches.
     */
    QRect hitRectAt(const QPoint& physicalPos);

private:
    void ensureEnumerated();

    // Topmost first, exactly the EnumWindows z-order. m_hwnds mirrors
    // m_windows entry by entry (void* to keep windows.h out of the header).
    QVector<QRect> m_windows;
    QVector<void*> m_hwnds;
    bool m_enumerated = false;

    // COM is initialized lazily on the GUI thread for the MSAA queries.
    bool m_comInitialized = false;

    // MSAA calls cross the process boundary, so repeated queries for the
    // same point within a short window reuse the previous result.
    QPoint m_lastQueryPos;
    qint64 m_lastQueryMs = 0;
    QRect m_lastQueryResult;
};
