// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "iconstore.h"

#include <QBitmap>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <algorithm>
#include <limits>

namespace {
const int CACHE_LIMIT_KB = 8192;

// Most user supplied stickers are solid background bitmaps without an alpha
// channel. Knocking out the corner color keeps them usable on screenshots.
QPixmap knockOutSolidBackground(const QPixmap& source)
{
    QImage img = source.toImage();
    if (img.isNull() || !img.rect().contains(0, 0)) {
        return source;
    }
    const QColor bg = img.pixelColor(0, 0);
    if (bg.alpha() == 0) {
        return source;
    }
    QPixmap masked;
    masked.convertFromImage(img);
    masked.setMask(masked.createMaskFromColor(bg, Qt::MaskInColor));
    return masked;
}

// Group folders may carry a numeric prefix such as "03_moving" so the user
// can pin the order of the tabs. The prefix is hidden when the tab is drawn.
struct GroupKey
{
    QString name;
    int order;
    QString text;
};

GroupKey parseGroupName(const QString& name)
{
    static const QRegularExpression re(QStringLiteral("^(\\d+)[-_. ](.+)$"));
    const QRegularExpressionMatch match = re.match(name);
    if (match.hasMatch()) {
        return { name, match.captured(1).toInt(), match.captured(2) };
    }
    return { name, std::numeric_limits<int>::max(), name };
}

bool groupLessThan(const GroupKey& a, const GroupKey& b)
{
    if (a.order != b.order) {
        return a.order < b.order;
    }
    return a.text.compare(b.text, Qt::CaseInsensitive) < 0;
}
} // namespace

IconStore::IconStore()
{
    const QString base = QCoreApplication::applicationDirPath();
    m_directory = base + QDir::separator() + QStringLiteral("custom_icons");
    // QCache cost unit is bytes here, keep roughly 8 MB of decoded icons.
    m_cache.setMaxCost(CACHE_LIMIT_KB * 1024);
}

IconStore& IconStore::instance()
{
    static IconStore store;
    return store;
}

QString IconStore::directory() const
{
    return m_directory;
}

QStringList IconStore::availableGroups() const
{
    QDir dir(m_directory);
    if (!dir.exists()) {
        return {};
    }
    const QStringList subDirs =
      dir.entryList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot,
                    QDir::Name | QDir::IgnoreCase);

    QList<GroupKey> keys;
    keys.reserve(subDirs.size());
    for (const QString& subDir : subDirs) {
        keys.append(parseGroupName(subDir));
    }
    std::sort(keys.begin(), keys.end(), groupLessThan);

    QStringList groups;
    groups.reserve(keys.size() + 1);
    for (const GroupKey& key : keys) {
        groups.append(key.name);
    }
    // Files dropped straight into "custom_icons" end up in the last tab.
    if (!availableIcons(QString()).isEmpty()) {
        groups.append(QString());
    }
    return groups;
}

QStringList IconStore::availableIcons(const QString& group) const
{
    QDir dir(m_directory);
    if (!dir.exists()) {
        return {};
    }
    if (!group.isEmpty() && !dir.cd(group)) {
        return {};
    }
    QStringList filters;
    filters << QStringLiteral("*.png") << QStringLiteral("*.jpg")
            << QStringLiteral("*.jpeg") << QStringLiteral("*.svg")
            << QStringLiteral("*.bmp");
    const QStringList files = dir.entryList(
      filters, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase);
    if (group.isEmpty()) {
        return files;
    }
    QStringList result;
    result.reserve(files.size());
    for (const QString& file : files) {
        result.append(group + QStringLiteral("/") + file);
    }
    return result;
}

QString IconStore::groupLabel(const QString& group)
{
    if (group.isEmpty()) {
        return QCoreApplication::translate("IconStore", "Ungrouped");
    }
    return parseGroupName(group).text;
}

QString IconStore::iconPath(const QString& name) const
{
    return QDir(m_directory).absoluteFilePath(name);
}

QPixmap IconStore::pixmap(const QString& name, int size)
{
    if (name.isEmpty() || size <= 0) {
        return {};
    }
    QString key = QStringLiteral("%1@%2").arg(name).arg(size);
    if (QPixmap* cached = m_cache.object(key)) {
        return *cached;
    }

    QPixmap result = loadPixmap(iconPath(name), size);
    if (result.isNull()) {
        return {};
    }
    // approximate decoded memory footprint
    int cost = result.width() * result.height() * 4;
    m_cache.insert(key, new QPixmap(result), cost);
    return result;
}

QPixmap IconStore::loadPixmap(const QString& path, int size) const
{
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return {};
    }
    if (path.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)) {
        // QIcon relies on the SVG icon plugin which renders the vector at the
        // requested size, so the icon stays crisp whatever the tool size is.
        QIcon icon(path);
        if (icon.isNull()) {
            return {};
        }
        return icon.pixmap(size, size);
    }

    QPixmap source(path);
    if (source.isNull()) {
        return {};
    }
    if (!source.hasAlphaChannel()) {
        source = knockOutSolidBackground(source);
    }
    return source.scaled(
      size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void IconStore::clearCache()
{
    m_cache.clear();
}
