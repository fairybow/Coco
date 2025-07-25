#pragma once

#include <type_traits>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QMargins>
#include <Qt>
#include <QVBoxLayout>
#include <QWidget>

#include "Concepts.h"

namespace Coco::Layout
{
    template<typename T>
    concept QLayoutPointer = Concepts::DerivedPointer<QLayout, T>;

    template <QLayoutPointer T>
    inline T make(QWidget* target, Qt::Alignment alignment = Qt::AlignCenter)
    {
        auto layout = new std::remove_pointer_t<T>(target);
        layout->setAlignment(alignment);
        return layout;
    }

    template <QLayoutPointer T>
    inline T make(Qt::Alignment alignment = Qt::AlignCenter)
    {
        return make<T>(nullptr, alignment);
    }

    template <QLayoutPointer T>
    inline T make
    (
        QMargins margins,
        int spacing,
        QWidget* target = nullptr,
        Qt::Alignment alignment = Qt::AlignCenter
    )
    {
        auto layout = make<T>(target, alignment);
        layout->setContentsMargins(margins);
        layout->setSpacing(spacing);
        return layout;
    }

    template <QLayoutPointer T>
    inline T make
    (
        int equalMargins,
        int spacing,
        QWidget* target = nullptr,
        Qt::Alignment alignment = Qt::AlignCenter
    )
    {
        return make<T>
        (
            { equalMargins, equalMargins, equalMargins, equalMargins },
            spacing,
            target,
            alignment
        );
    }

    template <QLayoutPointer T>
    inline T makeDense(QWidget* target, Qt::Alignment alignment = Qt::AlignCenter)
    {
        return make<T>(QMargins{}, 0, target, alignment);
    }

    template <QLayoutPointer T>
    inline T makeDense(Qt::Alignment alignment = Qt::AlignCenter)
    {
        return make<T>(QMargins{}, 0, nullptr, alignment);
    }

} // Coco::Layout
