/*
* Coco: Path.h  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-03-02
*/

/// @todo Code style (for all)
/// @todo Documentation (for all)
/// @todo IWYU (for all)
/// @todo No using namespace declarations for Coco ns

#pragma once

#include <QChar>
#include <QDebug>
#include <QDirIterator>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QWidget>

#include <filesystem>
#include <ostream>
#include <string>
#include <unordered_map>

//------------------------------------------------------------
// std::hash
//------------------------------------------------------------

namespace Coco
{
    //------------------------------------------------------------
    // Path
    //------------------------------------------------------------

    // TODO: Error handling, other fs methods
    // TODO: Fully document
    // TODO: Make path always preferred / native separators?
    // Wraps std::filesystem::path for use with Qt
    class Path
    {
    public:
        enum class Normalize { No = 0, Yes };
        enum class Recursive { No = 0, Yes };

        enum System
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

        Path();
        Path(const Path& path);
        Path(Path&& path) noexcept;
        Path(const std::filesystem::path& path);
        Path(const char* path);
        Path(const std::string& path);
        Path(const QString& path);
        Path(System location);

        // NTS: not needed, but a reminder that this is not to be inherited (obvious
        // for Path, less obvious for Private<T>)
        ~Path() = default;

        //------------------------------------------------------------
        // Operators
        //------------------------------------------------------------

        // Stream
        //------------------------------------------------------------

        friend QTextStream& operator<<(QTextStream& outStream, const Path& path);
        friend std::ostream& operator<<(std::ostream& outStream, const Path& path);

        // By returning a QDebug object (not a reference), we allow the chaining of
        // multiple operator<< calls. This is similar to how std::ostream works, but
        // with the added benefit of managing QDebug's internal state
        friend QDebug operator<<(QDebug debug, const Path& path);

        // Assignment
        //------------------------------------------------------------

        Path& operator=(const Path& other);
        Path& operator=(Path&& other) noexcept;

        // Comparison
        //------------------------------------------------------------

        bool operator==(const Path& other) const noexcept;
        bool operator!=(const Path& other) const noexcept;

        // Concatenation
        //------------------------------------------------------------

        Path operator/(const Path& other) const;
        Path& operator/=(const Path& other);
        Path& operator+=(const Path& other);

        // Conversion
        //------------------------------------------------------------

        explicit operator bool() const noexcept;
        operator std::filesystem::path() const noexcept;
        // operator QVariant() const;

        //------------------------------------------------------------
        // Queries
        //------------------------------------------------------------

        bool isEmpty() const noexcept;
        bool isFile() const;
        bool isFolder() const;
        bool isValid() const;
        static bool isEmpty(const Path& path) noexcept;
        static bool isFile(const Path& path);
        static bool isFolder(const Path& path);
        static bool isValid(const Path& path);

        //------------------------------------------------------------
        // Decomposition
        //------------------------------------------------------------

        Path rootName() const;
        Path rootDirectory() const;
        Path root() const;
        Path relative() const;
        Path parent() const;
        Path file() const;
        Path stem() const;
        Path extension() const;

        //------------------------------------------------------------
        // Modification
        //------------------------------------------------------------

        Path arg
        (
            const QString& a,
            int fieldWidth = 0,
            QChar fillChar = u' '
        ) const;

        Path arg
        (
            int a,
            int fieldWidth = 0,
            int base = 10,
            QChar fillChar = u' '
        ) const;

        Path arg
        (
            char a,
            int fieldWidth = 0,
            QChar fillChar = u' '
        ) const;

        Path arg
        (
            QChar a,
            int fieldWidth = 0,
            QChar fillChar = u' '
        ) const;

        void clear() noexcept;
        Path& replaceExt(const Path& replacement = {});
        Path& makePreferred() noexcept;

        //------------------------------------------------------------
        // Conversion
        //------------------------------------------------------------

        std::string extString() const;
        QString extQString() const;
        QString fileQString() const;
        std::string fileString() const;
        std::string stemString() const;
        QString stemQString() const;
        QString toQString(Normalize normalize = Normalize::No, char separator = '/') const;
        // QVariant toQVariant() const;
        std::filesystem::path toStd() const noexcept;
        std::string toString(Normalize normalize = Normalize::No, char separator = '/') const;

        //------------------------------------------------------------
        // Utility
        //------------------------------------------------------------

        // Creates all directories in the specified path
        static bool mkdir(const Path& path);

        // Resolve an extension (with or without dot) to ".{ extension }"
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
        std::filesystem::path m_path{};

        static QStringList _findInDir_extHelper(const QString& extensions);
        static QStringList _findInArgs_extHelper(const QString& extensions);

        constexpr static QDirIterator::IteratorFlags
            _findIn_flagsHelper(Recursive recursive) noexcept;

        /*static void _fromArgs_helper
        (
            const QString& arg,
            QList<Path>& paths,
            ValidOnly validOnly
        );*/

        Path _fromSystem(System type) const;
        std::string _normalizer(const std::string& str, char separator) const noexcept;
        Path _qStandardLocation(QStandardPaths::StandardLocation type) const;

        const std::unordered_map
            <
            System,
            QStandardPaths::StandardLocation
            > _systemToQtType() const noexcept;

    }; // class Coco::Path

    //------------------------------------------------------------
    // Utility
    //------------------------------------------------------------

    namespace PathDialog
    {
        Path directory
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {}
        );

        Path file
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {},
            const QString& filter = {},
            QString* selectedFilter = nullptr
        );

        Path save
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {},
            const QString& filter = {},
            QString* selectedFilter = nullptr
        );

    } // namespace Coco::PathDialog

} // namespace Coco

namespace std
{
    template <>
    struct hash<Coco::Path>
    {
        std::size_t operator()(const Coco::Path& path) const;
    };
}
