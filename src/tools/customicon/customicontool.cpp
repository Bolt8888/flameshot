// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customicontool.h"
#include "customiconconfig.h"
#include "iconstore.h"

#include <QPainter>
#include <QPixmap>

namespace {
#define PADDING_VALUE 2
#define THICKNESS_OFFSET 15
}

CustomIconTool::CustomIconTool(QObject* parent)
  : AbstractTwoPointTool(parent)
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
    // the real rendered size instead of a square twice as big. No leader line
    // is drawn, so the second point never widens the frame.
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
    connect(confW,
            &CustomIconConfig::iconSelected,
            this,
            &CustomIconTool::setIconName);
    return confW;
}

void CustomIconTool::setIconName(const QString& iconName)
{
    m_iconName = iconName;
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

    // Keep the pixmap aspect ratio and center it on the anchor point, wide
    // stickers must not be stretched into a square.
    painter.drawPixmap(points().first.x() - iconPixmap.width() / 2,
                       points().first.y() - iconPixmap.height() / 2,
                       iconPixmap);
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
