/*
* Coco: Path.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-2-5
*/

#include "../include/Coco/Path.h"
#include "Debug.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

#include <algorithm>

using namespace Coco;

//------------------------------------------------------------
// std::hash definition
//------------------------------------------------------------

std::size_t std::hash<Path>::operator()(const Path& path) const
{
    return std::hash<std::filesystem::path>()(path.toStd());
}

//------------------------------------------------------------
// Path definitions
//------------------------------------------------------------

// Default constructor implicitly noexcept if member var constructors are (which
// it is here)
Path::Path() = default;
Path::Path(const Path& path) = default;
Path::Path(Path&& path) noexcept = default;

Path::Path(const std::filesystem::path& path)
    : m_path(path)
{
}

Path::Path(const char* path)
    : m_path(path)
{
}

Path::Path(const std::string& path)
    : m_path(path)
{
}

Path::Path(const QString& path)
    : m_path(path.toStdString())
{
}

Path::Path(System location)
    : m_path(_fromSystem(location))
{
}

QTextStream& Coco::operator<<(QTextStream& outStream, const Path& path)
{
    return outStream << path.toQString(Path::Normalize::Yes);
}

std::ostream& Coco::operator<<(std::ostream& outStream, const Path& path)
{
    return outStream << path.toString(Path::Normalize::Yes);
}

// By returning a QDebug object (not a reference), we allow the chaining of
// multiple operator<< calls. This is similar to how std::ostream works, but
// with the added benefit of managing QDebug's internal state
QDebug Coco::operator<<(QDebug debug, const Path& path)
{
    //debug.nospace() << path.toQString();
    //return debug.maybeSpace();
    return debug << path.toQString(Path::Normalize::Yes);
}

Path& Path::operator=(const Path& other) = default;
Path& Path::operator=(Path&& other) noexcept = default;
bool Path::operator==(const Path& other) const noexcept = default;
bool Path::operator!=(const Path& other) const noexcept = default;

Path Path::operator/(const Path& other) const
{
    Path path = *this;
    path /= other;

    return path;
}

Path& Path::operator/=(const Path& other)
{
    m_path /= other.m_path;
    return *this;
}

Path& Path::operator+=(const Path& other)
{
    m_path += other.m_path;
    return *this;
}

Path::operator bool() const noexcept
{
    return !m_path.empty();
}

Path::operator std::filesystem::path() const noexcept
{
    return m_path;
}

// Path::operator QVariant() const
// {
//     return toQVariant();
// }

// A-Z
//------------------------------------------------------------

QStringList Path::_findInArgs_extHelper(const QString& extensions)
{
    QStringList resolved{};

    for (auto& ext : extensions.split(","))
        resolved << resolveExtension(ext).toQString();

    return resolved;
}

QStringList Path::_findInDir_extHelper(const QString& extensions)
{
    QStringList resolved{};

    for (auto& ext : extensions.split(","))
        resolved << "*" + resolveExtension(ext).toQString();

    return resolved;
}

// Handle other flags in future, maybe
constexpr QDirIterator::IteratorFlags Path::_findIn_flagsHelper(Recursive recursive) noexcept
{
    return (recursive == Recursive::Yes)
        ? QDirIterator::Subdirectories
        : QDirIterator::NoIteratorFlags;
}

Path Path::_fromSystem(System type) const
{
    if (type == Root)
        return Path(QDir::rootPath());

    auto type_map = _systemToQtType();
    auto it = type_map.find(type);

    if (it != type_map.end())
        return _qStandardLocation(it->second);

    return {};
}

std::string Path::_normalizer(const std::string& str, char separator) const noexcept
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

Path Path::_qStandardLocation(QStandardPaths::StandardLocation type) const
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

const std::unordered_map
<
    Path::System,
    QStandardPaths::StandardLocation
>
Path::_systemToQtType() const noexcept
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
        { Downloads, QStandardPaths::DownloadLocation },
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

Path Path::arg
(
    const QString& a,
    int fieldWidth,
    QChar fillChar
)
const
{
    return toQString().arg(a, fieldWidth, fillChar);
}

Path Path::arg
(
    int a,
    int fieldWidth,
    int base,
    QChar fillChar
)
const
{
    return toQString().arg(a, fieldWidth, base, fillChar);
}

Path Path::arg
(
    char a,
    int fieldWidth,
    QChar fillChar
)
const
{
    return toQString().arg(a, fieldWidth, fillChar);
}

Path Path::arg
(
    QChar a,
    int fieldWidth,
    QChar fillChar
)
const
{
    return toQString().arg(a, fieldWidth, fillChar);
}

void Path::clear() noexcept
{
    m_path.clear();
}

Path Path::extension() const
{
    return m_path.extension();
}

std::string Path::extString() const
{
    return extension().toString();
}

QString Path::extQString() const
{
    return extension().toQString();
}

Path Path::file() const
{
    return m_path.filename();
}

std::string Path::fileString() const
{
    return file().toString();
}

QString Path::fileQString() const
{
    return file().toQString();
}

QList<Path> Path::findInDir
(
    const Path& directory,
    const QString& extensions,
    Recursive recursive
)
{
    QList<Path> paths{};

    QDirIterator it
    (
        directory.toQString(),
        _findInDir_extHelper(extensions),
        QDir::Files,
        _findIn_flagsHelper(recursive)
    );

    while (it.hasNext())
    {
        it.next();
        paths << it.filePath();
    }

    return paths;
}

QList<Path> Path::findInDir
(
    const QList<Path>& directories,
    const QString& extensions,
    Recursive recursive
)
{
    QList<Path> paths{};
    auto exts = _findInDir_extHelper(extensions);
    auto flags = _findIn_flagsHelper(recursive);

    for (auto& dir : directories)
    {
        QDirIterator it
        (
            dir.toQString(),
            exts,
            QDir::Files,
            flags
        );

        while (it.hasNext())
        {
            it.next();
            paths << it.filePath();
        }
    }

    return paths;
}

QList<Path> Path::findInArgs
(
    const QStringList& args,
    const QString& extensions
)
{
    QList<Path> paths{};
    auto exts = _findInArgs_extHelper(extensions);

    std::copy_if
    (
        args.begin(),
        args.end(),
        std::back_inserter(paths),
        [&exts](const Path& path)
        {
            return exts.contains(path.extQString());
        }
    );

    return paths;
}

// For isolating paths from main function arguments
QList<Path> Path::findInArgs
(
    const int& argc,
    const char* const* argv,
    const QString& extensions
)
{
    QStringList args{};
    for (auto i = 0; i < argc; ++i)
        args << QString::fromUtf8(argv[i]);

    return findInArgs(args, extensions);
}

bool Path::isEmpty() const noexcept
{
    return m_path.empty();
}

bool Path::isEmpty(const Path& path) noexcept
{
    return path.isEmpty();
}

bool Path::isFile() const
{
    //return std::filesystem::is_regular_file(m_path);
    // ^ Valid paths with non-standard characters won't return valid
    return QFileInfo(toQString()).isFile();
}

bool Path::isFile(const Path& path)
{
    return path.isFile();
}

bool Path::isFolder() const
{
    //return std::filesystem::is_directory(m_path);
    // ^ Valid paths with non-standard characters won't return valid
    return QFileInfo(toQString()).isDir();
}

bool Path::isFolder(const Path& path)
{
    return path.isFolder();
}

bool Path::isValid() const
{
    //return std::filesystem::exists(m_path);
    // ^ Valid paths with non-standard characters won't return valid
    return QFileInfo(toQString()).exists();
}

bool Path::isValid(const Path& path)
{
    return path.isValid();
}

Path& Path::makePreferred() noexcept
{
    m_path.make_preferred();
    return *this;
}

bool Path::mkdir(const Path& path)
{
    return std::filesystem::create_directories(path.m_path);
}

Path Path::parent() const
{
    return m_path.parent_path();
}

Path Path::relative() const
{
    return m_path.relative_path();
}

Path& Path::replaceExt(const Path& replacement)
{
    m_path.replace_extension(replacement);
    return *this;
}

Path Path::resolveExtension(const QString& extension)
{
    auto ext = extension.trimmed();
    constexpr auto dot = ".";
    if (ext.isEmpty() || ext == dot || ext == "..")
        return {};

    constexpr auto dummy = "C:/Dir/Stem";
    QString resolved_dummy_path = dummy;

    if (!ext.contains(dot))
        resolved_dummy_path += dot;

    resolved_dummy_path += ext;
    auto resolved = Path(resolved_dummy_path).extension();

    if (resolved != extension)
    {
        constexpr auto format = \
            "Resolved extension \"%1\" to \"%2\"";

        qDebug(log) << qUtf8Printable(QString(format)
            .arg(extension)
            .arg(resolved.toQString()));
    }

    return resolved;
}

Path Path::root() const
{
    return m_path.root_path();
}

Path Path::rootDirectory() const
{
    return m_path.root_directory();
}

Path Path::rootName() const
{
    return m_path.root_name();
}

Path Path::stem() const
{
    return m_path.stem();
}

QString Path::stemQString() const
{
    return stem().toQString();
}

std::string Path::stemString() const
{
    return stem().toString();
}

QString Path::toQString(Normalize normalize, char separator) const
{
    return QString::fromStdString
    (
        toString(normalize, separator)
    );
}

std::filesystem::path Path::toStd() const noexcept
{
    return m_path;
}

std::string Path::toString(Normalize normalize, char separator) const
{
    auto string = m_path.string();

    if (normalize == Normalize::Yes)
        return _normalizer(string, separator);

    return string;
}

// QVariant Path::toQVariant() const
// {
//     return QVariant::fromValue(toQString());
// }

//------------------------------------------------------------
// Utility
//------------------------------------------------------------

Path PathDialog::directory
(
    QWidget* parent,
    const QString& caption,
    const Path& startPath
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

Path PathDialog::file
(
    QWidget* parent,
    const QString& caption,
    const Path& startPath,
    const QString& filter,
    QString* selectedFilter
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

Path PathDialog::save
(
    QWidget* parent,
    const QString& caption,
    const Path& startPath,
    const QString& filter,
    QString* selectedFilter
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
