/*
* Coco AbstractPageWidget.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-10
*/

#include "../include/Coco/AbstractPageWidget.h"

#include <QVBoxLayout>

BEGIN_COCO_NAMESPACE

AbstractPageWidget::AbstractPageWidget(QWidget* parent)
    : QWidget(parent)
{
    _initLayouts();
    _initStacks();
}

void AbstractPageWidget::setContentsMargins(const QMargins& margins)
{
    for (auto& layout : m_layouts)
        layout->setContentsMargins(margins);
}

void AbstractPageWidget::setContentsMargins(int left, int top, int right, int bottom)
{
    setContentsMargins({ left, top, right, bottom });
}

void AbstractPageWidget::setSpacing(int spacing)
{
    for (auto& layout : m_layouts)
        layout->setSpacing(spacing);
}

void AbstractPageWidget::layoutDump() const
{
    for (auto& layout : m_layouts)
    {
        layout->dumpObjectTree();
        layout->dumpObjectInfo();
    }
}

QWidget* AbstractPageWidget::cornerWidget(Side side) const
{
    switch (side)
    {
    case Left:
        return m_leftCornerWidget;

    default:
    case Right:
        return m_rightCornerWidget;
    }
}

void AbstractPageWidget::setCornerWidget(QWidget* widget, Side side)
{
    switch (side)
    {
    case Left:
        _setMember
        (
            m_leftCornerWidget,
            m_leftCornerWidgetLayout,
            widget
        );

        break;

    default:
    case Right:
        _setMember
        (
            m_rightCornerWidget,
            m_rightCornerWidgetLayout,
            widget
        );
    }
}

QPixmap AbstractPageWidget::underlayPixmap() const
{
    return m_underlay->pixmap();
}

void AbstractPageWidget::setUnderlayPixmap(const QPixmap& pixmap)
{
    m_underlay->setPixmap(pixmap);
}

QString AbstractPageWidget::underlayText() const
{
    return m_underlay->text();
}

void AbstractPageWidget::setUnderlayText(const QString& text)
{
    m_underlay->setText(text);
}

QVariant AbstractPageWidget::data(int index) const
{
    return m_widgetStack->data(widgetAt(index));
}

void AbstractPageWidget::setData(int index, const QVariant& data)
{
    m_widgetStack->setData(widgetAt(index), data);
}

QVariant AbstractPageWidget::currentData() const
{
    return data(currentIndex());
}

int AbstractPageWidget::findData(const QVariant& data) const
{
    if (auto widget = m_widgetStack->findData(data))
        return indexOf(widget);

    return -1;
}

int AbstractPageWidget::count() const
{
    return m_widgetStack->count();
}

QWidget* AbstractPageWidget::currentWidget() const
{
    return widgetAt(currentIndex());
}

// For subclasses to access the private widget stack. Since we don't give a shit
// about the widget stack's index, we can return bool
bool AbstractPageWidget::addWidget(QWidget* widget, const QVariant& data)
{
    // I don't know that a QStackedWidget would ever be unsuccessful, but I
    // imagine it would return -1 if it was
    if (m_widgetStack->addWidget(widget) > -1)
    {
        m_widgetStack->setData(widget, data);
        return true;
    }

    return false;
}

QWidget* AbstractPageWidget::controller() const
{
    return m_controller;
}

void AbstractPageWidget::setController(QWidget* controller)
{
    _setMember
    (
        m_controller,
        m_controllerLayout,
        controller
    );
}

// Change widget stack in response to controller index using subclass-defined
// result of widgetAt. Subclasses must connect their controller's "index
// changed" signal to this
void AbstractPageWidget::onControllerIndexChanged(int index)
{
    if (auto widget = widgetAt(index))
    {
        m_widgetStack->setCurrentWidget(widget);
        emit currentIndexChanged(index);
    }
}

void AbstractPageWidget::_initStacks()
{
    m_underlay->setAlignment(Qt::AlignCenter);

    m_mainStack->addWidget(m_underlay);
    m_mainStack->addWidget(m_widgetStack);
    m_mainStack->setCurrentIndex(0);

    connect
    (
        m_widgetStack,
        &StackedWidget::widgetCountChanged,
        this,
        &AbstractPageWidget::_onWidgetStackCountChanged
    );
}

void AbstractPageWidget::_initLayouts()
{
    m_leftCornerWidgetLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_controllerLayout->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_rightCornerWidgetLayout->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    auto top_layout = new QHBoxLayout;

    top_layout->addLayout(m_leftCornerWidgetLayout);
    top_layout->addLayout(m_controllerLayout);
    top_layout->addLayout(m_rightCornerWidgetLayout);

    auto main_layout = new QVBoxLayout(this);

    main_layout->addLayout(top_layout);
    main_layout->addWidget(m_mainStack);

    m_layouts = QList<QLayout*>
    {
        m_leftCornerWidgetLayout,
        m_controllerLayout,
        m_rightCornerWidgetLayout,
        top_layout,
        main_layout
    };
}

void AbstractPageWidget::_setMember
(
    QPointer<QWidget>& member,
    QLayout* parentLayout,
    QWidget* newWidget
)
{
    if (member)
    {
        parentLayout->removeWidget(member);
        member->setVisible(false);
    }

    if (newWidget)
        parentLayout->addWidget(newWidget);

    member = newWidget;
}

// Maybe show underlay based on widget sub-stack count
void AbstractPageWidget::_onWidgetStackCountChanged(int count)
{
    // I am assuming setting current widget to same widget does nothing, but
    // hopefully there's no weird flicker
    count
        ? m_mainStack->setCurrentWidget(m_widgetStack)
        : m_mainStack->setCurrentWidget(m_underlay);

    emit widgetCountChanged(count);
}

END_COCO_NAMESPACE
