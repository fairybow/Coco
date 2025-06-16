#pragma once

#include <algorithm>
#include <type_traits>

#include <QList>
#include <QObject>
#include <QSet>

#include "Global.h"

#define COCO_TEMPLATE_PTR_ASSERT(T)                                                             \
    static_assert(std::is_pointer_v<T>, "Template parameter must be a pointer type!");

#define COCO_TEMPLATE_DERIVED_ASSERT(BaseT, T)                                                  \
    static_assert(std::is_base_of_v<BaseT, T>, #T " must be " #BaseT " or " #BaseT "-derived!")

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

    template <typename ParentT>
    inline ParentT findParent(QObject* object)
    {
        COCO_TEMPLATE_PTR_ASSERT(ParentT);

        for (auto obj = object; obj; obj = obj->parent())
            if (auto parent = qobject_cast<ParentT>(obj))
                return parent;

        return nullptr;
    }

} // namespace Coco::Utility
