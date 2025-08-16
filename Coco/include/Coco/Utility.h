#pragma once

#include <algorithm>
#include <type_traits>

#include <QCoreApplication>
#include <QEventLoop>
#include <QList>
#include <QObject>
#include <QSet>
#include <QTime>

#include "Concepts.h"
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

    // https://stackoverflow.com/a/11487434
    // Questionable
    inline void delay(unsigned int msecs)
    {
        auto die_time = QTime::currentTime().addMSecs(msecs);

        while (QTime::currentTime() < die_time)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

} // namespace Coco::Utility
