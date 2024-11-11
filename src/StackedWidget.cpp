/*
* Coco StackedWidget.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-10
*/

#include "../include/Coco/StackedWidget.h"

BEGIN_COCO_NAMESPACE

// Via Qt: "If the QStackedWidget is empty before this function is called, widget
// becomes the current widget." Implies not otherwise!
int StackedWidget::addWidget(QWidget* widget)
{
    auto index = QStackedWidget::addWidget(widget);
    emit widgetCountChanged(count());

    return index;
}

int StackedWidget::insertWidget(int index, QWidget* widget)
{
    auto insert_index = QStackedWidget::insertWidget(index, widget);
    emit widgetCountChanged(count());

    return insert_index;
}

void StackedWidget::removeWidget(QWidget* widget)
{
    m_data.remove(widget);
    QStackedWidget::removeWidget(widget);

    emit widgetCountChanged(count());
}

QVariant StackedWidget::data(QWidget* widget) const
{
    if (widget)
        return m_data[widget];

    return {};
}

void StackedWidget::setData(QWidget* widget, const QVariant& data)
{
    if (widget)
        m_data[widget] = data;
}

QWidget* StackedWidget::findData(const QVariant& data) const
{
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
    {
        if (it.value() != data) continue;
        return it.key();
    }

    return nullptr;
}

// In practice, for DDPW et al, we don't care about or use the stacked
// widget's indexing (at least, I hope we don't care)
// QWidget* StackedWidget::widgetAt(int index) const
// {
//     return widget(index);
// }

END_COCO_NAMESPACE
