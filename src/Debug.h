/*
* Coco Debug.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-16
*/

#pragma once

#include <QDebug>
#include <QLoggingCategory>

// Q_LOGGING_CATEGORY
inline const QLoggingCategory& log()
{
    static const QLoggingCategory category("Coco");
    return category;
}
