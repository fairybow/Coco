/*
* CocoTabPageWidget.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-10
*/

#pragma once

#include "../src/CocoGlobal.hpp"
#include "CocoAbstractPageWidget.h"

#include <QHash>
#include <QPointer>
#include <QString>
#include <QTabBar>
#include <QUuid>

BEGIN_COCO_NAMESPACE

class TabPageWidget : public AbstractPageWidget
{
    Q_OBJECT

public:
    TabPageWidget(QWidget* parent = nullptr);

    int addPage(QWidget* widget, const QString& label, const QVariant& data = {}) override;
    QTabBar* tabBar() const;
    bool tabsMovable() const;
    void setTabsMovable(bool movable);
    int indexOf(const QWidget* widget) const override;
    QWidget* widgetAt(int index) const override;
    int currentIndex() const override;

public slots:
    void setCurrentIndex(int index) override;

private:
    /// @todo Need to be really clear about these temp data hashes used to find
    /// our widgets. They get confusing.
    QHash<QUuid, QPointer<QWidget>> m_tabBarItemIdsToWidgets{};

    void _initTabBar();

}; // class Coco::TabPageWidget

END_COCO_NAMESPACE
