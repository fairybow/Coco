/*
* Coco: NestedPrivate.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* Updated: 2024-12-10
*/

#pragma once

#include "Global.h"

BEGIN_COCO_NAMESPACE

// Will I be embarrassed about this later?
template <typename PublicT>
class NestedPrivate
{
public:
    NestedPrivate(PublicT* publicClass = nullptr)
        : m_public(publicClass)
    {
    }

protected:
    PublicT* publicClass()
    {
        return m_public;
    }

private:
    PublicT* m_public;

}; // class Coco::NestedPrivate

END_COCO_NAMESPACE
