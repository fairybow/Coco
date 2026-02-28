/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE, or
 * visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#include "../include/Coco/Path.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QList>
#include <QMetaType>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

static const int cocoPathQMetaTypeInitializer_ = [] {
    qRegisterMetaType<Coco::Path>("Coco::Path");

    // Lets QSettings write Coco::Path as a plain string in INI files
    QMetaType::registerConverter<Coco::Path, QString>(
        [](const Coco::Path& p) { return p.toQString(); });
    QMetaType::registerConverter<QString, Coco::Path>(
        [](const QString& s) { return Coco::Path(s); });

    return 0;
}();

// TODO: These are duplicated from header!
#define TO_QSTRING_(StdFsPath) QString::fromStdString(StdFsPath.string())
#define CACHED_STRING_(DPtr)                                                   \
    (DPtr->cacheValid ? DPtr->cachedString : DPtr->path.string())
#define CACHED_QSTRING_(DPtr)                                                  \
    (DPtr->cacheValid ? DPtr->cachedQString : TO_QSTRING_(DPtr->path))

// TODO: Why not inline?
std::size_t std::hash<Coco::Path>::operator()(const Coco::Path& path) const
{
    return std::hash<std::filesystem::path>()(path.toStd());
}

namespace Coco {

// TODO: Why not inline?
std::string Path::prettyString() const
{
    std::string pretty{};
    auto last_ch_was_sep = false;

    for (auto& ch : CACHED_STRING_(d_)) {
        if (ch == '/' || ch == '\\') {
            if (!last_ch_was_sep) {
                pretty += '/';
                last_ch_was_sep = true;
            }
        } else {
            pretty += ch;
            last_ch_was_sep = false;
        }
    }

    return pretty;
}

} // namespace Coco

// TODO: These are duplicated from header!
#undef TO_QSTRING_
#undef CACHED_QSTRING_
#undef CACHED_STRING_
