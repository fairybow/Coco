#pragma once

#include <algorithm>

#include <QList>
#include <QObject>
#include <QSet>

#include "Global.h"

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
    inline ParentT* findParent(QObject* object)
    {
        ParentT* parent = nullptr;

        for (auto obj = object; obj; obj = obj->parent())
        {
            if (auto next = qobject_cast<ParentT*>(obj))
            {
                parent = next;
                break;
            }
        }

        return parent;
    }

} // namespace Coco::Utility
