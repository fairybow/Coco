#pragma once

#include <algorithm>
#include <type_traits>

#include <QList>
#include <QObject>
#include <QSet>

#include "Concepts.h"
#include "Global.h"

#define COCO_TEMPLATE_PTR_ASSERT(T)                                                             \
    static_assert(std::is_pointer_v<T>, "Template parameter " #T " must be a pointer type!");

#define COCO_DERIVED_ASSERT(BaseT, T)                                                           \
    static_assert(std::is_base_of_v<BaseT, T>, #T " must be " #BaseT " or " #BaseT "-derived!")

#define COCO_QOBJECT_ASSERT(T)                                                                  \
    COCO_DERIVED_ASSERT(QObject, T)

/// @brief I occasionally subclass solely in order to: 1) check memory via
/// destructor debug output; and 2) have debug output show up as a custom class
/// and not, say, QWidget or something unhelpful. Additionally, this can be
/// useful for quickly creating a nested class that doesn't need the Q_OBJECT
/// macro
#define COCO_TRIVIAL_CLASS(Class, Base)                         \
    class Class : public Base                                   \
    {                                                           \
    public:                                                     \
        using Base::Base;                                       \
        virtual ~Class() override { COCO_TRACER; }              \
    }

namespace Coco::Utility
{
    template <typename T>
    inline void sort(QList<T>& list)
    {
        std::sort(list.begin(), list.end());
    }

    template <typename T>
    inline QList<T> toList(const QSet<T>& set, Sort shouldSort = Sort::No)
    {
        QList<T> list{ set.begin(), set.end() };
        if (shouldSort) sort<T>(list);
        return list;
    }

    template <Utility::QObjectPointer ParentT>
    inline ParentT findParent(QObject* object)
    {
        //COCO_TEMPLATE_PTR_ASSERT(ParentT);
        //COCO_QOBJECT_ASSERT(std::remove_pointer_t<ParentT>);

        for (auto obj = object; obj; obj = obj->parent())
            if (auto parent = qobject_cast<ParentT>(obj))
                return parent;

        return nullptr;
    }

    template <typename ParentT>
    inline ParentT findParent(const QObject* object)
    {
        COCO_TEMPLATE_PTR_ASSERT(ParentT);
        COCO_QOBJECT_ASSERT(std::remove_pointer_t<ParentT>);

        for (auto obj = object; obj; obj = obj->parent())
            if (auto parent = qobject_cast<ParentT>(obj))
                return parent;

        return nullptr;
    }

} // namespace Coco::Utility
