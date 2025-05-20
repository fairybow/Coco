#pragma once

#include <memory>

//#define COCO_PRIVATE(VarName) class Private; friend class Private; std::unique_ptr<Private> VarName
//#define COCO_INIT_PRIVATE(VarName) VarName(std::make_unique<Private>(this))

namespace Coco
{
    /// @class Coco::Private
    /// @brief A utility struct for implementing the PIMPL (Pointer to
    /// Implementation) idiom in libraries.
    ///
    /// @note This struct provides access to the public class instance via the
    /// protected "pub" member, allowing the private implementation to interact
    /// with its owner. A Coco::Private struct is meant to be nested in its
    /// public class (see example) and used as a full replacement for private
    /// members and member functions (to avoid binary compatibility problems;
    /// see: https://wiki.qt.io/D-Pointer). Since implementation details are the
    /// purpose of this struct, most members should be public.
    /// 
    /// @example
    /// ```cpp
    /// // include/Widget.h:
    /// #include <memory>
    /// #include "Coco/Private.h"
    /// 
    /// class Widget
    /// {
    /// public:
    ///     Widget();
    /// 
    /// private:
    ///     struct Private;
    ///     std::unique_ptr<Private> private_;
    /// };
    ///
    /// // src/WidgetPrivate.h:
    /// struct Widget::Private : public Coco::Private<Widget>
    /// {
    ///     // ...
    /// };
    /// 
    /// // src/Widget.cpp:
    /// #include "Widget.h"
    /// #include "WidgetPrivate.h"
    /// 
    /// Widget::Widget()
    ///     : private_(std::make_unique<Private>(this))
    /// {
    /// }
    /// 
    /// // Widget::Private definitions.
    /// ```
    template <typename PublicT>
    struct Private
    {
        /// @brief Constructs a Private implementation linked to its public
        /// class
        /// @param publicClass Pointer to the public class that owns this
        /// implementation
        /// 
        /// @note Without the explicit keyword, the base class constructor can
        /// be implicitly used for constructing derived classes.
        /*explicit*/ Private(PublicT* publicClass)
            : pub(publicClass)
        {
        }

        /// @note When the base class doesn't have a virtual destructor, the
        /// compiler generates a simple, non-virtual destructor for the derived
        /// class automatically. This simplifies the inheritance relationship.
        /*virtual*/ ~Private() = default;

    protected:
        /// @brief Pointer to the public class
        /// @note This pointer cannot be reassigned but can call non-const
        /// methods
        PublicT* const pub;

    }; // struct Coco::Private

} // namespace Coco
