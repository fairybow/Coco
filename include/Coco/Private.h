/*
* Coco: Private.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* Updated: 2025-2-5
*/

#pragma once

#include "Global.h"

BEGIN_COCO_NAMESPACE

// Will I be embarrassed about this later?
template <typename PublicT>
class Private
{
public:
    Private(PublicT* publicClass = nullptr)
        : m_public(publicClass)
    {
    }

    // NTS: not needed, but a reminder that this is not to be inherited
    ~Private() = default;

protected:
    PublicT* publicClass() const noexcept
    {
        return m_public;
    }

private:
    PublicT* m_public;

}; // class Coco::Private

END_COCO_NAMESPACE
