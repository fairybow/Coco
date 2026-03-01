/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE, or
 * visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#pragma once

#include <compare>
#include <filesystem>
#include <format>
#include <istream>
#include <optional>
#include <ostream>
#include <string>

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QWidget>

#include "Bool.h"

#define STD_TO_QSTR_(StdPath) QString::fromStdString(StdPath.string())

namespace Coco {

// Path is a Swiss Army class designed to be a `std::filesystem::path` surrogate
// for Qt (instead of relying on `QString`). It includes `std::filesystem::path`
// functionality as well as various utility functions to make it easier to work
// with (and to allow avoidance of `QDir`, `QFile`, and related classes unless
// really needed)
class Path
{
public:
    Path()
        : d_(new SharedData_)
    {
    }

    Path(const Path& other) = default;
    Path(Path&& other) noexcept = default;

    Path(const std::filesystem::path& path)
        : d_(new SharedData_(path))
    {
    }

    Path(const char* path)
        : d_(new SharedData_(path))
    {
    }

    Path(const std::string& path)
        : d_(new SharedData_(path))
    {
    }

    Path(const QString& path)
        : d_(new SharedData_(path.toStdString()))
    {
    }

    // ----- Stream operators -----

    friend QDataStream& operator>>(QDataStream& in, Path& path)
    {
        QString s{};
        in >> s;
        path = Path(s);
        return in;
    }

    friend QDataStream& operator<<(QDataStream& out, const Path& path)
    {
        return out << path.d_->qstr();
    }

    template <class CharT, class TraitsT>
    friend std::basic_istream<CharT, TraitsT>&
    operator>>(std::basic_istream<CharT, TraitsT>& in, Path& path)
    {
        std::filesystem::path p{};
        in >> p;
        path = Path(p);
        return in;
    }

    template <class CharT, class TraitsT>
    friend std::basic_ostream<CharT, TraitsT>&
    operator<<(std::basic_ostream<CharT, TraitsT>& out, const Path& path)
    {
        return out << path.d_->path;
    }

    // Output only. By returning a QDebug object (not a reference), we allow the
    // chaining of multiple operator<< calls
    friend QDebug operator<<(QDebug debug, const Path& path)
    {
        return debug << path.d_->qstr();
    }

    // Output only (input skipped due to whitespace limitation)
    friend QTextStream& operator<<(QTextStream& out, const Path& path)
    {
        return out << path.d_->qstr();
    }

    // ----- Assignment operators -----

    Path& operator=(const Path& other) = default;
    Path& operator=(Path&& other) noexcept = default;

    // ----- Comparison operators -----
    // operator== and operator<=> are sufficient; the compiler synthesizes !=,
    // <, >, <=, and >= from these two (C++20)

    bool operator==(const Path& other) const noexcept
    {
        return d_->path == other.d_->path;
    }

    std::strong_ordering operator<=>(const Path& other) const noexcept
    {
        return d_->path <=> other.d_->path;
    }

    // ----- Concatenation operators -----

    friend Path operator/(const Path& lhs, const Path& rhs)
    {
        return Path(lhs) /= rhs;
    }

    Path& operator/=(const Path& other)
    {
        d_->path /= other.d_->path;
        d_->invalidateCache();
        return *this;
    }

    Path& operator+=(const Path& other)
    {
        d_->path += other.d_->path;
        d_->invalidateCache();
        return *this;
    }

    // ----- Queries -----

    bool isEmpty() const noexcept { return d_->path.empty(); }

    bool isFile() const
    {
        // return std::filesystem::is_regular_file(d_->path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(d_->qstr()).isFile();
    }

    bool isDir() const
    {
        // return std::filesystem::is_directory(d_->path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(d_->qstr()).isDir();
    }

    bool exists() const
    {
        // return std::filesystem::exists(d_->path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(d_->qstr()).exists();
    }

    // ----- Decomposition -----

    Path rootName() const { return d_->path.root_name(); }
    Path rootDir() const { return d_->path.root_directory(); }
    Path root() const { return d_->path.root_path(); }
    Path relative() const { return d_->path.relative_path(); }
    Path parent() const { return d_->path.parent_path(); }
    Path name() const { return d_->path.filename(); }
    Path stem() const { return d_->path.stem(); }
    Path ext() const { return d_->path.extension(); }

    // ----- Modification -----

    void clear()
    {
        d_->path.clear();
        d_->invalidateCache();
    }

    Path& makePreferred()
    {
        d_->path.make_preferred();
        d_->invalidateCache();
        return *this;
    }

    Path& replaceExt(const Path& replacement = {})
    {
        d_->path.replace_extension(replacement.d_->path);
        d_->invalidateCache();
        return *this;
    }

    Path& replaceName(const Path& replacement)
    {
        d_->path.replace_filename(replacement.d_->path);
        d_->invalidateCache();
        return *this;
    }

    Path& removeName()
    {
        d_->path.remove_filename();
        d_->invalidateCache();
        return *this;
    }

    void swap(Path& other) noexcept { d_.swap(other.d_); }

    // ----- Conversion -----

    Path rebase(const Path& oldBase, const Path& newBase) const
    {
        auto rel = d_->path.lexically_relative(oldBase.d_->path);
        if (rel.empty()) return {};
        if (rel == std::filesystem::path(".")) return newBase;
        return newBase.d_->path / rel;
    }

    QString extQString() const { return STD_TO_QSTR_(d_->path.extension()); }
    std::string extString() const { return d_->path.extension().string(); }
    QString nameQString() const { return STD_TO_QSTR_(d_->path.filename()); }
    std::string nameString() const { return d_->path.filename().string(); }

    // For a uniform display path (single forward slashes and no trailing slash,
    // with no other changes (keeps dot and dot-dot))
    // TODO (maybe): Caching? If this was used to display a path in a tree view,
    // for example, we might need it?
    QString prettyQString() const
    {
        return QString::fromStdString(prettyString());
    }

    // For a uniform display path (single forward slashes and no trailing slash,
    // with no other changes (keeps dot and dot-dot))
    // TODO (maybe): Caching? If this was used to display a path in a tree view,
    // for example, we might need it?
    std::string prettyString() const
    {
        auto& d_str = d_->str();
        std::string pretty{};
        pretty.reserve(d_str.size());
        auto last_was_sep = false;

        for (auto& ch : d_str) {
            if (ch == '/' || ch == '\\') {
                if (!last_was_sep) {
                    pretty += '/';
                    last_was_sep = true;
                }
            } else {
                pretty += ch;
                last_was_sep = false;
            }
        }

        // Don't strip if the slash is the root directory component
        if (pretty.size() > 1 && pretty.back() == '/'
            && pretty[pretty.size() - 2] != ':') {
            pretty.pop_back();
        }

        return pretty;
    }

    QString stemQString() const { return STD_TO_QSTR_(d_->path.stem()); }
    std::string stemString() const { return d_->path.stem().string(); }

    std::filesystem::path toStd() const noexcept { return d_->path; }
    QString toQString() const { return d_->qstr(); }
    std::string toString() const { return d_->str(); }

    // For batch queries
    QFileInfo toQFileInfo() const { return QFileInfo(d_->qstr()); }

    // ----- Utility -----

#define GEN_STD_DIR_METHOD_1_(Name, Fn)                                        \
    static Path Name(const char* cStrPath = {})                                \
    {                                                                          \
        Path base(Fn);                                                         \
        return (!cStrPath || !*cStrPath) ? base : base / cStrPath;             \
    }
#define GEN_STD_DIR_METHOD_2_(Name, QtLocation)                                \
    GEN_STD_DIR_METHOD_1_(Name, QStandardPaths::writableLocation(QtLocation))

    GEN_STD_DIR_METHOD_1_(Root, QDir::rootPath())
    GEN_STD_DIR_METHOD_2_(AppConfig, QStandardPaths::AppConfigLocation)
    GEN_STD_DIR_METHOD_2_(AppData, QStandardPaths::AppDataLocation)
    GEN_STD_DIR_METHOD_2_(AppLocalData, QStandardPaths::AppLocalDataLocation)
    GEN_STD_DIR_METHOD_2_(Applications, QStandardPaths::ApplicationsLocation)
    GEN_STD_DIR_METHOD_2_(Cache, QStandardPaths::CacheLocation)
    GEN_STD_DIR_METHOD_2_(Config, QStandardPaths::ConfigLocation)
    GEN_STD_DIR_METHOD_2_(Desktop, QStandardPaths::DesktopLocation)
    GEN_STD_DIR_METHOD_2_(Downloads, QStandardPaths::DownloadLocation)
    GEN_STD_DIR_METHOD_2_(Documents, QStandardPaths::DocumentsLocation)
    GEN_STD_DIR_METHOD_2_(Fonts, QStandardPaths::FontsLocation)
    GEN_STD_DIR_METHOD_2_(GenericCache, QStandardPaths::GenericCacheLocation)
    GEN_STD_DIR_METHOD_2_(GenericConfig, QStandardPaths::GenericConfigLocation)
    GEN_STD_DIR_METHOD_2_(GenericData, QStandardPaths::GenericDataLocation)
    GEN_STD_DIR_METHOD_2_(Home, QStandardPaths::HomeLocation)
    GEN_STD_DIR_METHOD_2_(Movies, QStandardPaths::MoviesLocation)
    GEN_STD_DIR_METHOD_2_(Music, QStandardPaths::MusicLocation)
    GEN_STD_DIR_METHOD_2_(Pictures, QStandardPaths::PicturesLocation)
    GEN_STD_DIR_METHOD_2_(PublicShare, QStandardPaths::PublicShareLocation)
    GEN_STD_DIR_METHOD_2_(Runtime, QStandardPaths::RuntimeLocation)
    GEN_STD_DIR_METHOD_2_(Temp, QStandardPaths::TempLocation)
    GEN_STD_DIR_METHOD_2_(Templates, QStandardPaths::TemplatesLocation)

#undef GEN_STD_DIR_METHOD_1_
#undef GEN_STD_DIR_METHOD_2_

private:
    // Thread safety: SharedData_ relies on QSharedData's copy-on-write for
    // mutation safety, but const methods (str(), qstr()) lazily populate
    // mutable cache fields. If two threads share the same underlying data (no
    // prior write to trigger COW detachment) and both call a const method
    // concurrently, they will race on the cache. This is fine for typical
    // GUI-thread usage but needs care in multithreaded console applications
    class SharedData_ : public QSharedData
    {
    public:
        explicit SharedData_(const std::filesystem::path& other = {})
            : path(other)
        {
        }

        std::filesystem::path path;

        void invalidateCache() noexcept
        {
            stringValid_ = false;
            qStringValid_ = false;
        }

        const QString& qstr() const
        {
            if (!qStringValid_) {
                cachedQString_ = QString::fromStdString(str());
                qStringValid_ = true;
            }

            return cachedQString_;
        }

        const std::string& str() const
        {
            if (!stringValid_) {
                cachedString_ = path.string();
                stringValid_ = true;
            }

            return cachedString_;
        }

    private:
        mutable bool qStringValid_ = false;
        mutable bool stringValid_ = false;
        mutable QString cachedQString_{};
        mutable std::string cachedString_{};
    };

    QSharedDataPointer<SharedData_> d_;
};

inline void swap(Path& a, Path& b) noexcept { a.swap(b); }

using PathList = QList<Path>;

// Creates all directories in the specified path.
inline bool mkdir(
    const Path& path,
    std::optional<QFile::Permissions> permissions = std::nullopt)
{
    return QDir().mkpath(path.toQString(), permissions);
}

COCO_BOOL(Overwrite);

// Renames the file at the specified path
inline bool rename(const Path& oldPath, const Path& newPath)
{
    return QFile::rename(oldPath.toQString(), newPath.toQString());
}

// Copies the file at the specified path to the new path
inline bool
copy(const Path& path, const Path& newPath, Overwrite overwrite = Overwrite::No)
{
    if (overwrite) QFile::remove(newPath.toQString());
    return QFile::copy(path.toQString(), newPath.toQString());
}

// Removes the file at the specified path
inline bool remove(const Path& path) { return QFile::remove(path.toQString()); }

// Copies the contents of one directory to another
inline bool copyContents(const Path& srcDir, const Path& dstDir)
{
    if (!srcDir.exists() || !srcDir.isDir()) return false;
    if (!dstDir.exists() && !mkdir(dstDir)) return false;

    QDir src_dir(srcDir.toQString());
    auto entries = src_dir.entryList(
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    for (const auto& entry : entries) {
        auto src_path = srcDir / entry;
        auto dst_path = dstDir / entry;

        if (src_path.isDir()) {
            // Recurse
            if (!mkdir(dst_path)) return false;
            if (!copyContents(src_path, dst_path)) return false;
        } else {
            if (!copy(src_path, dst_path)) return false;
        }
    }

    return true;
}

inline bool exists(const Path& path)
{
    // For convenience, e.g. Coco::exists("path/to/thing")
    return path.exists();
}

// Iterator wrappers

// Provide extensions as: `{ "*.mp3", "*.wav" }`
inline PathList paths(
    const Path& dir,
    const QStringList& exts,
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    PathList result{};
    QDirIterator it(dir.toQString(), exts, filters, flags);

    while (it.hasNext()) {
        it.next();
        result << it.filePath();
    }

    return result;
}

// Provide extensions as: `{ "*.mp3", "*.wav" }`
inline PathList paths(
    const PathList& dirs,
    const QStringList& exts,
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    PathList result{};

    for (auto& dir : dirs) {
        result << paths(dir, exts, filters, flags);
    }

    return result;
}

inline PathList paths(
    const Path& dir,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dir, {}, QDir::AllEntries | QDir::NoDotAndDotDot, flags);
}

inline PathList paths(
    const Path& dir,
    QDir::Filters filters,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dir, {}, filters, flags);
}

inline PathList paths(
    const PathList& dirs,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dirs, {}, QDir::AllEntries | QDir::NoDotAndDotDot, flags);
}

inline PathList paths(
    const PathList& dirs,
    QDir::Filters filters,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dirs, {}, filters, flags);
}

inline PathList filePaths(
    const Path& dir,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dir, {}, QDir::Files, flags);
}

inline PathList allFilePaths(const Path& dir)
{
    return paths(dir, {}, QDir::Files, QDirIterator::Subdirectories);
}

// Provide extensions as: `{ "*.mp3", "*.wav" }`
inline PathList filePaths(
    const Path& dir,
    const QStringList& exts,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dir, exts, QDir::Files, flags);
}

// Provide extensions as: `{ "*.mp3", "*.wav" }`
inline PathList allFilePaths(const Path& dir, const QStringList& exts)
{
    return paths(dir, exts, QDir::Files, QDirIterator::Subdirectories);
}

inline PathList filePaths(
    const PathList& dirs,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dirs, {}, QDir::Files, flags);
}

inline PathList allFilePaths(const PathList& dirs)
{
    return paths(dirs, {}, QDir::Files, QDirIterator::Subdirectories);
}

// Provide extensions as: `{ "*.mp3", "*.wav" }`
inline PathList filePaths(
    const PathList& dirs,
    const QStringList& exts,
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
{
    return paths(dirs, exts, QDir::Files, flags);
}

// Provide extensions as: `{ "*.mp3", "*.wav" }`
inline PathList allFilePaths(const PathList& dirs, const QStringList& exts)
{
    return paths(dirs, exts, QDir::Files, QDirIterator::Subdirectories);
}

inline Path getDir(
    QWidget* parent = nullptr,
    const QString& caption = {},
    const Path& startPath = {})
{
    return Path(
        QFileDialog::getExistingDirectory(
            parent,
            caption,
            startPath.toQString()));
}

inline Path getFile(
    QWidget* parent = nullptr,
    const QString& caption = {},
    const Path& startPath = {},
    const QString& filter = {},
    QString* selectedFilter = nullptr,
    QFileDialog::Options options = {})
{
    return Path(
        QFileDialog::getOpenFileName(
            parent,
            caption,
            startPath.toQString(),
            filter,
            selectedFilter,
            options));
}

inline PathList getFiles(
    QWidget* parent = nullptr,
    const QString& caption = {},
    const Path& startPath = {},
    const QString& filter = {},
    QString* selectedFilter = nullptr,
    QFileDialog::Options options = {})
{
    auto string_paths = QFileDialog::getOpenFileNames(
        parent,
        caption,
        startPath.toQString(),
        filter,
        selectedFilter,
        options);

    PathList paths{};
    paths.reserve(string_paths.size());
    for (const auto& str : string_paths)
        paths << Path(str);

    return paths;
}

inline Path getSaveFile(
    QWidget* parent = nullptr,
    const QString& caption = {},
    const Path& startPath = {},
    const QString& filter = {},
    QString* selectedFilter = nullptr)
{
    return Path(
        QFileDialog::getSaveFileName(
            parent,
            caption,
            startPath.toQString(),
            filter,
            selectedFilter));
}

} // namespace Coco

namespace std {

template <> struct hash<Coco::Path>
{
    size_t operator()(const Coco::Path& path) const
    {
        return hash<filesystem::path>()(path.toStd());
    }
};

template <> struct formatter<Coco::Path> : formatter<string>
{
    auto format(const Coco::Path& path, format_context& ctx) const
    {
        return formatter<string>::format(path.toString(), ctx);
    }
};

} // namespace std

inline Coco::Path operator"" _ccpath(const char* cString, std::size_t)
{
    return Coco::Path(cString);
}

#undef STD_TO_QSTR_

Q_DECLARE_METATYPE(Coco::Path)

/*
// Stream Operator Tests:

#include "Coco/Path.h"

#include <sstream>

#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QTextStream>

int main()
{
    auto original = Coco::Path("C:/My Documents/test file.txt");

    // std streams (quoted roundtrip)
    {
        std::stringstream ss{};
        ss << original;

        Coco::Path roundtripped{};
        ss >> roundtripped;

        qDebug() << "std out:" << original;
        qDebug() << "std in: " << roundtripped;
        qDebug() << "match:  " << (original == roundtripped);
    }

    // QDataStream (binary roundtrip)
    {
        QByteArray buffer{};

        {
            QDataStream out(&buffer, QIODevice::WriteOnly);
            out << original;
        }

        Coco::Path roundtripped{};

        {
            QDataStream in(&buffer, QIODevice::ReadOnly);
            in >> roundtripped;
        }

        qDebug() << "QDataStream out:" << original;
        qDebug() << "QDataStream in: " << roundtripped;
        qDebug() << "match:          " << (original == roundtripped);
    }

    // QTextStream (output only)
    {
        QString buffer{};

        {
            QTextStream out(&buffer, QIODevice::WriteOnly);
            out << original;
        }

        qDebug() << "QTextStream out:" << buffer;
    }

    // QDebug (output only)
    {
        qDebug() << "QDebug:" << original;
    }

    // Empty path roundtrip
    {
        auto empty = Coco::Path();

        std::stringstream ss{};
        ss << empty;

        Coco::Path roundtripped{};
        ss >> roundtripped;

        qDebug() << "empty std match:" << (empty == roundtripped);

        QByteArray buffer{};

        {
            QDataStream out(&buffer, QIODevice::WriteOnly);
            out << empty;
        }

        {
            QDataStream in(&buffer, QIODevice::ReadOnly);
            in >> roundtripped;
        }

        qDebug() << "empty QDS match:" << (empty == roundtripped);
    }

    // Multiple paths in one std stream
    {
        auto a = Coco::Path("C:/first path/file.txt");
        auto b = Coco::Path("D:/second path/other.txt");

        std::stringstream ss{};
        ss << a << ' ' << b;

        Coco::Path ra{};
        Coco::Path rb{};
        ss >> ra >> rb;

        qDebug() << "multi std a match:" << (a == ra);
        qDebug() << "multi std b match:" << (b == rb);
    }

    return 0;
}
*/

/*
// Construction & Comparison Tests:

#include "Coco/Path.h"

#include <QDebug>

int main()
{
    // Construction from various types
    {
        auto from_cstr = Coco::Path("C:/test/file.txt");
        auto from_std = Coco::Path(std::string("C:/test/file.txt"));
        auto from_qstr = Coco::Path(QString("C:/test/file.txt"));
        auto from_fspath =
            Coco::Path(std::filesystem::path("C:/test/file.txt"));
        auto from_literal = "C:/test/file.txt"_ccpath;

        qDebug() << "all equal:"
                 << (from_cstr == from_std && from_std == from_qstr
                     && from_qstr == from_fspath
                     && from_fspath == from_literal);
    }

    // Copy and move semantics (COW)
    {
        auto original = Coco::Path("C:/test/file.txt");
        auto copied = original;

        qDebug() << "copy equal:" << (original == copied);

        copied /= "subdir";

        qDebug() << "diverged:     " << (original != copied);
        qDebug() << "original safe:"
                 << (original == Coco::Path("C:/test/file.txt"));
        qDebug() << "copy changed: "
                 << (copied == Coco::Path("C:/test/file.txt/subdir"));
    }

    // Comparison operators
    {
        auto a = Coco::Path("aaa");
        auto b = Coco::Path("bbb");
        auto a2 = Coco::Path("aaa");

        qDebug() << "a == a2:" << (a == a2);
        qDebug() << "a != b: " << (a != b);
        qDebug() << "a < b:  " << (a < b);
        qDebug() << "b > a:  " << (b > a);
        qDebug() << "a <= a2:" << (a <= a2);
    }

    // Empty path
    {
        auto empty = Coco::Path();
        auto also_empty = Coco::Path("");

        qDebug() << "default empty:" << empty.isEmpty();
        qDebug() << "string empty: " << also_empty.isEmpty();
        qDebug() << "empties equal:" << (empty == also_empty);
    }

    return 0;
}
*/

/*
// Decomposition Tests:

#include "Coco/Path.h"

#include <QDebug>

int main()
{
    auto p = Coco::Path("C:/Users/fairybow/Documents/report.tar.gz");

    qDebug() << "rootName:" << p.rootName();
    qDebug() << "rootDir: " << p.rootDir();
    qDebug() << "root:    " << p.root();
    qDebug() << "relative:" << p.relative();
    qDebug() << "parent:  " << p.parent();
    qDebug() << "name:    " << p.name();
    qDebug() << "stem:    " << p.stem();
    qDebug() << "ext:     " << p.ext();

    // Edge cases
    auto root_only = Coco::Path("C:/");

    qDebug() << "root name empty: " << root_only.name().isEmpty();
    qDebug() << "root stem empty: " << root_only.stem().isEmpty();
    qDebug() << "root ext empty:  " << root_only.ext().isEmpty();

    auto no_ext = Coco::Path("C:/Users/Makefile");

    qDebug() << "no ext name:" << no_ext.name();
    qDebug() << "no ext stem:" << no_ext.stem();
    qDebug() << "no ext ext:  " << no_ext.ext().isEmpty();

    auto dotfile = Coco::Path("C:/Users/.gitignore");

    qDebug() << "dotfile name:" << dotfile.name();
    qDebug() << "dotfile stem:" << dotfile.stem();
    qDebug() << "dotfile ext: " << dotfile.ext();

    return 0;
}
*/

/*
// Modification Tests:

#include <format>

#include "Coco/Path.h"

#include <QDebug>

int main()
{
    // replaceExt
    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.replaceExt(".md");

        qDebug() << "replaceExt:" << p;
        qDebug() << "match:     " << (p == Coco::Path("C:/docs/file.md"));
    }

    // replaceExt (remove)
    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.replaceExt();

        qDebug() << "removeExt:" << p;
        qDebug() << "match:    " << (p == Coco::Path("C:/docs/file"));
    }

    // replaceName
    {
        auto p = Coco::Path("C:/docs/old.txt");
        p.replaceName("new.txt");

        qDebug() << "replaceName:" << p;
        qDebug() << "match:      " << (p == Coco::Path("C:/docs/new.txt"));
    }

    // removeName
    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.removeName();

        qDebug() << "removeName:" << p;
    }

    // clear
    {
        auto p = Coco::Path("C:/docs/file.txt");
        p.clear();

        qDebug() << "clear empty:" << p.isEmpty();
    }

    // swap
    {
        auto a = Coco::Path("C:/first");
        auto b = Coco::Path("D:/second");
        a.swap(b);

        qDebug() << "swap a:" << (a == Coco::Path("D:/second"));
        qDebug() << "swap b:" << (b == Coco::Path("C:/first"));
    }

    // Concatenation
    {
        auto base = Coco::Path("C:/Users");
        auto joined = base / "fairybow" / "Documents";

        qDebug() << "operator/:" << joined;

        auto appended = Coco::Path("C:/file");
        appended += ".txt";

        qDebug() << "operator+=:" << appended;
        qDebug() << "match:     " << (appended == Coco::Path("C:/file.txt"));
    }

    // makePreferred
    {
        auto p = Coco::Path("C:/Users/fairybow/Documents");
        p.makePreferred();

        qDebug() << "makePreferred:" << p;
    }

    // std::format
    {
        auto p = Coco::Path("C:/My Documents/test.txt");
        auto formatted = std::format("Path is: {}", p);
        qDebug() << "std::format:" << QString::fromStdString(formatted);
    }

    return 0;
}
*/

/*
// Conversion Tests:
#include "Coco/Path.h"

#include <QDebug>

int main()
{
    auto p = Coco::Path("C:/Users/fairybow/Documents/file.txt");

    // String conversions
    {
        auto std_str = p.toString();
        auto qstr = p.toQString();
        auto fspath = p.toStd();

        qDebug() << "toString:  " << QString::fromStdString(std_str);
        qDebug() << "toQString: " << qstr;
        qDebug() << "toStd:     " << QString::fromStdString(fspath.string());
    }

    // Component string shortcuts
    {
        qDebug() << "extQString: " << p.extQString();
        qDebug() << "extString:  " << QString::fromStdString(p.extString());
        qDebug() << "nameQString:" << p.nameQString();
        qDebug() << "nameString: " << QString::fromStdString(p.nameString());
        qDebug() << "stemQString:" << p.stemQString();
        qDebug() << "stemString: " << QString::fromStdString(p.stemString());
    }

    // rebase (normal case)
    {
        auto file = Coco::Path("C:/old/project/src/main.cpp");
        auto rebased = file.rebase("C:/old/project", "D:/new/project");

        qDebug() << "rebase:" << rebased;
        qDebug() << "match: "
                 << (rebased == Coco::Path("D:/new/project/src/main.cpp"));
    }

    // rebase (unrelated base, should return empty)
    {
        auto file = Coco::Path("C:/completely/different/path.txt");
        auto rebased = file.rebase("D:/unrelated", "E:/target");

        qDebug() << "rebase unrelated empty:" << rebased.isEmpty();
    }

    // rebase (same base)
    {
        auto file = Coco::Path("C:/project/file.txt");
        auto rebased = file.rebase("C:/project", "C:/project");

        qDebug() << "rebase same:" << rebased;
        qDebug() << "match:      " << (rebased == file);
    }

    // rebase (path equals base)
    {
        auto dir = Coco::Path("C:/project");
        auto rebased = dir.rebase("C:/project", "D:/new");

        qDebug() << "rebase identity:" << (rebased == Coco::Path("D:/new"));
    }

    // std::hash
    {
        auto a = Coco::Path("C:/test/file.txt");
        auto b = Coco::Path("C:/test/file.txt");
        auto c = Coco::Path("C:/test/other.txt");

        auto ha = std::hash<Coco::Path>{}(a);
        auto hb = std::hash<Coco::Path>{}(b);
        auto hc = std::hash<Coco::Path>{}(c);

        qDebug() << "hash equal match:  " << (ha == hb);
        qDebug() << "hash differ differ:" << (ha != hc);
    }

    // QVariant roundtrip
    {
        auto original = Coco::Path("C:/test/file.txt");
        auto variant = QVariant::fromValue(original);
        auto back = variant.value<Coco::Path>();

        qDebug() << "QVariant roundtrip:" << (original == back);

        // Through QString conversion
        auto as_string = variant.value<QString>();
        qDebug() << "QVariant->QString: " << as_string;
    }

    // "Pretty" strings
    {
        // Mixed separators
        {
            auto p = Coco::Path("C:/Users") / "fairybow" / "Documents";

            qDebug() << "raw:    " << p;
            qDebug() << "pretty: " << p.prettyQString();
            qDebug() << "match:  "
                     << (p.prettyQString() == "C:/Users/fairybow/Documents");
        }

        // Duplicate separators
        {
            auto p = Coco::Path("C://Users////fairybow");

            qDebug() << "dupes pretty:" << p.prettyQString();
            qDebug() << "match:       "
                     << (p.prettyQString() == "C:/Users/fairybow");
        }

        // Preserves dot and dot-dot
        {
            auto p = Coco::Path("C:/Users/./fairybow/../Documents");

            qDebug() << "dots pretty:" << p.prettyQString();
            qDebug() << "match:      "
                     << (p.prettyQString()
                         == "C:/Users/./fairybow/../Documents");
        }

        // Backslashes only
        {
            auto p = Coco::Path("C:\\Users\\fairybow\\Documents");

            qDebug() << "backslash pretty:" << p.prettyQString();
            qDebug() << "match:           "
                     << (p.prettyQString() == "C:/Users/fairybow/Documents");
        }

        // Trailing slash normalization
        {
            auto p = Coco::Path("C:/Users/fairybow/");

            qDebug() << "trailing slash pretty:" << p.prettyQString();
            qDebug() << "match:                "
                     << (p.prettyQString() == "C:/Users/fairybow");
        }

        // Root paths preserved
        {
            auto unix_root = Coco::Path("/");
            auto win_root = Coco::Path("C:/");

            qDebug() << "unix root pretty:" << unix_root.prettyQString();
            qDebug() << "unix preserved:  "
                     << (unix_root.prettyQString() == "/");
            qDebug() << "win root pretty: " << win_root.prettyQString();
            qDebug() << "win preserved:   "
                     << (win_root.prettyQString() == "C:/");
        }
    }

    return 0;
}
*/

/*
// Utility Tests (standard directory methods):

#include "Coco/Path.h"

#include <QDebug>

int main()
{
    // Basic locations resolve to non-empty paths
    qDebug() << "Root:      " << Coco::Path::Root();
    qDebug() << "Home:      " << Coco::Path::Home();
    qDebug() << "Desktop:   " << Coco::Path::Desktop();
    qDebug() << "Documents: " << Coco::Path::Documents();
    qDebug() << "Downloads: " << Coco::Path::Downloads();
    qDebug() << "AppData:   " << Coco::Path::AppData();
    qDebug() << "Cache:     " << Coco::Path::Cache();
    qDebug() << "Temp:      " << Coco::Path::Temp();

    // Subpath appending
    {
        auto app_config = Coco::Path::AppData("settings");

        qDebug() << "AppData subpath:" << app_config;
        qDebug() << "ends correctly: "
                 << app_config.toQString().endsWith("settings");
    }

    // Null vs empty c-string
    {
        auto with_null = Coco::Path::Home(nullptr);
        auto with_empty = Coco::Path::Home("");
        auto plain = Coco::Path::Home();

        qDebug() << "null == plain: " << (with_null == plain);
        qDebug() << "empty == plain:" << (with_empty == plain);
    }

    return 0;
}
*/
