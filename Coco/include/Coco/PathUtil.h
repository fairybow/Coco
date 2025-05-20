#pragma once

#include <filesystem>

#include <QFileDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "Global.h"
#include "Path.h"

namespace Coco::PathUtil
{
    // Creates all directories in the specified path.
    inline static bool mkdir(const Path& path)
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

    /// @brief Identifies the file type based on its signature.
    /// 
    /// @param filter Specifies the file types to check against. For example,
    /// `Coco::Pdf | Coco::Png` will limit the checks to PDFs and PNGs. If no
    /// matching type is found, UnknownOrUtf8 is always returned.
    /// 
    /// @details Allows exclusion of specific file types by applying a bitwise
    /// negation. For instance, use `Coco::FileTypes() & ~Coco::Pdf` to check
    /// all types except PDF.
    FileSignature fileType(const Path& path, FileSignatures filter = FileSignature::UnknownOrUtf8); // maybe "any" default

    /// @brief Checks if the file matches the specified type.
    inline bool is(FileSignature type, const Path& path)
    {
        return fileType(path, type) == type; // If no filter is passed, then this is always true, I think.
    }

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

    } // namespace Coco::PathUtil::Dialog

} // namespace Coco::PathUtil
