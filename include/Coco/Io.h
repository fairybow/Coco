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

#include "CocoGlobal.h"
#include "Path.h"

#include <QFlags>
#include <QJsonDocument>
#include <QString>

namespace Coco::Io
{
    /// @brief Identifies the file type based on its signature.
    /// @param filter Specifies the file types to check against. For example,
    /// `Coco::Pdf | Coco::Png` will limit the checks to PDFs and PNGs. If no
    /// matching type is found, UnknownOrUtf8 is always returned.
    /// @details Allows exclusion of specific file types by applying a bitwise
    /// negation. For instance, use `Coco::FileTypes() & ~Coco::Pdf` to check
    /// all types except PDF.
    FileType fileType(const Path& path, FileTypes filter = UnknownOrUtf8);

    /// @brief Checks if the file matches the specified type.
    inline bool is(FileType type, const Path& path)
    {
        return fileType(path, type) == type;
    }

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
