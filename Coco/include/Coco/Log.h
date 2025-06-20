#pragma once

#include <QDebug>
#include <QString>

// Temporary implementation!
#define COCO_LOG(Message) qDebug() << qUtf8Printable(Message)
