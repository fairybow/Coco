#pragma once

#include <algorithm>
#include <type_traits>

#include <QList>
#include <QObject>
#include <QSet>

#include "Concepts.h"
#include "Debug.h"
#include "Global.h"

/*
* @brief I occasionally subclass solely in order to: 1) check memory via
* destructor debug output; and 2) have debug output show up as a custom class
* and not, say, QWidget or something unhelpful. Additionally, this can be useful
* for quickly creating a nested class that doesn't need the Q_OBJECT macro
*/
#define COCO_TRIVIAL_CLASS(Class, Base)                         \
    class Class : public Base                                   \
    {                                                           \
    public:                                                     \
        using Base::Base;                                       \
        virtual ~Class() override { COCO_TRACER; }              \
    }

/*
* @brief Since this adds the Q_OBJECT macro, it cannot be nested
*/
#define COCO_TRIVIAL_QCLASS(Class, Base)                        \
    class Class : public Base                                   \
    {                                                           \
        Q_OBJECT                                                \
                                                                \
    public:                                                     \
        using Base::Base;                                       \
        virtual ~Class() override { COCO_TRACER; }              \
    }

namespace Coco::Utility
{
    // This is dumb. Why did you do this?
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

    template <Concepts::QObjectPointer ParentT>
    inline ParentT findParent(QObject* object)
    {
        for (auto obj = object; obj; obj = obj->parent())
            if (auto parent = qobject_cast<ParentT>(obj))
                return parent;

        return nullptr;
    }

    template <Concepts::QObjectPointer ParentT>
    inline ParentT findParent(const QObject* object)
    {
        for (auto obj = object; obj; obj = obj->parent())
            if (auto parent = qobject_cast<ParentT>(obj))
                return parent;

        return nullptr;
    }

} // namespace Coco::Utility
