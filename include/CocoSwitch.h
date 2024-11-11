/*
* CocoSwitch.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Coco::Switch based on the hard work of user3559721, found here:
* <https://codereview.stackexchange.com/questions/249076/>
*
* Updated: 2024-11-10
*/

#pragma once

#include "../src/CocoGlobal.hpp"

#include <QAbstractButton>
#include <QBrush>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QSize>

BEGIN_COCO_NAMESPACE

/// @todo Make track and ball stylable
/// @todo Dynamic sizing (shrinking to fit small spaces)
class Switch : public QAbstractButton
{
    Q_OBJECT;
    Q_PROPERTY(qreal position READ position WRITE setPosition)

public:
    Switch(QWidget* parent = nullptr);

    virtual QSize sizeHint() const;
    qreal position() const;
    void setPosition(qreal position);
    void setChecked(bool checked);

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    QPropertyAnimation* m_animation = new QPropertyAnimation(this);
    constexpr static auto ANIMATION_DURATION = 100;
    constexpr static auto BALL_POSITION_PROPERTY = "position";

    qreal m_trackRadius = 10;
    qreal m_ballRadius = 8;
    qreal m_currentBallPosition = _offPosition();

    qreal _trackWidth() const;
    qreal _trackHeight() const;
    qreal _ballMargin() const;
    qreal _marginsSpace() const;
    qreal _offPosition() const;
    qreal _onPosition() const;
    qreal _currentDestination() const;
    QBrush _ballBrush() const;
    QBrush _trackBrush() const;
    qreal _opacity(qreal opacity) const;
    void _paintTrack(QPainter& painter) const;
    void _paintBall(QPainter& painter) const;

}; // class Coco::Switch

END_COCO_NAMESPACE
