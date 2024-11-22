/*
* Coco: StackedWidget.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-16
*/

#pragma once

#include "Global.h"

#include <QHash>
#include <QStackedWidget>
#include <QVariant>

BEGIN_COCO_NAMESPACE

class StackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    using QStackedWidget::QStackedWidget;

    int addWidget(QWidget* widget);
    int insertWidget(int index, QWidget* widget);
    void removeWidget(QWidget* widget);
    QVariant data(QWidget* widget) const;

    template <typename DataT>
    DataT data(QWidget* widget) const
    {
        return data(widget).value<DataT>();
    }

    void setData(QWidget* widget, const QVariant& data);
    QWidget* findData(const QVariant& data) const;
    // QWidget* widgetAt(int index) const;

signals:
    void widgetCountChanged(int count);

private:
    // Is this a problem?
    QHash<QWidget*, QVariant> m_data{};

}; // class Coco::StackedWidget

END_COCO_NAMESPACE
