#pragma once

#include <QDebug>
#include <QString>

#include "Macros.h"

// @brief The general point is to have a convenient way to avoid ambiguous
// boolean function parameters. Creates a strongly-typed, named boolean class
// with Yes/No static constants that implicitly converts to bool for natural
// use.
//
// @note Default value is false, but using CocoBool{} or {} sorta defeats the
// purpose...
//
// @note Performance: The template-based implementation compiles to the same
// machine code as raw booleans after optimization. All operations are marked
// constexpr and COCO_ALWAYS_INLINE, ensuring (hopefully) zero runtime overhead.
//
// EXAMPLE:
// ```cpp
// // Ambiguous boolean parameters:
// void saveFile(const Path& path, bool createDirs, bool overwrite);
// saveFile(path, true, false);  // What do these mean?
//
// // Instead:
// COCO_BOOL(CreateDirs);
// COCO_BOOL(Overwrite);
// void saveFile(const Path& path, CreateDirs createDirs = CreateDirs::No,
//               Overwrite overwrite = Overwrite::No);
//
// // Self-documenting calls:
// saveFile(path, CreateDirs::Yes, Overwrite::No);
//
// CreateDirs shouldCreate = CreateDirs::Yes;
// if (shouldCreate) {
//   // create directories
// }
//
// // Implicitly converts to bool:
// COCO_BOOL(EnableLogging);
// auto logging = EnableLogging::Yes;
// void libraryFunction(bool enable);
// libraryFunction(logging);
//
// // Each type is distinct:
// TypeA a = TypeA::Yes;
// TypeB b = TypeB::Yes;
// qDebug() << (a == b);  // Compile error
// ```
namespace Coco
{
    template <typename TagT>
    class Bool
    {
    public:
        static const Bool Yes;
        static const Bool No;

        COCO_ALWAYS_INLINE friend QDebug operator<<(QDebug debug, const Bool& b) { return debug << qUtf8Printable(name_(b)); }
        COCO_ALWAYS_INLINE constexpr operator bool() const { return value_; }
        COCO_ALWAYS_INLINE constexpr Bool operator!() const { return value_ ? No : Yes; }

        // Allow comparison with same type only
        template<typename OtherTagT>
        constexpr bool operator==(const Bool<OtherTagT>& other) const = delete;
        COCO_ALWAYS_INLINE constexpr bool operator==(const Bool<TagT>& other) const = default;

        // Anything else?

    private:
        bool value_;

        Bool() = delete;
        //COCO_ALWAYS_INLINE constexpr Bool() : value_(false) {}
        COCO_ALWAYS_INLINE constexpr explicit Bool(bool value) : value_(value) {}
        COCO_ALWAYS_INLINE static QString name_(const Bool& b) { return QString(TagT::name()) + "::" + (b.value_ ? "Yes" : "No"); }

    }; // class Coco::Bool

    // Define the static members outside the class (see above)
    template <typename TagT>
    const Bool<TagT> Bool<TagT>::Yes{ true };

    template <typename TagT>
    const Bool<TagT> Bool<TagT>::No{ false };

} // namespace Coco

#define COCO_BOOL(Name)                             \
    struct Name##Tag {                              \
        static constexpr const char* name() {       \
            return #Name;                           \
        }                                           \
    };                                              \
    using Name = Coco::Bool<Name##Tag>              \

// Old, notes, etc:

// Macro to make an inheriting struct with custom type names
// #define COCO_CUSTOM_BOOL(Name, YesVal, NoVal)

// Doesn't work (may be MSVC?):
// static constexpr Bool<TagT> Yes{ true };
// static constexpr Bool<TagT> No{ false };

// Would these work? Can't remember if I tried...
// static inline const Bool Yes{ true };
// static inline const Bool No{ false };
