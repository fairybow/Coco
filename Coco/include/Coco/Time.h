/*
 * Coco — Common code for my Qt projects
 * Copyright (C) 2025-2026 fairybow
 * <https://github.com/fairybow/Coco>
 *
 * This program is free software, redistributable and/or modifiable under the
 * terms of the GNU GPL v3. It's distributed in the hope that it will be useful
 * but without any warranty (even the implied warranty of merchantability or
 * fitness for a particular purpose)
 *
 * See the LICENSE file or visit <https://www.gnu.org/licenses/>
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringView>
#include <QTimer>
#include <QtMinMax>
#include <QtTypes>

#include "Coco/Bool.h"

namespace Coco::Time {

COCO_BOOL(RoundUp)

// Milliseconds since the Unix epoch, by the wall clock. It counts on while the
// machine sleeps, and moves if the system clock is set by hand; daylight
// saving time doesn't touch it
inline qint64 epoch() noexcept { return QDateTime::currentMSecsSinceEpoch(); }

// The current local date and time
inline QDateTime now() { return QDateTime::currentDateTime(); }

// The current local date and time, written by a QDateTime format pattern
// (u"yyyy-MM-dd HH:mm:ss.zzz", for instance). Qt writes these patterns in the C
// locale, so the result reads the same on every machine
inline QString nowQString(QStringView pattern)
{
    return now().toString(pattern);
}

// A length of time written as a clock reads: "m:ss", or "h:mm:ss" from an hour
// up. Not a time of day. Whole seconds only: a countdown rounds what is left
// up, so it never reads "0:00" with time still to go, and elapsed time rounds
// down, as a stopwatch does. Zero or less reads "0:00"
inline QString clockQString(qint64 ms, RoundUp roundUp)
{
    auto total_secs = qint64{ 0 };

    if (ms > 0) {
        total_secs = roundUp ? (ms + 999) / 1000 : ms / 1000;
    }

    auto hours = total_secs / 3600;
    auto minutes = (total_secs % 3600) / 60;
    auto seconds = total_secs % 60;

    if (hours > 0) {
        return QString::asprintf(
            "%lld:%02lld:%02lld",
            static_cast<long long>(hours),
            static_cast<long long>(minutes),
            static_cast<long long>(seconds));
    }

    return QString::asprintf(
        "%lld:%02lld",
        static_cast<long long>(minutes),
        static_cast<long long>(seconds));
}

// A countdown that can be paused, kept by its end time rather than by counting
// ticks, so it can't drift however late it's asked. A plain value: no timer,
// no signals, and no clock of its own. Every function that depends on the time
// takes it as nowMs (epoch(), or whatever clock the caller keeps), so a test
// can hand it any time it likes. Something that redraws while it runs keeps
// its own Ticker and asks remainingMs on each tick.
//
// Idle until started, then running or paused. Finished isn't a state of its
// own: a running countdown is finished once nowMs reaches its end. What is
// left is kept between zero and the duration, so a clock set back by hand
// can't make it longer than it started
class Countdown
{
public:
    // Start from the full duration, whatever the countdown was doing
    void start(qint64 durationMs, qint64 nowMs)
    {
        durationMs_ = qMax(durationMs, qint64{ 0 });
        endMs_ = nowMs + durationMs_;
        pausedRemainingMs_ = 0;
        state_ = State_::Running;
    }

    // Keep what is left. Nothing happens unless it's running and not yet
    // finished
    void pause(qint64 nowMs)
    {
        if (state_ != State_::Running || isFinished(nowMs)) {
            return;
        }

        pausedRemainingMs_ = remainingMs(nowMs);
        state_ = State_::Paused;
    }

    // Run on from what was left. Nothing happens unless it's paused
    void resume(qint64 nowMs)
    {
        if (state_ != State_::Paused) {
            return;
        }

        endMs_ = nowMs + pausedRemainingMs_;
        pausedRemainingMs_ = 0;
        state_ = State_::Running;
    }

    // Back to idle, with no duration
    void reset()
    {
        durationMs_ = 0;
        endMs_ = 0;
        pausedRemainingMs_ = 0;
        state_ = State_::Idle;
    }

    [[nodiscard]] bool isIdle() const noexcept
    {
        return state_ == State_::Idle;
    }

    [[nodiscard]] bool isPaused() const noexcept
    {
        return state_ == State_::Paused;
    }

    // Started, not paused, and not yet finished
    [[nodiscard]] bool isRunning(qint64 nowMs) const
    {
        return state_ == State_::Running && !isFinished(nowMs);
    }

    // Started, not paused, and out of time
    [[nodiscard]] bool isFinished(qint64 nowMs) const
    {
        return state_ == State_::Running && nowMs >= endMs_;
    }

    // What it was started with; zero while idle
    [[nodiscard]] qint64 durationMs() const noexcept { return durationMs_; }

    // Zero while idle and once finished
    [[nodiscard]] qint64 remainingMs(qint64 nowMs) const
    {
        switch (state_) {
        case State_::Running:
            return qBound(qint64{ 0 }, endMs_ - nowMs, durationMs_);
        case State_::Paused:
            return pausedRemainingMs_;
        case State_::Idle:
        default:
            return 0;
        }
    }

private:
    enum class State_
    {
        Idle,
        Running,
        Paused,
    };

    State_ state_ = State_::Idle;
    qint64 durationMs_ = 0;
    qint64 endMs_ = 0;
    qint64 pausedRemainingMs_ = 0;
};

template <typename SlotT>
inline void delay(int msecs, const QObject* context, SlotT slot)
{
    QTimer::singleShot(msecs, context, slot);
}

template <typename SlotT> inline void delay(int msecs, SlotT slot)
{
    QTimer::singleShot(msecs, slot);
}

template <typename SlotT>
inline void onNextTick(const QObject* context, SlotT slot)
{
    QTimer::singleShot(0, context, slot);
}

template <typename SlotT> inline void onNextTick(SlotT slot)
{
    QTimer::singleShot(0, slot);
}

template <typename SlotT>
inline QTimer* newQTimer(QObject* parent, SlotT slot, int msec = -1)
{
    auto timer = new QTimer(parent);

    if (msec >= 0) {
        timer->setInterval(msec);
    }

    QObject::connect(timer, &QTimer::timeout, parent, slot);
    return timer;
}

using Ticker = QTimer;
using Delayer = QTimer;
using Debouncer = QTimer;

// Non-single-shot (polling) member
template <typename SlotT>
inline Ticker* newTicker(QObject* parent, SlotT slot, int msec = -1)
{
    auto timer = newQTimer<SlotT>(parent, slot, msec);
    timer->setSingleShot(false);
    return timer;
}

// Single-shot delay member
template <typename SlotT>
inline Delayer* newDelayer(QObject* parent, SlotT slot, int msec = -1)
{
    auto timer = newQTimer<SlotT>(parent, slot, msec);
    timer->setSingleShot(true);
    return timer;
}

// Single-shot delay member
template <typename SlotT>
inline Debouncer* newDebouncer(QObject* parent, SlotT slot, int msec = -1)
{
    return newDelayer<SlotT>(parent, slot, msec);
}

} // namespace Coco::Time
