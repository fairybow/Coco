#pragma once

#include <QJsonDocument>
#include <QString>

#include "Global.h"
#include "Path.h"

namespace Coco::Io::Json
{
    QJsonDocument read(const Path& path);
    bool write(const QJsonDocument& document, const Path& path, CreateDirs createDirs = CreateDirs::Yes);
}

namespace Coco::Io::Txt
{
    QString read(const Path& path);
    bool write(const QString& text, const Path& path, CreateDirs createDirs = CreateDirs::Yes);
}
