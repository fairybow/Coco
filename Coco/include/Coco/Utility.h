#pragma once

#include <algorithm>

#include <QList>
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

} // namespace Coco::Utility
