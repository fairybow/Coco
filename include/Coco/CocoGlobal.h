/*
* Coco: CocoGlobal.h  Copyright (C) 2025  fairybow
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

#include <QFlags>

namespace Coco
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
        Rtf             = 1 << 2,
        TarXz           = 1 << 3,
        Pdf             = 1 << 4,
        Tar             = 1 << 5,
        Gif             = 1 << 6,
        Jpg             = 1 << 7,
        Utf8Bom         = 1 << 8,
        Zip             = 1 << 9,
        TarGz           = 1 << 10
    };

    typedef QFlags<FileType> FileTypes;

} // namespace Coco

Q_DECLARE_OPERATORS_FOR_FLAGS(Coco::FileTypes)
