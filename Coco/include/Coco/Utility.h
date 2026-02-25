/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE and
 * ADDITIONAL_TERMS files, or visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#pragma once

#include <QDebug>
#include <QObject>

#include "Concepts.h"

// Shorthand for getting the sender of a signal inside a slot.
//
// Via Qt: "Warning: This function violates the object-oriented principle of
// modularity. However, getting access to the sender might be useful when many
// signals are connected to a single slot."
#define qSender(T) qobject_cast<T>(QObject::sender())

#define COCO_TRACER qDebug() << __FUNCTION__

// I occasionally subclass solely in order to: 1) check memory via destructor
// debug output; and 2) have debug output show up as a custom class and not,
// say, QWidget or something unhelpful. Additionally, this can be useful for
// quickly creating a nested class that doesn't need the Q_OBJECT macro
#define COCO_TRIVIAL_CLASS(Class, Base)                                        \
    class Class : public Base                                                  \
    {                                                                          \
    public:                                                                    \
        using Base::Base;                                                      \
        virtual ~Class() override { COCO_TRACER; }                             \
    }

// Since this adds the Q_OBJECT macro, it cannot be nested
#define COCO_TRIVIAL_QCLASS(Class, Base)                                       \
    class Class : public Base                                                  \
    {                                                                          \
        Q_OBJECT                                                               \
                                                                               \
    public:                                                                    \
        using Base::Base;                                                      \
        virtual ~Class() override { COCO_TRACER; }                             \
    }

namespace Coco {

template <Concepts::QObjectPointer ParentT>
inline ParentT findParent(QObject* object)
{
    for (auto obj = object; obj; obj = obj->parent())
        if (auto parent = qobject_cast<ParentT>(obj)) return parent;

    return nullptr;
}

template <Concepts::QObjectPointer ParentT>
inline ParentT findParent(const QObject* object)
{
    for (auto obj = object; obj; obj = obj->parent())
        if (auto parent = qobject_cast<ParentT>(obj)) return parent;

    return nullptr;
}

} // namespace Coco
