#pragma once

#include "Macros.h"

/// @brief The general point is to have a convenient way to avoid ambiguous
/// boolean function parameters. Creates a strongly-typed, named, boolean class
/// with Yes/No static constants and implicit conversion for natural use in
/// conditionals.
///
/// @note Default Value: defaults can be specified in two equivalent ways:
/// `FunctionName(ParamType = ParamType::No)` or `FunctionName(ParamType = {})`.
///
/// @note Performance: The template-based implementation compiles to the same
/// machine code as raw booleans after optimization. All operations are marked
/// constexpr and COCO_ALWAYS_INLINE, ensuring the compiler optimizes away any
/// overhead. Boolean checks reduce to simple comparisons at the assembly level,
/// with zero runtime cost compared to using primitive bool types directly.
///
/// @example
///     // Define a new boolean type
///     COCO_BOOL(CreateDirs);
///
///     // Use in function declaration
///     bool saveFile(const Path& path, CreateDirs createDirs = CreateDirs::No);
///
///     // Usage examples
///     saveFile(path);                     // Uses default (No)
///     saveFile(path, CreateDirs::Yes);    // Explicitly set to Yes
///     saveFile(path, {});                 // Alternative way to use default
namespace Coco
{
    template <typename TagT>
    class Bool
    {
    public:
        COCO_ALWAYS_INLINE constexpr Bool() : value_(false) {}
        COCO_ALWAYS_INLINE constexpr explicit Bool(bool v) : value_(v) {}
        COCO_ALWAYS_INLINE constexpr operator bool() const { return value_; }

        static const Bool Yes;
        static const Bool No;

        // Doesn't work (may be MSVC?):
        // static constexpr Bool<TagT> Yes{ true };
        // static constexpr Bool<TagT> No{ false };

    private:
        bool value_;

    }; // class Coco::Bool

    // Define the static members outside the class to ensure they're properly
    // instantiated
    template <typename TagT>
    const Bool<TagT> Bool<TagT>::Yes{true};

    template <typename TagT>
    const Bool<TagT> Bool<TagT>::No{false};

} // namespace Coco

#define COCO_BOOL(Name) \
    struct Name##Tag    \
    {                   \
    };                  \
    using Name = Coco::Bool<Name##Tag>
