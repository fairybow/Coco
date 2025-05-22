#pragma once

#include <QDebug>

// Can be replaced later with more complex (and optional & toggleable) logging
#define COCO_TRACER qDebug() << __FUNCTION__
