/*
* Coco/DropDownPageWidget.h  Copyright (C) 2024  fairybow
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
#include "AbstractPageWidget.h"

#include <QComboBox>
#include <QHash>
#include <QPointer>
#include <QString>
#include <QUuid>
#include <QVariant>

BEGIN_COCO_NAMESPACE

/// @todo Abstract away all possible
class DropDownPageWidget : public AbstractPageWidget
{
    Q_OBJECT

public:
    DropDownPageWidget(QWidget* parent = nullptr);

    int addPage(QWidget* widget, const QString& label, const QVariant& data = {}) override;
    QComboBox* comboBox() const;
    bool isEditable() const;
    void setEditable(bool editable);
    bool duplicatesEnabled() const;
    void setDuplicatesEnabled(bool enabled);
    int indexOf(const QWidget* widget) const override;
    QWidget* widgetAt(int index) const override;
    int currentIndex() const override;

public slots:
    void setCurrentIndex(int index) override;
    void setCurrentText(const QString& text);

private:
    /// @todo Need to be really clear about these temp data hashes used to find
    /// our widgets. They get confusing.
    QHash<QUuid, QPointer<QWidget>> m_comboBoxItemIdsToWidgets{};

    void _initComboBox();

}; // class Coco::DropDownPageWidget

END_COCO_NAMESPACE
