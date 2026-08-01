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

// Coco smoke test.
//
// Assumes the Hearth->Coco fold is done (toQString lives in namespace Coco,
// headers included as <Coco/...>). Returns non-zero on failure so CTest catches
// it. Covers three things:
//   1. COCO_HAS_* macro propagation to a consumer TU (compile-time, both ways)
//   2. Path meta-type converter registration (runtime; proves Path.cpp linked)
//   3. StartCop meta-object linkage (link-time; proves AUTOMOC ran)

#include <QCoreApplication>
#if defined(COCO_HAS_XML)
#    include <QDomDocument>
#endif
#include <QString>
#include <QVariant>

#include <Coco/Debug.h>
#include <Coco/Path.h>
#if defined(COCO_HAS_NETWORK)
#    include <Coco/StartCop.h>
#endif
#include <Coco/ToQString.h>

// COCO_TEST_EXPECT_* come from this test's own CMake and encode what the
// configure step requested, independent of Coco. Cross-checking them against
// the COCO_HAS_* macros proves the PUBLIC compile definitions actually reached
// this translation unit -- and fails loudly (here) if they didn't.
#if defined(COCO_HAS_XML)
static_assert(
    COCO_TEST_EXPECT_XML == 1,
    "COCO_HAS_XML is defined but the build configured XML OFF");
#else
static_assert(
    COCO_TEST_EXPECT_XML == 0,
    "COCO_HAS_XML is not defined but the build configured XML ON");
#endif

#if defined(COCO_HAS_NETWORK)
static_assert(
    COCO_TEST_EXPECT_NET == 1,
    "COCO_HAS_NETWORK is defined but the build configured Network OFF");
#else
static_assert(
    COCO_TEST_EXPECT_NET == 0,
    "COCO_HAS_NETWORK is not defined but the build configured Network ON");
#endif

static int failures = 0;

static void check(bool ok, const char* what)
{
    if (ok) {
        INFO("ok  : {}", what);
    } else {
        WARN("FAIL:", what);
        ++failures;
    }
}

int main(int argc, char* argv[])
{
    using namespace Qt::StringLiterals;

    QCoreApplication app(argc, argv);

    Coco::Debug::initialize(true);

    // --- Path converter registration --------------------------------------
    // The converters are installed ONLY by Path.cpp's static initializer, and
    // Qt's automatic type registration never creates them. So canConvert<> here
    // is the reliable signal that Path.cpp's object file actually linked. If
    // Coco were a STATIC lib and the member got dropped, these go false.
    auto asVariant = QVariant::fromValue(Coco::Path("C:/x/y.txt"));
    check(
        asVariant.canConvert<QString>(),
        "Path -> QString converter installed");
    check(
        !asVariant.value<QString>().isEmpty(),
        "Path -> QString yields a value");

    auto fromString = QVariant(u"a/b/c.md"_s);
    check(
        fromString.canConvert<Coco::Path>(),
        "QString -> Path converter installed");
    check(
        fromString.value<Coco::Path>() == Coco::Path("a/b/c.md"),
        "QString -> Path round-trips");

    // --- ToQString core paths (no optional modules) -----------------------
    check(Coco::toQString(42) == u"42"_s, "toQString(int)");
    check(Coco::toQString(u"hi"_s) == u"hi"_s, "toQString(QString)");

    // --- Optional: Qt Xml -------------------------------------------------
#if defined(COCO_HAS_XML)
    QDomDocument doc;
    auto el = doc.createElement(u"t"_s);
    el.setAttribute(u"a"_s, u"b"_s);
    INFO("xml: {}", Coco::toQString(el));
    check(!Coco::toQString(el).isEmpty(), "toQString(QDomElement) works");
#else
    INFO("xml : disabled at configure time");
#endif

    // --- Optional: Qt Network (also link-checks StartCop's moc output) -----
#if defined(COCO_HAS_NETWORK)
    // Constructing + connecting to the typed signal references StartCop's
    // staticMetaObject; if AUTOMOC didn't run, this fails to LINK.
    Coco::StartCop cop(u"coco-smoke-test"_s, argc, argv);
    QObject::connect(
        &cop,
        &Coco::StartCop::appRelaunched,
        [](const QStringList&) {});
    INFO("net : StartCop constructed and connected");
#else
    INFO("net : disabled at configure time");
#endif

    INFO(failures ? "SMOKE TEST FAILED" : "smoke test passed");
    return failures ? 1 : 0;
}
