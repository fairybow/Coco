/*
* Coco: Io.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-2-5
*/

#include "../include/Coco/Io.h"
#include "Debug.h"

#include <QFile>
#include <QJsonParseError>
#include <QTextStream>

using namespace Coco;

//------------------------------------------------------------
// Internal
//------------------------------------------------------------

static void _maybeCreateDirs(const Path& path, Io::CreateDirs createDirectories)
{
    if (createDirectories == Io::CreateDirs::Yes)
    {
        auto parent_path = path.parent();

        if (!parent_path.isValid())
            Path::mkdir(parent_path);
    }
}

//------------------------------------------------------------
// Plain text
//------------------------------------------------------------

QString Io::readTxt(const Path& path)
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
        qDebug(log)
            << __FUNCTION__
            << "Failed to read text file.";
    }

    return text;
}

bool Io::writeTxt
(
    const Path& path,
    const QString& text,
    CreateDirs createDirectories
)
{
    _maybeCreateDirs(path, createDirectories);

    QFile file(path.toQString());

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << text;

        return true;
    }

    qDebug(log)
        << __FUNCTION__
        << "Failed to write text file.";

    return false;
}

//------------------------------------------------------------
// JSON
//------------------------------------------------------------

QJsonDocument Io::readJson(const Path& path)
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
            qDebug(log)
                << __FUNCTION__
                << "JSON parse error:"
                << err.errorString();

            return {};
        }

        return json;
    }

    qDebug(log)
        << __FUNCTION__
        << "Failed to read JSON file.";

    return {};
}

bool Io::writeJson
(
    const Path& path,
    const QJsonDocument& document,
    CreateDirs createDirectories
)
{
    _maybeCreateDirs(path, createDirectories);

    QFile file(path.toQString());

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream out(&file);
        out << document.toJson(QJsonDocument::Indented);

        return true;
    }

    qDebug(log)
        << __FUNCTION__
        << "Failed to write JSON file.";

    return false;
}
