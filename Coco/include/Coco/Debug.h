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

// Maybe conditional tracing?
//#ifdef DEBUG
//#define COCO_DESTRUCTOR_TRACE { COCO_TRACER; }
//#else
//#define COCO_DESTRUCTOR_TRACE {}
//#endif
// Using ~Class() override COCO_DESTRUCTOR_TRACE in the below:

/// @brief I occasionally subclass in order to: 1) check memory via destructor
/// debug output; and 2) have debug output  show up as a custom class and not,
/// say, QWidget or something unhelpful. Obviously, this doesn't work directly
/// as a nested class nor with `std::unique_ptr`.
#define COCO_TRIVIAL_QCLASS(Class, QtObject)        \
    class Class : public QtObject                   \
    {                                               \
        Q_OBJECT                                    \
                                                    \
    public:                                         \
        using QtObject::QtObject;                   \
        virtual ~Class() override { COCO_TRACER; }  \
    }

#define COCO_TRIVIAL_NESTED_QCLASS(OuterClass, InnerClass, QtObject)    \
    class OuterClass::InnerClass : public QtObject                      \
    {                                                                   \
        Q_OBJECT                                                        \
                                                                        \
    public:                                                             \
        using QtObject::QtObject;                                       \
        virtual ~InnerClass() override { COCO_TRACER; }                 \
    }
