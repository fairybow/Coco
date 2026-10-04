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

#include <compare>
#include <filesystem>
#include <format>
#include <functional>
#include <iomanip>
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

#include "Coco/Bool.h"

namespace Coco {

// Path is a Swiss Army class designed to be a `std::filesystem::path` surrogate
// for Qt (instead of relying on `QString`). It includes `std::filesystem::path`
// functionality as well as various utility functions to make it easier to work
// with (and to allow avoidance of `QDir`, `QFile`, and related classes unless
// really needed)
//
// Encoding: every narrow string this class takes or returns (const char*,
// std::string, std::format output, std streams) is UTF-8, on every platform.
// Qt does all the converting. std::filesystem::path is only ever handed
// UTF-16 or its own native string, and is never asked for a narrow one: on
// Windows it would use the system code page, which reads UTF-8 as something
// else and throws for a character the code page lacks
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

    // UTF-8
    Path(const char* path)
        : d_(new SharedData_(toStd_(QString::fromUtf8(path))))
    {
    }

    // UTF-8
    Path(const std::string& path)
        : d_(new SharedData_(toStd_(QString::fromStdString(path))))
    {
    }

    Path(const QString& path)
        : d_(new SharedData_(toStd_(path)))
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

    // UTF-8, quoted (as std::filesystem::path's own operators quote), so a
    // path with spaces reads back whole
    friend std::istream& operator>>(std::istream& in, Path& path)
    {
        std::string s{};
        in >> std::quoted(s);
        path = Path(s);
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Path& path)
    {
        return out << std::quoted(path.d_->str());
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

    // isFile, isDir, and exists ask QFileInfo, not std::filesystem, so they
    // also answer for Qt resource paths (":/...")

    bool isFile() const { return QFileInfo(d_->qstr()).isFile(); }
    bool isDir() const { return QFileInfo(d_->qstr()).isDir(); }

    bool isEmptyDir() const
    {
        if (!isDir()) {
            return false;
        }

        return QDir(d_->qstr())
            .isEmpty(QDir::AllEntries | QDir::NoDotAndDotDot);
    }

    bool exists() const { return QFileInfo(d_->qstr()).exists(); }

    // True if this path is `base` itself or nested inside it. A pure walk up
    // the parents, comparing with operator== (std::filesystem::path's own,
    // which treats both separators as separators) — so it never routes through
    // prettyString, toQString, or any display form. Bounded by the parent fixed
    // point (a root's parent_path is itself), so an unrelated absolute path
    // terminates rather than looping
    bool isAtOrUnder(const Path& base) const
    {
        auto p = *this;
        while (true) {
            if (p == base) {
                return true;
            }

            auto up = p.parent();
            if (up == p) { // a root: parent_path stops shrinking
                return false;
            }

            p = up;
        }
    }

    // True if this path is nested inside `base`, and is not `base` itself. The
    // same walk up the parents as isAtOrUnder, starting one step up, so the
    // path is never compared against `base` directly
    bool isUnder(const Path& base) const
    {
        auto p = *this;
        while (true) {
            auto up = p.parent();
            if (up == p) { // a root: parent_path stops shrinking
                return false;
            }

            if (up == base) {
                return true;
            }

            p = up;
        }
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

    // Re: noexcept: Non-const access to d_ (QSharedDataPointer::operator->())
    // may call detach(), which copies via `new` and can throw std::bad_alloc.
    // So despite the underlying std::filesystem::path operations and
    // invalidateCache() all being noexcept, these mutating methods cannot
    // guarantee noexcept

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

    friend void swap(Path& a, Path& b) noexcept { a.swap(b); }

    // ----- Conversion -----

    Path rebase(const Path& oldBase, const Path& newBase) const
    {
        auto rel = d_->path.lexically_relative(oldBase.d_->path);

        if (rel.empty()) {
            return {};
        }

        if (rel == std::filesystem::path(".")) {
            return newBase;
        }
        
        return newBase.d_->path / rel;
    }

    Path lexicallyRelative(const Path& base) const
    {
        return d_->path.lexically_relative(base.d_->path);
    }

    // The std::string forms below are UTF-8

    // Forward slashes only. Asked for in the path's own character type, so
    // std::filesystem converts nothing
    std::string genericString() const
    {
        using NativeChar = std::filesystem::path::value_type;
        return toUtf8_(d_->path.generic_string<NativeChar>());
    }

    QString extQString() const { return toQString_(d_->path.extension()); }
    std::string extString() const { return toUtf8_(d_->path.extension()); }
    QString nameQString() const { return toQString_(d_->path.filename()); }
    std::string nameString() const { return toUtf8_(d_->path.filename()); }

    // For a uniform display path (single forward slashes and no trailing slash,
    // with no other changes (keeps dot and dot-dot))
    QString prettyQString() const
    {
        return QString::fromStdString(prettyString());
    }

    // For a uniform display path (single forward slashes and no trailing slash,
    // with no other changes (keeps dot and dot-dot))
    // Edge case: a bare "//" input will be reduced to "/". This is acceptable
    // since bare UNC prefixes are not valid paths
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
        if (pretty.size() > 1 && pretty.back() == '/' &&
            pretty[pretty.size() - 2] != ':') {
            pretty.pop_back();
        }

        return pretty;
    }

    QString stemQString() const { return toQString_(d_->path.stem()); }
    std::string stemString() const { return toUtf8_(d_->path.stem()); }

    std::filesystem::path toStd() const { return d_->path; }
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
    // The three conversions every string in or out of a Path goes through.
    // They match Qt's own (QtPrivate::toFilesystemPath and fromFilesystemPath,
    // in qfile.h): UTF-16 in, the native string out. On Windows the native
    // string is already UTF-16, so nothing is converted and nothing can fail

    static std::filesystem::path toStd_(const QString& s)
    {
        return std::filesystem::path(s.toStdU16String());
    }

    static QString toQString_(const std::filesystem::path& p)
    {
#if defined(Q_OS_WIN)
        return QString::fromStdWString(p.native());
#else
        return QString::fromStdString(p.native());
#endif
    }

    static std::string toUtf8_(const std::filesystem::path& p)
    {
        return toQString_(p).toStdString();
    }

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
                cachedQString_ = toQString_(path);
                qStringValid_ = true;
            }

            return cachedQString_;
        }

        // UTF-8
        const std::string& str() const
        {
            if (!stringValid_) {
                cachedString_ = qstr().toStdString();
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

using PathList = QList<Path>;

// Creates a single directory (parent must exist)
inline bool mkdir(
    const Path& dir,
    std::optional<QFile::Permissions> permissions = std::nullopt)
{
    return QDir().mkdir(dir.toQString(), permissions);
}

// Creates all directories in the specified path (creates parents)
inline bool mkpath(
    const Path& path,
    std::optional<QFile::Permissions> permissions = std::nullopt)
{
    return QDir().mkpath(path.toQString(), permissions);
}

// Removes a single empty directory
inline bool rmdir(const Path& dir) { return QDir().rmdir(dir.toQString()); }

// Removes the directory and all empty parent directories in the path
inline bool rmpath(const Path& path) { return QDir().rmpath(path.toQString()); }

// Removes the directory and all its contents
inline bool purge(const Path& dir)
{
    return QDir(dir.toQString()).removeRecursively();
}

COCO_BOOL(Overwrite)

// Renames the file at the specified path
inline bool rename(const Path& oldPath, const Path& newPath)
{
    return QFile::rename(oldPath.toQString(), newPath.toQString());
}

// Copies the file at the specified path to the new path
inline bool
copy(const Path& path, const Path& newPath, Overwrite overwrite = Overwrite::No)
{
    if (overwrite) {
        QFile::remove(newPath.toQString());
    }

    return QFile::copy(path.toQString(), newPath.toQString());
}

// Removes the file at the specified path
inline bool remove(const Path& path) { return QFile::remove(path.toQString()); }

// Moves the file or directory at the specified path to the system trash.
// Returns false, leaving the entry in place, if the platform has no trash for
// it (on Windows, a network share or an item too large for the Recycle Bin —
// Qt refuses rather than deleting permanently)
inline bool moveToTrash(const Path& path)
{
    return QFile::moveToTrash(path.toQString());
}

// Copies the contents of one directory to another
inline bool copyContents(const Path& srcDir, const Path& dstDir)
{
    if (!srcDir.exists() || !srcDir.isDir()) {
        return false;
    }

    if (!dstDir.exists() && !mkdir(dstDir)) {
        return false;
    }

    QDir src_dir(srcDir.toQString());
    auto entries = src_dir.entryList(
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    for (const auto& entry : entries) {
        auto src_path = srcDir / entry;
        auto dst_path = dstDir / entry;

        if (src_path.isDir()) {
            // Recurse
            if (!mkdir(dst_path)) {
                return false;
            }

            if (!copyContents(src_path, dst_path)) {
                return false;
            }

        } else {
            if (!copy(src_path, dst_path)) {
                return false;
            }
        }
    }

    return true;
}

inline bool exists(const Path& path)
{
    // For convenience, e.g. Coco::exists("path/to/thing")
    return path.exists();
}

inline QStringList toQStringList(const PathList& paths)
{
    QStringList result{};
    result.reserve(paths.size());

    for (auto& p : paths) {
        result << p.toQString();
    }

    return result;
}

inline QStringList toPrettyQStringList(const PathList& paths)
{
    QStringList result{};
    result.reserve(paths.size());

    for (auto& p : paths) {
        result << p.prettyQString();
    }

    return result;
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

// Every file under dir, recursively, descending only into the subdirectories
// shouldDescend accepts. Unlike allFilePaths, a rejected subdirectory is
// pruned, not filtered afterward: nothing beneath it is ever visited, so a
// large subtree the caller doesn't want (a .git/, say) costs one predicate
// call, not a walk.
//
// Symlinked and junctioned directories are never descended — QDirIterator's
// own default (no FollowSymlinks), and it rules out cycles. Entries with the
// Windows hidden attribute are skipped, since QDir::Hidden isn't set. Order
// is unspecified
inline PathList walkFilePaths(
    const Path& dir,
    const std::function<bool(const Path& subdir)>& shouldDescend)
{
    PathList result{};
    PathList pending{ dir };

    while (!pending.isEmpty()) {
        auto current = pending.takeLast();

        QDirIterator it(
            current.toQString(),
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

        while (it.hasNext()) {
            it.next();
            auto info = it.fileInfo();
            Path path(it.filePath());

            if (!info.isDir()) {
                result << path;
                continue;
            }

            if (info.isSymLink() || info.isJunction()) {
                continue;
            }

            if (shouldDescend(path)) {
                pending << path;
            }
        }
    }

    return result;
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

    for (const auto& str : string_paths) {
        paths << Path(str);
    }

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

Q_DECLARE_METATYPE(Coco::Path)
