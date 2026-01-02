#if !defined(QT_NO_NETWORK)

#    include "../include/Coco/StartCop.h"

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

// ----- Internal -----

constexpr auto WAIT_ = 100;
constexpr auto TIMEOUT_ = 1000;
static const QString DELIMITER_ =
    QStringLiteral("\x1F\x1E\x1F"); // Ancient ASCII unit & record separators

static QByteArray serialize_(const QStringList& args)
{
    return args.join(DELIMITER_).toUtf8();
}

static QStringList deserialize_(const QByteArray& data)
{
    return QString::fromUtf8(data).split(DELIMITER_);
}

static QStringList qStringListArgs_(const int& argc, const char* const* argv)
{
    QStringList args{};

#    ifdef Q_OS_WIN

    // Windows uses the local code page (Windows-1252), so QString::fromUtf8()
    // mangles non-ASCII characters!

    (void)argc;
    (void)argv;

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
    if (serverExists_()) return true;

    startServer_();
    return false;
}

bool StartCop::serverExists_() const
{
    QLocalSocket socket{};
    socket.connectToServer(key_);
    auto exists = socket.waitForConnected(WAIT_);

    if (exists) sendArgs_(socket);

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
    if (debouncer_->isActive()) return;

    auto next = server_->nextPendingConnection();
    if (!next) return;

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
