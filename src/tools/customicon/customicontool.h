// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "tools/abstracttwopointtool.h"

class QWidget;

class CustomIconTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit CustomIconTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QString info() override;
    bool isValid() const override;

    QRect mousePreviewRect(const CaptureContext& context) const override;
    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    void paintMousePreview(QPainter& painter,
                           const CaptureContext& context) override;

    QWidget* configurationWidget() override;

    QString iconName() const { return m_iconName; }
    bool leaderLineEnabled() const { return m_showLeaderLine; }

protected:
    CaptureTool::Type type() const override;
    void copyParams(const CustomIconTool* from, CustomIconTool* to);

public slots:
    void drawStart(const CaptureContext& context) override;
    void pressed(CaptureContext& context) override;
    void setIconName(const QString& iconName);
    void setLeaderLineEnabled(bool enabled);

private:
    QString m_tempString;
    QString m_iconName;
    bool m_showLeaderLine;
    bool m_valid;
};
