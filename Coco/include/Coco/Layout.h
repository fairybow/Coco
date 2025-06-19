#pragma once

#include <type_traits>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMargins>
#include <Qt>
#include <QVBoxLayout>
#include <QWidget>

#include "Utility.h"

namespace Coco::Layout
{
    template <typename QLayoutT>
    inline QLayoutT make
    (
        QMargins margins,
        int spacing,
        QWidget* parent = nullptr,
        Qt::Alignment alignment = {}
    )
    {
        COCO_TEMPLATE_PTR_ASSERT(QLayoutT);

        auto layout = new std::remove_pointer_t<QLayoutT>(parent);
        layout->setContentsMargins(margins);
        layout->setSpacing(spacing);

        if (alignment != Qt::AlignmentFlag(0))
            layout->setAlignment(alignment);

        return layout;
    }

    template <typename QLayoutT>
    inline QLayoutT zeroPadded(QWidget* parent = nullptr, Qt::Alignment alignment = {})
    {
        COCO_TEMPLATE_PTR_ASSERT(QLayoutT);
        return make<QLayoutT>({}, 0, parent, alignment);
    }

} // Coco::Layout
