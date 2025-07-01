#pragma once

#if defined(QT_NO_NETWORK)

#warning "Qt Network module not available. Coco::StartCop class will be disabled."

#else // #if !defined(QT_NO_NETWORK)

#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

namespace Coco
{
    /// @brief Coco::StartCop is a QObject-derived utility designed to manage
    /// single-instance applications by detecting if another instance is running
    /// and stopping it after sending the second instance's command line
    /// arguments to the primary instance.
    ///
    /// @example
    ///     #include "Coco/StartCop.h"
    ///
    ///     int main(int argc, char* argv[])
    ///     {
    ///         Coco::StartCop s("My App", argc, argv);
    ///         if (s.isRunning()) return 0;
    ///
    ///         QApplication app(argc, argv);
    ///
    ///         MainWindow w;
    ///         w.connect(&s, &StartCop::appRelaunched,
    ///             &w, &MainWindow::slot);
    ///         w.show();
    ///
    ///         return app.exec();
    ///     }
    class StartCop : public QObject
    {
        Q_OBJECT

    public:
        StartCop(
            const QString &key,
            const int &argc = 0,
            const char *const *argv = nullptr);

        virtual ~StartCop() override = default;

        /// @brief Returns true if another instance is already running. In that
        /// case the new instance will also send its args to the primary.
        bool isRunning();

    signals:
        /// @brief Emitted in the primary instance when a secondary instance
        /// connects. The provided args are those received from the new
        /// instance.
        void appRelaunched(QStringList args);

    private:
        QStringList args_;
        QString key_;

        QLocalServer *server_ = nullptr;
        QTimer *debouncer_ = new QTimer(this);

        bool serverExists_() const;
        void sendArgs_(QLocalSocket &socket) const;
        void startServer_();

    private slots:
        void onServerNewConnection_();

    }; // class Coco::StartCop

} // namespace Coco

#endif // #if defined(QT_NO_NETWORK)
