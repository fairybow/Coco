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
    concept QLayoutPointer = Concepts::Pointer<T> && Concepts::Derived<QLayout, std::remove_pointer_t<T>>;

    template <QLayoutPointer T>
    inline T make
    (
        QMargins margins,
        int spacing,
        QWidget* parent = nullptr,
        Qt::Alignment alignment = {}
    )
    {
        //COCO_TEMPLATE_PTR_ASSERT(QLayoutT);

        auto layout = new std::remove_pointer_t<T>(parent);
        layout->setContentsMargins(margins);
        layout->setSpacing(spacing);

        if (alignment != Qt::AlignmentFlag(0))
            layout->setAlignment(alignment);

        return layout;
    }

    template <QLayoutPointer T>
    inline T zeroPadded(QWidget* parent = nullptr, Qt::Alignment alignment = {})
    {
        //COCO_TEMPLATE_PTR_ASSERT(QLayoutT);
        return make<T>({}, 0, parent, alignment);
    }

} // Coco::Layout
