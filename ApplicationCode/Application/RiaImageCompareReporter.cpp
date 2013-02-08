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

#include "RiaImageCompareReporter.h"
#include <iostream>
#include <fstream>
#include <QDir>

RiaImageCompareReporter::RiaImageCompareReporter(void)
{
}


RiaImageCompareReporter::~RiaImageCompareReporter(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaImageCompareReporter::addImageDirectoryComparisonSet(const std::string& title, const std::string& baseImageDir, const std::string& newImagesDir, const std::string& diffImagesDir)
{
    m_directorySets.push_back(DirSet(title, baseImageDir, newImagesDir, diffImagesDir));
}

std::string removeCommonStart(const std::string& mask, const std::string& filename)
{
    size_t i;
    for (i = 0; i < mask.size() && i < filename.size(); ++i)
    {
        if (mask[i] != filename[i]) break;
    }
    
    return filename.substr(i);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaImageCompareReporter::generateHTMLReport(const std::string& fileName)
{
    if (m_directorySets.size() == 0) return;

    std::ofstream output(fileName.c_str());
    if (!output) 
    {
        std::cout << "Trouble opening test report file: " << fileName;
        return;
    }

    std::string html;

    html += "<html>\n";
    html += "<head>\n";
    html += "<title>Regression-Test Report</title>\n";
    html += "</head>\n";
    html += "\n";
    html += "<body>\n";
    html += "\n";

    for (size_t dsIdx = 0; dsIdx < m_directorySets.size(); ++dsIdx)
    {
        std::vector<std::string> baseImageNames = getPngFilesInDirectory(m_directorySets[dsIdx].m_baseImageDir);
        
        html += "<table>\n";
        html += "  <tr>\n";
        html += "    <td colspan=\"3\" bgcolor=\"darkblue\" height=\"40\">  <b><font color=\"white\" size=\"3\"> " + m_directorySets[dsIdx].m_title + " </font></b> </td>\n";
        html += "  </tr>\n";

        std::string baseImageFolder = removeCommonStart(fileName, m_directorySets[dsIdx].m_baseImageDir );
        std::string genImageFolder  = removeCommonStart(fileName, m_directorySets[dsIdx].m_newImagesDir );
        std::string diffImageFolder = removeCommonStart(fileName, m_directorySets[dsIdx].m_diffImagesDir);

        for (size_t fIdx = 0; fIdx < baseImageNames.size(); ++fIdx)
        {
            html += "  <tr>\n";
            html += "    <td colspan=\"3\" bgcolor=\"lightgray\"> " + baseImageNames[fIdx] + "</td>\n";
            html += "  </tr>\n";

            html += "  <tr>\n";
            html += "    <td>  <img src=\"" + baseImageFolder  + "/" + baseImageNames[fIdx] + "\" width=\"100%\" alt=\"" + baseImageFolder  + "/" + baseImageNames[fIdx] + "\" >  </td>\n";
            html += "    <td>  <img src=\"" + genImageFolder   + "/" + baseImageNames[fIdx] + "\" width=\"100%\" alt=\"" + genImageFolder   + "/" + baseImageNames[fIdx] + "\" >  </td>\n";
            html += "    <td>  <img src=\"" + diffImageFolder  + "/" + baseImageNames[fIdx] + "\" width=\"100%\" alt=\"" + diffImageFolder  + "/" + baseImageNames[fIdx] + "\" >  </td>\n";
            html += "  </tr>\n";

            // A little air between images
            html += "  <tr> <td height=\"10\"> </td> </tr>\n";
        }

        html += "</table>\n";

        html += "\n";
    }

    output << html;
}

//--------------------------------------------------------------------------------------------------
/// Retuns the names of the *.png files in a directory. The names are without path, but with extention
//--------------------------------------------------------------------------------------------------

std::vector<std::string> RiaImageCompareReporter::getPngFilesInDirectory(const std::string& searchPath)
{
    QDir searchDir(QString::fromStdString(searchPath));

    searchDir.setFilter(QDir::Files);
    //QStringList filter;
    //filter.append("*.png");
    //searchDir.setNameFilters(filter);

    QStringList imageFiles = searchDir.entryList();

    std::vector<std::string> fileNames;
    for (int i = 0; i < imageFiles.size(); ++i)
    {
        fileNames.push_back(imageFiles[i].toStdString());
    }

    return fileNames;
}
