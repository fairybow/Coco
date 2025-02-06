/*
* Coco: Io.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-2-5
*/

#pragma once

#include "Path.h"

#include <QJsonDocument>
#include <QString>

namespace Coco::Io
{
    enum class CreateDirs { No = 0, Yes };

    QString readTxt(const Path& path);

    bool writeTxt
    (
        const Path& path,
        const QString& text = {},
        CreateDirs createDirectories = CreateDirs::Yes
    );

    // May return a null document
    QJsonDocument readJson(const Path& path);

    bool writeJson
    (
        const Path& path,
        const QJsonDocument& document = {},
        CreateDirs createDirectories = CreateDirs::Yes
    );

} // namespace Coco::Io
