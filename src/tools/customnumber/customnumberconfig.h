// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QVBoxLayout;
class QLineEdit;
class QPushButton;

class CustomNumberConfig : public QWidget
{
    Q_OBJECT
public:
    explicit CustomNumberConfig(QWidget* parent = nullptr);

    void setLabel(const QString& label);

signals:
    void labelChanged(const QString& label);

private slots:
    void incrementLabel();
    void onLabelEdited(const QString& text);

private:
    QVBoxLayout* m_layout;
    QLineEdit* m_lineEdit;
    QPushButton* m_incrementButton;
};
