#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QIODevice>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QtTypes>

#include "../include/Coco/Path.h"
#include "../include/Coco/PathUtil.h"

// ----- Internal -----

//struct FileTypeDatum_
//{
//    Coco::FileSignature type{};
//    QByteArray signature{};
//    int offset = 0;
//};

// https://en.wikipedia.org/wiki/List_of_file_signatures
//static const QList<FileTypeDatum_> FILE_TYPE_DATA_ =
//{
//    // For zip, there are 3 signatures, but they all share the same first 2
//    // bytes. And, to be fair, we could probably check only the first 3 or so
//    // bytes for any signature. I don't see many sharing similar first bytes.
//    // .docx is a .zip file!
//    { Coco::FileSignature::Png,        QByteArray::fromHex("89504E470D0A1A0A") },  // 8 bytes
//    { Coco::FileSignature::SevenZip,   QByteArray::fromHex("377ABCAF271C") },      // 6
//    { Coco::FileSignature::Rtf,        QByteArray::fromHex("7B5C72746631") },      // 6
//    { Coco::FileSignature::TarXz,      QByteArray::fromHex("FD377A585A00") },      // 6
//    { Coco::FileSignature::Pdf,        QByteArray::fromHex("255044462D") },        // 5
//    { Coco::FileSignature::Tar,        QByteArray::fromHex("7573746172"), 257 },   // 5
//    { Coco::FileSignature::Gif,        QByteArray::fromHex("47494638") },          // 4
//    { Coco::FileSignature::Jpg,        QByteArray::fromHex("FFD8FF") },            // 3
//    { Coco::FileSignature::Utf8Bom,    QByteArray::fromHex("EFBBBF") },            // 3
//    { Coco::FileSignature::Zip,        QByteArray::fromHex("504B") },              // 2
//    { Coco::FileSignature::TarGz,      QByteArray::fromHex("1F8B") }               // 2
//};

static QStringList fromArgsExtHelper_(const QString& extensions)
{
    QStringList resolved{};

    for (auto& ext : extensions.split(","))
        resolved << Coco::PathUtil::resolveExt(ext);

    return resolved;
}

static QStringList fromDirExtHelper_(const QString& extensions)
{
    QStringList resolved{};

    for (auto& ext : extensions.split(","))
        resolved << "*" + Coco::PathUtil::resolveExt(ext);

    return resolved;
}

// Handle other flags in future, maybe
static constexpr QDirIterator::IteratorFlags flagsHelper_(Coco::Recursive recursive) noexcept
{
    return recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
}

// ----- End Internal -----

namespace Coco::PathUtil
{
    QList<Path> fromDir
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
            fromDirExtHelper_(extensions),
            QDir::Files,
            flagsHelper_(recursive)
        );

        while (it.hasNext())
        {
            it.next();
            paths << it.filePath();
        }

        return paths;
    }

    QList<Path> fromDir
    (
        const QList<Path>& directories,
        const QString& extensions,
        Recursive recursive
    )
    {
        QList<Path> paths{};
        auto exts = fromDirExtHelper_(extensions);
        auto flags = flagsHelper_(recursive);

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

    QList<Path> fromArgs
    (
        const QStringList& args,
        const QString& extensions
    )
    {
        QList<Path> paths{};
        auto exts = fromArgsExtHelper_(extensions);

        for (const auto& arg : args)
        {
            Path path(arg);

            if (exts.contains(path.extQString()))
                paths.append(path);
        }

        return paths;
    }

    // For isolating paths from main function arguments
    QList<Path> fromArgs
    (
        int argc,
        const char* const* argv,
        const QString& extensions
    )
    {
        QStringList args{};

        for (auto i = 0; i < argc; ++i)
        {
            auto arg = argv[i];
            if (arg) args << QString::fromUtf8(arg);
        }

        return fromArgs(args, extensions);
    }

    //FileSignature fileType(const Path& path, FileSignatures filter)
    //{
    //    // If the caller does not provide any possible types (or intentionally
    //    // passes UnknownOrUtf8), compute the union of all types based on
    //    // FILE_TYPE_DATA_.
    //    if (filter == FileSignature::UnknownOrUtf8)
    //    {
    //        static const auto all = []
    //            {
    //                FileSignatures types{};

    //                for (const auto& datum : FILE_TYPE_DATA_)
    //                    types |= datum.type;

    //                return types;
    //            }();

    //        filter = all;
    //    }

    //    QFile file(path.toQString());

    //    if (!file.open(QIODevice::ReadOnly))
    //    {
    //        qDebug() << "Unable to open file:" << path.toQString();
    //        return FileSignature::UnknownOrUtf8;
    //    }

    //    // Determine max signature length and offset required
    //    auto length = 0;
    //    auto offset = 0;

    //    for (const auto& datum : FILE_TYPE_DATA_)
    //    {
    //        if (filter & datum.type)
    //        {
    //            length = qMax(length, datum.signature.size());
    //            offset = qMax(offset, datum.offset);
    //        }
    //    }

    //    // Read file header
    //    auto maxlen = static_cast<qint64>(offset + length);
    //    auto file_header = file.read(maxlen);

    //    // Check against available signatures
    //    for (const auto& datum : FILE_TYPE_DATA_)
    //    {
    //        // Ensure that all flags in datum.type are present in filter.
    //        if ((datum.type & filter) != datum.type) continue;

    //        auto sig_size = datum.signature.size();
    //        auto slice = file_header.mid(datum.offset, sig_size);

    //        if (slice == datum.signature)
    //            return datum.type;
    //    }

    //    return FileSignature::UnknownOrUtf8;
    //}

    QString resolveExt(const QString& extension)
    {
        auto ext = extension.trimmed();
        if (ext.isEmpty()) return {};

        // Ensure extension starts with a dot
        constexpr QChar dot('.');
        return (ext.startsWith(dot) ? ext : dot + ext);
    }

} // namespace Coco::PathUtil
