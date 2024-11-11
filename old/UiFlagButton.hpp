/*
* cc/UiFlagButton.hpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* This file uses cc/cc.qrc, which uses Google Material Symbols. Google Material
* Symbols are free and open-source. For more information, visit
* <https://fonts.google.com/icons>.
*
* Updated: 2024-09-17
*/

/*#pragma once

#include "cc_namespaces.hpp"

#include <QChar>
#include <QEvent>
#include <QEnterEvent>
#include <QFont>
#include <QFontDatabase>
#include <QPushButton>
#include <QString>
#include <QToolButton>

#include <array>
#include <utility>

BEGIN_CC_UI_NAMESPACE

/// @todo Update for Symbols
enum Icon
{
    None = 0,
    Add,
    ChevronLeft,
    ChevronRight,
    Close,
    Ellipse,
    ExpandLess,
    ExpandMore,
    FormatPaint,
    Menu,
    MenuOpen,
    Refresh
};

inline constexpr static std::array<std::pair<Icon, QChar>, 11> ICON_HEX_MAP =
{
    {
        { Add, QChar(0xe145) },
        { ChevronLeft, QChar(0xe5cb) },
        { ChevronRight, QChar(0xe5cc) },
        { Close, QChar(0xe5cd) },
        { Ellipse, QChar(0xe061) }, // not Material
        { ExpandLess, QChar(0xe5ce) },
        { ExpandMore, QChar(0xe5cf) },
        { FormatPaint, QChar(0xe243) },
        { Menu, QChar(0xe5d2) },
        { MenuOpen, QChar(0xe9bd) },
        { Refresh, QChar(0xe5d5) }
    }
};

inline const QFont uiFont()
{
    constexpr static auto qrc = \
        ":/cc/external/MaterialSymbolsRounded-VariableFont_FILL,GRAD,opsz,wght.ttf";

    static const auto id = QFontDatabase::addApplicationFont(qrc);
    QString font = QFontDatabase::applicationFontFamilies(id).at(0);

    return font;
}

inline constexpr QChar getIconHex(Icon icon)
{
    for (const auto& pair : ICON_HEX_MAP)
    {
        if (pair.first == icon)
            return pair.second;
    }

    return {};
}

/// @todo Rework
template <typename QButtonT>
class FlagButtonBase : public QButtonT
{
public:
    FlagButtonBase
    (
        const QString& text,
        QWidget* parent = nullptr,
        const QString& flaggedText = QString{}
    )
        :
        QButtonT(parent),
        m_label(text),
        m_flag(flaggedText)
    {
        _updateText();
    }

    // Lmao
    FlagButtonBase
    (
        Icon icon,
        QWidget* parent = nullptr,
        Icon flag = Icon{}
    )
        :
        FlagButtonBase
        (
            _iconText(icon),
            parent,
            _iconText(flag)
        )
    {
        QButtonT::setFont(uiFont());
    }

    virtual ~FlagButtonBase() = default;

    bool hoveredOver() const noexcept
    {
        return m_hoveredOver;
    }

    QString label() const
    {
        return m_label;
    }

    void setLabel(const QString& text)
    {
        m_label = text;
    }

    void setLabel(Icon icon)
    {
        m_label = _iconText(icon);
    }

    QString flag() const
    {
        return m_flag;
    }

    void setFlag(const QString& text)
    {
        m_flag = text;
    }

    void setFlag(Icon icon)
    {
        m_flag = _iconText(icon);
    }

    bool flagged() const
    {
        return m_flagged;
    }

    void setFlagged(bool flagged = true)
    {
        m_flagged = flagged;

        _updateText();
    }

    void toggleFlagged()
    {
        setFlagged(!m_flagged);
    }

protected:
    virtual void enterEvent(QEnterEvent* event) override
    {
        QButtonT::enterEvent(event);
        m_hoveredOver = true;

        _updateText();
    }

    virtual void leaveEvent(QEvent* event) override
    {
        QButtonT::leaveEvent(event);
        m_hoveredOver = false;

        _updateText();
    }

private:
    constexpr static auto FLAG_PROPERTY = "flagged";

    QString m_label;
    QString m_flag;
    bool m_flagged = false;
    bool m_hoveredOver = false;

    const QString _iconText(Icon icon) const
    {
        return getIconHex(icon);
    }

    void _updateText()
    {
        auto flagged = _flagShouldDisplay();

        flagged
            ? QButtonT::setText(m_flag)
            : QButtonT::setText(m_label);

        // This is for style sheets
        QButtonT::setProperty(FLAG_PROPERTY, flagged);

        QButtonT::update();
    }

    bool _flagShouldDisplay() const
    {
        return m_flagged
            && !m_hoveredOver
            && !m_flag.isNull();
    }

}; // class cc::Ui::FlagButtonBase

class FlagButton : public FlagButtonBase<QPushButton>
{
    Q_OBJECT

public:
    using FlagButtonBase<QPushButton>::FlagButtonBase;
};

class FlagToolButton : public FlagButtonBase<QToolButton>
{
    Q_OBJECT

public:
    using FlagButtonBase<QToolButton>::FlagButtonBase;
};

END_CC_UI_NAMESPACE*/
