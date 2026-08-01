#pragma once

#include <QString>
#include <QStringList>

#include "Path.h"

namespace Coco::Disk {

inline void
prune(const Coco::Path& dir, const QString& prefix, const QString& ext, int cap)
{
    if (cap < 1) return;

    auto all_files = Coco::filePaths(dir);
    QStringList matches{};

    for (auto& path : all_files) {
        auto name = path.nameQString();
        if (name.startsWith(prefix) && name.endsWith(ext)) matches << name;
    }

    if (matches.size() <= cap) return;

    matches.sort();

    auto to_remove = matches.size() - cap;

    for (qsizetype i = 0; i < to_remove; ++i)
        Coco::remove(dir / matches[i]);
}

}
