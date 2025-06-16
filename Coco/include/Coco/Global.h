#pragma once

#include <QFlags>
#include <QObject>

#include "Bool.h"
#include "Macros.h"

namespace Coco
{
    COCO_BOOL(CreateDirs);
    COCO_BOOL(Recursive);
    COCO_BOOL(Sort);

    enum class SystemLocation
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

    /// @note I would like a default "any" value, but technically, Utf8 is the
    /// "base" file type, and adding an extra non-file type would mean weirdness
    /// with flags, I think. Need to sort this out...
    enum class FileSignature
    {
        UnknownOrUtf8 = 0,
        Png = 1 << 0,
        SevenZip = 1 << 1,
        Rtf = 1 << 2,
        TarXz = 1 << 3,
        Pdf = 1 << 4,
        Tar = 1 << 5,
        Gif = 1 << 6,
        Jpg = 1 << 7,
        Utf8Bom = 1 << 8,
        Zip = 1 << 9,
        TarGz = 1 << 10
    };

    typedef QFlags<FileSignature> FileSignatures;

} // namespace Coco

Q_DECLARE_OPERATORS_FOR_FLAGS(Coco::FileSignatures)
