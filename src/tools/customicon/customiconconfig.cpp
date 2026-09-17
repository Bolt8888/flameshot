// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customiconconfig.h"
#include "iconstore.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
const int GRID_COLUMNS = 3;
const int PREVIEW_SIZE = 24;
}

CustomIconConfig::CustomIconConfig(QWidget* parent)
  : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_grid(new QGridLayout())
  , m_leaderLineCB(nullptr)
  , m_refreshButton(nullptr)
{
    m_refreshButton = new QPushButton(tr("Refresh"));
    m_refreshButton->setToolTip(tr("Rescan the custom_icons folder"));
    connect(m_refreshButton,
            &QPushButton::clicked,
            this,
            &CustomIconConfig::refreshIcons);

    m_leaderLineCB = new QCheckBox(tr("Pointer line"));
    m_leaderLineCB->setToolTip(tr("Draw a line from the icon to the cursor"));
    connect(m_leaderLineCB,
            &QCheckBox::toggled,
            this,
            &CustomIconConfig::leaderLineToggled);

    m_layout->addLayout(m_grid);
    m_layout->addWidget(m_refreshButton);
    m_layout->addWidget(m_leaderLineCB);

    rebuildGrid();
}

void CustomIconConfig::rebuildGrid()
{
    QLayoutItem* child = nullptr;
    while ((child = m_grid->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    IconStore::instance().clearCache();
    const QStringList icons = IconStore::instance().availableIcons();

    if (icons.isEmpty()) {
        auto* hint = new QLabel(tr("Put PNG or SVG files into custom_icons/ "
                                   "next to flameshot.exe"));
        hint->setWordWrap(true);
        m_grid->addWidget(hint, 0, 0, 1, GRID_COLUMNS);
        return;
    }

    for (int i = 0; i < icons.size(); ++i) {
        const QString name = icons.at(i);
        QPixmap preview = IconStore::instance().pixmap(name, PREVIEW_SIZE);
        if (preview.isNull()) {
            continue;
        }
        auto* button = new QToolButton();
        button->setIcon(QIcon(preview));
        button->setIconSize(QSize(PREVIEW_SIZE, PREVIEW_SIZE));
        button->setToolTip(name);
        button->setCheckable(true);
        button->setChecked(name == m_currentIcon);
        connect(button,
                &QToolButton::clicked,
                this,
                [this, name]() { selectIcon(name); });
        m_grid->addWidget(button, i / GRID_COLUMNS, i % GRID_COLUMNS);
    }
}

void CustomIconConfig::refreshIcons()
{
    rebuildGrid();
}

void CustomIconConfig::selectIcon(const QString& iconName)
{
    if (m_currentIcon == iconName) {
        return;
    }
    m_currentIcon = iconName;

    for (int i = 0; i < m_grid->count(); ++i) {
        auto* item = m_grid->itemAt(i);
        if (item == nullptr) {
            continue;
        }
        auto* button = qobject_cast<QToolButton*>(item->widget());
        if (button != nullptr) {
            button->setChecked(button->toolTip() == iconName);
        }
    }

    emit iconSelected(iconName);
}

void CustomIconConfig::setIconName(const QString& iconName)
{
    m_currentIcon = iconName;
}

void CustomIconConfig::setLeaderLineChecked(bool checked)
{
    QSignalBlocker blocker(m_leaderLineCB);
    m_leaderLineCB->setChecked(checked);
}
