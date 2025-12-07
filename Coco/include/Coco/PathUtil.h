#pragma once

#include <filesystem>

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "Bool.h"
#include "Global.h"
#include "Path.h"

// TODO: Maybe just reintegrate with Path, for convenience
// TODO: Own Path iterator

namespace Coco::PathUtil
{
    // Creates all directories in the specified path.
    inline bool mkdir(const Path& path)
    {
        return std::filesystem::create_directories(path.toStd());
    }

    // Add sorting option
    // Provide extensions as ".h, .cpp" (one QString)
    QList<Path> fromDir
    (
        const Path& directory,
        const QString& extensions,
        Recursive recursive = Recursive::Yes
    );

    QList<Path> fromDir
    (
        const QList<Path>& directories,
        const QString& extensions,
        Recursive recursive = Recursive::Yes
    );

    // For isolating paths from QApplication arguments
    QList<Path> fromArgs
    (
        const QStringList& args,
        const QString& extensions
    );

    // For isolating paths from main function arguments
    QList<Path> fromArgs
    (
        int argc,
        const char* const* argv,
        const QString& extensions
    );

    inline bool copy(const Path& path, const Path& newPath, Overwrite overwrite = Overwrite::No)
    {
        if (overwrite && newPath.exists()) QFile::remove(newPath.toQString());
        return QFile::copy(path.toQString(), newPath.toQString());
    }

    inline bool copyContents(const Path& srcDir, const Path& dstDir)
    {
        if (!srcDir.exists() || !srcDir.isFolder()) return false;
        if (!dstDir.exists() && !mkdir(dstDir)) return false;

        QDir src_dir(srcDir.toQString());
        auto entries = src_dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

        for (const auto& entry : entries) {
            auto src_path = srcDir / entry;
            auto dst_path = dstDir / entry;

            if (src_path.isFolder()) {
                // Recurse
                if (!mkdir(dst_path)) return false;
                if (!copyContents(src_path, dst_path)) return false;
            }
            else {
                if (!copy(src_path, dst_path)) return false;
            }
        }

        return true;
    }

    /// @brief Identifies the file type based on its signature.
    /// 
    /// @param filter Specifies the file types to check against. For example,
    /// `Coco::Pdf | Coco::Png` will limit the checks to PDFs and PNGs. If no
    /// matching type is found, UnknownOrUtf8 is always returned.
    /// 
    /// @details Allows exclusion of specific file types by applying a bitwise
    /// negation. For instance, use `Coco::FileTypes() & ~Coco::Pdf` to check
    /// all types except PDF.
    //FileSignature fileType(const Path& path, FileSignatures filter = FileSignature::UnknownOrUtf8); // maybe "any" default

    /// @brief Checks if the file matches the specified type.
    //inline bool is(FileSignature type, const Path& path)
    //{
    //    return fileType(path, type) == type; // If no filter is passed, then this is always true, I think.
    //}

    QString resolveExt(const QString& extension);

    namespace Dialog
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
            QString* selectedFilter = nullptr,
            QFileDialog::Options options = {}
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
                    selectedFilter,
                    options
                )
            );
        }

        inline QList<Path> files
        (
            QWidget* parent = nullptr,
            const QString& caption = {},
            const Path& startPath = {},
            const QString& filter = {},
            QString* selectedFilter = nullptr,
            QFileDialog::Options options = {}
        )
        {
            auto string_paths = QFileDialog::getOpenFileNames
            (
                parent,
                caption,
                startPath.toQString(),
                filter,
                selectedFilter,
                options
            );

            QList<Path> paths{};
            for (const auto& str : string_paths)
                paths << Path(str);

            return paths;
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

    } // namespace Coco::PathUtil::Dialog

} // namespace Coco::PathUtil
