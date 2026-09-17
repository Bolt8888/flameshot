// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customnumbertool.h"
#include "customnumberconfig.h"
#include "utils/colorutils.h"

#include <QPainter>
#include <QPainterPath>

namespace {
#define PADDING_VALUE 2
#define THICKNESS_OFFSET 15
}

CustomNumberTool::CustomNumberTool(QObject* parent)
  : AbstractTwoPointTool(parent)
  , m_label(QStringLiteral("1"))
  , m_valid(false)
{}

QIcon CustomNumberTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "customnumber-outline.svg");
}

QString CustomNumberTool::info()
{
    m_tempString = QString("%1 - %2").arg(name(), m_label);
    return m_tempString;
}

bool CustomNumberTool::isValid() const
{
    return m_valid && !m_label.isEmpty();
}

QRect CustomNumberTool::mousePreviewRect(const CaptureContext& context) const
{
    int width = (context.toolSize + THICKNESS_OFFSET) * 2;
    QRect rect(0, 0, width, width);
    rect.moveCenter(context.mousePos);
    return rect;
}

QRect CustomNumberTool::boundingRect() const
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

QString CustomNumberTool::name() const
{
    return tr("Custom Number");
}

CaptureTool::Type CustomNumberTool::type() const
{
    return CaptureTool::TYPE_CUSTOMNUMBER;
}

void CustomNumberTool::copyParams(const CustomNumberTool* from,
                                  CustomNumberTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_label = from->m_label;
    to->m_valid = from->m_valid;
}

QString CustomNumberTool::description() const
{
    return tr("Add a bubble with a number or short label of your choice");
}

CaptureTool* CustomNumberTool::copy(QObject* parent)
{
    auto* tool = new CustomNumberTool(parent);
    copyParams(this, tool);
    return tool;
}

QWidget* CustomNumberTool::configurationWidget()
{
    auto* confW = new CustomNumberConfig();
    confW->setLabel(m_label);
    connect(confW,
            &CustomNumberConfig::labelChanged,
            this,
            &CustomNumberTool::setLabel);
    return confW;
}

void CustomNumberTool::setLabel(const QString& label)
{
    m_label = label;
}

void CustomNumberTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    if (m_label.isEmpty()) {
        return;
    }

    // save current pen, brush, and font state
    auto orig_pen = painter.pen();
    auto orig_brush = painter.brush();
    auto orig_font = painter.font();

    QColor contrastColor =
      ColorUtils::colorIsDark(color()) ? Qt::white : Qt::black;
    QColor antiContrastColor =
      ColorUtils::colorIsDark(color()) ? Qt::black : Qt::white;

    int bubble_size = size() + THICKNESS_OFFSET;

    QLineF line(points().first, points().second);
    // if the mouse is outside of the bubble, draw the pointer
    if (line.length() > bubble_size) {
        painter.setPen(QPen(color(), 0));
        painter.setBrush(color());

        int middleX = points().first.x();
        int middleY = points().first.y();

        QLineF normal = line.normalVector();
        normal.setLength(bubble_size);
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

    painter.setPen(contrastColor);
    painter.setBrush(antiContrastColor);
    painter.drawEllipse(
      points().first, bubble_size + PADDING_VALUE, bubble_size + PADDING_VALUE);
    painter.setBrush(color());
    painter.drawEllipse(points().first, bubble_size, bubble_size);
    QRect textRect = QRect(points().first.x() - bubble_size / 2,
                           points().first.y() - bubble_size / 2,
                           bubble_size,
                           bubble_size);
    auto new_font = orig_font;
    auto fontSize = bubble_size;
    new_font.setPixelSize(fontSize);
    new_font.setBold(true);
    painter.setFont(new_font);

    QRect bRect = painter.boundingRect(textRect, Qt::AlignCenter, m_label);

    // Calculate font size
    while (bRect.width() > textRect.width()) {
        fontSize--;
        if (fontSize == 0) {
            break;
        }
        new_font.setPixelSize(fontSize);
        painter.setFont(new_font);
        bRect = painter.boundingRect(textRect, Qt::AlignCenter, m_label);
    }

    // Draw text
    painter.setPen(contrastColor);
    painter.drawText(textRect, Qt::AlignCenter, m_label);
    // restore original font, brush, and pen
    painter.setFont(orig_font);
    painter.setBrush(orig_brush);
    painter.setPen(orig_pen);
}

void CustomNumberTool::paintMousePreview(QPainter& painter,
                                         const CaptureContext& context)
{
    onSizeChanged(context.toolSize + PADDING_VALUE);

    // Thickness for pen is *2 to range from radius to diameter to match the
    // ellipse draw function
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

void CustomNumberTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    m_valid = true;
}

void CustomNumberTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
