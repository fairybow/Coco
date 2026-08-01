/*
 * Coco — Common code for Qt projects
 * Copyright (C) 2025-2026 fairybow
 *
 * This program is free software, redistributable and/or modifiable under the
 * terms of the GNU GPL v3. It's distributed in the hope that it will be useful
 * but without any warranty (even the implied warranty of merchantability or
 * fitness for a particular purpose)
 *
 * See the LICENSE file or visit <https://www.gnu.org/licenses/>
 */

#include "Coco/Path.h"

#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QVariant>

// Registers Path with Qt's meta-type system and adds bidirectional QString
// converters, allowing Path to be stored in and retrieved from QVariant
static const int qMetaTypeInitializer_ = [] {
    constexpr auto name = "Coco::Path";

    qRegisterMetaType<Coco::Path>(name);
    QMetaType::registerConverter<Coco::Path, QString>(
        [](const Coco::Path& p) { return p.toQString(); });
    QMetaType::registerConverter<QString, Coco::Path>(
        [](const QString& s) { return Coco::Path(s); });

#ifdef QT_DEBUG

    auto id = QMetaType::fromName(name);
    qDebug() << "Coco::Path registered:" << id.isValid() << "| id:" << id.id();
    auto v1 = QVariant::fromValue(Coco::Path("debug/test"));
    qDebug() << "Path -> QString:" << v1.canConvert<QString>()
             << "| value:" << v1.value<QString>();
    auto v2 = QVariant::fromValue(QString("debug/test"));
    qDebug() << "QString -> Path:" << v2.canConvert<Coco::Path>()
             << "| value:" << v2.value<Coco::Path>().toQString();

#endif

    return 0;
}();
