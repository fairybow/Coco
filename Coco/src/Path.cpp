#include <algorithm>
#include <cstddef>
#include <filesystem>

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QHash>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QtTypes>

#include "../include/Coco/Path.h"

#define TO_QSTRING_(StdFsPath) QString::fromStdString(StdFsPath.string())
#define CACHED_STRING_(DPtr) (DPtr->cacheValid ? DPtr->cachedString : DPtr->path.string())
#define CACHED_QSTRING_(DPtr) (DPtr->cacheValid ? DPtr->cachedQString : TO_QSTRING_(DPtr->path))

std::size_t std::hash<Coco::Path>::operator()(const Coco::Path& path) const
{
    return std::hash<std::filesystem::path>()(path.toStd());
}

static const QHash<Coco::SystemLocation, QStandardPaths::StandardLocation> SYSTEM_MAP_ =
{
    { Coco::SystemLocation::AppConfig, QStandardPaths::AppConfigLocation },
    { Coco::SystemLocation::AppData, QStandardPaths::AppDataLocation },
    { Coco::SystemLocation::AppLocalData, QStandardPaths::AppLocalDataLocation },
    { Coco::SystemLocation::Applications, QStandardPaths::ApplicationsLocation },
    { Coco::SystemLocation::Cache, QStandardPaths::CacheLocation },
    { Coco::SystemLocation::Config, QStandardPaths::ConfigLocation },
    { Coco::SystemLocation::Desktop, QStandardPaths::DesktopLocation },
    { Coco::SystemLocation::Downloads, QStandardPaths::DownloadLocation },
    { Coco::SystemLocation::Documents, QStandardPaths::DocumentsLocation },
    { Coco::SystemLocation::Fonts, QStandardPaths::FontsLocation },
    { Coco::SystemLocation::GenericCache, QStandardPaths::GenericCacheLocation },
    { Coco::SystemLocation::GenericConfig, QStandardPaths::GenericConfigLocation },
    { Coco::SystemLocation::GenericData, QStandardPaths::GenericDataLocation },
    { Coco::SystemLocation::Home, QStandardPaths::HomeLocation },
    { Coco::SystemLocation::Movies, QStandardPaths::MoviesLocation },
    { Coco::SystemLocation::Music, QStandardPaths::MusicLocation },
    { Coco::SystemLocation::Pictures, QStandardPaths::PicturesLocation },
    { Coco::SystemLocation::PublicShare, QStandardPaths::PublicShareLocation },
    { Coco::SystemLocation::Runtime, QStandardPaths::RuntimeLocation },
    { Coco::SystemLocation::Temp, QStandardPaths::TempLocation },
    { Coco::SystemLocation::Templates, QStandardPaths::TemplatesLocation }
};

namespace Coco
{
    std::string Path::prettyString() const
    {
        std::string pretty{};
        auto last_ch_was_sep = false;

        for (auto& ch : CACHED_STRING_(d_))
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

    QString Path::fromSystem_(SystemLocation value) const
    {
        if (value == SystemLocation::Root)
            return QDir::rootPath();

        auto it = SYSTEM_MAP_.find(value);

        if (it != SYSTEM_MAP_.end())
            return standardLocation_(*it);

        return {};
    }

} // namespace Coco

#undef TO_QSTRING_
#undef CACHED_QSTRING_
#undef CACHED_STRING_
