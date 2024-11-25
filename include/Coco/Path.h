/*
* Coco: Path.h  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-24
*/

#pragma once

#include "Global.h"

#include <QChar>
#include <QDebug>
#include <QDirIterator>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#include <filesystem>
#include <ostream>
#include <string>
#include <unordered_map>

namespace Coco
{
    class Path;
}

namespace std
{
    template <>
    struct hash<Coco::Path>
    {
        inline std::size_t operator()(const Coco::Path& path) const;
    };
}

BEGIN_COCO_NAMESPACE

// TODO: Error handling, other fs methods
// TODO: Fully document
// TODO: Make path always preferred / native separators?
// Wraps std::filesystem::path for use with Qt
class Path
{
public:
    enum class Normalize { No = 0, Yes };
    enum class Recursive { No = 0, Yes };
    enum class SkipArg0 { No = 0, Yes };
    enum class ValidOnly { No = 0, Yes };

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
    Path(const std::filesystem::path& path);
    Path(const char* path);
    Path(const std::string& path);
    Path(const QString& path);
    Path(System location);

    // Creates all directories in the specified path
    static bool mkdir(const Path& path);

    // Resolve an extension (with or without dot) to ".{ extension }"
    static Path resolveExtension(const QString& extension);

    // Returns a list of Paths from Qt application arguments
    static QList<Path> fromArgs
    (
        const QStringList& args,
        ValidOnly validOnly = ValidOnly::Yes,
        SkipArg0 skipArg0 = SkipArg0::Yes
    );

    // Returns a list of Paths from application arguments
    static QList<Path> fromArgs
    (
        int argc,
        char* argv[],
        ValidOnly validOnly = ValidOnly::Yes,
        SkipArg0 skipArg0 = SkipArg0::Yes
    );

    // Add sorting option
    // Provide extensions as ".h, .cpp" (one QString)
    static QList<Path> findIn
    (
        const Path& directory,
        const QString& extensions,
        Recursive recursive = Recursive::Yes
    );

    static QList<Path> findIn
    (
        const QList<Path>& directories,
        const QString& extensions,
        Recursive recursive = Recursive::Yes
    );

    // Stream
    friend QTextStream& operator<<(QTextStream& outStream, const Path& path);
    friend std::ostream& operator<<(std::ostream& outStream, const Path& path);

    // By returning a QDebug object (not a reference), we allow the chaining of
    // multiple operator<< calls. This is similar to how std::ostream works, but
    // with the added benefit of managing QDebug's internal state
    friend QDebug operator<<(QDebug debug, const Path& path);

    // Assignment
    Path& operator=(const Path& other);

    // Comparison
    bool operator==(const Path& other) const = default;
    bool operator!=(const Path& other) const = default;

    // Concatenation
    Path operator/(const Path& other) const;
    Path& operator/=(const Path& other);
    Path& operator+=(const Path& other);

    // Conversion
    explicit operator bool() const;
    operator std::filesystem::path() const;
    // operator QVariant() const;
    std::string extString() const;
    QString extQString() const;
    QString fileQString() const;
    std::string fileString() const;
    std::string stemString() const;
    QString stemQString() const;
    QString toQString(Normalize normalize = Normalize::No, char separator = '/') const;
    // QVariant toQVariant() const;
    std::filesystem::path toStd() const;
    std::string toString(Normalize normalize = Normalize::No, char separator = '/') const;

    // Queries
    bool isEmpty() const;
    bool isFile() const;
    bool isFolder() const;
    bool isValid() const;

    // Decomposition
    Path rootName() const;
    Path rootDirectory() const;
    Path root() const;
    Path relative() const;
    Path parent() const;
    Path file() const;
    Path stem() const;
    Path extension() const;

    // Modification
    void clear() noexcept;
    Path& replaceExt(const Path& replacement = {});
    Path arg(const QString& a, int fieldWidth = 0, QChar fillChar = u' ') const;
    Path arg(int a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const;
    Path arg(char a, int fieldWidth = 0, QChar fillChar = u' ') const;
    Path arg(QChar a, int fieldWidth = 0, QChar fillChar = u' ') const;
    Path& makePreferred() noexcept;

private:
    std::filesystem::path m_path;

    static void _fromArgs_helper
    (
        const QString& arg,
        QList<Path>& paths,
        ValidOnly validOnly
    );

    static QStringList _findIn_extHelper(const QString& extensions);
    static QDirIterator::IteratorFlags _findIn_flagsHelper(Recursive recursive);
    Path _qStandardLocation(QStandardPaths::StandardLocation type) const;
    Path _fromSystem(System type) const;
    const std::unordered_map<System, QStandardPaths::StandardLocation> _systemToQtType() const;
    std::string _normalizer(const std::string& str, char separator) const;

}; // class Coco::Path

namespace PathDialog
{
    inline Path directory
    (
        QWidget* parent = nullptr,
        const QString& caption = {},
        const Path& startPath = {}
    );

    inline Path file
    (
        QWidget* parent = nullptr,
        const QString& caption = {},
        const Path& startPath = {},
        const QString& filter = {},
        QString* selectedFilter = nullptr
    );

    inline Path save
    (
        QWidget* parent = nullptr,
        const QString& caption = {},
        const Path& startPath = {},
        const QString& filter = {},
        QString* selectedFilter = nullptr
    );

} // namespace Coco::PathDialog

END_COCO_NAMESPACE
