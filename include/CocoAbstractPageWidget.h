/*
* CocoAbstractPageWidget.h  Copyright (C) 2024  fairybow
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
#include "CocoStackedWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMargins>
#include <QPixmap>
#include <QPointer>
#include <QString>
#include <QVariant>

BEGIN_COCO_NAMESPACE

/// @details The point of this class is to remove the ever-present threat of me
/// confusing the widget stack index with the controller's index. The subclass
/// should keep track of its widgets and associate them with its controller's
/// (chosen by subclass) index
class AbstractPageWidget : public QWidget
{
    Q_OBJECT

public:
    enum Side
    {
        Right = 0,
        Left
    };

    AbstractPageWidget(QWidget* parent = nullptr);

    void setContentsMargins(const QMargins& margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    void setSpacing(int spacing);
    void layoutDump() const;

    QWidget* cornerWidget(Side side) const;
    void setCornerWidget(QWidget* widget, Side side = Right);
    QPixmap underlayPixmap() const;
    void setUnderlayPixmap(const QPixmap& pixmap);
    QString underlayText() const;
    void setUnderlayText(const QString& text);

    QVariant data(int index) const;
    void setData(int index, const QVariant& data);
    QVariant currentData() const;

    template <typename DataT>
    DataT data(int index) const
    {
        return data(index).value<DataT>();
    }

    template <typename DataT>
    DataT currentData() const
    {
        return currentData().value<DataT>();
    }

    int findData(const QVariant& data) const;
    int count() const;
    QWidget* currentWidget() const;

    virtual int addPage(QWidget* widget, const QString& label, const QVariant& data = {}) = 0;
    // insert and remove

    // These functions will define widget lookup using controller's indexing
    virtual int indexOf(const QWidget* widget) const = 0;
    virtual QWidget* widgetAt(int index) const = 0;
    virtual int currentIndex() const = 0;

public slots:
    // Controller controls. So, don't change widget stack here. Only change
    // widget stack in response to the signal from controller
    virtual void setCurrentIndex(int index) = 0;

signals:
    void widgetCountChanged(int count);
    void currentIndexChanged(int index);

protected:
    // These protected methods allow the subclass to interact with (and set)
    // base members
    bool addWidget(QWidget* widget, const QVariant& data = {});

    /// @todo removeWidget, insertWidget

    QWidget* controller() const;
    void setController(QWidget* controller);

protected slots:
    void onControllerIndexChanged(int index);

private:
    // Holds the underlay and the widget stack. Underlay will be exposed when
    // widget stack is empty
    StackedWidget* m_mainStack = new StackedWidget(this);

    // Holds all given widgets
    StackedWidget* m_widgetStack = new StackedWidget(this);

    // These become parented by the top_layout, itself a child of main_layout,
    // in _initLayouts()
    QHBoxLayout* m_leftCornerWidgetLayout = new QHBoxLayout;
    QHBoxLayout* m_controllerLayout = new QHBoxLayout;
    QHBoxLayout* m_rightCornerWidgetLayout = new QHBoxLayout;

    // For resizing, respacing from outside
    QList<QLayout*> m_layouts{};

    QLabel* m_underlay = new QLabel(this);
    QPointer<QWidget> m_leftCornerWidget{};
    QPointer<QWidget> m_controller{};
    QPointer<QWidget> m_rightCornerWidget{};

    void _initStacks();
    void _initLayouts();

    void _setMember
    (
        QPointer<QWidget>& member,
        QLayout* parentLayout,
        QWidget* newWidget
    );

private slots:
    void _onWidgetStackCountChanged(int count);

}; // class Coco::AbstractPageWidget

END_COCO_NAMESPACE
