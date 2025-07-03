#pragma once

#include <concepts>
#include <type_traits>

#include <QObject>
#include <QWidget>

namespace Coco::Concepts
{
    template<typename T>
    concept Pointer = std::is_pointer_v<T>;

    template<typename BaseT, typename T>
    concept Derived = std::is_base_of_v<BaseT, T>;

    template<typename BaseT, typename T>
    concept DerivedPointer = Pointer<T> && Derived<BaseT, std::remove_pointer_t<T>>;

    template<typename T>
    concept QObjectDerived = Derived<QObject, T>;

    template<typename T>
    concept QObjectPointer = DerivedPointer<QObject, T>;

    template<typename T>
    concept QWidgetDerived = Derived<QWidget, T>;

    template<typename T>
    concept QWidgetPointer = DerivedPointer<QWidget, T>;

} // namespace Coco::Concepts
