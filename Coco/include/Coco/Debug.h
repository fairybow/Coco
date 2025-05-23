#pragma once

#include <QDebug>
#include <QTimer>

// Can be replaced later with more complex (and optional & toggleable) logging
#define COCO_TRACER qDebug() << __FUNCTION__

#define COCO_TEST_TIMER(Interval, Receiver, Slot) \
    QTimer::singleShot(Interval, Receiver, Slot);

#define COCO_TEST_TIMER(Interval, Slot) \
    QTimer::singleShot(Interval, this, Slot);
