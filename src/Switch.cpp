/*
* Coco Switch.cpp  Copyright (C) 2024  fairybow
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

#include "../include/Coco/Switch.h"

#include <QSizePolicy>

BEGIN_COCO_NAMESPACE

Switch::Switch(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setChecked(false);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_animation->setTargetObject(this);
}

QSize Switch::sizeHint() const
{
    auto margins_space = _marginsSpace();

    return QSize
    (
        _trackWidth() + margins_space,
        _trackHeight() + margins_space
    );
}

qreal Switch::position() const
{
    return m_currentBallPosition;
}

void Switch::setPosition(qreal position)
{
    m_currentBallPosition = position;
    update();
}

// QAbstractButton::setChecked does not emit the clicked signal
void Switch::setChecked(bool checked)
{
    QAbstractButton::setChecked(checked);
    m_currentBallPosition = _currentDestination();
}

void Switch::enterEvent(QEnterEvent* event)
{
    setCursor(Qt::PointingHandCursor);
    QAbstractButton::enterEvent(event);
}

void Switch::mouseReleaseEvent(QMouseEvent* event)
{
    QAbstractButton::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        m_animation->setDuration(ANIMATION_DURATION);

        if (m_animation->state() != QPropertyAnimation::Running)
            m_animation->setPropertyName(BALL_POSITION_PROPERTY);

        m_animation->setStartValue(m_currentBallPosition);
        m_animation->setEndValue(_currentDestination());

        m_animation->start();
    }
}

void Switch::paintEvent(QPaintEvent* event)
{
    (void)event;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    _paintTrack(painter);
    _paintBall(painter);
}

void Switch::resizeEvent(QResizeEvent* event)
{
    QAbstractButton::resizeEvent(event);
    m_currentBallPosition = _currentDestination();
}

qreal Switch::_trackWidth() const
{
    return 3.6 * m_trackRadius;
}

qreal Switch::_trackHeight() const
{
    return 2 * m_trackRadius;
}

// If the ball is smaller than the track, we can return 0. Otherwise, we
// need a margin that accounts for the amount by which the ball is larger
// than the track
qreal Switch::_ballMargin() const
{
    auto radius_diff = m_ballRadius - m_trackRadius;
    return (radius_diff < 0) ? 0 : radius_diff;
}

qreal Switch::_marginsSpace() const
{
    return 2 * _ballMargin();
}

qreal Switch::_offPosition() const
{
    return m_ballRadius > m_trackRadius
        ? m_ballRadius
        : m_trackRadius;
}

qreal Switch::_onPosition() const
{
    return _trackWidth() + _marginsSpace() - _offPosition();
}

qreal Switch::_currentDestination() const
{
    return isChecked() ? _onPosition() : _offPosition();
}

QBrush Switch::_ballBrush() const
{
    if (isEnabled())
    {
        return isChecked()
            ? palette().highlight()
            : palette().light();
    }

    return palette().mid();
}

QBrush Switch::_trackBrush() const
{
    if (isEnabled())
    {
        return isChecked()
            ? palette().highlight()
            : palette().dark();
    }

    return palette().shadow();
}

qreal Switch::_opacity(qreal opacity) const
{
    return isEnabled() ? opacity : opacity * 0.3;
}

void Switch::_paintTrack(QPainter& painter) const
{
    painter.setBrush(_trackBrush());
    painter.setOpacity(_opacity(0.5));

    auto margin = _ballMargin();
    auto margins_space = _marginsSpace();

    painter.drawRoundedRect
    (
        margin,
        margin,
        //width() - _marginsSpace(),
        _trackWidth(),
        //height() - _marginsSpace(),
        _trackHeight(),
        m_trackRadius,
        m_trackRadius
    );
}

void Switch::_paintBall(QPainter& painter) const
{
    painter.setBrush(_ballBrush());
    painter.setOpacity(_opacity(1.0));

    painter.drawEllipse
    (
        m_currentBallPosition - m_ballRadius,
        _offPosition() - m_ballRadius,
        2 * m_ballRadius,
        2 * m_ballRadius
    );
}

END_COCO_NAMESPACE
