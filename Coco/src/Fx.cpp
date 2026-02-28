/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE, or
 * visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#include "../include/Coco/Fx.h"

#include <cmath>
#include <numbers>

#include <QColor>
#include <QList>
#include <QtTypes>

namespace Coco::Fx
{
    // https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
    QList<QColor> goldenRatioColors(int count, const QColor& startColor)
    {
        QList<QColor> result{};
        if (count < 1) return result;

        double h = startColor.hsvHueF();

        // Different levels for variety
        QList<double> saturations = { 0.9, 0.7, 0.8 };
        QList<double> values = { 0.9, 0.8, 0.95 };

        constexpr double conjugate = 1.0 / std::numbers::phi;

        for (auto i = 0; i < count; ++i)
        {
            // Select saturation and value based on position
            double s = saturations[i % saturations.size()];
            double v = values[i % values.size()];

            result << QColor::fromHsvF(h, s, v);

            // Advance hue by golden ratio conjugate and keep in range [0, 1)
            h += conjugate;
            h = std::fmod(h, 1.0);
        }

        return result;
    }

} // namespace Coco::Fx
