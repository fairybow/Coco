/*
* Coco/Io.h  Copyright (C) 2024  fairybow
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

#include <QString>

BEGIN_COCO_NAMESPACE

namespace Io
{
    enum class CreateDirs { No = 0, Yes };

    inline QString readTxt(const Path& path);

    inline bool writeTxt
    (
        const Path& path,
        QString text = {},
        CreateDirs createDirectories = CreateDirs::Yes
    );

} // namespace Coco::Io

END_COCO_NAMESPACE
