// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QPushButton;
class QTabWidget;
class QVBoxLayout;

class CustomIconConfig : public QWidget
{
    Q_OBJECT
public:
    explicit CustomIconConfig(QWidget* parent = nullptr);

    void setIconName(const QString& iconName);

signals:
    void iconSelected(const QString& iconName);

private slots:
    void refreshIcons();
    void selectIcon(const QString& iconName);

private:
    void rebuildTabs();
    QWidget* createGroupPage(const QString& group, QWidget* parent);

    QVBoxLayout* m_layout;
    QTabWidget* m_tabs;
    QPushButton* m_refreshButton;
    QString m_currentIcon;
};
