#pragma once

#include <QApplication>
#include <QDebug>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QTimer>

#include "Utility.h"

/// Need to figure out which macros go where

// Can be replaced later with more complex (and optional & toggleable) logging.
#define COCO_TRACER qDebug() << __FUNCTION__

#define COCO_PTR_QSTR(Object) Coco::Debug::qMemoryAddress(Object)

#define COCO_PTR_PRINT(Object) qUtf8Printable(COCO_PTR_QSTR(Object))

// Convenience macro for quick tests. Not for regular use.
#define COCO_SST(Interval, Slot) QTimer::singleShot(Interval, this, Slot)

// Convenience macro for quick tests. Not for regular use.
#define COCO_REPEATER(interval, repeats, slot)                  \
    do {                                                        \
        auto timer = new QTimer(QApplication::instance());      \
        QObject::connect(timer, &QTimer::timeout, slot);        \
        QObject::connect(timer, &QTimer::timeout, [timer] {     \
            static auto counter = 0;                            \
            counter++;                                          \
            if (counter >= repeats) {                           \
                timer->stop();                                  \
                timer->deleteLater();                           \
            }                                                   \
        });                                                     \
        timer->start(interval);                                 \
    } while(0)

// Maybe conditional tracing?
//#ifdef DEBUG
//#define COCO_DESTRUCTOR_TRACE { COCO_TRACER; }
//#else
//#define COCO_DESTRUCTOR_TRACE {}
//#endif
// Using ~Class() override COCO_DESTRUCTOR_TRACE in the below:

namespace Coco::Debug
{
    // Could have a version for non-QObjects, perhaps? Would be more verbose if
    // using std to get type name
    template<typename PtrT>
    inline QString qMemoryAddress(PtrT* ptr)
    {
        COCO_QOBJECT_ASSERT(PtrT);

        return QString("%1(%2)")
            .arg(ptr->metaObject()->className())
            .arg(QString::asprintf("%p", ptr));
    }

} // namespace Coco::Debug
