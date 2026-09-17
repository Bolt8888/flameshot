// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customnumberconfig.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace {
const int QUICK_COUNT = 10;
const int QUICK_COLUMNS = 5;
const int QUICK_BUTTON_WIDTH = 34;
} // namespace

CustomNumberConfig::CustomNumberConfig(QWidget* parent)
  : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_lineEdit(new QLineEdit())
  , m_incrementButton(nullptr)
{
    m_lineEdit->setMaxLength(8);
    m_lineEdit->setPlaceholderText(tr("Number"));
    m_lineEdit->setToolTip(tr("Value shown inside the bubble"));
    connect(m_lineEdit,
            &QLineEdit::textChanged,
            this,
            &CustomNumberConfig::onLabelEdited);

    m_incrementButton = new QPushButton(tr("+1"));
    m_incrementButton->setToolTip(tr("Increment the current number"));
    connect(m_incrementButton,
            &QPushButton::clicked,
            this,
            &CustomNumberConfig::incrementLabel);

    auto* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_lineEdit);
    inputLayout->addWidget(m_incrementButton);

    m_layout->addLayout(inputLayout);

    // Picking one of 1..10 is the fastest path, the input above stays for
    // any other label.
    auto* quickLayout = new QGridLayout();
    for (int i = 1; i <= QUICK_COUNT; ++i) {
        const QString value = QString::number(i);
        auto* button = new QPushButton(value, this);
        button->setFixedWidth(QUICK_BUTTON_WIDTH);
        button->setToolTip(tr("Set the number to %1").arg(value));
        connect(button, &QPushButton::clicked, this, [this, value]() {
            m_lineEdit->setText(value);
        });
        quickLayout->addWidget(
          button, (i - 1) / QUICK_COLUMNS, (i - 1) % QUICK_COLUMNS);
    }
    m_layout->addLayout(quickLayout);
}

void CustomNumberConfig::setLabel(const QString& label)
{
    QSignalBlocker blocker(m_lineEdit);
    m_lineEdit->setText(label);
}

void CustomNumberConfig::onLabelEdited(const QString& text)
{
    emit labelChanged(text);
}

void CustomNumberConfig::incrementLabel()
{
    bool ok = false;
    int value = m_lineEdit->text().toInt(&ok);
    if (!ok) {
        return;
    }
    m_lineEdit->setText(QString::number(value + 1));
}
