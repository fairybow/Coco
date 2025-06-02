#pragma once

#include <QDebug>
#include <QTimer>

// Can be replaced later with more complex (and optional & toggleable) logging.
#define COCO_TRACER qDebug() << __FUNCTION__

// Convenience macro for quick tests. Not for regular use.
#define COCO_QSST(Interval, Slot) QTimer::singleShot(Interval, this, Slot)

// Convenience macro for quick tests. Not for regular use.
#define COCO_QREPEATER(interval, repeats, slot)                             \
    do {                                                                    \
        static QTimer* timer = nullptr;                                     \
        static int count = 0;                                               \
        if (!timer)                                                         \
        {                                                                   \
            timer = new QTimer(QCoreApplication::instance());               \
            QObject::connect(timer, &QTimer::timeout, slot);                \
            QObject::connect(timer, &QTimer::timeout, [&] {                 \
                count++;                                                    \
                if (count < repeats) return;                                \
                timer->deleteLater();                                       \
                timer = nullptr;                                            \
                count = 0;                                                  \
            });                                                             \
                                                                            \
            timer->start(interval);                                         \
        }                                                                   \
    } while(0)
