#pragma once

namespace Coco::Concepts
{
    template<typename T>
    concept Pointer = std::is_pointer_v<T>;

    template<typename T>
    concept QObjectDerived = std::is_base_of_v<QObject, T>;

    template<typename T>
    concept QObjectPointer = Pointer<T> && QObjectDerived<std::remove_pointer_t<T>>;

} // namespace Coco::Concepts
