/*
 * Coco — Common code for Qt projects
 * Copyright (C) 2025-2026 fairybow
 *
 * This program is free software, redistributable and/or modifiable under the
 * terms of the GNU GPL v3. It's distributed in the hope that it will be useful
 * but without any warranty (even the implied warranty of merchantability or
 * fitness for a particular purpose)
 *
 * See the LICENSE file or visit <https://www.gnu.org/licenses/>
 */

#pragma once

#include <functional>
#include <utility>

#include <QObject>
#include <QString>
#include <QStringView>
#include <QtLogging>

#include "Coco/Fmt.h"
#include "Coco/Path.h"

namespace Coco::Debug {

using namespace Qt::StringLiterals;
using LogSink = std::function<void(const QString&)>;

// Qt's QtMsgType enum is NOT ordered by severity, so use this
inline constexpr int severity(QtMsgType type) noexcept
{
    switch (type) {
    case QtDebugMsg:
        return 0;
    case QtInfoMsg:
        return 1;
    case QtWarningMsg:
        return 2;
    case QtCriticalMsg:
        return 3;
    case QtFatalMsg:
        return 4;
    }

    return 0;
}

// To be safe, don't call this before Qt has finished app construction
void initialize(
    bool verbose = false, // Coco::Bool?
    const Path& logDir = {},
    const QString& logPrefix = {},
    int logCap = 15);

void setLogSink(LogSink sink);
QtMsgType minimumLevel() noexcept;

struct Log
{
    Log(QtMsgType type, const char* file, int line, const char* function)
        : type(type)
        , file(file)
        , line(line)
        , function(function)
    {
    }

    QtMsgType type;
    const char* file;
    int line;
    const char* function;

    template <typename... Args>
    inline void
    print(const QObject* obj, QStringView format, Args&&... args) const
    {
        if (severity(type) < severity(minimumLevel()))
            return;

        QString msg = sizeof...(Args) > 0
                          ? Fmt::format(format, std::forward<Args>(args)...)
                          : format.toString();

        dispatch_(type, file, line, function, obj, std::move(msg));
    }

    template <typename... Args>
    inline void print(QStringView format, Args&&... args) const
    {
        return print(
            static_cast<const QObject*>(nullptr),
            format,
            std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void
    print(const QObject* obj, const char* format, Args&&... args) const
    {
        auto q_format = QString::fromUtf8(format);
        print(obj, QStringView(q_format), std::forward<Args>(args)...);
    }

    template <typename... Args>
    inline void print(const char* format, Args&&... args) const
    {
        auto q_format = QString::fromUtf8(format);
        print(
            static_cast<const QObject*>(nullptr),
            QStringView(q_format),
            std::forward<Args>(args)...);
    }

private:
    void dispatch_(
        QtMsgType type,
        const char* file,
        int line,
        const char* function,
        const QObject* obj,
        QString msg) const;
};

namespace Internal {

inline void assertionFailed_(
    const char* condition,
    const char* file,
    int line,
    const char* function,
    QStringView message = {})
{
    auto msg =
        message.isEmpty()
            ? Fmt::format(u"Assertion failed:\n{}"_s, condition)
            : Fmt::format(u"Assertion failed:\n{}\n\n{}"_s, condition, message);

    Log(QtFatalMsg, file, line, function).print(msg);
}

template <typename... Args>
inline void assertionFailed_(
    const char* condition,
    const char* file,
    int line,
    const char* function,
    QStringView format,
    Args&&... args)
{
    auto message = Fmt::format(format, std::forward<Args>(args)...);
    assertionFailed_(condition, file, line, function, message);
}

// Message-only, const char*
inline void assertionFailed_(
    const char* condition,
    const char* file,
    int line,
    const char* function,
    const char* message)
{
    auto q_message = QString::fromUtf8(message);
    assertionFailed_(condition, file, line, function, QStringView(q_message));
}

// Variadic, const char* format
template <typename... Args>
inline void assertionFailed_(
    const char* condition,
    const char* file,
    int line,
    const char* function,
    const char* format,
    Args&&... args)
{
    auto q_format = QString::fromUtf8(format);
    assertionFailed_(
        condition,
        file,
        line,
        function,
        QStringView(q_format),
        std::forward<Args>(args)...);
}

} // namespace Internal

}

#define LOG(Level)                                                             \
    Coco::Debug::Log(Level, __FILE__, __LINE__, __FUNCTION__).print
#define DEBUG LOG(QtDebugMsg)
#define INFO LOG(QtInfoMsg)
#define WARN LOG(QtWarningMsg)
#define CRITICAL LOG(QtCriticalMsg)
#define FATAL LOG(QtFatalMsg)

#define TRACER DEBUG(__FUNCTION__)

#ifdef COCO_DEBUG
#    define ASSERT(condition, ...)                                             \
        ((condition) ? static_cast<void>(0)                                    \
                     : Coco::Debug::Internal::assertionFailed_(                \
                           #condition,                                         \
                           __FILE__,                                           \
                           __LINE__,                                           \
                           __FUNCTION__ __VA_OPT__(, ) __VA_ARGS__))
#else
#    define ASSERT(condition, ...) static_cast<void>(false && (condition))
#endif

#define UNREACHABLE(...)                                                       \
    Coco::Debug::Internal::assertionFailed_(                                   \
        "UNREACHABLE",                                                         \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __FUNCTION__ __VA_OPT__(, ) __VA_ARGS__)
