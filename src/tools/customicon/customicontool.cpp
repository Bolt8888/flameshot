// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customicontool.h"
#include "customiconconfig.h"
#include "iconstore.h"

#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QPixmap>

namespace {
#define PADDING_VALUE 2
#define THICKNESS_OFFSET 15
}

CustomIconTool::CustomIconTool(QObject* parent)
  : AbstractTwoPointTool(parent)
  , m_showLeaderLine(true)
  , m_valid(false)
{}

QIcon CustomIconTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "customicon-outline.svg");
}

QString CustomIconTool::info()
{
    if (m_iconName.isEmpty()) {
        m_tempString = name();
    } else {
        m_tempString = QStringLiteral("%1 - %2").arg(name(), m_iconName);
    }
    return m_tempString;
}

bool CustomIconTool::isValid() const
{
    return m_valid && !m_iconName.isEmpty();
}

QRect CustomIconTool::mousePreviewRect(const CaptureContext& context) const
{
    int width = qMax(8, context.toolSize + THICKNESS_OFFSET);
    QRect rect(0, 0, width, width);
    rect.moveCenter(context.mousePos);
    return rect;
}

QRect CustomIconTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }
    // The pixmap is drawn centered on the anchor point, so the frame must hug
    // the real rendered size instead of a square twice as big. The pointer
    // line is decoration only, the drag end point is deliberately left out so
    // the frame stays the size of the icon however far it was dragged.
    int icon_size = qMax(8, size() + THICKNESS_OFFSET);
    QPixmap pixmap = IconStore::instance().pixmap(m_iconName, icon_size);
    int frame_width = icon_size + PADDING_VALUE * 2;
    int frame_height = frame_width;
    if (!pixmap.isNull()) {
        frame_width = pixmap.width() + PADDING_VALUE * 2;
        frame_height = pixmap.height() + PADDING_VALUE * 2;
    }

    QRect rect(0, 0, frame_width, frame_height);
    rect.moveCenter(points().first);
    return rect;
}

QString CustomIconTool::name() const
{
    return tr("Custom Icon");
}

CaptureTool::Type CustomIconTool::type() const
{
    return CaptureTool::TYPE_CUSTOMICON;
}

void CustomIconTool::copyParams(const CustomIconTool* from, CustomIconTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_iconName = from->m_iconName;
    to->m_showLeaderLine = from->m_showLeaderLine;
    to->m_valid = from->m_valid;
}

QString CustomIconTool::description() const
{
    return tr("Add an icon from the custom_icons folder to mark a spot");
}

CaptureTool* CustomIconTool::copy(QObject* parent)
{
    auto* tool = new CustomIconTool(parent);
    copyParams(this, tool);
    return tool;
}

QWidget* CustomIconTool::configurationWidget()
{
    auto* confW = new CustomIconConfig();
    confW->setIconName(m_iconName);
    confW->setLeaderLineChecked(m_showLeaderLine);
    connect(confW,
            &CustomIconConfig::iconSelected,
            this,
            &CustomIconTool::setIconName);
    connect(confW,
            &CustomIconConfig::leaderLineToggled,
            this,
            &CustomIconTool::setLeaderLineEnabled);
    return confW;
}

void CustomIconTool::setIconName(const QString& iconName)
{
    m_iconName = iconName;
}

void CustomIconTool::setLeaderLineEnabled(bool enabled)
{
    m_showLeaderLine = enabled;
}

void CustomIconTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    if (m_iconName.isEmpty()) {
        return;
    }

    int icon_size = qMax(8, size() + THICKNESS_OFFSET);
    QPixmap iconPixmap = IconStore::instance().pixmap(m_iconName, icon_size);
    if (iconPixmap.isNull()) {
        return;
    }

    // save current pen and brush state
    auto orig_pen = painter.pen();
    auto orig_brush = painter.brush();

    // The pointer line starts on the edge of the icon instead of its centre
    // so the sticker itself stays readable.
    const QLineF line(points().first, points().second);
    const double gap = qMin(iconPixmap.width(), iconPixmap.height()) / 2.0;
    if (m_showLeaderLine && line.length() > gap) {
        const int line_width = qMax(1, size() / 3);
        QLineF trimmed = line;
        trimmed.setP1(line.pointAt(gap / line.length()));

        painter.setPen(QPen(color(), line_width, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(trimmed);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color());
        painter.drawEllipse(points().second, line_width + 1, line_width + 1);
    }

    // Keep the pixmap aspect ratio and center it on the anchor point, wide
    // stickers must not be stretched into a square.
    painter.drawPixmap(points().first.x() - iconPixmap.width() / 2,
                       points().first.y() - iconPixmap.height() / 2,
                       iconPixmap);

    // restore original brush and pen
    painter.setBrush(orig_brush);
    painter.setPen(orig_pen);
}

void CustomIconTool::paintMousePreview(QPainter& painter,
                                       const CaptureContext& context)
{
    onSizeChanged(context.toolSize + PADDING_VALUE);

    auto orig_pen = painter.pen();
    auto orig_opacity = painter.opacity();
    painter.setOpacity(0.35);
    painter.setPen(QPen(context.color,
                        qMax(8, size() + THICKNESS_OFFSET),
                        Qt::SolidLine,
                        Qt::RoundCap));
    painter.drawLine(context.mousePos,
                     { context.mousePos.x() + 1, context.mousePos.y() + 1 });
    painter.setOpacity(orig_opacity);
    painter.setPen(orig_pen);
}

void CustomIconTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    m_valid = true;
}

void CustomIconTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
