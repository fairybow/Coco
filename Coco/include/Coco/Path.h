#pragma once

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <string>
#include <utility>

#include <QChar>
#include <QDataStream>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFlags>
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
#include <QtTypes>

#include "Global.h"

// TODO: Would possibly prefer to cache strings when needed instead of right
// away (could be a problem for arg methods, for example, although they rely on
// QString's arg function to even work at all, so idk)
//
// clang-format off
// 
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
//
// Or do this in PathUtil (rename File?) or similar!

#define TO_QSTRING_(StdFsPath) QString::fromStdString(StdFsPath.string())
#define CACHED_STRING_(DPtr)                                                   \
    (DPtr->cacheValid ? DPtr->cachedString : DPtr->path.string())
#define CACHED_QSTRING_(DPtr)                                                  \
    (DPtr->cacheValid ? DPtr->cachedQString : TO_QSTRING_(DPtr->path))
// #define GENERATE_SYS_METHOD_(EnumValue) COCO_ALWAYS_INLINE static Path
// EnumValue() { return Path(SystemLocation::EnumValue); }

#define GENERATE_SYS_METHOD_(EnumValue)                                        \
    COCO_ALWAYS_INLINE static Path EnumValue(const char* path = {})            \
    {                                                                          \
        return !path ? Path(SystemLocation::EnumValue)                         \
                     : Path(SystemLocation::EnumValue) / path;                 \
    }

namespace Coco {

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
    COCO_ALWAYS_INLINE void invalidateCache()
    {
        cacheValid = false;
        cachedString = path.string();
        cachedQString = QString::fromStdString(cachedString);
        cacheValid = true;
    }
};

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

    COCO_ALWAYS_INLINE bool operator==(const Path& other) const noexcept
    {
        return d_->path == other.d_->path;
    }

    COCO_ALWAYS_INLINE bool
    operator!=(const Path& other) const noexcept = default;

    COCO_ALWAYS_INLINE bool operator<(const Path& other) const noexcept
    {
        return d_->path < other.d_->path;
    }

    COCO_ALWAYS_INLINE bool
    operator>(const Path& other) const noexcept = default;

    // ----- Concatenation operators -----

    COCO_ALWAYS_INLINE Path operator/(const Path& other) const
    {
        Path path = *this;
        path /= other;
        return path;
    }

    COCO_ALWAYS_INLINE Path& operator/=(const Path& other)
    {
        d_->path /= other.d_->path;
        d_->invalidateCache();
        return *this;
    }

    COCO_ALWAYS_INLINE Path& operator+=(const Path& other)
    {
        d_->path += other.d_->path;
        d_->invalidateCache();
        return *this;
    }

    // ----- Conversion operators -----

    COCO_ALWAYS_INLINE explicit operator bool() const noexcept
    {
        return !d_->path.empty();
    }

    COCO_ALWAYS_INLINE operator std::filesystem::path() const noexcept
    {
        return d_->path;
    }

    // ----- Queries -----

    COCO_ALWAYS_INLINE bool isEmpty() const noexcept
    {
        return d_->path.empty();
    }

    COCO_ALWAYS_INLINE bool isFile() const
    {
        // return std::filesystem::is_regular_file(m_path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(CACHED_QSTRING_(d_)).isFile();
    }

    COCO_ALWAYS_INLINE bool isFolder() const
    {
        // return std::filesystem::is_directory(m_path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(CACHED_QSTRING_(d_)).isDir();
    }

    COCO_ALWAYS_INLINE bool exists() const
    {
        // return std::filesystem::exists(m_path);
        //  ^ Valid paths with non-standard characters won't return valid.
        return QFileInfo(CACHED_QSTRING_(d_)).exists();
    }

    COCO_ALWAYS_INLINE static bool exists(const Path& path)
    {
        // For convenience. Coco::Path::exists(baseQssPath) instead of
        // Coco::Path(baseQssPath).exists(). Shut up.
        return path.exists();
    }

    // ----- Decomposition -----

    COCO_ALWAYS_INLINE Path rootName() const { return d_->path.root_name(); }

    COCO_ALWAYS_INLINE Path rootDirectory() const
    {
        return d_->path.root_directory();
    }

    COCO_ALWAYS_INLINE Path root() const { return d_->path.root_path(); }

    COCO_ALWAYS_INLINE Path relative() const
    {
        return d_->path.relative_path();
    }

    COCO_ALWAYS_INLINE Path parent() const { return d_->path.parent_path(); }

    COCO_ALWAYS_INLINE Path filename() const { return d_->path.filename(); }

    COCO_ALWAYS_INLINE Path file() const { return d_->path.filename(); }

    COCO_ALWAYS_INLINE Path stem() const { return d_->path.stem(); }

    COCO_ALWAYS_INLINE Path ext() const { return d_->path.extension(); }

    COCO_ALWAYS_INLINE Path extension() const { return d_->path.extension(); }

    // ----- Modification -----

    COCO_ALWAYS_INLINE void clear() // noexcept
    {
        d_->path.clear();
        d_->invalidateCache();
    }

    COCO_ALWAYS_INLINE Path& makePreferred() // noexcept
    {
        d_->path.make_preferred();
        d_->invalidateCache();
        return *this;
    }

    COCO_ALWAYS_INLINE Path& replaceExt(const Path& replacement = {})
    {
        d_->path.replace_extension(replacement);
        d_->invalidateCache();
        return *this;
    }

    COCO_ALWAYS_INLINE Path& replaceFilename(const Path& replacement)
    {
        d_->path.replace_filename(replacement);
        d_->invalidateCache();
        return *this;
    }

    COCO_ALWAYS_INLINE Path& removeFilename() // noexcept
    {
        d_->path.remove_filename();
        d_->invalidateCache();
        return *this;
    }

    COCO_ALWAYS_INLINE void swap(Path& other) // noexcept
    {
        d_->path.swap(other.d_->path);
        d_->invalidateCache();
        other.d_->invalidateCache();
    }

    template <typename... Args>
    COCO_ALWAYS_INLINE Path arg(Args&&... args) const
    {
        return CACHED_QSTRING_(d_).arg(std::forward<Args>(args)...);
    }

    COCO_ALWAYS_INLINE Path
    arg(const QString& a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(QChar a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(QLatin1StringView a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(QStringView a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(char a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(int a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(long a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(qlonglong a,
        int fieldWidth = 0,
        int base = 10,
        QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(qulonglong a,
        int fieldWidth = 0,
        int base = 10,
        QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(short a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(ulong a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
    arg(ushort a,
        int fieldWidth = 0,
        int base = 10,
        QChar fillChar = u' ') const
    {
        return CACHED_QSTRING_(d_).arg(a, fieldWidth, base, fillChar);
    }

    COCO_ALWAYS_INLINE Path
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

    COCO_ALWAYS_INLINE Path
    rebase(const Path& oldBase, const Path& newBase) const
    {
        auto relative = d_->path.lexically_relative(oldBase.d_->path);
        return newBase.d_->path / relative;
    }

    COCO_ALWAYS_INLINE QString extQString() const
    {
        return TO_QSTRING_(d_->path.extension());
    }

    COCO_ALWAYS_INLINE std::string extString() const
    {
        return d_->path.extension().string();
    }

    COCO_ALWAYS_INLINE QString fileQString() const
    {
        return TO_QSTRING_(d_->path.filename());
    }

    COCO_ALWAYS_INLINE std::string fileString() const
    {
        return d_->path.filename().string();
    }

    QString prettyQString() const
    {
        return QString::fromStdString(prettyString());
    }

    std::string prettyString() const;

    COCO_ALWAYS_INLINE QString stemQString() const
    {
        return TO_QSTRING_(d_->path.stem());
    }

    COCO_ALWAYS_INLINE std::string stemString() const
    {
        return d_->path.stem().string();
    }

    COCO_ALWAYS_INLINE QString toQString() const { return CACHED_QSTRING_(d_); }

    COCO_ALWAYS_INLINE std::filesystem::path toStd() const noexcept
    {
        return d_->path;
    }

    COCO_ALWAYS_INLINE std::string toString() const
    {
        return CACHED_STRING_(d_);
    }

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

} // namespace Coco

inline Coco::Path operator"" _ccpath(const char* cString, std::size_t)
{
    return Coco::Path(cString);
}

namespace std {

template <> struct hash<Coco::Path>
{
    std::size_t operator()(const Coco::Path& path) const;
};

} // namespace std

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
