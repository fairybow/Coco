#pragma once

#include <QColor>
#include <QList>
#include <QPixmap>
#include <Qt>

namespace Coco::Fx
{
    QPixmap greyscale(const QPixmap& pixmap);
    QList<QColor> goldenRatioColors(int count, const QColor& startColor = Qt::red);
}
