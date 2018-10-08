/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

class RiaImageCompareReporter
{
public:
    RiaImageCompareReporter();
    virtual ~RiaImageCompareReporter();

    void addImageDirectoryComparisonSet(const std::string& title, const std::string& baseImageDir, const std::string& newImagesDir, const std::string& diffImagesDir  );
    void generateHTMLReport(const std::string& filenName, const std::string& pathToDiff2html);

    void showInteractiveOnly();

 
private:
    static std::vector<std::string> getPngFilesInDirectory(const std::string& searchPath);
    std::string cssString() const;

private:
    struct DirSet
    {
        DirSet(const std::string& title, const std::string& baseImageDir, const std::string& newImagesDir, const std::string& diffImagesDir )
            : m_title(title),
            m_baseImageDir(baseImageDir),
            m_newImagesDir(newImagesDir),
            m_diffImagesDir(diffImagesDir)
        {}

        std::string m_title;
        std::string m_baseImageDir;
        std::string m_newImagesDir;
        std::string m_diffImagesDir;
    };

    std::vector<DirSet> m_directorySets;

    bool m_showOriginal;
    bool m_showGenerated;
    bool m_showInteractiveDiff;
};

