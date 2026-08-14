/*
 * Coco — Common code for Qt projects
 * Copyright (C) 2025-2026 fairybow
 *
 * This program is free software, redistributable and/or modifiable under the
 * terms of the GNU GPL v3. It's distributed in the hope that it will be useful
 * but without any warranty (even the implied warranty of merchantability or
 * fitness for a particular purpose)
 *
 * See the LICENSE file or visit <https://www.gnu.org/licenses/>
 */

#pragma once

#include <QObject>
#include <QVariant>

#include "Coco/Concepts.h"

// Shorthand for getting the sender of a signal inside a slot. Via Qt: "Warning:
// This function violates the object-oriented principle of modularity. However,
// getting access to the sender might be useful when many signals are connected
// to a single slot."
// TODO: Prefix with cc_?
#define qSender(T) qobject_cast<T>(QObject::sender())

// idc I'm the only one using this lol
// TODO: Prefix with cc_?
#define qVar(Value) QVariant::fromValue(Value)

// I occasionally subclass solely in order to: 1) check memory via destructor
// debug output; and 2) have debug output show up as a custom class and not,
// say, QWidget or something unhelpful. Additionally, this can be useful for
// quickly creating a nested class that doesn't need the Q_OBJECT macro
#define COCO_TRIVIAL_CLASS(Class, Base)                                        \
    class Class : public Base                                                  \
    {                                                                          \
    public:                                                                    \
        using Base::Base;                                                      \
    }

// Since this adds the Q_OBJECT macro, it cannot be nested
#define COCO_TRIVIAL_QCLASS(Class, Base)                                       \
    class Class : public Base                                                  \
    {                                                                          \
        Q_OBJECT                                                               \
                                                                               \
    public:                                                                    \
        using Base::Base;                                                      \
    }

namespace Coco {

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
    return findParent<ParentT>(const_cast<QObject*>(object));
}

} // namespace Coco
