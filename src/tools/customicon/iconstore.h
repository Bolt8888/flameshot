// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QCache>
#include <QPixmap>
#include <QString>
#include <QStringList>

/**
 * Scans and caches user supplied icons stored next to the executable.
 * The icons are read from the "custom_icons" directory which sits beside
 * flameshot.exe, keeping the portable build self contained.
 */
class IconStore
{
public:
    static IconStore& instance();

    QString directory() const;
    QStringList availableIcons() const;
    QString iconPath(const QString& name) const;
    QPixmap pixmap(const QString& name, int size);
    void clearCache();

private:
    IconStore();
    QPixmap loadPixmap(const QString& path, int size) const;

    QCache<QString, QPixmap> m_cache;
    QString m_directory;
};
