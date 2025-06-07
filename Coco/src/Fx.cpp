
#include <cmath>
#include <numbers>

#include <QColor>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>
#include <QRgb>
#include <QtGlobal>
#include <QtTypes>

#include "../include/Coco/Fx.h"

namespace Coco::Fx
{
    // I'm sure there's a Qt function that does this, but I also know I chose to
    // make this for some reason (perhaps I tried the Qt version before and did
    // not like the output)
    //
    // Update: `QImage::convertToFormat(QImage::Format_Grayscale8)` is bad
    QPixmap toGreyscale(const QPixmap& pixmap)
    {
        if (pixmap.isNull()) return {};
        auto image = pixmap.toImage();
        QImage grayscaled_image = image;

        for (auto x = 0; x < image.width(); ++x)
        {
            for (auto y = 0; y < image.height(); ++y)
            {
                auto pixel = image.pixel(x, y);
                auto grey = qGray(pixel);
                auto rgba = qRgba(grey, grey, grey, qAlpha(pixel));
                grayscaled_image.setPixel(x, y, rgba);
            }
        }

        return QPixmap::fromImage(grayscaled_image);
    }

    // https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
    QList<QColor> goldenRatioColors(int count, const QColor& startColor)
    {
        // Try thread-safe cache
        static QMutex mutex{};
        static auto last_count = 0;
        static QColor last_start_color = Qt::red;
        static QList<QColor> cached{};

        // Check cache under lock
        {
            QMutexLocker locker(&mutex);
            if (count == last_count
                && startColor == last_start_color
                && !cached.isEmpty())
                return cached;
        }

        // Otherwise, generate new colors
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

        // Update cache under lock
        {
            QMutexLocker locker(&mutex);
            last_count = count;
            last_start_color = startColor;
            cached = result;
        }

        return result;
    }

    // double rec601Luminance(const QColor& color)
    // {
    //     return ((0.299 * color.red())
    //         + (0.587 * color.green())
    //         + (0.114 * color.blue()))
    //         / 255.0;
    // }

} // namespace Coco::Fx
