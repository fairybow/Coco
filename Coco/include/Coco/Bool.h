/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE and
 * ADDITIONAL_TERMS files, or visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#pragma once

#include <format>
#include <string>

#include <QDebug>
#include <QString>

// The general point is to have a convenient way to avoid ambiguous boolean
// function parameters. Creates a strongly-typed, named boolean class with
// Yes/No static constants that implicitly converts to bool for natural use. If
// the function has only one parameter (bool), then the function name itself
// should make it clear what that parameter is for (meaning this class is
// unnecessary there)
//
// Performance: The template-based implementation compiles to the same machine
// code as raw booleans after optimization
//
// clang-format off
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
//
// clang-format on
namespace Coco {
template <typename TagT> class Bool
{
public:
    static const Bool Yes;
    static const Bool No;

    friend QDebug operator<<(QDebug debug, const Bool& b)
    {
        return debug << qUtf8Printable(name_(b));
    }

    constexpr operator bool() const { return value_; }
    constexpr Bool operator!() const { return value_ ? No : Yes; }
    constexpr bool operator==(const Bool<TagT>& other) const = default;

    // Allow comparison with same type only
    template <typename OtherTagT>
    constexpr bool operator==(const Bool<OtherTagT>& other) const = delete;

    // TODO: Anything else?

private:
    bool value_;

    // Default value could be false but then that defeats the purpose (back to
    // ambiguous bools)
    Bool() = delete;

    constexpr explicit Bool(bool value)
        : value_(value)
    {
    }

    static QString name_(const Bool& b)
    {
        return QString::fromUtf8(
            std::format("{}::{}", TagT::name(), b.value_ ? "Yes" : "No"));
    }
};

// Define the static members outside the class (see above)
template <typename TagT> const Bool<TagT> Bool<TagT>::Yes{ true };
template <typename TagT> const Bool<TagT> Bool<TagT>::No{ false };

#define COCO_BOOL(Name)                                                        \
    struct Name##Tag                                                           \
    {                                                                          \
        static constexpr const char* name() { return #Name; }                  \
    };                                                                         \
    using Name = Coco::Bool<Name##Tag>

} // namespace Coco

template <typename TagT>
struct std::formatter<Coco::Bool<TagT>> : std::formatter<std::string>
{
    auto format(const Coco::Bool<TagT>& b, std::format_context& ctx) const
    {
        return std::format_to(
            ctx.out(),
            "{}::{}",
            TagT::name(),
            b ? "Yes" : "No");
    }
};

// Old, notes, etc:

// Macro to make an inheriting struct with custom type names
// #define COCO_CUSTOM_BOOL(Name, YesVal, NoVal)

// Doesn't work (may be MSVC?):
// static constexpr Bool<TagT> Yes{ true };
// static constexpr Bool<TagT> No{ false };

// Would these work? Can't remember if I tried...
// static inline const Bool Yes{ true };
// static inline const Bool No{ false };
