#pragma once

#include <type_traits>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMargins>
#include <Qt>
#include <QVBoxLayout>
#include <QWidget>

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
        static_assert(std::is_pointer_v<QLayoutT>, "Template parameter must be a pointer type!");
        using Type = typename std::remove_pointer<QLayoutT>::type;

        auto layout = new Type(parent);
        layout->setContentsMargins(margins);
        layout->setSpacing(spacing);

        if (alignment != Qt::AlignmentFlag(0))
            layout->setAlignment(alignment);

        return layout;
    }

    template <typename QLayoutT>
    inline QLayoutT zeroPadded(QWidget* parent = nullptr, Qt::Alignment alignment = {})
    {
        return make<QLayoutT>({}, 0, parent, alignment);
    }

} // Coco::Layout
