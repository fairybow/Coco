#pragma once

#include <QColor>
#include <QList>
#include <QPixmap>
#include <Qt>

namespace Coco::Fx
{
    QPixmap toGreyscale(const QPixmap& pixmap);
    QList<QColor> goldenRatioColors(int count, const QColor& startColor = Qt::red);
    //double rec601Luminance(const QColor& color);

    // Qt's HSL lightness, 0-255 range
    inline bool isDark(const QColor& color) noexcept { return color.lightness() < 128; }
    inline bool isLight(const QColor& color) noexcept { return color.lightness() >= 128; }
}
