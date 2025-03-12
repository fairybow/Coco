/*
* Coco: Private.h  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* Updated: 2025-03-12
*/

#pragma once

namespace Coco
{
    template <typename PublicT>
    class Private
    {
    public:
        Private(PublicT* publicClass = nullptr)
            : public_(publicClass)
        {
        }

        ~Private() = default;

    protected:
        PublicT* publicClass() const noexcept
        {
            return public_;
        }

    private:
        PublicT* public_;

    }; // class Coco::Private

} // namespace Coco
