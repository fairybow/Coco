/*
* Coco: Io.cpp  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-03-12
*/

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

//------------------------------------------------------------
// Internal
//------------------------------------------------------------

struct Pair_
{
    Coco::Io::FileType type{};
    QByteArray signature{};
};

// https://en.wikipedia.org/wiki/List_of_file_signatures
static const QList<Pair_> PAIRS_ =
{
    { Coco::Io::Png,       QByteArray::fromHex("89504E470D0A1A0A") },
    { Coco::Io::SevenZip,  QByteArray::fromHex("377ABCAF271C") },
    { Coco::Io::Pdf,       QByteArray::fromHex("255044462D") },
    { Coco::Io::Jpg,       QByteArray::fromHex("FFD8FF") },
    { Coco::Io::Utf8Bom,   QByteArray::fromHex("EFBBBF") }
};

static void maybeCreateDirs_(const Coco::Path& path, Coco::Io::CreateDirs createDirectories)
{
    if (createDirectories != Coco::Io::CreateDirs::Yes) return;

    auto parent_path = path.parent();

    if (!parent_path.isValid())
        Coco::Path::mkdir(parent_path);
}

Coco::Io::FileType Coco::Io::fileType(const Path& path, FileTypes types)
{
    // If the caller does not provide any types (or intentionally passes None),
    // compute the union of all types based on PAIRS.
    if (types == Unknown)
    {
        static const auto all = []
            {
                FileTypes types{};

                for (const auto& pair : PAIRS_)
                    types |= pair.type;

                return types;
            }();

        types = all;
    }

    QFile file(path.toQString());
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open file:" << path.toQString();
        return Unknown;
    }

    // Determine max signature length required
    auto length = 0;

    for (const auto& pair : PAIRS_)
        if (types & pair.type)
            length = qMax(length, pair.signature.size());

    // Read file header
    auto file_header = file.read(length);

    // Check against available signatures
    for (const auto& pair : PAIRS_)
        if ((types & pair.type) && file_header.startsWith(pair.signature))
            return pair.type;

    return Unknown;
}

//------------------------------------------------------------
// Plain text
//------------------------------------------------------------

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

//------------------------------------------------------------
// JSON
//------------------------------------------------------------

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
