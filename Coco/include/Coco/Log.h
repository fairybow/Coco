#pragma once

#include <QDebug>
#include <QString>

#include "Debug.h"

// Temporary implementation!
#define COCO_LOG(Message) qDebug() << qUtf8Printable(Message)

// Temporary implementation! Adds a "vocative" prefix
#define COCO_LOG_THIS(Message) qDebug() << qUtf8Printable(QString("%0: %1").arg(COCO_PTR_QSTR(this)).arg(Message))
