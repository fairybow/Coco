/*
* Coco: Io.cpp  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-03-13
*/

#include "../include/Coco/CocoGlobal.h"
#include "../include/Coco/Io.h"
#include "../include/Coco/Path.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QFlags>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QList>
#include <QString>
#include <QTextStream>

struct Data_
{
    Coco::FileType type{};
    QByteArray signature{};
    int offset = 0;
};

// https://en.wikipedia.org/wiki/List_of_file_signatures
static const QList<Data_> DATA_ =
{
    // For zip, there are 3 signatures, but they all share the same first 2
    // bytes. And, to be fair, we could probably check only the first 3 or so
    // bytes for any signature. I don't see many sharing similar first bytes.
    // .docx is a .zip file!
    { Coco::Png,        QByteArray::fromHex("89504E470D0A1A0A") },  // 8 bytes
    { Coco::SevenZip,   QByteArray::fromHex("377ABCAF271C") },      // 6
    { Coco::Rtf,        QByteArray::fromHex("7B5C72746631") },      // 6
    { Coco::TarXz,      QByteArray::fromHex("FD377A585A00") },      // 6
    { Coco::Pdf,        QByteArray::fromHex("255044462D") },        // 5
    { Coco::Tar,        QByteArray::fromHex("7573746172"), 257 },   // 5
    { Coco::Gif,        QByteArray::fromHex("47494638") },          // 4
    { Coco::Jpg,        QByteArray::fromHex("FFD8FF") },            // 3
    { Coco::Utf8Bom,    QByteArray::fromHex("EFBBBF") },            // 3
    { Coco::Zip,        QByteArray::fromHex("504B") },              // 2
    { Coco::TarGz,      QByteArray::fromHex("1F8B") }               // 2
};

static void maybeCreateDirs_(const Coco::Path& path, Coco::CreateDirs createDirectories)
{
    if (createDirectories != Coco::CreateDirs::Yes) return;

    auto parent_path = path.parent();

    if (!parent_path.isValid())
        Coco::Path::mkdir(parent_path);
}

Coco::FileType Coco::Io::fileType(const Path& path, FileTypes filter)
{
    // If the caller does not provide any possible types (or intentionally
    // passes UnknownOrUtf8), compute the union of all types based on PAIRS_.
    if (filter == UnknownOrUtf8)
    {
        static const auto all = []
            {
                FileTypes types{};

                for (const auto& datum : DATA_)
                    types |= datum.type;

                return types;
            }();

        filter = all;
    }

    QFile file(path.toQString());
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open file:" << path.toQString();
        return UnknownOrUtf8;
    }

    // Determine max signature length and offset required
    auto length = 0;
    auto offset = 0;

    for (const auto& datum : DATA_)
    {
        if (filter & datum.type)
        {
            length = qMax(length, datum.signature.size());
            offset = qMax(offset, datum.offset);
        }
    }

    // Read file header
    auto maxlen = static_cast<qint64>(offset + length);
    auto file_header = file.read(maxlen);

    // Check against available signatures
    for (const auto& datum : DATA_)
    {
        // Ensure that all flags in datum.type are present in filter.
        if ((datum.type & filter) != datum.type) continue;

        auto sig_size = datum.signature.size();
        auto slice = file_header.mid(datum.offset, sig_size);

        if (slice == datum.signature)
            return datum.type;
    }

    return UnknownOrUtf8;
}

QString Coco::Io::readTxt(const Path& path)
{
    QString text{};
    QFile file(path.toQString());

    if (file.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        text = in.readAll();
    }
    else
    {
        qDebug()
            << __FUNCTION__
            << "Failed to read text file.";
    }

    return text;
}

bool Coco::Io::writeTxt
(
    const Path& path,
    const QString& text,
    CreateDirs createDirectories
)
{
    if (path.isEmpty())
        return false;

    maybeCreateDirs_(path, createDirectories);

    QFile file(path.toQString());

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << text;

        return true;
    }

    qDebug()
        << __FUNCTION__
        << "Failed to write text file.";

    return false;
}

QJsonDocument Coco::Io::readJson(const Path& path)
{
    QFile file(path.toQString());

    if (file.open(QFile::ReadOnly | QIODevice::Text))
    {
        QString text{};
        QTextStream in(&file);
        text = in.readAll();

        QJsonParseError err{};
        auto json = QJsonDocument::fromJson(text.toUtf8(), &err);

        if (err.error != QJsonParseError::NoError)
        {
            qDebug()
                << __FUNCTION__
                << "JSON parse error:"
                << err.errorString();

            return {};
        }

        return json;
    }

    qDebug()
        << __FUNCTION__
        << "Failed to read JSON file.";

    return {};
}

bool Coco::Io::writeJson
(
    const Path& path,
    const QJsonDocument& document,
    CreateDirs createDirectories
)
{
    if (path.isEmpty())
        return false;

    maybeCreateDirs_(path, createDirectories);

    QFile file(path.toQString());

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream out(&file);
        out << document.toJson(QJsonDocument::Indented);

        return true;
    }

    qDebug()
        << __FUNCTION__
        << "Failed to write JSON file.";

    return false;
}
