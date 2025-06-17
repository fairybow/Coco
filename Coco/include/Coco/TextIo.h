#pragma once

#include <QJsonDocument>
#include <QString>
#include <QStringConverter>

#include "Global.h"
#include "Path.h"

namespace Coco::TextIo
{
    QString read(const Path& path, QStringConverter::Encoding encoding = QStringConverter::Utf8);

    bool write
    (
        const QString& text,
        const Path& path,
        CreateDirs createDirs = CreateDirs::Yes,
        QStringConverter::Encoding encoding = QStringConverter::Utf8
    );
}

namespace Coco::TextIo::Json
{
    QJsonDocument read(const Path& path);

    bool write
    (
        const QJsonDocument& document,
        const Path& path,
        CreateDirs createDirs = CreateDirs::Yes
    );
}
