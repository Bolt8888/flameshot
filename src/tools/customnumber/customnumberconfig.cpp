// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customnumberconfig.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

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
