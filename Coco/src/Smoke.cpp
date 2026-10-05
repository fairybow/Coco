/*
 * Coco — Common code for my Qt projects
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
// it. Covers five things:
//   1. COCO_HAS_* macro propagation to a consumer TU (compile-time, both ways)
//   2. Path meta-type converter registration (runtime; proves Path.cpp linked)
//   3. Path behavior: streams, construction, comparison, decomposition,
//      modification, conversion, the standard-directory functions, and names
//      outside ASCII
//   4. toQString output, for each overload and for each kind of QVariant
//   5. StartCop meta-object linkage (link-time; proves AUTOMOC ran)

#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <sstream>
#include <string>
#include <system_error>

#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#if defined(COCO_HAS_XML)
#    include <QDomDocument>
#endif
#include <QFile>
#include <QIODevice>
#include <QLatin1StringView>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QTemporaryDir>
#include <QTextStream>
#include <QVariant>
#include <QVariantHash>
#include <QVariantMap>

#include <Coco/Bool.h>
#include <Coco/Debug.h>
#include <Coco/Path.h>
#if defined(COCO_HAS_NETWORK)
#    include <Coco/StartCop.h>
#endif
#include <Coco/ToQString.h>

using namespace Qt::StringLiterals;

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

COCO_BOOL(SmokeFlag)

static int failures = 0;

static void check(bool ok, const char* what)
{
    if (ok) {
        INFO("ok  : {}", what);
    } else {
        WARN("FAIL: {}", what);
        ++failures;
    }
}

// The "C:/..." literals below are used on every platform. Off Windows they are
// relative paths whose first component is "C:", which changes nothing for the
// lexical operations checked here. Only root decomposition differs, and that
// is checked per platform in testPathDecomposition

static void testPathStreams()
{
    auto original = Coco::Path("C:/My Documents/test file.txt");

    // std streams write a path quoted, so one with spaces reads back whole
    {
        std::stringstream ss{};
        ss << original;

        Coco::Path round_tripped{};
        ss >> round_tripped;

        check(original == round_tripped, "Path std stream round-trips");
    }

    {
        QByteArray buffer{};

        {
            QDataStream out(&buffer, QIODevice::WriteOnly);
            out << original;
        }

        Coco::Path round_tripped{};

        {
            QDataStream in(&buffer, QIODevice::ReadOnly);
            in >> round_tripped;
        }

        check(original == round_tripped, "Path QDataStream round-trips");
    }

    // Output only
    {
        QString buffer{};

        {
            QTextStream out(&buffer, QIODevice::WriteOnly);
            out << original;
        }

        check(buffer == original.toQString(), "Path QTextStream output");
    }

    // Output only. QDebug quotes a string and adds a trailing space, so look
    // for the path inside the output
    {
        QString buffer{};

        {
            QDebug out(&buffer);
            out << original;
        }

        check(buffer.contains(original.toQString()), "Path QDebug output");
    }

    // An empty path must overwrite what the target held, on both stream kinds
    {
        auto empty = Coco::Path();

        std::stringstream ss{};
        ss << empty;

        auto round_tripped = Coco::Path("not/empty");
        ss >> round_tripped;

        check(round_tripped.isEmpty(), "empty Path std stream round-trips");
    }

    {
        auto empty = Coco::Path();
        QByteArray buffer{};

        {
            QDataStream out(&buffer, QIODevice::WriteOnly);
            out << empty;
        }

        auto round_tripped = Coco::Path("not/empty");

        {
            QDataStream in(&buffer, QIODevice::ReadOnly);
            in >> round_tripped;
        }

        check(round_tripped.isEmpty(), "empty Path QDataStream round-trips");
    }

    {
        auto a = Coco::Path("C:/first path/file.txt");
        auto b = Coco::Path("D:/second path/other.txt");

        std::stringstream ss{};
        ss << a << ' ' << b;

        Coco::Path read_a{};
        Coco::Path read_b{};
        ss >> read_a >> read_b;

        check(a == read_a, "two Paths in one std stream: first");
        check(b == read_b, "two Paths in one std stream: second");
    }
}

static void testPathConstructionAndComparison()
{
    {
        auto from_cstr = Coco::Path("C:/test/file.txt");
        auto from_std = Coco::Path(std::string("C:/test/file.txt"));
        auto from_qstr = Coco::Path(u"C:/test/file.txt"_s);
        auto from_fspath =
            Coco::Path(std::filesystem::path("C:/test/file.txt"));

        check(
            from_cstr == from_std && from_std == from_qstr &&
                from_qstr == from_fspath,
            "Path constructors agree");
    }

    // Copies share data until one is written to
    {
        auto original = Coco::Path("C:/test/file.txt");
        auto copied = original;

        check(original == copied, "Path copy equals its source");

        copied /= "subdir";

        check(original != copied, "Path copy diverges when modified");
        check(
            original == Coco::Path("C:/test/file.txt"),
            "Path source is unchanged by a modified copy");
        check(
            copied == Coco::Path("C:/test/file.txt/subdir"),
            "Path copy holds the modification");
    }

    {
        auto a = Coco::Path("aaa");
        auto b = Coco::Path("bbb");
        auto a2 = Coco::Path("aaa");

        check(a == a2, "Path ==");
        check(a != b, "Path !=");
        check(a < b, "Path <");
        check(b > a, "Path >");
        check(a <= a2, "Path <=");
        check(a >= a2, "Path >=");
    }

    {
        auto empty = Coco::Path();
        auto also_empty = Coco::Path("");

        check(empty.isEmpty(), "default Path is empty");
        check(also_empty.isEmpty(), "Path from \"\" is empty");
        check(empty == also_empty, "empty Paths are equal");
    }
}

static void testPathQueries()
{
    check(
        Coco::Path("C:/a/b").isUnder(Coco::Path("C:/a")),
        "Path isUnder a parent");
    check(
        !Coco::Path("C:/a").isUnder(Coco::Path("C:/a")),
        "Path is not under itself");
    check(
        !Coco::Path("C:/ab").isUnder(Coco::Path("C:/a")),
        "Path is not under a name it only starts with");

    check(!Coco::Path("a/b").hasRoot(), "relative Path has no root");
    check(!Coco::Path().hasRoot(), "empty Path has no root");
    check(Coco::Path("/a").hasRoot(), "Path with a root directory has a root");

#if defined(Q_OS_WIN)

    check(Coco::Path("C:/a").hasRoot(), "Path with a drive has a root");
    check(Coco::Path("C:a").hasRoot(), "drive-relative Path has a root");
    check(Coco::Path("C:\\a").hasRoot(), "backslash Path has a root");

#endif

    check(Coco::Path("a/b").isPlain(), "Path of names is plain");
    check(Coco::Path("/a/b").isPlain(), "rooted Path of names is plain");
    check(Coco::Path().isPlain(), "empty Path is plain");
    check(Coco::Path("/").isPlain(), "bare root is plain");
    check(Coco::Path("a//b").isPlain(), "repeated separator is plain");
    check(Coco::Path(".hidden").isPlain(), "leading-dot name is plain");
    check(Coco::Path("a/..b").isPlain(), "name starting with dots is plain");
    check(Coco::Path("...").isPlain(), "three dots is a name, so plain");

    check(!Coco::Path(".").isPlain(), "\".\" is not plain");
    check(!Coco::Path("..").isPlain(), "\"..\" is not plain");
    check(!Coco::Path("a/./b").isPlain(), "Path through \".\" is not plain");
    check(!Coco::Path("a/../b").isPlain(), "Path through \"..\" is not plain");
    check(!Coco::Path("a/b/..").isPlain(), "Path ending \"..\" is not plain");
    check(!Coco::Path("/a/../b").isPlain(), "rooted \"..\" is not plain");
    check(!Coco::Path("a/b/").isPlain(), "trailing separator is not plain");

#if defined(Q_OS_WIN)

    check(Coco::Path("C:/a/b").isPlain(), "drive Path of names is plain");
    check(Coco::Path("C:/").isPlain(), "bare drive root is plain");
    check(!Coco::Path("C:/a/../b").isPlain(), "drive \"..\" is not plain");
    check(
        !Coco::Path("C:\\a\\..\\b").isPlain(),
        "backslash \"..\" is not plain");
    check(
        !Coco::Path("C:\\a\\b\\").isPlain(),
        "trailing backslash is not plain");

#endif
}

static void testPathDecomposition()
{
    auto p = Coco::Path("C:/Users/fairybow/Documents/report.tar.gz");

    check(
        p.parent() == Coco::Path("C:/Users/fairybow/Documents"),
        "Path parent");
    check(p.name() == Coco::Path("report.tar.gz"), "Path name");
    check(p.stem() == Coco::Path("report.tar"), "Path stem");
    check(p.ext() == Coco::Path(".gz"), "Path ext");

#if defined(Q_OS_WIN)

    check(p.rootName() == Coco::Path("C:"), "Path rootName");
    check(p.rootDir() == Coco::Path("/"), "Path rootDir");
    check(p.root() == Coco::Path("C:/"), "Path root");
    check(
        p.relative() == Coco::Path("Users/fairybow/Documents/report.tar.gz"),
        "Path relative");

#else

    auto rooted = Coco::Path("/Users/fairybow/Documents/report.tar.gz");

    check(rooted.rootName().isEmpty(), "Path rootName");
    check(rooted.rootDir() == Coco::Path("/"), "Path rootDir");
    check(rooted.root() == Coco::Path("/"), "Path root");
    check(
        rooted.relative() ==
            Coco::Path("Users/fairybow/Documents/report.tar.gz"),
        "Path relative");

#endif

    auto root_only = Coco::Path("C:/");

    check(root_only.name().isEmpty(), "root-only Path has no name");
    check(root_only.stem().isEmpty(), "root-only Path has no stem");
    check(root_only.ext().isEmpty(), "root-only Path has no ext");

    auto no_ext = Coco::Path("C:/Users/Makefile");

    check(no_ext.name() == Coco::Path("Makefile"), "extensionless Path name");
    check(no_ext.stem() == Coco::Path("Makefile"), "extensionless Path stem");
    check(no_ext.ext().isEmpty(), "extensionless Path has no ext");

    // A leading dot starts a name, not an extension
    auto dotfile = Coco::Path("C:/Users/.gitignore");

    check(dotfile.name() == Coco::Path(".gitignore"), "dotfile Path name");
    check(dotfile.stem() == Coco::Path(".gitignore"), "dotfile Path stem");
    check(dotfile.ext().isEmpty(), "dotfile Path has no ext");
}

static void testPathModification()
{
    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.replaceExt(".md");

        check(p == Coco::Path("C:/docs/file.md"), "Path replaceExt");
    }

    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.replaceExt();

        check(p == Coco::Path("C:/docs/file"), "Path replaceExt removes");
    }

    {
        auto p = Coco::Path("C:/docs/old.txt");
        p.replaceName("new.txt");

        check(p == Coco::Path("C:/docs/new.txt"), "Path replaceName");
    }

    // The trailing separator stays
    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.removeName();

        check(p == Coco::Path("C:/docs/"), "Path removeName");
    }

    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.clear();

        check(p.isEmpty(), "Path clear");
    }

    {
        auto a = Coco::Path("C:/first");
        auto b = Coco::Path("D:/second");
        a.swap(b);

        check(a == Coco::Path("D:/second"), "Path swap: first");
        check(b == Coco::Path("C:/first"), "Path swap: second");
    }

    {
        auto base = Coco::Path("C:/Users");
        auto joined = base / "fairybow" / "Documents";

        check(
            joined == Coco::Path("C:/Users/fairybow/Documents"),
            "Path operator/");

        auto appended = Coco::Path("C:/file");
        appended += ".txt";

        check(appended == Coco::Path("C:/file.txt"), "Path operator+=");
    }

    // Changes the separators' spelling, not the path's value
    {
        auto p = Coco::Path("C:/Users/fairybow/Documents");
        p.makePreferred();

        check(
            p == Coco::Path("C:/Users/fairybow/Documents"),
            "Path makePreferred keeps the value");

#if defined(Q_OS_WIN)

        check(
            !p.toQString().contains(u'/'),
            "Path makePreferred uses backslashes");

#endif
    }

    {
        auto p = Coco::Path("C:/My Documents/test.txt");
        auto formatted = std::format("Path is: {}", p);

        check(
            formatted == "Path is: C:/My Documents/test.txt",
            "Path std::format");
    }
}

static void testPathConversion()
{
    auto p = Coco::Path("C:/Users/fairybow/Documents/file.txt");

    check(
        p.toQString() == u"C:/Users/fairybow/Documents/file.txt"_s,
        "Path toQString");
    check(
        p.toString() == "C:/Users/fairybow/Documents/file.txt",
        "Path toString");
    check(Coco::Path(p.toStd()) == p, "Path toStd round-trips");

    check(p.extQString() == u".txt"_s, "Path extQString");
    check(p.extString() == ".txt", "Path extString");
    check(p.nameQString() == u"file.txt"_s, "Path nameQString");
    check(p.nameString() == "file.txt", "Path nameString");
    check(p.stemQString() == u"file"_s, "Path stemQString");
    check(p.stemString() == "file", "Path stemString");

    {
        auto file = Coco::Path("C:/old/project/src/main.cpp");
        auto rebased = file.rebase("C:/old/project", "D:/new/project");

        check(
            rebased == Coco::Path("D:/new/project/src/main.cpp"),
            "Path rebase");
    }

    // A path that can't be expressed relative to the old base rebases to an
    // empty path: a different drive on Windows, an absolute path against a
    // relative base elsewhere
    {

#if defined(Q_OS_WIN)

        auto file = Coco::Path("C:/completely/different/path.txt");
        auto rebased = file.rebase("D:/unrelated", "E:/target");

#else

        auto file = Coco::Path("/completely/different/path.txt");
        auto rebased = file.rebase("unrelated", "/target");

#endif

        check(rebased.isEmpty(), "Path rebase from an unreachable base");
    }

    {
        auto file = Coco::Path("C:/project/file.txt");
        auto rebased = file.rebase("C:/project", "C:/project");

        check(rebased == file, "Path rebase onto the same base");
    }

    {
        auto dir = Coco::Path("C:/project");
        auto rebased = dir.rebase("C:/project", "D:/new");

        check(rebased == Coco::Path("D:/new"), "Path rebase of the base");
    }

    {
        auto a = Coco::Path("C:/test/file.txt");
        auto b = Coco::Path("C:/test/file.txt");
        auto c = Coco::Path("C:/test/other.txt");

        auto hash_a = std::hash<Coco::Path>{}(a);
        auto hash_b = std::hash<Coco::Path>{}(b);
        auto hash_c = std::hash<Coco::Path>{}(c);

        check(hash_a == hash_b, "equal Paths hash equal");
        check(hash_a != hash_c, "different Paths hash differently");
    }

    {
        auto original = Coco::Path("C:/test/file.txt");
        auto variant = QVariant::fromValue(original);

        check(
            variant.value<Coco::Path>() == original,
            "Path QVariant round-trips");
        check(
            variant.value<QString>() == original.toQString(),
            "Path QVariant converts to QString");
    }
}

// prettyQString gives single forward slashes and no trailing slash, and
// changes nothing else
static void testPathPrettyString()
{
    {
        auto p = Coco::Path("C:/Users") / "fairybow" / "Documents";

        check(
            p.prettyQString() == u"C:/Users/fairybow/Documents"_s,
            "pretty Path: joined");
    }

    {
        auto p = Coco::Path("C://Users////fairybow");

        check(
            p.prettyQString() == u"C:/Users/fairybow"_s,
            "pretty Path: repeated separators");
    }

    {
        auto p = Coco::Path("C:/Users/./fairybow/../Documents");

        check(
            p.prettyQString() == u"C:/Users/./fairybow/../Documents"_s,
            "pretty Path: dot and dot-dot kept");
    }

    {
        auto p = Coco::Path("C:\\Users\\fairybow\\Documents");

        check(
            p.prettyQString() == u"C:/Users/fairybow/Documents"_s,
            "pretty Path: backslashes");
    }

    {
        auto p = Coco::Path("C:/Users/fairybow/");

        check(
            p.prettyQString() == u"C:/Users/fairybow"_s,
            "pretty Path: trailing slash");
    }

    {
        auto unix_root = Coco::Path("/");
        auto win_root = Coco::Path("C:/");

        check(unix_root.prettyQString() == u"/"_s, "pretty Path: / kept");
        check(win_root.prettyQString() == u"C:/"_s, "pretty Path: C:/ kept");
    }
}

// The locations themselves differ per machine, so only their shape is checked
static void testPathStandardDirs()
{
    check(!Coco::Path::Root().isEmpty(), "Path::Root is not empty");
    check(!Coco::Path::Home().isEmpty(), "Path::Home is not empty");
    check(!Coco::Path::Desktop().isEmpty(), "Path::Desktop is not empty");
    check(!Coco::Path::Documents().isEmpty(), "Path::Documents is not empty");
    check(!Coco::Path::Downloads().isEmpty(), "Path::Downloads is not empty");
    check(!Coco::Path::AppData().isEmpty(), "Path::AppData is not empty");
    check(!Coco::Path::Cache().isEmpty(), "Path::Cache is not empty");
    check(!Coco::Path::Temp().isEmpty(), "Path::Temp is not empty");

    {
        auto sub = Coco::Path::AppData("settings");

        check(
            sub.name() == Coco::Path("settings"),
            "Path standard dir appends a subpath");
        check(
            sub.parent() == Coco::Path::AppData(),
            "Path standard dir subpath sits under the dir");
    }

    {
        auto plain = Coco::Path::Home();

        check(
            Coco::Path::Home(nullptr) == plain,
            "Path standard dir with nullptr");
        check(
            Coco::Path::Home("") == plain,
            "Path standard dir with an empty string");
    }
}

// Runs a check whose body may throw: a string conversion inside
// std::filesystem::path throws when it can't convert. A throw is a failure,
// reported with its message, and the run goes on
template <typename TestT>
static void checkNoThrow(const QByteArray& what, TestT test)
{
    try {
        check(test(), what.constData());
    } catch (const std::exception& e) {
        WARN("threw: {}", e.what());
        check(false, what.constData());
    }
}

// One name outside ASCII, as UTF-16 text and as the same text's UTF-8 bytes.
// The stored path is compared against a std::filesystem::path built from the
// UTF-16 text, which converts the same way under any system code page
static void testPathNonAsciiName(
    const char* tag,
    const QString& name,
    const std::string& nameUtf8)
{
    auto label = [tag](const char* what) {
        return QByteArray(tag) + ": " + what;
    };

    auto expected = std::filesystem::path(name.toStdU16String());

    checkNoThrow(label("Path(QString) stores the name"), [&] {
        return Coco::Path(name).toStd() == expected;
    });

    checkNoThrow(label("Path(const char*) stores the name"), [&] {
        return Coco::Path(nameUtf8.c_str()).toStd() == expected;
    });

    checkNoThrow(label("Path(std::string) stores the name"), [&] {
        return Coco::Path(nameUtf8).toStd() == expected;
    });

    checkNoThrow(label("Path(std::filesystem::path) reads back"), [&] {
        return Coco::Path(expected).toQString() == name;
    });

    checkNoThrow(label("Path(QString) reads back"), [&] {
        return Coco::Path(name).toQString() == name;
    });

    checkNoThrow(label("Path constructors agree"), [&] {
        return Coco::Path(name) == Coco::Path(nameUtf8.c_str()) &&
               Coco::Path(name) == Coco::Path(expected);
    });

    checkNoThrow(label("Path toString is UTF-8"), [&] {
        return Coco::Path(name).toString() == nameUtf8;
    });

    checkNoThrow(label("Path name, stem, and ext"), [&] {
        auto file = Coco::Path(u"dir"_s) / Coco::Path(name + u".txt"_s);

        return file.nameQString() == name + u".txt"_s &&
               file.stemQString() == name && file.extQString() == u".txt"_s;
    });

    checkNoThrow(label("pretty Path"), [&] {
        auto file = Coco::Path(u"dir"_s) / Coco::Path(name);
        return file.prettyQString() == u"dir/"_s + name;
    });

    checkNoThrow(label("Path std::format is UTF-8"), [&] {
        return std::format("{}", Coco::Path(name)) == nameUtf8;
    });

    checkNoThrow(label("Path genericString is UTF-8"), [&] {
        auto file = Coco::Path(u"dir"_s) / Coco::Path(name);
        return file.genericString() == "dir/" + nameUtf8;
    });

    checkNoThrow(label("Path std stream writes UTF-8"), [&] {
        std::stringstream ss{};
        ss << Coco::Path(name);

        return ss.str() == "\"" + nameUtf8 + "\"";
    });

    checkNoThrow(label("Path std stream round-trips"), [&] {
        std::stringstream ss{};
        ss << Coco::Path(name);

        Coco::Path round_tripped{};
        ss >> round_tripped;

        return round_tripped.toStd() == expected;
    });
}

// The same name as a real file. Qt creates it, so the file on disk has the
// right name however Path stores it
static void testPathNonAsciiFile(const char* tag, const QString& name)
{
    auto label = [tag](const char* what) {
        return QByteArray(tag) + ": " + what;
    };

    QTemporaryDir temp_dir{};
    auto file_name = name + u".txt"_s;
    auto file_path = temp_dir.filePath(file_name);

    {
        QFile file(file_path);

        if (!temp_dir.isValid() || !file.open(QIODevice::WriteOnly)) {
            check(false, label("temp file created").constData());
            return;
        }
    }

    checkNoThrow(label("Path exists and isFile"), [&] {
        auto path = Coco::Path(file_path);
        return path.exists() && path.isFile();
    });

    checkNoThrow(label("std::filesystem finds Path's toStd"), [&] {
        std::error_code error{};
        return std::filesystem::exists(Coco::Path(file_path).toStd(), error);
    });

    checkNoThrow(label("filePaths lists the file by name"), [&] {
        for (const auto& path : Coco::filePaths(Coco::Path(temp_dir.path()))) {
            if (path.nameQString() == file_name) {
                return true;
            }
        }

        return false;
    });
}

// Written as escapes so the checks don't depend on how the compiler reads this
// file: U+00E9, then U+65E5 U+672C U+8A9E, then U+1F4C1 (outside the BMP, so
// two UTF-16 code units)
static void testPathNonAscii()
{
    auto accented = u"\u00E9"_s;
    auto japanese = u"\u65E5\u672C\u8A9E"_s;
    auto emoji = u"\U0001F4C1"_s;

    testPathNonAsciiName("accented", accented, "\xC3\xA9");
    testPathNonAsciiName(
        "japanese",
        japanese,
        "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E");
    testPathNonAsciiName("emoji", emoji, "\xF0\x9F\x93\x81");

    testPathNonAsciiFile("accented", accented);
    testPathNonAsciiFile("japanese", japanese);
    testPathNonAsciiFile("emoji", emoji);
}

static void testToQString()
{
    check(
        Coco::toQString(QStringView(u"view")) == u"view"_s,
        "toQString(QStringView)");
    check(
        Coco::toQString(QLatin1StringView("latin")) == u"latin"_s,
        "toQString(QLatin1StringView)");
    check(Coco::toQString("chars") == u"chars"_s, "toQString(const char*)");

    check(Coco::toQString(true) == u"true"_s, "toQString(true)");
    check(Coco::toQString(false) == u"false"_s, "toQString(false)");

    check(Coco::toQString(55u) == u"55"_s, "toQString(unsigned)");
    check(Coco::toQString(-55LL) == u"-55"_s, "toQString(long long)");
    check(Coco::toQString(1.5) == u"1.5"_s, "toQString(double)");

    // The type name in a plain pointer's text is implementation-defined, so
    // only the null case has an exact form
    {
        const int* null_ptr = nullptr;
        auto value = 0;

        check(
            Coco::toQString(null_ptr) == u"nullptr"_s,
            "toQString(null pointer)");

        auto text = Coco::toQString(&value);
        check(!text.isEmpty() && text != u"nullptr"_s, "toQString(pointer)");
    }

    // A QObject pointer names its class, from the meta-object
    {
        QObject* null_object = nullptr;
        QObject object{};

        check(
            Coco::toQString(null_object) == u"nullptr"_s,
            "toQString(null QObject*)");
        check(
            Coco::toQString(&object).startsWith(u"QObject("_s),
            "toQString(QObject*)");
        check(
            Coco::toQString(QCoreApplication::instance())
                .startsWith(u"QCoreApplication("_s),
            "toQString(QObject subclass*)");
    }

    check(
        Coco::toQString(QModelIndex()) == u"QModelIndex(Invalid)"_s,
        "toQString(invalid QModelIndex)");
    check(
        Coco::toQString(QPoint(10, 20)) == u"QPoint(x:10, y:20)"_s,
        "toQString(QPoint)");
    check(
        Coco::toQString(QStringList{ u"one"_s, u"two"_s, u"three"_s }) ==
            u"one, two, three"_s,
        "toQString(QStringList)");

    check(
        Coco::toQString(Coco::Path("a/b/c.md")) == u"a/b/c.md"_s,
        "toQString(Path)");
    check(
        Coco::toQString(SmokeFlag::Yes) == u"SmokeFlag::Yes"_s,
        "toQString(Bool Yes)");
    check(
        Coco::toQString(SmokeFlag::No) == u"SmokeFlag::No"_s,
        "toQString(Bool No)");

    // Bool's std::format support, which nothing else here goes through
    check(
        std::format("{}", SmokeFlag::Yes) == "SmokeFlag::Yes",
        "Bool std::format");

#if defined(COCO_HAS_XML)

    {
        QDomDocument doc{};
        auto bare = doc.createElement(u"t"_s);
        auto with_attr = doc.createElement(u"t"_s);
        with_attr.setAttribute(u"a"_s, u"b"_s);

        check(
            Coco::toQString(QDomElement()) == u"QDomElement(Null)"_s,
            "toQString(null QDomElement)");
        check(
            Coco::toQString(bare) == u"QDomElement(<t>)"_s,
            "toQString(QDomElement)");
        check(
            Coco::toQString(with_attr) == u"QDomElement(<t a='b'>)"_s,
            "toQString(QDomElement with an attribute)");
    }

#endif
}

// A QVariant gives its value's text with no "QVariant(...)" wrapper, and a
// label only when there is no value to show
static void testToQStringVariant()
{
    check(
        Coco::toQString(QVariant()) == u"QVariant(Invalid)"_s,
        "toQString(invalid QVariant)");

    // A variant made from a type alone has a type and no value
    check(
        Coco::toQString(QVariant(QMetaType::fromType<QString>())) ==
            u"QVariant(Null)"_s,
        "toQString(null QVariant)");

    check(Coco::toQString(QVariant(55)) == u"55"_s, "toQString(QVariant int)");
    check(
        Coco::toQString(QVariant(true)) == u"true"_s,
        "toQString(QVariant bool)");
    check(
        Coco::toQString(QVariant(u"Hello"_s)) == u"Hello"_s,
        "toQString(QVariant QString)");

    // QVariant::toString gives nothing for these, so each has its own text
    check(
        Coco::toQString(QVariant(QPoint(10, 20))) == u"QPoint(x:10, y:20)"_s,
        "toQString(QVariant QPoint)");
    check(
        Coco::toQString(
            QVariant(QStringList{ u"one"_s, u"two"_s, u"three"_s })) ==
            u"one, two, three"_s,
        "toQString(QVariant QStringList)");
    check(
        Coco::toQString(QVariant::fromValue(QModelIndex())) ==
            u"QModelIndex(Invalid)"_s,
        "toQString(QVariant QModelIndex)");

    // A type with no text of its own and no overload here
    check(
        Coco::toQString(QVariant(QRect(1, 2, 3, 4))) ==
            u"QVariant(Non-printable)"_s,
        "toQString(QVariant with no text)");

    // A QMap iterates in key order, so two keys have one possible text
    {
        QVariantMap map{
            { u"a"_s, 1        },
            { u"b"_s, u"two"_s }
        };

        check(
            Coco::toQString(QVariantMap{}) == u"QVariantMap()"_s,
            "toQString(empty QVariantMap)");
        check(
            Coco::toQString(map) ==
                u"QVariantMap({ \"a\", 1 }, { \"b\", two })"_s,
            "toQString(QVariantMap)");
        check(
            Coco::toQString(QVariant(map)) == Coco::toQString(map),
            "toQString(QVariant QVariantMap)");
    }

    // A QHash has no fixed order, so one key keeps the text exact
    {
        QVariantHash hash{
            { u"key"_s, 1 }
        };

        check(
            Coco::toQString(QVariantHash{}) == u"QVariantHash()"_s,
            "toQString(empty QVariantHash)");
        check(
            Coco::toQString(hash) == u"QVariantHash({ \"key\", 1 })"_s,
            "toQString(QVariantHash)");
        check(
            Coco::toQString(QVariant(hash)) == Coco::toQString(hash),
            "toQString(QVariant QVariantHash)");
    }

    // A value inside a container goes through the same QVariant overload
    {
        QVariantMap map{
            { u"p"_s, QPoint(1, 2) }
        };

        check(
            Coco::toQString(map) ==
                u"QVariantMap({ \"p\", QPoint(x:1, y:2) })"_s,
            "toQString(QVariantMap holding a QPoint)");
    }

    // A QObject pointer held in a variant names its real class, whether the
    // variant holds it as a QObject* or as a pointer to the subclass. A null
    // one prints as it does outside a variant, though the variant is then
    // null too
    {
        QObject object{};
        auto* app = QCoreApplication::instance();

        check(
            Coco::toQString(QVariant::fromValue(&object))
                .startsWith(u"QObject("_s),
            "toQString(QVariant QObject*)");
        check(
            Coco::toQString(QVariant::fromValue(app))
                .startsWith(u"QCoreApplication("_s),
            "toQString(QVariant QObject subclass*)");
        check(
            Coco::toQString(QVariant::fromValue<QObject*>(nullptr)) ==
                u"nullptr"_s,
            "toQString(QVariant null QObject*)");
        check(
            Coco::toQString(QVariant::fromValue<QCoreApplication*>(nullptr)) ==
                u"nullptr"_s,
            "toQString(QVariant null QObject subclass*)");
    }

#if defined(COCO_HAS_XML)

    {
        QDomDocument doc{};
        auto element = doc.createElement(u"t"_s);
        element.setAttribute(u"a"_s, u"b"_s);

        check(
            Coco::toQString(QVariant::fromValue(element)) ==
                u"QDomElement(<t a='b'>)"_s,
            "toQString(QVariant QDomElement)");
    }

#endif
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    Coco::Debug::init(true);

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

    // --- Path behavior ----------------------------------------------------
    testPathStreams();
    testPathConstructionAndComparison();
    testPathQueries();
    testPathDecomposition();
    testPathModification();
    testPathConversion();
    testPathPrettyString();
    testPathStandardDirs();
    testPathNonAscii();

    // --- ToQString core paths (no optional modules) -----------------------
    check(Coco::toQString(42) == u"42"_s, "toQString(int)");
    check(Coco::toQString(u"hi"_s) == u"hi"_s, "toQString(QString)");
    testToQString();
    testToQStringVariant();

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
    QObject::connect(&cop, &Coco::StartCop::relaunched, [](const QStringList&) {
    });
    INFO("net : StartCop constructed and connected");
#else
    INFO("net : disabled at configure time");
#endif

    INFO(failures ? "SMOKE TEST FAILED" : "smoke test passed");
    return failures ? 1 : 0;
}
