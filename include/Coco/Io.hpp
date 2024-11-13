/*
* Coco/Io.hpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-12
*/

#pragma once

#include "../../src/CocoGlobal.hpp"
#include "Path.h"

#include <QFile>
#include <QString>
#include <QTextStream>

BEGIN_COCO_NAMESPACE

namespace Io
{
    enum class CreateDirs { No = 0, Yes };

    inline QString readTxt(const Path& path)
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

    inline bool writeTxt
    (
        const Path& path,
        QString text = {},
        CreateDirs createDirectories = CreateDirs::Yes
    )
    {
        if (createDirectories == CreateDirs::Yes)
        {
            auto parent_path = path.parent();

            if (!parent_path.isValid())
                Path::mkdir(parent_path);
        }

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

} // namespace Coco::Io

END_COCO_NAMESPACE
