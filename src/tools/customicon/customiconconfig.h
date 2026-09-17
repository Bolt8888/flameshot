// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QGridLayout;
class QCheckBox;
class QPushButton;
class QVBoxLayout;

class CustomIconConfig : public QWidget
{
    Q_OBJECT
public:
    explicit CustomIconConfig(QWidget* parent = nullptr);

    void setIconName(const QString& iconName);
    void setLeaderLineChecked(bool checked);

signals:
    void iconSelected(const QString& iconName);
    void leaderLineToggled(bool enabled);

private slots:
    void refreshIcons();
    void selectIcon(const QString& iconName);

private:
    void rebuildGrid();

    QVBoxLayout* m_layout;
    QGridLayout* m_grid;
    QCheckBox* m_leaderLineCB;
    QPushButton* m_refreshButton;
    QString m_currentIcon;
};
