#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QString>
#include <QTextStream>

#include "../include/Coco/Global.h"
#include "../include/Coco/Io.h"
#include "../include/Coco/PathUtil.h"

constexpr auto PATH_NOT_FOUND = "Path not found:";
constexpr auto FAIL_OPEN_READ = "Failed to open file for reading:";
constexpr auto FAIL_OPEN_WRITE = "Failed to open file for writing:";
constexpr auto FAIL_WRITE_JSON = "Failed to write JSON to file:";
constexpr auto FAIL_WRITE_TXT = "Failed to write text to file:";

static void maybeCreateDirs_(const Coco::Path& path, Coco::CreateDirs createDirectories)
{
    if (!createDirectories) return;

    auto parent_path = path.parent();

    if (!parent_path.exists())
        Coco::PathUtil::mkdir(parent_path);
}

namespace Coco::Io::Json
{
    QJsonDocument read(const Path& path)
    {
        if (!path.exists())
        {
            qWarning() << PATH_NOT_FOUND << path;
            return {};
        }

        QFile file(path.toQString());

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << FAIL_OPEN_READ << path;
            return {};
        }

        QTextStream in(&file);
        auto string = in.readAll();

        QJsonParseError err{};
        auto document = QJsonDocument::fromJson(string.toUtf8(), &err);

        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << err.errorString();
            return {};
        }

        return document;
    }

    bool write(const QJsonDocument& document, const Path& path, CreateDirs createDirs)
    {
        if (path.isEmpty()) return false;
        maybeCreateDirs_(path, createDirs);

        QFile file(path.toQString());

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qWarning() << FAIL_OPEN_WRITE << path;
            return false;
        }

        QTextStream out(&file);
        out << document.toJson(QJsonDocument::Indented);
        auto success = out.status() == QTextStream::Ok;

        if (!success)
        {
            qWarning() << FAIL_WRITE_JSON << path;
            return false;
        }

        return true;
    }

} // namespace Coco::Io::Json

namespace Coco::Io::Txt
{
    QString read(const Path& path)
    {
        if (!path.exists())
        {
            qWarning() << PATH_NOT_FOUND << path;
            return {};
        }

        QFile file(path.toQString());

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << FAIL_OPEN_READ << path;
            return {};
        }

        QTextStream in(&file);
        auto text = in.readAll();

        return text;
    }

    bool write(const QString& text, const Path& path, CreateDirs createDirs)
    {
        if (path.isEmpty()) return false;
        maybeCreateDirs_(path, createDirs);

        QFile file(path.toQString());

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qWarning() << FAIL_OPEN_WRITE << path;
            return false;
        }

        QTextStream out(&file);
        out << text;
        auto success = out.status() == QTextStream::Ok;

        if (!success)
        {
            qWarning() << FAIL_WRITE_TXT << path;
            return false;
        }

        return true;
    }

} // namespace Coco::Io::Txt
