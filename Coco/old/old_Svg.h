/*#pragma once

#if defined(QT_NO_SVG)

#warning "Qt SVG module not available. Coco::Svg namespace will be disabled."

#else

#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <Qt>

namespace Coco::Svg
{
    // - TODO: Nonsquare?
    // - TODO: Dynamic high res (see comment)
    inline QPixmap toPixmap(QSvgRenderer& renderer, int size)
    {
        if (!renderer.isValid() || size <= 0) return {};

        // Render at 3x resolution for crispness, then scale down
        auto high_res_size = size * 3; // Use some sort of algorithm to scale less and less as the provided size goes up?
        QPixmap svg_pixmap(high_res_size, high_res_size);
        svg_pixmap.fill(Qt::transparent);

        QPainter svg_painter(&svg_pixmap);
        svg_painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&svg_painter);
        svg_painter.end();

        // Scale down with smooth transformation
        return svg_pixmap.scaled
        (
            size,
            size,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
    }

} // namespace Coco::Svg

#endif // #if defined(QT_NO_SVG)
*/