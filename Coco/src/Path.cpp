/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE, or
 * visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#include "../include/Coco/Path.h"

#include <QMetaType>
#include <QString>

static const int cocoPathQMetaTypeInitializer_ = [] {
    qRegisterMetaType<Coco::Path>("Coco::Path");

    // Lets QSettings write Coco::Path as a plain string in INI files
    QMetaType::registerConverter<Coco::Path, QString>(
        [](const Coco::Path& p) { return p.toQString(); });
    QMetaType::registerConverter<QString, Coco::Path>(
        [](const QString& s) { return Coco::Path(s); });

    return 0;
}();
