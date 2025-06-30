#pragma once

#include <array>
#include <cstddef>
#include <utility>

#include <QColor>
#include <QImage>
#include <QLinearGradient>
#include <QList>
#include <QPixmap>
#include <QPointF>
#include <QRgb>
#include <QtGlobal>
#include <QtTypes>

// To-do:
// - SIMD?

namespace Coco::FxOp
{
    inline constexpr int clamp(int value)
    {
        return qBound(0, value, 255);
    }

    // Per-pixel operations, used with Fx::apply:

    inline auto greyscale = [](QRgb pixel)
        {
            auto grey = qGray(pixel);
            return qRgba(grey, grey, grey, qAlpha(pixel));
        };

    inline auto invert = [](QRgb pixel)
        {
            auto alpha = qAlpha(pixel);
            if (alpha < 1) return pixel; // Skip transparent pixels

            auto r = 255 - qRed(pixel);
            auto g = 255 - qGreen(pixel);
            auto b = 255 - qBlue(pixel);

            return qRgba(r, g, b, alpha);
        };

    // https://stackoverflow.com/questions/65344928/sepia-filter-inverting
    inline auto sepia = [](QRgb pixel)
        {
            auto alpha = qAlpha(pixel);
            if (alpha < 1) return pixel; // Skip transparent pixels

            auto r = qRed(pixel);
            auto g = qGreen(pixel);
            auto b = qBlue(pixel);

            auto sr = clamp(static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b));
            auto sg = clamp(static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b));
            auto sb = clamp(static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b));

            return qRgba(sr, sg, sb, alpha);
        };

    // Factories:

    /*
    * Brightness adjustment op factory
    *
    * Valid range: -255 to +255
    * - brightness(-255)   * Makes everything black
    * - brightness(-50)    * Darken significantly  
    * - brightness(-10)    * Slight darkening
    * - brightness(0)      * No change
    * - brightness(10)     * Slight brightening
    * - brightness(50)     * Brighten significantly
    * - brightness(255)    * Makes everything whit
    *
    * Practical range: -100 to +100
    */
    inline auto brightness = [](int adjustment)
        {
            adjustment = qBound(-255, adjustment, 255);

            return [adjustment](QRgb pixel)
                {
                    auto alpha = qAlpha(pixel);
                    if (alpha < 1) return pixel; // Skip transparent pixels

                    auto r = clamp(qRed(pixel) + adjustment);
                    auto g = clamp(qGreen(pixel) + adjustment);
                    auto b = clamp(qBlue(pixel) + adjustment);

                    return qRgba(r, g, b, alpha);
                };
        };

    /*
    * Contrast adjustment op factory
    *
    * Valid range: 0.0 to infinity (but practical limit around 4.0)
    *
    * - contrast(0.0)      * Everything becomes middle gray (128,128,128)
    * - contrast(0.5)      * Low contrast (washed out)
    * - contrast(1.0)      * No change
    * - contrast(1.5)      * Increased contrast
    * - contrast(2.0)      * High contrast
    * - contrast(3.0)      * Very high contrast (dramatic)
    *
    * Practical range: 0.1 to 3.0
    */
    inline auto contrast = [](double factor)
        {
            factor = qMax(0.0, factor);

            return [factor](QRgb pixel)
                {
                    auto alpha = qAlpha(pixel);
                    if (alpha < 1) return pixel; // Skip transparent pixels

                    constexpr auto apply_ = [](int value, double factor)
                        {
                            auto normalized = (value - 128) / 128.0;  // Force floating point division
                            normalized *= factor;
                            return clamp(static_cast<int>(normalized * 128 + 128));
                        };

                    auto r = apply_(qRed(pixel), factor);
                    auto g = apply_(qGreen(pixel), factor);
                    auto b = apply_(qBlue(pixel), factor);

                    return qRgba(r, g, b, alpha);
                };
        };

    /*
    * Color tint op factory
    *
    * Valid range: 0.0 to 1.0 (can go higher but gets weird)
    * - tint(red, 0.0)     * No tint (original image)
    * - tint(red, 0.2)     * Subtle red tint
    * - tint(red, 0.5)     * Moderate red tint (default)
    * - tint(red, 0.8)     * Strong red tint
    * - tint(red, 1.0)     * Full red replacement
    * - tint(red, 1.5)     * Over-tinted (may look unnatural)
    *
    * Practical range: 0.0 to 1.0
    */
    inline auto tint = [](QColor tintColor, double strength)
        {
            strength = qMax(0.0, strength);

            return [tintColor, strength](QRgb pixel)
                {
                    auto alpha = qAlpha(pixel);
                    if (alpha < 1) return pixel; // Skip transparent pixels

                    auto r = clamp(static_cast<int>(qRed(pixel) * (1 - strength) + tintColor.red() * strength));
                    auto g = clamp(static_cast<int>(qGreen(pixel) * (1 - strength) + tintColor.green() * strength));
                    auto b = clamp(static_cast<int>(qBlue(pixel) * (1 - strength) + tintColor.blue() * strength));

                    return qRgba(r, g, b, alpha);
                };
        };

    /*
    * Threshold (binary) op factory
    *
    * Valid range: 0 to 255
    * - threshold(0)       * Everything becomes white
    * - threshold(64)      * Only very dark pixels become black
    * - threshold(128)     * Middle threshold (default)
    * - threshold(192)     * Only light pixels become white
    * - threshold(255)     * Everything becomes black
    *
    * Practical range: 32 to 224
    */
    inline auto threshold = [](int thresholdValue)
        {
            thresholdValue = qBound(0, thresholdValue, 255);

            return [thresholdValue](QRgb pixel)
                {
                    auto alpha = qAlpha(pixel);
                    if (alpha < 1) return pixel; // Skip transparent pixels

                    auto grey = qGray(pixel);
                    auto binary = grey >= thresholdValue ? 255 : 0;

                    return qRgba(binary, binary, binary, alpha);
                };
        };

} // namespace Coco::FxOp

namespace Coco::Fx
{
    QList<QColor> goldenRatioColors(int count, const QColor& startColor = Qt::red);

    // Qt's HSL lightness, 0-255 range
    inline bool isDark(const QColor& color) noexcept { return color.lightness() < 128; }
    inline bool isLight(const QColor& color) noexcept { return color.lightness() >= 128; }

    /*
    * Usage examples:
    * - Subtle adjustments (good for photos):
    * - apply(pixmap, brightness(15), contrast(1.2), tint(QColor(255, 240, 200),
    *   0.1));
    * - Dramatic effects:
    * - apply(pixmap, contrast(2.5), brightness(-20), tint(QColor(100, 50, 200),
    *   0.3));
    * - Black and white high contrast:
    * - apply(pixmap, greyscale, contrast(1.8), threshold(120));
    */
    template<typename... FxOps>
    inline QPixmap apply(const QPixmap& pixmap, FxOps... ops)
    {
        if (pixmap.isNull()) return {};

        auto original = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        QImage edited = original;

        auto width = original.width();
        auto height = original.height();

        // Use scanLine for faster pixel access
        for (auto y = 0; y < height; ++y)
        {
            auto original_line = reinterpret_cast<const QRgb*>(original.constScanLine(y));
            auto edited_line = reinterpret_cast<QRgb*>(edited.scanLine(y));

            for (auto x = 0; x < width; ++x)
            {
                QRgb result = original_line[x];
                ((result = ops(result)), ...);  // Chain the ops
                edited_line[x] = result;
            }
        }

        return QPixmap::fromImage(edited);
    }

    inline QPixmap toGreyscale(const QPixmap& pixmap)
    {
        return apply(pixmap, FxOp::greyscale);
    }

    inline QPixmap toInverted(const QPixmap& pixmap)
    {
        return apply(pixmap, FxOp::invert);
    }

    inline QPixmap toSepia(const QPixmap& pixmap)
    {
        return apply(pixmap, FxOp::sepia);
    }

    template<typename T>
    concept QColorConstructible = requires(T&& t) { QColor(std::forward<T>(t)); };

    template <QColorConstructible... T>
    inline QLinearGradient bandedGradient_(qreal x1, qreal y1, qreal x2, qreal y2, T&&... colors) const
    {
        QLinearGradient gradient(x1, y1, x2, y2);
        constexpr std::size_t n = sizeof...(colors);

        if (n == 0)
        {
        defaultReturn:
            // Default semi-transparent black to white gradient
            gradient.setColorAt(0.0, QColor(0, 0, 0, 128));
            gradient.setColorAt(1.0, QColor(255, 255, 255, 128));
            return gradient;
        }

        auto color_array = std::array{ QColor(std::forward<T>(colors))... };

        // Check if any color construction failed
        for (auto& color : color_array)
            if (!color.isValid())
                goto defaultReturn; // idc

        // For 4 colors, we should have gradient positions of 0.0, 0.25, 0.5,
        // and 0.75
        for (std::size_t i = 0; i < n; ++i)
        {
            auto position = static_cast<qreal>(i) / n;
            gradient.setColorAt(position, color_array[i]);
        }

        return gradient;
    }

    template <QColorConstructible... T>
    inline QLinearGradient bandedGradient_(const QPointF& start, const QPointF& finalStop, T&&... colors) const
    {
        return bandedGradient_
        (
            start.x(), start.y(),
            finalStop.x(), finalStop.y(),
            std::forward<T>(colors)...
        );
    }

} // namespace Coco::Fx
