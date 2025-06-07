#pragma once

#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QTimer>

// Can be replaced later with more complex (and optional & toggleable) logging.
#define COCO_TRACER qDebug() << __FUNCTION__

// Convenience macro for quick tests. Not for regular use.
#define COCO_QSST(Interval, Slot) QTimer::singleShot(Interval, this, Slot)

// Convenience macro for quick tests. Not for regular use.
#define COCO_QREPEATER(interval, repeats, slot)             \
    do {                                                    \
        auto timer = new QTimer(QApplication::instance());  \
        QObject::connect(timer, &QTimer::timeout, slot);    \
        QObject::connect(timer, &QTimer::timeout, [timer] { \
            static auto counter = 0;                        \
            counter++;                                      \
            if (counter >= repeats) {                       \
                timer->stop();                              \
                timer->deleteLater();                       \
            }                                               \
        });                                                 \
        timer->start(interval);                             \
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
/// say, QWidget or something unhelpful.
#define COCO_TRIVIAL_CLASS(Class, Base)             \
    class Class : public Base                       \
    {                                               \
    public:                                         \
        using Base::Base;                           \
        virtual ~Class() override { COCO_TRACER; }  \
    }
