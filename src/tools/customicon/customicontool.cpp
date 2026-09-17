// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customicontool.h"
#include "customiconconfig.h"
#include "iconstore.h"

#include <QPainter>
#include <QPainterPath>
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
    m_tempString =
      m_iconName.isEmpty() ? name() : QString("%1 - %2").arg(name(), m_iconName);
    return m_tempString;
}

bool CustomIconTool::isValid() const
{
    return m_valid && !m_iconName.isEmpty();
}

QRect CustomIconTool::mousePreviewRect(const CaptureContext& context) const
{
    int width = (context.toolSize + THICKNESS_OFFSET) * 2;
    QRect rect(0, 0, width, width);
    rect.moveCenter(context.mousePos);
    return rect;
}

QRect CustomIconTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }
    int bubble_size = size() + THICKNESS_OFFSET + PADDING_VALUE;

    int line_pos_min_x =
      qMin(points().first.x() - bubble_size, points().second.x());
    int line_pos_min_y =
      qMin(points().first.y() - bubble_size, points().second.y());
    int line_pos_max_x =
      qMax(points().first.x() + bubble_size, points().second.x());
    int line_pos_max_y =
      qMax(points().first.y() + bubble_size, points().second.y());

    return { line_pos_min_x,
             line_pos_min_y,
             line_pos_max_x - line_pos_min_x,
             line_pos_max_y - line_pos_min_y };
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

    QLineF line(points().first, points().second);
    int half = icon_size / 2;
    // if the mouse is outside of the icon, draw the pointer line
    if (m_showLeaderLine && line.length() > half) {
        painter.setPen(QPen(color(), 0));
        painter.setBrush(color());

        int middleX = points().first.x();
        int middleY = points().first.y();

        QLineF normal = line.normalVector();
        normal.setLength(half);
        QPoint p1 = normal.p2().toPoint();
        QPoint p2(middleX - (p1.x() - middleX), middleY - (p1.y() - middleY));

        QPainterPath path;
        path.moveTo(points().first);
        path.lineTo(p1);
        path.lineTo(points().second);
        path.lineTo(p2);
        path.lineTo(points().first);
        painter.drawPath(path);
    }

    painter.drawPixmap(points().first.x() - half,
                       points().first.y() - half,
                       icon_size,
                       icon_size,
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
                        (size() + THICKNESS_OFFSET) * 2,
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
