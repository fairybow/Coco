/*
 * Coco  Copyright (C) 2025-2026  fairybow
 *
 * Licensed under GPL 3 with additional terms under Section 7. See LICENSE and
 * ADDITIONAL_TERMS files, or visit: <https://www.gnu.org/licenses/>
 *
 * Uses Qt 6 - <https://www.qt.io/>
 */

#pragma once

#include <filesystem>
#include <format>
#include <ostream>
#include <string>
#include <utility>

#include <QChar>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLatin1StringView>
#include <QList>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QTextStream>
#include <QWidget>

#include "Bool.h"

// TODO: Fix caching / cache clearing
// TODO: Would possibly prefer to cache strings when needed instead of right
// away (could be a problem for arg methods, for example, although they rely on
// QString's arg function to even work at all, so idk)
//
// clang-format off
// 
// TODO:
// Handle QMimeData/QUrl?, like:
// void MainWindow::dropEvent(QDropEvent* event)
// {
//      // We know we have URLs because dragEnterEvent already verified this
//      // We also know the first URL has .json extension
//      QUrl url = event->mimeData()->urls().at(0);
//      auto path = url.toLocalFile();
//
//      if (view_->load(path))
//      {
//          setWindowTitle(QFileInfo(path).fileName());
//          event->acceptProposedAction();
//          activateWindow();
//
//          return;
//      }
// 
//      // If we get here, loading failed
//      event->ignore();
// }
//
// clang-format on

#define TO_QSTRING_(StdFsPath) QString::fromStdString(StdFsPath.string())
#define CACHED_STRING_(DPtr)                                                   \
    (DPtr->cacheValid ? DPtr->cachedString : DPtr->path.string())
#define CACHED_QSTRING_(DPtr)                                                  \
    (DPtr->cacheValid ? DPtr->cachedQString : TO_QSTRING_(DPtr->path))
#define GENERATE_SYS_METHOD_(EnumValue)                                        \
    static Path EnumValue(const char* path = {})                               \
    {                                                                          \
        return !path ? Path(SystemLocation::EnumValue)                         \
                     : Path(SystemLocation::EnumValue) / path;                 \
    }

namespace Coco {

COCO_BOOL(Overwrite);

// TODO: Could maybe be private or removed altogether somehow?
enum class SystemLocation
{
    Root,
    AppConfig,
    AppData,
    AppLocalData,
    Applications,
    Cache,
    Config,
    Desktop,
    Downloads,
    Documents,
    Fonts,
    GenericCache,
    GenericConfig,
    GenericData,
    Home,
    Movies,
    Music,
    Pictures,
    PublicShare,
    Runtime,
    Temp,
    Templates
};

// TODO: Nest in path later
class PathData : public QSharedData
{
public:
    explicit PathData(const std::filesystem::path& other = {})
        : path(other)
        , cachedString(path.string())
        , cachedQString(QString::fromStdString(cachedString))
    {
    }

    std::filesystem::path path;

    // Cached conversions
    std::string cachedString;
    QString cachedQString;

    // Memoization flag (Do we need this?)
    bool cacheValid = true;

    // Just set cache false and then recompile strings as needed. Start with
    // doing both whenever one is requested, then maybe do one at a time as
    // needed
    void invalidateCache()
    {
        cacheValid = false;
        cachedString = path.string();
        cachedQString = QString::fromStdString(cachedString);
        cacheValid = true;
    }
};

// Swiss Army class for Qt/std::filesystem::path interop
class Path
{
public:
    Path()
        : d_(new PathData)
    {
    }

    Path(const Path& other)
        : d_(other.d_)
    {
    }

    Path(Path&& other) noexcept = default;

    Path(const std::filesystem::path& path)
        : d_(new PathData(path))
    {
    }

    Path(const char* path)
        : d_(new PathData(path))
    {
    }

    Path(const std::string& path)
        : d_(new PathData(path))
    {
    }

    Path(const QString& path)
        : d_(new PathData(path.toStdString()))
    {
    }

    Path(SystemLocation location)
        : d_(new PathData(fromSystem_(location).toStdString()))
    {
    }

    // ----- Stream output operators -----

    friend QTextStream& operator<<(QTextStream& outStream, const Path& path)
    {
        return outStream << CACHED_QSTRING_(path.d_);
    }

    friend std::ostream& operator<<(std::ostream& outStream, const Path& path)
    {
        return outStream << CACHED_STRING_(path.d_);
    }

    // By returning a QDebug object (not a reference), we allow the chaining of
    // multiple operator<< calls. This is similar to how std::ostream works, but
    // with the added benefit of managing QDebug's internal state
    friend QDebug operator<<(QDebug debug, const Path& path)
    {
        return debug << CACHED_QSTRING_(path.d_);
    }

    // ----- Assignment operators -----

    Path& operator=(const Path& other) = default;
    Path& operator=(Path&& other) noexcept = default;

    // ----- Comparison operators -----

    bool operator==(const Path& other) const noexcept
    {
        return d_->path == other.d_->path;
    }

    bool operator!=(const Path& other) const noexcept = default;

    bool operator<(const Path& other) const noexcept
    {
        return d_->path < other.d_->path;
    }

    bool operator>(const Path& other) const noexcept = default;

    // ----- Concatenation operators -----

    Path operator/(const Path& other) const
    {
        Path path = *this;
        path /= other;
        return path;
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

    // ----- Conversion operators -----

    explicit operator bool() const noexcept { return !d_->path.empty(); }

    operator std::filesystem::path() const noexcept { return d_->path; }

    // ----- Queries -----

    bool isEmpty() const noexcept { return d_->path.empty(); }

    bool isFile() const
    {
        // return std::filesystem::is_regular_file(m_path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(CACHED_QSTRING_(d_)).isFile();
    }

    bool isFolder() const
    {
        // return std::filesystem::is_directory(m_path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(CACHED_QSTRING_(d_)).isDir();
    }

    bool exists() const
    {
        // return std::filesystem::exists(m_path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(CACHED_QSTRING_(d_)).exists();
    }

    static bool exists(const Path& path)
    {
        // For convenience. Coco::Path::exists(baseQssPath) instead of
        // Coco::Path(baseQssPath).exists(). Shut up.
        return path.exists();
    }

    // ----- Decomposition -----

    Path rootName() const { return d_->path.root_name(); }
    Path rootDirectory() const { return d_->path.root_directory(); }
    Path root() const { return d_->path.root_path(); }
    Path relative() const { return d_->path.relative_path(); }
    Path parent() const { return d_->path.parent_path(); }
    Path filename() const { return d_->path.filename(); }
    Path file() const { return d_->path.filename(); }
    Path stem() const { return d_->path.stem(); }
    Path ext() const { return d_->path.extension(); }
    Path extension() const { return d_->path.extension(); }

    // ----- Modification -----

    void clear() // noexcept
    {
        d_->path.clear();
        d_->invalidateCache();
    }

    Path& makePreferred() // noexcept
    {
        d_->path.make_preferred();
        d_->invalidateCache();
        return *this;
    }

    Path& replaceExt(const Path& replacement = {})
    {
        d_->path.replace_extension(replacement);
        d_->invalidateCache();
        return *this;
    }

    Path& replaceFilename(const Path& replacement)
    {
        d_->path.replace_filename(replacement);
        d_->invalidateCache();
        return *this;
    }

    Path& removeFilename() // noexcept
    {
        d_->path.remove_filename();
        d_->invalidateCache();
        return *this;
    }

    void swap(Path& other) // noexcept
    {
        d_->path.swap(other.d_->path);
        d_->invalidateCache();
        other.d_->invalidateCache();
    }

    template <typename... Args> Path arg(Args&&... args) const
    {
        return CACHED_QSTRING_(d_).arg(std::forward<Args>(args)...);
    }

    Path arg(const QString& a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    Path arg(QChar a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    Path
    arg(QLatin1StringView a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    Path arg(QStringView a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    Path arg(char a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    Path
    arg(int a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(long a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(qlonglong a,
        int fieldWidth = 0,
        int base = 10,
        QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(qulonglong a,
        int fieldWidth = 0,
        int base = 10,
        QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(short a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(ulong a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path arg(ushort a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ')
        const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    Path
    arg(double a,
        int fieldWidth = 0,
        char format = 'g',
        int precision = -1,
        QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_)
            .arg(a, fieldWidth, format, precision, fillChar);
    }

    // ----- Conversion -----

    Path rebase(const Path& oldBase, const Path& newBase) const
    {
        auto relative = d_->path.lexically_relative(oldBase.d_->path);
        return newBase.d_->path / relative;
    }

    QString extQString() const { return TO_QSTRING_(d_->path.extension()); }
    std::string extString() const { return d_->path.extension().string(); }
    QString fileQString() const { return TO_QSTRING_(d_->path.filename()); }
    std::string fileString() const { return d_->path.filename().string(); }

    QString prettyQString() const
    {
        return QString::fromStdString(prettyString());
    }

    std::string prettyString() const;
    QString stemQString() const { return TO_QSTRING_(d_->path.stem()); }
    std::string stemString() const { return d_->path.stem().string(); }
    QString toQString() const { return CACHED_QSTRING_(d_); }
    std::filesystem::path toStd() const noexcept { return d_->path; }
    std::string toString() const { return CACHED_STRING_(d_); }

    // ----- Utility -----

    GENERATE_SYS_METHOD_(Root);
    GENERATE_SYS_METHOD_(AppConfig);
    GENERATE_SYS_METHOD_(AppData);
    GENERATE_SYS_METHOD_(AppLocalData);
    GENERATE_SYS_METHOD_(Applications);
    GENERATE_SYS_METHOD_(Cache);
    GENERATE_SYS_METHOD_(Config);
    GENERATE_SYS_METHOD_(Desktop);
    GENERATE_SYS_METHOD_(Downloads);
    GENERATE_SYS_METHOD_(Documents);
    GENERATE_SYS_METHOD_(Fonts);
    GENERATE_SYS_METHOD_(GenericCache);
    GENERATE_SYS_METHOD_(GenericConfig);
    GENERATE_SYS_METHOD_(GenericData);
    GENERATE_SYS_METHOD_(Home);
    GENERATE_SYS_METHOD_(Movies);
    GENERATE_SYS_METHOD_(Music);
    GENERATE_SYS_METHOD_(Pictures);
    GENERATE_SYS_METHOD_(PublicShare);
    GENERATE_SYS_METHOD_(Runtime);
    GENERATE_SYS_METHOD_(Temp);
    GENERATE_SYS_METHOD_(Templates);

private:
    QSharedDataPointer<PathData> d_;

    // Returning QString just because the Qt functions within this and
    // standardLocation_ also return QString
    QString fromSystem_(SystemLocation value) const;

    QString standardLocation_(QStandardPaths::StandardLocation value) const
    {
        return QStandardPaths::locate(
            value,
            {},
            QStandardPaths::LocateDirectory);
    }
};

using PathList = QList<Path>;

// Creates all directories in the specified path.
inline bool mkdir(const Path& path)
{
    return std::filesystem::create_directories(path.toStd());
}

// Copies a file at the specified path to a new path
inline bool
copy(const Path& path, const Path& newPath, Overwrite overwrite = Overwrite::No)
{
    if (overwrite && newPath.exists()) QFile::remove(newPath.toQString());
    return QFile::copy(path.toQString(), newPath.toQString());
}

// Removes a file at the specified path
inline bool remove(const Path& path) { return QFile::remove(path.toQString()); }

// Copies the contents of one directory to another
inline bool copyContents(const Path& srcDir, const Path& dstDir)
{
    if (!srcDir.exists() || !srcDir.isFolder()) return false;
    if (!dstDir.exists() && !mkdir(dstDir)) return false;

    QDir src_dir(srcDir.toQString());
    auto entries = src_dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (const auto& entry : entries) {
        auto src_path = srcDir / entry;
        auto dst_path = dstDir / entry;

        if (src_path.isFolder()) {
            // Recurse
            if (!mkdir(dst_path)) return false;
            if (!copyContents(src_path, dst_path)) return false;
        } else {
            if (!copy(src_path, dst_path)) return false;
        }
    }

    return true;
}

// inline QString resolveExt(const QString& ext)
//{
//     auto e = ext.trimmed();
//     if (e.isEmpty()) return {};
//     constexpr QChar dot('.');
//     return (e.startsWith(dot) ? e : dot + e);
// }

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

// For isolating paths from QApplication arguments
// TODO: Redo like the above (argPaths vs argFiles)
// PathList argPaths(const QStringList& args, const QString& extensions);

// For isolating paths from main function arguments
// TODO: Redo like the above (argPaths vs argFiles)
// PathList argPaths(int argc, const char* const* argv, const QString&
// extensions);

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
    size_t operator()(const Coco::Path& path) const;
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

inline QDataStream& operator<<(QDataStream& out, const Coco::Path& path)
{
    out << path.toQString();
    return out;
}

inline QDataStream& operator>>(QDataStream& in, Coco::Path& path)
{
    QString s{};
    in >> s;
    path = Coco::Path(s);
    return in;
}

#undef TO_QSTRING_
#undef CACHED_QSTRING_
#undef CACHED_STRING_
#undef GENERATE_SYS_METHOD_

Q_DECLARE_METATYPE(Coco::Path)
