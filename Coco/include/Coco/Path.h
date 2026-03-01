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

    Path(const Path& other)
        : d_(other.d_)
    {
    }

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

    void clear() noexcept
    {
        d_->path.clear();
        d_->invalidateCache();
    }

    Path& makePreferred() noexcept
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

    Path& removeName() noexcept
    {
        d_->path.remove_filename();
        d_->invalidateCache();
        return *this;
    }

    void swap(Path& other) noexcept
    {
        d_->path.swap(other.d_->path);
        d_->invalidateCache();
        other.d_->invalidateCache();
    }

    // ----- Conversion -----

    Path rebase(const Path& oldBase, const Path& newBase) const
    {
        auto relative = d_->path.lexically_relative(oldBase.d_->path);
        return newBase.d_->path / relative;
    }

    QString extQString() const { return STD_TO_QSTR_(d_->path.extension()); }
    std::string extString() const { return d_->path.extension().string(); }
    QString nameQString() const { return STD_TO_QSTR_(d_->path.filename()); }
    std::string nameString() const { return d_->path.filename().string(); }

    // TODO: For a display path (normalized forward slashes but no other changes
    // QString prettyQString() const
    //{
    //    return QString::fromStdString(prettyString());
    //}

    // std::string prettyString() const
    //{
    //     std::string pretty{};
    //     auto last_ch_was_sep = false;

    //    for (auto& ch : d_->str()) {
    //        if (ch == '/' || ch == '\\') {
    //            if (!last_ch_was_sep) {
    //                pretty += '/';
    //                last_ch_was_sep = true;
    //            }
    //        } else {
    //            pretty += ch;
    //            last_ch_was_sep = false;
    //        }
    //    }

    //    return pretty;
    //}

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
        mutable QString cachedQString_{};

        mutable bool stringValid_ = false;
        mutable std::string cachedString_{};
    };

    QSharedDataPointer<SharedData_> d_;
};

using PathList = QList<Path>;

// Creates all directories in the specified path.
inline bool mkdir(const Path& path)
{
    return std::filesystem::create_directories(path.toStd());
}

COCO_BOOL(Overwrite);

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
    if (!srcDir.exists() || !srcDir.isDir()) return false;
    if (!dstDir.exists() && !mkdir(dstDir)) return false;

    QDir src_dir(srcDir.toQString());
    auto entries = src_dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

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
// Stream Operator Test:
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

    // QTextStream (output only (input skipped due to whitespace limitation))
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

    return 0;
}
*/
