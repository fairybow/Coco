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
#include <QVariant>

// Registers Path with Qt's meta-type system and adds bidirectional QString
// converters, allowing Path to be stored in and retrieved from QVariant. This
// enables interop with any QVariant-based API, including QSettings (which uses
// the converters to serialize Path values as plain strings in INI files)
static const int qMetaTypeInitializer_ = [] {
    constexpr auto name = "Coco::Path";

    qRegisterMetaType<Coco::Path>(name);
    QMetaType::registerConverter<Coco::Path, QString>(
        [](const Coco::Path& p) { return p.toQString(); });
    QMetaType::registerConverter<QString, Coco::Path>(
        [](const QString& s) { return Coco::Path(s); });

    {
        auto id = QMetaType::fromName(name);
        qDebug() << "Coco::Path registered:" << id.isValid()
                 << "| id:" << id.id();
        auto v1 = QVariant::fromValue(Coco::Path("debug/test"));
        qDebug() << "Path -> QString:" << v1.canConvert<QString>()
                 << "| value:" << v1.value<QString>();
        auto v2 = QVariant::fromValue(QString("debug/test"));
        qDebug() << "QString -> Path:" << v2.canConvert<Coco::Path>()
                 << "| value:" << v2.value<Coco::Path>().toQString();
    }

    return 0;
}();
