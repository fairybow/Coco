/*#pragma once

#include <memory>

//#define COCO_DECLARE_PRIVATE(VarName) class Private; std::unique_ptr<Private> VarName
//#define COCO_INIT_PRIVATE(VarName) VarName(std::make_unique<Private>(this))

namespace Coco
{
    /// @class Coco::Private
    /// @brief A utility class for implementing the PIMPL (Pointer to
    /// Implementation) idiom.
    ///
    /// @note This class provides protected access to the public class instance
    /// via the publicClass() method, allowing the private implementation to
    /// interact with its owner. A Coco::Private class is meant to be nested in
    /// its public class (see example). It's also not meant to use private
    /// members itself, so bidirectional friendship isn't really necessary.
    /// 
    /// @example
    /// ```cpp
    /// // Widget.h:
    /// #include <memory>
    /// 
    /// class Widget
    /// {
    /// public:
    ///     Widget();
    /// 
    /// private:
    ///     class Private;
    ///     std::unique_ptr<Private> private_;
    /// };
    ///
    /// // WidgetPrivate.h:
    /// class Widget::Private : public Coco::Private<Widget>
    /// {
    /// public:
    ///     int data = 0;
    /// };
    /// 
    /// // Widget.cpp:
    /// #include "Widget.h"
    /// #include "WidgetPrivate.h"
    /// 
    /// Widget::Widget()
    ///     : private_(std::make_unique<Private>(this)) {}
    /// 
    /// //...
    /// 
    /// // Private definitions...
    /// ```
    template <typename PublicT>
    struct Private
    {
    public:
        explicit Private(PublicT* publicClass = nullptr)
            : public_(publicClass)
        {
        }

        virtual ~Private() = default;

    protected:
        PublicT* publicClass() const noexcept { return public_; }

    private:
        PublicT* public_;

    }; // struct Coco::Private

} // namespace Coco
*/