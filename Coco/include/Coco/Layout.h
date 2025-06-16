#pragma once

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMargins>
#include <Qt>
#include <QVBoxLayout>
#include <QWidget>

namespace Coco::Layout
{
    // TODO: make pointer required in template param (see: findParent)
    template <typename QLayoutT>
    inline QLayoutT* make
    (
        QMargins margins,
        int spacing,
        QWidget* parent = nullptr,
        Qt::Alignment alignment = {}
    )
    {
        auto layout = new QLayoutT(parent);
        layout->setContentsMargins(margins);
        layout->setSpacing(spacing);

        if (alignment != Qt::AlignmentFlag(0))
            layout->setAlignment(alignment);

        return layout;
    }

    // TODO: make pointer required in template param (see: findParent)
    template <typename QLayoutT>
    inline QLayoutT* zeroPadded(QWidget* parent = nullptr, Qt::Alignment alignment = {})
    {
        return make<QLayoutT>({}, 0, parent, alignment);
    }

} // Coco::Layout
