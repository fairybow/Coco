/*
* Coco: Path.cpp  Copyright (C) 2025  fairybow
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*
* This file uses Qt 6. Qt is a free and open-source widget toolkit for creating
* graphical user interfaces. For more information, visit <https://www.qt.io/>.
*
* Updated: 2025-03-27
*/

#include "../include/Coco/CocoGlobal.h"
#include "../include/Coco/Path.h"

#include <QDir>
#include <QDirIterator>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <cstddef>
#include <filesystem>

#define CACHED_STRING(DPtr) (DPtr->cacheValid ? DPtr->cachedString : DPtr->path.string())

std::size_t std::hash<Coco::Path>::operator()(const Coco::Path& path) const
{
    return std::hash<std::filesystem::path>()(path.toStd());
}

static const QHash<Coco::System, QStandardPaths::StandardLocation> SYSTEM_MAP_ =
{
    { Coco::System::AppConfig, QStandardPaths::AppConfigLocation },
    { Coco::System::AppData, QStandardPaths::AppDataLocation },
    { Coco::System::AppLocalData, QStandardPaths::AppLocalDataLocation },
    { Coco::System::Applications, QStandardPaths::ApplicationsLocation },
    { Coco::System::Cache, QStandardPaths::CacheLocation },
    { Coco::System::Config, QStandardPaths::ConfigLocation },
    { Coco::System::Desktop, QStandardPaths::DesktopLocation },
    { Coco::System::Downloads, QStandardPaths::DownloadLocation },
    { Coco::System::Documents, QStandardPaths::DocumentsLocation },
    { Coco::System::Fonts, QStandardPaths::FontsLocation },
    { Coco::System::GenericCache, QStandardPaths::GenericCacheLocation },
    { Coco::System::GenericConfig, QStandardPaths::GenericConfigLocation },
    { Coco::System::GenericData, QStandardPaths::GenericDataLocation },
    { Coco::System::Home, QStandardPaths::HomeLocation },
    { Coco::System::Movies, QStandardPaths::MoviesLocation },
    { Coco::System::Music, QStandardPaths::MusicLocation },
    { Coco::System::Pictures, QStandardPaths::PicturesLocation },
    { Coco::System::PublicShare, QStandardPaths::PublicShareLocation },
    { Coco::System::Runtime, QStandardPaths::RuntimeLocation },
    { Coco::System::Temp, QStandardPaths::TempLocation },
    { Coco::System::Templates, QStandardPaths::TemplatesLocation }
};

static QStringList findInArgsExtHelper_(const QString& extensions)
{
    QStringList resolved{};

    for (auto& ext : extensions.split(","))
        resolved << Coco::Path::resolveExtension(ext).toQString();

    return resolved;
}

static QStringList findInDirExtHelper_(const QString& extensions)
{
    QStringList resolved{};

    for (auto& ext : extensions.split(","))
        resolved << "*" + Coco::Path::resolveExtension(ext).toQString();

    return resolved;
}

// Handle other flags in future, maybe
static constexpr QDirIterator::IteratorFlags findInFlagsHelper_(Coco::Path::Recursive recursive) noexcept
{
    return (recursive == Coco::Path::Recursive::Yes)
        ? QDirIterator::Subdirectories
        : QDirIterator::NoIteratorFlags;
}

std::string Coco::Path::prettyString() const
{
    std::string pretty{};
    auto last_ch_was_sep = false;

    for (auto& ch : CACHED_STRING(d_))
    {
        if (ch == '/' || ch == '\\')
        {
            if (!last_ch_was_sep)
            {
                pretty += '/';
                last_ch_was_sep = true;
            }
        }
        else
        {
            pretty += ch;
            last_ch_was_sep = false;
        }
    }

    return pretty;
}

Coco::Path Coco::Path::resolveExtension(const QString& extension)
{
    auto ext = extension.trimmed();

    // Early return for invalid extensions
    if (ext.isEmpty())
        return {};

    // Ensure extension starts with a dot
    return (ext.startsWith('.') ? ext : '.' + ext);
}

QList<Coco::Path> Coco::Path::findInDir
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
        findInDirExtHelper_(extensions),
        QDir::Files,
        findInFlagsHelper_(recursive)
    );

    while (it.hasNext())
    {
        it.next();
        paths << it.filePath();
    }

    return paths;
}

QList<Coco::Path> Coco::Path::findInDir
(
    const QList<Path>& directories,
    const QString& extensions,
    Recursive recursive
)
{
    QList<Path> paths{};
    auto exts = findInDirExtHelper_(extensions);
    auto flags = findInFlagsHelper_(recursive);

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

QList<Coco::Path> Coco::Path::findInArgs
(
    const QStringList& args,
    const QString& extensions
)
{
    QList<Path> paths{};
    auto exts = findInArgsExtHelper_(extensions);

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
QList<Coco::Path> Coco::Path::findInArgs
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

QString Coco::Path::fromSystem_(System value) const
{
    if (value == System::Root)
        return QDir::rootPath();

    auto it = SYSTEM_MAP_.find(value);

    if (it != SYSTEM_MAP_.end())
        return standardLocation_(*it);

    return {};
}

#undef CACHED_STRING
