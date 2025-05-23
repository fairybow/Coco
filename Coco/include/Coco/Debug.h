#pragma once

#include <QDebug>
#include <QTimer>

// Can be replaced later with more complex (and optional & toggleable) logging
#define COCO_TRACER qDebug() << __FUNCTION__

// Convenience macro for quick tests. Not for regular use
#define COCO_QSST(Interval, Slot) QTimer::singleShot(Interval, this, Slot);
