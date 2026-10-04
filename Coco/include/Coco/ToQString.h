/*
 * Coco — Common code for Qt projects
 * Copyright (C) 2025-2026 fairybow
 *
 * This program is free software, redistributable and/or modifiable under the
 * terms of the GNU GPL v3. It's distributed in the hope that it will be useful
 * but without any warranty (even the implied warranty of merchantability or
 * fitness for a particular purpose)
 *
 * See the LICENSE file or visit <https://www.gnu.org/licenses/>
 */

#pragma once

#include <concepts>
#include <type_traits>

#ifdef COCO_HAS_XML
#    include <QDomAttr>
#    include <QDomElement>
#    include <QDomNamedNodeMap>
#endif
#include <QHashIterator>
#include <QLatin1StringView>
#include <QMapIterator>
#include <QMetaObject>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QVariant>
#include <QVariantHash>
#include <QVariantMap>

#include "Coco/Bool.h"
#include "Coco/Concepts.h"
#include "Coco/Path.h"

namespace Coco {

using namespace Qt::StringLiterals;

// Forward declarations for mutually-recursive overloads. QVariant can hold any
// of these, and the container overloads call back into toQString for values
QString toQString(const QVariant& variant);
QString toQString(const QVariantHash& variantHash);
QString toQString(const QVariantMap& variantMap);

// --- Strings ---

// Passthrough (implicitly shared, no copy)
inline QString toQString(const QString& s) { return s; }
inline QString toQString(QStringView s) { return s.toString(); }
inline QString toQString(QLatin1StringView s) { return s.toString(); }
inline QString toQString(const char* s) { return QString::fromUtf8(s); }

// --- Bool ---

inline QString toQString(bool b) { return b ? u"true"_s : u"false"_s; }

// --- Numerics ---

template <typename T>
    requires std::integral<T> && (!std::same_as<T, bool>) &&
             (!std::same_as<T, char>)
inline QString toQString(T value)
{
    return QString::number(value);
}

template <std::floating_point T> inline QString toQString(T value)
{
    return QString::number(value);
}

// --- Pointers ---

// Ptr can be nullptr
template <typename T> inline QString toQString(const T* ptr)
{
    if (!ptr)
        return u"nullptr"_s;

    // TODO: Untested - check print output (implementation defined)
    return QString::asprintf(
        "%s(%p)",
        typeid(T).name(),
        static_cast<const void*>(ptr));
}

// Ptr can be nullptr. Overrides the generic pointer overload via partial
// ordering when T derives from QObject
template <Concepts::QObjectDerived T> inline QString toQString(const T* ptr)
{
    if (!ptr)
        return u"nullptr"_s;

    return QString::asprintf(
        "%s(%p)",
        ptr->metaObject()->className(),
        static_cast<const void*>(ptr));
}

// --- Qt value types ---

inline QString toQString(const QModelIndex& index)
{
    if (!index.isValid())
        return u"QModelIndex(Invalid)"_s;

    return QString::asprintf(
        "QModelIndex(row:%d, col:%d, %p)",
        index.row(),
        index.column(),
        index.internalPointer());
}

inline QString toQString(const QPoint& point)
{
    return QString::asprintf("QPoint(x:%d, y:%d)", point.x(), point.y());
}

inline QString toQString(const QStringList& list) { return list.join(u", "_s); }

#ifdef COCO_HAS_XML

inline QString toQString(const QDomElement& element)
{
    if (element.isNull())
        return u"QDomElement(Null)"_s;

    auto tag = element.tagName();
    auto attrs = element.attributes();
    auto count = attrs.count();

    if (count == 0) {
        QString out{};
        out.reserve(tag.size() + 16); // "QDomElement(<>)" + tag
        out.append(u"QDomElement(<"_s);
        out.append(tag);
        out.append(u">)"_s);

        return out;
    }

    // Rough size estimate: tag + per-attr overhead + names/values
    qsizetype estimate = tag.size() + 16;

    for (auto i = 0; i < count; ++i) {
        auto attr = attrs.item(i).toAttr();
        estimate += attr.name().size() + attr.value().size() + 5; // " ='"
    }

    QString out{};
    out.reserve(estimate);
    out.append(u"QDomElement(<"_s);
    out.append(tag);

    for (auto i = 0; i < count; ++i) {
        auto attr = attrs.item(i).toAttr();
        out.append(u' ');
        out.append(attr.name());
        out.append(u"='"_s);
        out.append(attr.value());
        out.append(u'\'');
    }

    out.append(u">)"_s);
    return out;
}

#endif

// --- Variant containers ---

// Like QVariant::toString, we don't wrap printable values in "QVariant(...)"
inline QString toQString(const QVariant& variant)
{
    if (!variant.isValid())
        return u"QVariant(Invalid)"_s;
    if (variant.isNull())
        return u"QVariant(Null)"_s;

    // Check for QObject-derived pointer types via meta-type flags (the
    // documented-correct way). canConvert<QObject*>() + value<QObject*>() is
    // unreliable for subclasses
    if (variant.metaType().flags() & QMetaType::PointerToQObject) {
        return toQString(variant.value<QObject*>());
    }

#ifdef COCO_HAS_XML
    if (variant.canConvert<QDomElement>()) {
        return toQString(variant.value<QDomElement>());
    }
#endif

    // QVariant::toString returns an empty string for each of these types, so
    // each goes to its own overload
    switch (variant.typeId()) {
    case QMetaType::QVariantMap:
        return toQString(variant.value<QVariantMap>());

    case QMetaType::QVariantHash:
        return toQString(variant.value<QVariantHash>());

    case QMetaType::QModelIndex:
        return toQString(variant.value<QModelIndex>());

    case QMetaType::QPoint:
        return toQString(variant.value<QPoint>());

    case QMetaType::QStringList:
        return toQString(variant.value<QStringList>());

    default:
        auto text = variant.toString();
        return text.isEmpty() ? u"QVariant(Non-printable)"_s : text;
    }
}

inline QString toQString(const QVariantHash& variantHash)
{
    if (variantHash.isEmpty())
        return u"QVariantHash()"_s;

    QString out{};
    out.reserve(64 + variantHash.size() * 32); // rough guess
    out.append(u"QVariantHash("_s);

    auto first = true;
    QHashIterator<QString, QVariant> it(variantHash);

    while (it.hasNext()) {
        it.next();
        if (!first)
            out.append(u", "_s);
        first = false;

        out.append(u"{ \""_s);
        out.append(it.key());
        out.append(u"\", "_s);
        out.append(toQString(it.value()));
        out.append(u" }"_s);
    }

    out.append(u')');
    return out;
}

inline QString toQString(const QVariantMap& variantMap)
{
    if (variantMap.isEmpty())
        return u"QVariantMap()"_s;

    QString out{};
    out.reserve(64 + variantMap.size() * 32); // rough guess
    out.append(u"QVariantMap("_s);

    auto first = true;
    QMapIterator<QString, QVariant> it(variantMap);

    while (it.hasNext()) {
        it.next();
        if (!first)
            out.append(u", "_s);
        first = false;

        out.append(u"{ \""_s);
        out.append(it.key());
        out.append(u"\", "_s);
        out.append(toQString(it.value()));
        out.append(u" }"_s);
    }

    out.append(u')');
    return out;
}

// --- Coco types ---

inline QString toQString(const Path& path) { return path.toQString(); }

template <typename TagT> inline QString toQString(const Bool<TagT>& b)
{
    return Bool<TagT>::name(b);
}

} // namespace Coco
