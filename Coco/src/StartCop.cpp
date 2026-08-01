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

#if defined(COCO_HAS_NETWORK)

#    include "Coco/StartCop.h"

#    include <QByteArray>
#    include <QLocalServer>
#    include <QLocalSocket>
#    include <QObject>
#    include <QString>
#    include <QStringList>
#    include <QTimer>

#    ifdef Q_OS_WIN
#        include <Windows.h>
#        include <shellapi.h>
#    endif

using namespace Qt::StringLiterals;

constexpr auto WAIT_ = 100;
constexpr auto TIMEOUT_ = 1000;
static const auto DELIMITER_ =
    u"\x1F\x1E\x1F"_s; // Ancient ASCII unit & record separators

static QByteArray serialize_(const QStringList& args)
{
    return args.join(DELIMITER_).toUtf8();
}

static QStringList deserialize_(const QByteArray& data)
{
    return QString::fromUtf8(data).split(DELIMITER_);
}

static QStringList qStringListArgs_(
    [[maybe_unused]] const int& argc, [[maybe_unused]] const char* const* argv)
{
    QStringList args{};

#    ifdef Q_OS_WIN

    // Windows uses the local code page (Windows-1252), so QString::fromUtf8()
    // mangles non-ASCII characters!

    auto wargc = 0;
    auto wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

    if (wargv) {
        for (auto i = 0; i < wargc; ++i)
            args << QString::fromWCharArray(wargv[i]);
        LocalFree(wargv);
    }

#    else

    for (auto i = 0; i < argc; ++i)
        args << QString::fromUtf8(argv[i]);

#    endif

    return args;
}

// ----- End Internal -----

namespace Coco {
StartCop::StartCop(const QString& key, const int& argc, const char* const* argv)
    : QObject(nullptr)
    , args_(qStringListArgs_(argc, argv))
    , key_(key)
{
    debouncer_->setSingleShot(true);
}

bool StartCop::isRunning()
{
    if (serverExists_())
        return true;

    startServer_();
    return false;
}

bool StartCop::serverExists_() const
{
    QLocalSocket socket{};
    socket.connectToServer(key_);
    auto exists = socket.waitForConnected(WAIT_);

    if (exists)
        sendArgs_(socket);

    socket.close();
    return exists;
}

void StartCop::sendArgs_(QLocalSocket& socket) const
{
    auto data = serialize_(args_);
    socket.write(data);
    socket.flush();
    socket.waitForBytesWritten(WAIT_);
}

void StartCop::startServer_()
{
    server_ = new QLocalServer(this);
    server_->setSocketOptions(QLocalServer::WorldAccessOption);

    // Try to listen; if this fails (perhaps due to a stale socket), remove
    // it and try again. On Unix-like systems, if the application crashes or
    // terminates unexpectedly, the local socket file may not be removed.
    // This can cause subsequent calls to listen(key_) to fail.
    if (!server_->listen(key_)) {
        QLocalServer::removeServer(key_);
        server_->listen(key_);
    }

    connect(
        server_,
        &QLocalServer::newConnection,
        this,
        &StartCop::onServerNewConnection_);
}

void StartCop::onServerNewConnection_()
{
    if (debouncer_->isActive())
        return;

    auto next = server_->nextPendingConnection();
    if (!next)
        return;

    if (next->waitForReadyRead(WAIT_)) {
        auto data = next->readAll();
        auto new_args = deserialize_(data);

        emit appRelaunched(new_args);
    }

    next->disconnectFromServer();
    next->deleteLater();

    debouncer_->start(TIMEOUT_);
}

} // namespace Coco

#endif
