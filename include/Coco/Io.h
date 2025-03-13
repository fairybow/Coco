/*
* Coco: Io.h  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-03-13
*/

#pragma once

#include "Path.h"

#include <QFlags>
#include <QJsonDocument>
#include <QString>

namespace Coco::Io
{
    enum class CreateDirs
    {
        No = 0,
        Yes
    };

    enum FileType
    {
        UnknownOrUtf8   = 0,
        Png             = 1 << 0,
        SevenZip        = 1 << 1,
        Pdf             = 1 << 2,
        Jpg             = 1 << 3,
        Utf8Bom         = 1 << 4
    };

    typedef QFlags<FileType> FileTypes;

    /// @brief Determines the file type based on its signature.
    /// @param filter Limit the types checked.
    FileType fileType(const Path& path, FileTypes filter = UnknownOrUtf8);

    /// @brief Reads the content of a text file.
    QString readTxt(const Path& path);

    /// @brief Writes text to a file.
    bool writeTxt
    (
        const Path& path,
        const QString& text = {},
        CreateDirs createDirectories = CreateDirs::Yes
    );

    /// @brief Reads the content of a JSON file.
    /// @details May return a null document.
    QJsonDocument readJson(const Path& path);

    /// @brief Writes a JSON document to a file.
    bool writeJson
    (
        const Path& path,
        const QJsonDocument& document = {},
        CreateDirs createDirectories = CreateDirs::Yes
    );

} // namespace Coco::Io

Q_DECLARE_OPERATORS_FOR_FLAGS(Coco::Io::FileTypes)
