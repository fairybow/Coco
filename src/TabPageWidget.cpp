/*
* Coco: TabPageWidget.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-10
*/

#include "../include/Coco/TabPageWidget.h"

BEGIN_COCO_NAMESPACE

TabPageWidget::TabPageWidget(QWidget* parent)
    : AbstractPageWidget(parent)
{
    _initTabBar();
}

int TabPageWidget::addPage(QWidget* widget, const QString& label, const QVariant& data)
{
    // Add widget and set user data for the widget, if provided
    if (!addWidget(widget, data))
        return -1;

    // Associate widget with an ID for retrieval
    auto id = QUuid::createUuid();
    m_tabBarItemIdsToWidgets[id] = widget;

    auto tab_bar = tabBar();

    // Add tab along with widget association ID
    auto index = tab_bar->addTab(label);
    tab_bar->setTabData(index, id);

    // Return the tab bar index of the new item
    return index;
}

QTabBar* TabPageWidget::tabBar() const
{
    return  qobject_cast<QTabBar*>(controller());
}

bool TabPageWidget::tabsMovable() const
{
    return tabBar()->isMovable();
}

void TabPageWidget::setTabsMovable(bool movable)
{
    tabBar()->setMovable(movable);
}

int TabPageWidget::indexOf(const QWidget* widget) const
{
    auto it = m_tabBarItemIdsToWidgets.constBegin();
    auto end = m_tabBarItemIdsToWidgets.constEnd();

    while (it != end)
    {
        if (it.value() == widget)
        {
            auto tab_bar = tabBar();

            for (auto i = 0; i < tab_bar->count(); ++i)
                if (tab_bar->tabData(i) == it.key())
                    return i;
        }

        ++it;
    }

    return -1;
}

QWidget* TabPageWidget::widgetAt(int index) const
{
    auto id = tabBar()->tabData(index).value<QUuid>();
    return m_tabBarItemIdsToWidgets[id];
}

int TabPageWidget::currentIndex() const
{
    return tabBar()->currentIndex();
}

void TabPageWidget::setCurrentIndex(int index)
{
    tabBar()->setCurrentIndex(index);
}

void TabPageWidget::_initTabBar()
{
    auto tab_bar = new QTabBar;
    tab_bar->setMovable(true);

    tab_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect
    (
        tab_bar,
        &QTabBar::currentChanged,
        this,
        &TabPageWidget::onControllerIndexChanged
    );

    setController(tab_bar);
}

END_COCO_NAMESPACE
