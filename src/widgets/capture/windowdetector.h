// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QRect>
#include <QVector>

/**
 * @brief Offers the rectangle of the topmost regular window under a screen
 *        position, so the capture widget can pre-select that window on hover
 *        and select it with a plain click.
 *
 * Only implemented on Windows: the window list is taken once from the
 * Win32 API in z-order (EnumWindows) and hit tested per query. On every
 * other platform every query simply returns a null rect, so callers need
 * no platform guards.
 */
class WindowDetector
{
public:
    /**
     * Physical-pixel rectangle (global screen coordinates) of the topmost
     * regular window containing the given physical point, or a null QRect
     * when no window matches.
     */
    QRect windowRectAt(const QPoint& physicalPos);

private:
    void ensureEnumerated();

    // Topmost first, exactly the EnumWindows z-order.
    QVector<QRect> m_windows;
    bool m_enumerated = false;
};
