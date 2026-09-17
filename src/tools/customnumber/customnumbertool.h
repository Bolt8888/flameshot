// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "tools/abstracttwopointtool.h"

class QWidget;

class CustomNumberTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit CustomNumberTool(QObject* parent = nullptr);

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

    QString label() const { return m_label; }

protected:
    CaptureTool::Type type() const override;
    void copyParams(const CustomNumberTool* from, CustomNumberTool* to);

public slots:
    void drawStart(const CaptureContext& context) override;
    void pressed(CaptureContext& context) override;
    void setLabel(const QString& label);

private:
    QString m_tempString;
    QString m_label;
    bool m_valid;
};
