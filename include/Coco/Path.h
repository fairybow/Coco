/*
* Coco: Path.h  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-03-27
*/

#pragma once

#include "CocoGlobal.h"

#include <QChar>
#include <QDebug>
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
#include <QtTypes>
#include <QWidget>

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <string>
#include <utility>

/// @todo Cache the string and QString versions in PathData.
// To avoid excess function calls.
#define TO_QSTRING(Path) QString::fromStdString(Path.string())
#define GEN_SYS_UTIL(EnumValue) static Path EnumValue() { return Path(System::EnumValue); }

namespace Coco
{
    class PathData : public QSharedData
    {
    public:
        PathData() = default;
        explicit PathData(const std::filesystem::path& other) : path(other) {}
        std::filesystem::path path;
    };

    class Path
    {
    public:
        enum class Recursive { No = 0, Yes };

        Path() : d_(new PathData) {}
        Path(const Path& other) : d_(other.d_) {}
        Path(Path&& other) noexcept = default;

        Path(const std::filesystem::path& path) : d_(new PathData(path)) {}
        Path(const char* path) : d_(new PathData(path)) {}
        Path(const std::string& path) : d_(new PathData(path)) {}
        Path(const QString& path) : d_(new PathData(path.toStdString())) {}
        Path(System location) : d_(new PathData(fromSystem_(location).toStdString())) {}

        // ----- Stream output operators -----

        friend QTextStream& operator<<(QTextStream& outStream, const Path& path)
        {
            return outStream << TO_QSTRING(path.d_->path);
        }

        friend std::ostream& operator<<(std::ostream& outStream, const Path& path)
        {
            return outStream << path.d_->path.string();
        }

        // By returning a QDebug object (not a reference), we allow the chaining
        // of multiple operator<< calls. This is similar to how std::ostream
        // works, but with the added benefit of managing QDebug's internal
        // state.
        friend QDebug operator<<(QDebug debug, const Path& path)
        {
            return debug << TO_QSTRING(path.d_->path);
        }

        // ----- Assignment operators -----

        Path& operator=(const Path& other) = default;
        Path& operator=(Path&& other) noexcept = default;

        // ----- Comparison operators -----

        bool operator==(const Path& other) const noexcept = default;
        bool operator!=(const Path& other) const noexcept = default;

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
            return *this;
        }

        Path& operator+=(const Path& other)
        {
            d_->path += other.d_->path;
            return *this;
        }

        // ----- Conversion operators -----

        explicit operator bool() const noexcept
        {
            return !d_->path.empty();
        }

        operator std::filesystem::path() const noexcept
        {
            return d_->path;
        }

        // ----- Queries -----

        bool isEmpty() const noexcept
        {
            return d_->path.empty();
        }

        bool isFile() const
        {
            //return std::filesystem::is_regular_file(m_path);
            // ^ Valid paths with non-standard characters won't return valid.
            return QFileInfo(TO_QSTRING(d_->path)).isFile();
        }

        bool isFolder() const
        {
            //return std::filesystem::is_directory(m_path);
            // ^ Valid paths with non-standard characters won't return valid.
            return QFileInfo(TO_QSTRING(d_->path)).isDir();
        }

        bool isValid() const
        {
            //return std::filesystem::exists(m_path);
            // ^ Valid paths with non-standard characters won't return valid.
            return QFileInfo(TO_QSTRING(d_->path)).exists();
        }

        // ----- Decomposition -----

        Path rootName() const
        {
            return d_->path.root_name();
        }

        Path rootDirectory() const
        {
            return d_->path.root_directory();
        }

        Path root() const
        {
            return d_->path.root_path();
        }

        Path relative() const
        {
            return d_->path.relative_path();
        }

        Path parent() const
        {
            return d_->path.parent_path();
        }

        Path filename() const
        {
            return d_->path.filename();
        }

        Path file() const
        {
            return d_->path.filename();
        }

        Path stem() const
        {
            return d_->path.stem();
        }

        Path ext() const
        {
            return d_->path.extension();
        }

        Path extension() const
        {
            return d_->path.extension();
        }

        // ----- Modification -----

        void clear() noexcept
        {
            d_->path.clear();
        }

        Path& makePreferred() noexcept
        {
            d_->path.make_preferred();
            return *this;
        }

        Path& replaceExt(const Path& replacement = {})
        {
            d_->path.replace_extension(replacement);
            return *this;
        }

        Path& replaceFilename(const Path& replacement)
        {
            d_->path.replace_filename(replacement);
            return *this;
        }

        Path& removeFilename()
        {
            d_->path.remove_filename();
            return *this;
        }

        void swap(Path& other) noexcept
        {
            d_->path.swap(other.d_->path);
        }

        // ----- Conversion -----

        QString extQString() const
        {
            return TO_QSTRING(d_->path.extension());
        }

        std::string extString() const
        {
            return d_->path.extension().string();
        }

        QString fileQString() const
        {
            return TO_QSTRING(d_->path.filename());
        }

        std::string fileString() const
        {
            return d_->path.filename().string();
        }

        QString prettyQString() const
        {
            return QString::fromStdString(prettyString());
        }

        std::string prettyString() const;

        QString stemQString() const
        {
            return TO_QSTRING(d_->path.stem());
        }

        std::string stemString() const
        {
            return d_->path.stem().string();
        }

        QString toQString() const
        {
            return TO_QSTRING(d_->path);
        }

        std::filesystem::path toStd() const noexcept
        {
            return d_->path;
        }

        std::string toString() const
        {
            return d_->path.string();
        }

        // ----- Utility -----

        GEN_SYS_UTIL(Root);
        GEN_SYS_UTIL(AppConfig);
        GEN_SYS_UTIL(AppData);
        GEN_SYS_UTIL(AppLocalData);
        GEN_SYS_UTIL(Applications);
        GEN_SYS_UTIL(Cache);
        GEN_SYS_UTIL(Config);
        GEN_SYS_UTIL(Desktop);
        GEN_SYS_UTIL(Downloads);
        GEN_SYS_UTIL(Documents);
        GEN_SYS_UTIL(Fonts);
        GEN_SYS_UTIL(GenericCache);
        GEN_SYS_UTIL(GenericConfig);
        GEN_SYS_UTIL(GenericData);
        GEN_SYS_UTIL(Home);
        GEN_SYS_UTIL(Movies);
        GEN_SYS_UTIL(Music);
        GEN_SYS_UTIL(Pictures);
        GEN_SYS_UTIL(PublicShare);
        GEN_SYS_UTIL(Runtime);
        GEN_SYS_UTIL(Temp);
        GEN_SYS_UTIL(Templates);

        template <typename... Args>
        Path arg(Args&&... args) const
        {
            return TO_QSTRING(d_->path).arg(std::forward<Args>(args)...);
        }

        Path arg(const QString& a, int fieldWidth = 0, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, fillChar);
        }

        Path arg(QChar a, int fieldWidth = 0, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, fillChar);
        }

        Path arg(QLatin1StringView a, int fieldWidth = 0, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, fillChar);
        }

        Path arg(QStringView a, int fieldWidth = 0, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, fillChar);
        }

        Path arg(char a, int fieldWidth = 0, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, fillChar);
        }

        Path arg(int a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(long a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(qlonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(qulonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(short a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(ulong a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(ushort a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, base, fillChar);
        }

        Path arg(double a, int fieldWidth = 0, char format = 'g', int precision = -1, QChar fillChar = u' ') const
        {
            return TO_QSTRING(d_->path).arg(a, fieldWidth, format, precision, fillChar);
        }

        // Creates all directories in the specified path.
        static bool mkdir(const Path& path)
        {
            return std::filesystem::create_directories(path.d_->path);
        }

        // Resolve an extension (with or without dot) to ".ext"
        static Path resolveExtension(const QString& extension);

        // Add sorting option
        // Provide extensions as ".h, .cpp" (one QString)
        static QList<Path> findInDir
        (
            const Path& directory,
            const QString& extensions,
            Recursive recursive = Recursive::Yes
        );

        static QList<Path> findInDir
        (
            const QList<Path>& directories,
            const QString& extensions,
            Recursive recursive = Recursive::Yes
        );

        // For isolating paths from QApplication arguments
        static QList<Path> findInArgs
        (
            const QStringList& args,
            const QString& extensions
        );

        // For isolating paths from main function arguments
        static QList<Path> findInArgs
        (
            const int& argc,
            const char* const* argv,
            const QString& extensions
        );

    private:
        QSharedDataPointer<PathData> d_;

        QString fromSystem_(System value) const;
        QString standardLocation_(QStandardPaths::StandardLocation value) const
        {
            return QStandardPaths::locate
            (
                value,
                {},
                QStandardPaths::LocateDirectory
            );
        }

    }; // class Coco::Path

    namespace PathDialog
    {
        inline Path directory
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {}
        )
        {
            return Path
            (
                QFileDialog::getExistingDirectory
                (
                    parent,
                    caption,
                    startPath.toQString()
                )
            );
        }

        inline Path file
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {},
            const QString& filter = {},
            QString* selectedFilter = nullptr
        )
        {
            return Path
            (
                QFileDialog::getOpenFileName
                (
                    parent,
                    caption,
                    startPath.toQString(),
                    filter,
                    selectedFilter
                )
            );
        }

        inline Path save
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {},
            const QString& filter = {},
            QString* selectedFilter = nullptr
        )
        {
            return Path
            (
                QFileDialog::getSaveFileName
                (
                    parent,
                    caption,
                    startPath.toQString(),
                    filter,
                    selectedFilter
                )
            );
        }

    } // namespace Coco::PathDialog

} // namespace Coco

inline Coco::Path operator"" _ccpath(const char* cString, std::size_t)
{
    return Coco::Path(cString);
}

namespace std
{
    template <>
    struct hash<Coco::Path>
    {
        std::size_t operator()(const Coco::Path& path) const;
    };
}

#undef TO_QSTRING
#undef GEN_SYS_UTIL
