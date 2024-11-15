/*
* Coco Path.cpp  Copyright (C) 2024  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2024-11-10
*/

#include "../include/Coco/Path.h"

#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>

std::size_t std::hash<Coco::Path>::operator()(const Coco::Path& path) const
{
    return std::hash<std::filesystem::path>()(path.toStd());
}

BEGIN_COCO_NAMESPACE

Path::Path()
    : m_path(std::filesystem::path{})
{}

Path::Path(const std::filesystem::path& path)
    : m_path(path)
{}

Path::Path(const char* path)
    : m_path(path)
{}

Path::Path(const std::string& path)
    : m_path(path)
{}

Path::Path(const QString& path)
    : m_path(path.toStdString())
{}

Path::Path(System location)
    : m_path(_fromSystem(location))
{}

bool Path::mkdir(const Path& path)
{
    return std::filesystem::create_directories(path.m_path);
}

/// @brief Returns a list of Paths from Qt application arguments
QList<Path> Path::fromArgs
(
    const QStringList& args,
    ValidOnly validOnly,
    SkipArg0 skipArg0
)
{
    QList<Path> paths{};

    for (int i = (skipArg0 == SkipArg0::Yes); i < args.size(); ++i)
        _argHelper(args[i], paths, validOnly);

    return paths;
}

/// @brief Returns a list of Paths from application arguments
QList<Path> Path::fromArgs
(
    int argc,
    char* argv[],
    ValidOnly validOnly,
    SkipArg0 skipArg0
)
{
    QList<Path> paths{};

    for (int i = (skipArg0 == SkipArg0::Yes); i < argc; ++i)
        _argHelper(argv[i], paths, validOnly);

    return paths;
}

/// @todo Sort?
QList<Path> Path::findIn
(
    const Path& directory,
    const QString& extension,
    Recursive recursive
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

QTextStream& operator<<(QTextStream& outStream, const Path& path)
{
    return outStream << path.toQString(Path::Normalize::Yes);
}

std::ostream& operator<<(std::ostream& outStream, const Path& path)
{
    return outStream << path.toString(Path::Normalize::Yes);
}

/// @brief By returning a QDebug object (not a reference), we allow the
/// chaining of multiple operator<< calls. This is similar to how
/// std::ostream works, but with the added benefit of managing QDebug's
/// internal state
QDebug operator<<(QDebug debug, const Path& path)
{
    //debug.nospace() << path.toQString();
    //return debug.maybeSpace();
    return debug << path.toQString(Path::Normalize::Yes);
}

Path& Path::operator=(const Path& other)
{
    if (this != &other)
        m_path = other.m_path;

    return *this;
}

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

Path::operator bool() const
{
    return !m_path.empty();
}

Path::operator std::filesystem::path() const
{
    return m_path;
}

// Path::operator QVariant() const
// {
//     return toQVariant();
// }

std::string Path::extString() const
{
    return extension().toString();
}

QString Path::extQString() const
{
    return extension().toQString();
}

QString Path::fileQString() const
{
    return file().toQString();
}

std::string Path::fileString() const
{
    return file().toString();
}

std::string Path::stemString() const
{
    return stem().toString();
}

QString Path::stemQString() const
{
    return stem().toQString();
}

QString Path::toQString(Normalize normalize, char separator) const
{
    return QString::fromStdString
    (
        toString(normalize, separator)
    );
}

// QVariant Path::toQVariant() const
// {
//     return QVariant::fromValue(toQString());
// }

std::filesystem::path Path::toStd() const
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

bool Path::isEmpty() const
{
    return m_path.empty();
}

bool Path::isFile() const
{
    //return std::filesystem::is_regular_file(m_path);
    // ^ Valid paths with non-standard characters won't return valid
    return QFileInfo(toQString()).isFile();
}

bool Path::isFolder() const
{
    //return std::filesystem::is_directory(m_path);
    // ^ Valid paths with non-standard characters won't return valid
    return QFileInfo(toQString()).isDir();
}

bool Path::isValid() const
{
    //return std::filesystem::exists(m_path);
    // ^ Valid paths with non-standard characters won't return valid
    return QFileInfo(toQString()).exists();
}

Path Path::rootName() const
{
    return m_path.root_name();
}

Path Path::rootDirectory() const
{
    return m_path.root_directory();
}

Path Path::root() const
{
    return m_path.root_path();
}

Path Path::relative() const
{
    return m_path.relative_path();
}

Path Path::parent() const
{
    return m_path.parent_path();
}

Path Path::file() const
{
    return m_path.filename();
}

Path Path::stem() const
{
    return m_path.stem();
}

Path Path::extension() const
{
    return m_path.extension();
}

void Path::clear() noexcept
{
    m_path.clear();
}

Path& Path::replaceExt(const Path& replacement)
{
    m_path.replace_extension(replacement);
    return *this;
}

Path Path::arg(const QString& a, int fieldWidth, QChar fillChar) const
{
    return toQString().arg(a, fieldWidth, fillChar);
}

Path Path::arg(int a, int fieldWidth, int base, QChar fillChar) const
{
    return toQString().arg(a, fieldWidth, base, fillChar);
}

Path Path::arg(char a, int fieldWidth, QChar fillChar) const
{
    return toQString().arg(a, fieldWidth, fillChar);
}

Path Path::arg(QChar a, int fieldWidth, QChar fillChar) const
{
    return toQString().arg(a, fieldWidth, fillChar);
}

Path& Path::makePreferred() noexcept
{
    m_path.make_preferred();
    return *this;
}

void Path::_argHelper
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

const std::unordered_map
<
    Path::System,
    QStandardPaths::StandardLocation
>
Path::_systemToQtType() const
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

std::string Path::_normalizer(const std::string& str, char separator) const
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

END_COCO_NAMESPACE
