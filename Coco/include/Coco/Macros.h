#pragma once

#include <QObject>

/// @note I think this is maybe pointless, but also it might matter more that
/// any function using this is not itself making function calls to stuff that is
/// not inlined. This is for small functions returning members, basically.
#ifdef __GNUC__
#define COCO_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define COCO_ALWAYS_INLINE __forceinline
#else
#define COCO_ALWAYS_INLINE inline
#endif

/// @brief Shorthand for getting the sender of a signal inside a slot.
/// 
/// @note Via Qt: "Warning: This function violates the object-oriented principle
/// of modularity. However, getting access to the sender might be useful when
/// many signals are connected to a single slot."
#define COCO_qSender(T) qobject_cast<T>(QObject::sender())

/// @brief Creates a strongly typed boolean enumeration with Yes/No values,
/// along with a convenience function, isYes(), to check if a value equals Yes
/// in conditional statements.
//#define COCO_BOOL_ENUM(Name)                                                    \
//    enum class Name { No = 0, Yes };                                            \
//    COCO_ALWAYS_INLINE constexpr bool isYes(Name value) {                       \
//        return value == Name::Yes;                                              \
//    }

/// @brief (The purpose of this is to avoid ambiguous boolean parameters.)
/// Creates a strongly typed boolean struct with Yes/No static constants,
/// providing implicit conversion to bool for natural use in conditions while
/// maintaining type safety between different boolean concepts. As function
/// parameters, both the following can be used to default to false:
/// `Coco::NamedBool = Coco::NamedBool::No`, `Coco::NamedBool = {}`.
/// 
/// @note Performance comparison with COCO_BOOL_ENUM: Despite the more complex
/// implementation, this approach compiles to essentially identical machine code
/// as COCO_BOOL_ENUM after optimization. The bool conversion operator and
/// static constants are all marked constexpr and inline, allowing the compiler
/// to optimize away any overhead. In both cases, the check reduces to a simple
/// comparison at the assembly level.
//#define COCO_BOOL(Name)                                                         \
//    struct Name                                                                 \
//    {                                                                           \
//        bool value;                                                             \
//        static const Name Yes;                                                  \
//        static const Name No;                                                   \
//        COCO_ALWAYS_INLINE constexpr operator bool() const { return value; }    \
//        COCO_ALWAYS_INLINE constexpr explicit Name(bool v) : value(v) {}        \
//        COCO_ALWAYS_INLINE constexpr Name() : value(false) {}                   \
//    };                                                                          \
//    inline constexpr Name Name::No{ false };                                    \
//    inline constexpr Name Name::Yes{ true }
