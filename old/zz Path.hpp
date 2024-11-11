//#pragma once

/*
* cc/Path.hpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-10-27
*/

/*#include "cc_namespaces.hpp"

#include <QChar>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#include <filesystem>
#include <ostream>
#include <string>
#include <unordered_map>

// Forward declare for std::hash forward declaration lol
namespace cc
{
    class Path;
}

namespace std
{
    // Forward declare std::hash compatibility function
    template <>
    struct hash<cc::Path>
    {
        inline std::size_t operator()(const cc::Path& path) const;
    };
}

BEGIN_CC_NAMESPACE

/// @todo Error handling, other fs methods
/// @todo Fully document
/// @todo Make path always preferred / native separators?
/// @brief Wraps std::filesystem::path for use with Qt
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
        Download,
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

    Path() : m_path(std::filesystem::path{}) {}
    Path(const std::filesystem::path& path) : m_path(path) {}
    Path(const char* path) : m_path(path) {}
    Path(const std::string& path) : m_path(path) {}
    Path(const QString& path) : m_path(path.toStdString()) {}
    Path(System location) : m_path(_fromSystem(location)) {}

    /// @brief Creates all directories in the specified path
    static bool mkdir(const Path& path)
    {
        return std::filesystem::create_directories(path.m_path);
    }

    /// @brief Returns a list of Paths from Qt application arguments
    static QList<Path> fromArgs
    (
        const QStringList& args,
        ValidOnly validOnly = ValidOnly::Yes,
        SkipArg0 skipArg0 = SkipArg0::Yes
    )
    {
        QList<Path> paths{};

        for (int i = (skipArg0 == SkipArg0::Yes); i < args.size(); ++i)
            _argHelper(args[i], paths, validOnly);

        return paths;
    }

    /// @brief Returns a list of Paths from application arguments
    static QList<Path> fromArgs
    (
        int argc,
        char* argv[],
        ValidOnly validOnly = ValidOnly::Yes,
        SkipArg0 skipArg0 = SkipArg0::Yes
    )
    {
        QList<Path> paths{};

        for (int i = (skipArg0 == SkipArg0::Yes); i < argc; ++i)
            _argHelper(argv[i], paths, validOnly);

        return paths;
    }

    /// @todo Sort?
    static QList<Path> findIn
    (
        const Path& directory,
        const QString& extension,
        Recursive recursive = Recursive::Yes
    )
    {
        QList<Path> paths{};

        auto iterator_flag = (recursive == Recursive::Yes)
            ? QDirIterator::Subdirectories
            : QDirIterator::NoIteratorFlags;

        QDirIterator it
        (
            directory.toQString(),
            QStringList{} << "*." + extension,
            QDir::Files,
            iterator_flag
        );

        while (it.hasNext())
        {
            it.next();
            paths << it.filePath();
        }

        return paths;
    }

    // Stream:

    friend QTextStream& operator<<(QTextStream& outStream, const Path& path)
    {
        return outStream << path.toQString(Normalize::Yes);
    }

    friend std::ostream& operator<<(std::ostream& outStream, const Path& path)
    {
        return outStream << path.toString(Normalize::Yes);
    }

    /// @brief By returning a QDebug object (not a reference), we allow the
    /// chaining of multiple operator<< calls. This is similar to how
    /// std::ostream works, but with the added benefit of managing QDebug's
    /// internal state
    friend QDebug operator<<(QDebug debug, const Path& path)
    {
        //debug.nospace() << path.toQString();
        //return debug.maybeSpace();
        return debug << path.toQString(Normalize::Yes);
    }

    // Assignment:

    Path& operator=(const Path& other)
    {
        if (this != &other)
            m_path = other.m_path;

        return *this;
    }

    // Comparison:

    bool operator==(const Path& other) const = default;
    bool operator!=(const Path& other) const = default;

    // Concatenation:

    Path operator/(const Path& other) const
    {
        Path path = *this;
        path /= other;

        return path;
    }

    Path& operator/=(const Path& other)
    {
        m_path /= other.m_path;
        return *this;
    }

    Path& operator+=(const Path& other)
    {
        m_path += other.m_path;
        return *this;
    }

    // Conversion:

    explicit operator bool() const
    {
        return !m_path.empty();
    }

    operator std::filesystem::path() const
    {
        return m_path;
    }

    // operator QVariant() const
    // {
    //     return toQVariant();
    // }

    std::string extString() const
    {
        return extension().toString();
    }

    QString extQString() const
    {
        return extension().toQString();
    }

    QString fileQString() const
    {
        return file().toQString();
    }

    std::string fileString() const
    {
        return file().toString();
    }

    std::string stemString() const
    {
        return stem().toString();
    }

    QString stemQString() const
    {
        return stem().toQString();
    }

    QString toQString
    (
        Normalize normalize = Normalize::No,
        char separator = '/'
    )
        const
    {
        return QString::fromStdString
        (
            toString(normalize, separator)
        );
    }

    // QVariant toQVariant() const
    // {
    //     return QVariant::fromValue(toQString());
    // }

    std::filesystem::path toStd() const
    {
        return m_path;
    }

    std::string toString
    (
        Normalize normalize = Normalize::No,
        char separator = '/'
    )
        const
    {
        auto string = m_path.string();

        if (normalize == Normalize::Yes)
            return _normalizer(string, separator);

        return string;
    }

    // Queries:

    bool isEmpty() const
    {
        return m_path.empty();
    }

    bool isFile() const
    {
        //return std::filesystem::is_regular_file(m_path);
        // ^ Valid paths with non-standard characters won't return valid
        return QFileInfo(toQString()).isFile();
    }

    bool isFolder() const
    {
        //return std::filesystem::is_directory(m_path);
        // ^ Valid paths with non-standard characters won't return valid
        return QFileInfo(toQString()).isDir();
    }

    bool isValid() const
    {
        //return std::filesystem::exists(m_path);
        // ^ Valid paths with non-standard characters won't return valid
        return QFileInfo(toQString()).exists();
    }

    // Decomposition:

    Path rootName() const
    {
        return m_path.root_name();
    }

    Path rootDirectory() const
    {
        return m_path.root_directory();
    }

    Path root() const
    {
        return m_path.root_path();
    }

    Path relative() const
    {
        return m_path.relative_path();
    }

    Path parent() const
    {
        return m_path.parent_path();
    }

    Path file() const
    {
        return m_path.filename();
    }

    Path stem() const
    {
        return m_path.stem();
    }

    Path extension() const
    {
        return m_path.extension();
    }

    // Modification:

    void clear() noexcept
    {
        m_path.clear();
    }

    Path& replaceExt(const Path& replacement = {})
    {
        m_path.replace_extension(replacement);
        return *this;
    }

    Path arg(const QString& a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return toQString().arg(a, fieldWidth, fillChar);
    }

    Path arg(int a, int fieldWidth = 0, int base = 10, QChar fillChar = u' ') const
    {
        return toQString().arg(a, fieldWidth, base, fillChar);
    }

    Path arg(char a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return toQString().arg(a, fieldWidth, fillChar);
    }

    Path arg(QChar a, int fieldWidth = 0, QChar fillChar = u' ') const
    {
        return toQString().arg(a, fieldWidth, fillChar);
    }

    Path& makePreferred() noexcept
    {
        m_path.make_preferred();
        return *this;
    }

private:
    std::filesystem::path m_path;

    static void _argHelper
    (
        const QString& arg,
        QList<Path>& paths,
        ValidOnly validOnly
    )
    {
        Path path(arg);

        if (validOnly == ValidOnly::Yes)
        {
            if (path.isValid())
                paths << path;
        }
        else
            paths << path;
    }

    Path _qStandardLocation(QStandardPaths::StandardLocation type) const
    {
        return Path
        (
            QStandardPaths::locate
            (
                type,
                {},
                QStandardPaths::LocateDirectory
            )
        );
    }

    Path _fromSystem(System type) const
    {
        if (type == Root)
            return Path(QDir::rootPath());

        auto type_map = _systemToQtType();
        auto it = type_map.find(type);

        if (it != type_map.end())
            return _qStandardLocation(it->second);

        return {};
    }

    const std::unordered_map
        <
        System,
        QStandardPaths::StandardLocation
        >
        _systemToQtType() const
    {
        static const std::unordered_map
            <
            System,
            QStandardPaths::StandardLocation
            > map =
        {
            { AppConfig, QStandardPaths::AppConfigLocation },
            { AppData, QStandardPaths::AppDataLocation },
            { AppLocalData, QStandardPaths::AppLocalDataLocation },
            { Applications, QStandardPaths::ApplicationsLocation },
            { Cache, QStandardPaths::CacheLocation },
            { Config, QStandardPaths::ConfigLocation },
            { Desktop, QStandardPaths::DesktopLocation },
            { Download, QStandardPaths::DownloadLocation },
            { Documents, QStandardPaths::DocumentsLocation },
            { Fonts, QStandardPaths::FontsLocation },
            { GenericCache, QStandardPaths::GenericCacheLocation },
            { GenericConfig, QStandardPaths::GenericConfigLocation },
            { GenericData, QStandardPaths::GenericDataLocation },
            { Home, QStandardPaths::HomeLocation },
            { Movies, QStandardPaths::MoviesLocation },
            { Music, QStandardPaths::MusicLocation },
            { Pictures, QStandardPaths::PicturesLocation },
            { PublicShare, QStandardPaths::PublicShareLocation },
            { Runtime, QStandardPaths::RuntimeLocation },
            { Temp, QStandardPaths::TempLocation },
            { Templates, QStandardPaths::TemplatesLocation }
        };

        return map;
    }

    std::string _normalizer(const std::string& str, char separator) const
    {
        std::string normalized{};
        auto last_ch_was_sep = false;

        for (auto& ch : str)
        {
            if (ch == '/' || ch == '\\')
            {
                if (!last_ch_was_sep)
                {
                    normalized += separator;
                    last_ch_was_sep = true;
                }
            }
            else
            {
                normalized += ch;
                last_ch_was_sep = false;
            }
        }

        return normalized;
    }

}; // class cc::Path

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

} // namespace cc::PathDialog

END_CC_NAMESPACE

// Provides std::hash compatibility
std::size_t std::hash<cc::Path>::operator()(const cc::Path& path) const
{
    return std::hash<std::filesystem::path>()(path.toStd());
}

// I believe this is unneeded, since we have std::hash compatibility
// inline uint qHash(const Path& path, uint seed = 0)
// {
//     return qHash(path.toStd(), seed);
// }
*/
