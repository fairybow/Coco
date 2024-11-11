/*
* Coco DropDownPageWidget.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-10
*/

#include "../include/Coco/DropDownPageWidget.h"

BEGIN_COCO_NAMESPACE

DropDownPageWidget::DropDownPageWidget(QWidget* parent)
    : AbstractPageWidget(parent)
{
    _initComboBox();
}

int DropDownPageWidget::addPage(QWidget* widget, const QString& label, const QVariant& data)
{
    // Add widget and set user data for the widget, if provided
    if (!addWidget(widget, data))
        return -1;

    // Associate widget with an ID for retrieval (duplicates are allowed, so
    // we can't use combo box text and also we would NEVER do that anyway
    // would we now)
    auto id = QUuid::createUuid();
    m_comboBoxItemIdsToWidgets[id] = widget;

    // Add label to combo box along with widget association ID
    comboBox()->addItem(label, id);

    // Return the combo box index of the new item
    return comboBox()->findData(id);
}

QComboBox* DropDownPageWidget::comboBox() const
{
    return  qobject_cast<QComboBox*>(controller());
}

bool DropDownPageWidget::isEditable() const
{
    return comboBox()->isEditable();
}

void DropDownPageWidget::setEditable(bool editable)
{
    comboBox()->setEditable(editable);
}

bool DropDownPageWidget::duplicatesEnabled() const
{
    return comboBox()->duplicatesEnabled();
}

void DropDownPageWidget::setDuplicatesEnabled(bool enabled)
{
    comboBox()->setDuplicatesEnabled(enabled);
}

int DropDownPageWidget::indexOf(const QWidget* widget) const
{
    auto it = m_comboBoxItemIdsToWidgets.constBegin();
    auto end = m_comboBoxItemIdsToWidgets.constEnd();

    while (it != end)
    {
        if (it.value() == widget)
            return comboBox()->findData(it.key());

        ++it; // I'm so sorry little guy; I'll never forget you again
    }

    return -1;
}

QWidget* DropDownPageWidget::widgetAt(int index) const
{
    auto id = comboBox()->itemData(index).value<QUuid>();
    return m_comboBoxItemIdsToWidgets[id];
}

int DropDownPageWidget::currentIndex() const
{
    return comboBox()->currentIndex();
}

void DropDownPageWidget::setCurrentIndex(int index)
{
    comboBox()->setCurrentIndex(index);
}

void DropDownPageWidget::setCurrentText(const QString& text)
{
    comboBox()->setCurrentText(text);
}

void DropDownPageWidget::_initComboBox()
{
    auto combo_box = new QComboBox;
    combo_box->setEditable(false);
    combo_box->setDuplicatesEnabled(false);

    combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect
    (
        combo_box,
        &QComboBox::currentIndexChanged,
        this,
        &DropDownPageWidget::onControllerIndexChanged
    );

    setController(combo_box);
}

END_COCO_NAMESPACE
