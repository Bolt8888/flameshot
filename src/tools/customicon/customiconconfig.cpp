// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customiconconfig.h"
#include "iconstore.h"

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSize>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
const int GRID_COLUMNS = 3;
const int PREVIEW_SIZE = 24;
} // namespace

CustomIconConfig::CustomIconConfig(QWidget* parent)
  : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_tabs(new QTabWidget(this))
  , m_leaderLineCB(nullptr)
  , m_refreshButton(nullptr)
{
    // Every folder of "custom_icons" becomes one tab, listed on the left so
    // that long group names stay readable.
    m_tabs->setTabPosition(QTabWidget::West);
    m_tabs->setUsesScrollButtons(true);

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

    m_layout->addWidget(m_tabs, 1);
    m_layout->addWidget(m_refreshButton);
    m_layout->addWidget(m_leaderLineCB);

    rebuildTabs();
}

void CustomIconConfig::rebuildTabs()
{
    while (m_tabs->count() > 0) {
        QWidget* page = m_tabs->widget(0);
        m_tabs->removeTab(0);
        delete page;
    }

    IconStore::instance().clearCache();
    const QStringList groups = IconStore::instance().availableGroups();

    if (groups.isEmpty()) {
        auto* page = new QWidget(m_tabs);
        auto* layout = new QVBoxLayout(page);
        auto* hint = new QLabel(
          tr("Put PNG or SVG files into custom_icons/ next to flameshot.exe"));
        hint->setWordWrap(true);
        layout->addWidget(hint);
        layout->addStretch();
        m_tabs->addTab(page, tr("Icons"));
        return;
    }

    for (const QString& group : groups) {
        auto* area = new QScrollArea(m_tabs);
        area->setWidgetResizable(true);
        area->setFrameShape(QFrame::NoFrame);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        area->setWidget(createGroupPage(group, area));
        m_tabs->addTab(area, IconStore::groupLabel(group));
    }
}

QWidget* CustomIconConfig::createGroupPage(const QString& group,
                                           QWidget* parent)
{
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);

    const QStringList icons = IconStore::instance().availableIcons(group);
    auto* grid = new QGridLayout();
    for (int i = 0; i < icons.size(); ++i) {
        const QString name = icons.at(i);
        QPixmap preview = IconStore::instance().pixmap(name, PREVIEW_SIZE);
        if (preview.isNull()) {
            continue;
        }
        auto* button = new QToolButton(page);
        button->setIcon(QIcon(preview));
        button->setIconSize(QSize(PREVIEW_SIZE, PREVIEW_SIZE));
        button->setToolTip(name);
        button->setCheckable(true);
        button->setChecked(name == m_currentIcon);
        connect(button, &QToolButton::clicked, this, [this, name]() {
            selectIcon(name);
        });
        grid->addWidget(button, i / GRID_COLUMNS, i % GRID_COLUMNS);
    }
    // Keep the buttons packed at the top left instead of spreading them out.
    grid->setColumnStretch(GRID_COLUMNS, 1);
    grid->setRowStretch((icons.size() + GRID_COLUMNS - 1) / GRID_COLUMNS + 1,
                        1);
    layout->addLayout(grid);
    layout->addStretch();
    return page;
}

void CustomIconConfig::refreshIcons()
{
    rebuildTabs();
}

void CustomIconConfig::selectIcon(const QString& iconName)
{
    if (m_currentIcon == iconName) {
        return;
    }
    m_currentIcon = iconName;

    const QList<QToolButton*> buttons = m_tabs->findChildren<QToolButton*>();
    for (QToolButton* button : buttons) {
        button->setChecked(button->toolTip() == iconName);
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
