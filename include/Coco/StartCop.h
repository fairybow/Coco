/*
* Coco: StartCop.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-12-8
*/

// A guard against relaunch of Qt applications
// https://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection

//------------------------------------------------------------
// Usage
//------------------------------------------------------------

//  Requires the Qt Network module.

//  ```cpp
//  int main(int argc, char* argv[])
//  {
//      StartCop start_cop("MyApplication");
//
//      if (start_cop.exists())
//      {
//          // ...
//
//          return 0;
//      }
//
//      QApplication a(argc, argv);
//      QMainWindow w{};
//      w.show();
//
//      return a.exec();
//  }
//  ```

#pragma once

#include "Global.h"

#include <QObject>
#include <QString>
#include <QTimer>

BEGIN_COCO_NAMESPACE

class StartCop : public QObject
{
    Q_OBJECT

public:
    StartCop(const QString& keyWord);
    StartCop(const QString& keyWord, int signalTimeout);
    bool exists();

signals:
    void relaunched(QPrivateSignal);

private:
    const QString m_keyWord;
    const int m_signalTimeout;
    QTimer m_signalBlockClock{};

    bool _serverExists() const;
    void _serverStart();

private slots:
    void _onServerNewConnection();

}; // class Coco::StartCop

END_COCO_NAMESPACE
