#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QString>
#include <QStringConverter>
#include <QTextStream>

#include "../include/Coco/Global.h"
#include "../include/Coco/PathUtil.h"
#include "../include/Coco/TextIo.h"

constexpr auto PATH_NOT_FOUND = "Path not found:";
constexpr auto FAIL_OPEN_READ = "Failed to open file for reading:";
constexpr auto FAIL_OPEN_WRITE = "Failed to open file for writing:";
constexpr auto FAIL_WRITE_TXT = "Failed to write text to file:";
constexpr auto JSON_PARSE_ERROR = "JSON parse error:";

namespace Coco::TextIo
{
    QString read(const Path& path, QStringConverter::Encoding encoding)
    {
        if (!path.exists())
        {
            qWarning() << PATH_NOT_FOUND << path;
            return {};
        }

        QFile file(path.toQString());

        // QIODevice::Text automatically normalizes line endings. If we later
        // want to preserve original line endings, we'll need to remove that
        // flag and handle conversion manually
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << FAIL_OPEN_READ << path;
            return {};
        }

        QTextStream in(&file); // QStringDecoder for BOM handling?
        in.setEncoding(encoding);
        auto text = in.readAll();

        return text;
    }

    bool write
    (
        const QString& text,
        const Path& path,
        CreateDirs createDirs,
        QStringConverter::Encoding encoding
    )
    {
        if (path.isEmpty()) return false;

        if (createDirs)
        {
            auto parent_path = path.parent();

            if (!parent_path.exists())
                Coco::PathUtil::mkdir(parent_path);
        }

        QFile file(path.toQString());

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qWarning() << FAIL_OPEN_WRITE << path;
            return false;
        }

        QTextStream out(&file);
        out.setEncoding(encoding);
        out << text;
        auto success = out.status() == QTextStream::Ok;

        if (!success) qWarning() << FAIL_WRITE_TXT << path;

        return success;
    }

} // namespace Coco::TextIo

namespace Coco::TextIo::Json
{
    QJsonDocument read(const Path& path)
    {
        auto json_string = TextIo::read(path, QStringConverter::Utf8);
        if (json_string.isEmpty()) return {};

        QJsonParseError err{};
        auto document = QJsonDocument::fromJson(json_string.toUtf8(), &err);

        if (err.error != QJsonParseError::NoError)
        {
            qWarning() << JSON_PARSE_ERROR << err.errorString();
            return {};
        }

        return document;
    }

    bool write(const QJsonDocument& document, const Path& path, CreateDirs createDirs)
    {
        auto json_string = document.toJson(QJsonDocument::Indented);
        return TextIo::write(json_string, path, createDirs, QStringConverter::Utf8);
    }

} // namespace Coco::TextIo::Json
