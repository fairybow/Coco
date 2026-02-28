/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE, or
 * visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#pragma once

#if defined(QT_NO_NETWORK)

#    warning                                                                   \
        "Qt Network module not available. Coco::StartCop class will be disabled."

#else

#    include <QLocalServer>
#    include <QLocalSocket>
#    include <QObject>
#    include <QString>
#    include <QStringList>
#    include <QTimer>

namespace Coco {

// Coco::StartCop is a QObject-derived utility designed to manage
// single-instance applications by detecting if another instance is running and
// stopping it after sending the second instance's command line arguments to the
// primary instance.
//
// clang-format off
//
// Example:
//
// ```
// #include "Coco/StartCop.h"
//
// int main(int argc, char* argv[])
// {
//     Coco::StartCop s("My App", argc, argv);
//     if (s.isRunning()) return 0;
//
//     QApplication app(argc, argv);
//
//     MainWindow w;
//     w.connect(&s, &StartCop::appRelaunched, &w, &MainWindow::slot);
//     w.show();
//
//     return app.exec();
// }
// ```
// clang-format on
class StartCop : public QObject
{
    Q_OBJECT

public:
    StartCop(
        const QString& key,
        const int& argc = 0,
        const char* const* argv = nullptr);

    virtual ~StartCop() override = default;

    bool isRunning();

signals:
    void appRelaunched(const QStringList& args);

private:
    QStringList args_;
    QString key_;

    QLocalServer* server_ = nullptr;
    QTimer* debouncer_ = new QTimer(this);

    bool serverExists_() const;
    void sendArgs_(QLocalSocket& socket) const;
    void startServer_();

private slots:
    void onServerNewConnection_();
};

} // namespace Coco

#endif
