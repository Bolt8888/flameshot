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
 *
 * Every sub directory of "custom_icons" becomes one icon group, so users
 * create, rename or delete groups simply by editing folders. Icon names
 * handed to this store are relative paths such as "group/icon.svg".
 */
class IconStore
{
public:
    static IconStore& instance();

    QString directory() const;
    // Folder names below "custom_icons", already sorted for display. An
    // empty entry stands for the files that sit directly in the root.
    QStringList availableGroups() const;
    // Icons of one group, as paths relative to "custom_icons".
    QStringList availableIcons(const QString& group) const;
    // Human readable tab title: "03_moving" becomes "moving".
    static QString groupLabel(const QString& group);
    QString iconPath(const QString& name) const;
    QPixmap pixmap(const QString& name, int size);
    void clearCache();

private:
    IconStore();
    QPixmap loadPixmap(const QString& path, int size) const;

    QCache<QString, QPixmap> m_cache;
    QString m_directory;
};
