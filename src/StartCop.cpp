/*
* Coco: StartCop.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-12-8
*/

#include "../include/Coco/StartCop.h"

#include <QLocalServer>
#include <QLocalSocket>

BEGIN_COCO_NAMESPACE

constexpr auto MAX_TIMEOUT = 1000;

StartCop::StartCop(const QString& keyWord)
    : m_keyWord(keyWord)
    , m_signalTimeout(MAX_TIMEOUT)
{
}

StartCop::StartCop(const QString& keyWord, int signalTimeout)
    : m_keyWord(keyWord)
    , m_signalTimeout(qBound(0, signalTimeout, MAX_TIMEOUT))
{
}

void StartCop::_onServerNewConnection()
{
    if (m_signalBlockClock.isActive()) return;

    emit relaunched(QPrivateSignal{});
    m_signalBlockClock.start(m_signalTimeout);
}

bool StartCop::_serverExists() const
{
    QLocalSocket socket{};
    socket.connectToServer(m_keyWord);

    auto exists = socket.isOpen();
    socket.close();

    return exists;
}

void StartCop::_serverStart()
{
    // This is default:
    m_signalBlockClock.setSingleShot(true);

    auto server = new QLocalServer(this);
    server->setSocketOptions(QLocalServer::WorldAccessOption);
    server->listen(m_keyWord);

    connect
    (
        server,
        &QLocalServer::newConnection,
        this,
        &StartCop::_onServerNewConnection
    );
}

bool StartCop::exists()
{
    if (_serverExists())
        return true;

    _serverStart();
    return false;
}

END_COCO_NAMESPACE
