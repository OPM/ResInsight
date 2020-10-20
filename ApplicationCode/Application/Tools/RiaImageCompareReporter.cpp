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
#include <QDir>
#include <fstream>
#include <iostream>

RiaImageCompareReporter::RiaImageCompareReporter( void )
{
    m_showOriginal        = true;
    m_showGenerated       = true;
    m_showInteractiveDiff = false;
}

RiaImageCompareReporter::~RiaImageCompareReporter( void )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaImageCompareReporter::addImageDirectoryComparisonSet( const std::string& title,
                                                              const std::string& baseImageDir,
                                                              const std::string& newImagesDir,
                                                              const std::string& diffImagesDir )
{
    m_directorySets.push_back( DirSet( title, baseImageDir, newImagesDir, diffImagesDir ) );
}

std::string removeCommonStart( const std::string& mask, const std::string& filename )
{
    size_t i;
    for ( i = 0; i < mask.size() && i < filename.size(); ++i )
    {
        if ( mask[i] != filename[i] ) break;
    }

    return filename.substr( i );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaImageCompareReporter::generateHTMLReport( const std::string& fileName, const std::string& diff2htmlHeaderText )
{
    if ( m_directorySets.size() == 0 ) return;

    std::ofstream output( fileName.c_str() );
    if ( !output )
    {
        std::cout << "Trouble opening test report file: " << fileName;
        return;
    }

    std::string html;

    html += "<html>\n";
    html += "<head>\n";

    if ( m_showInteractiveDiff )
    {
        html += cssString();
    }

    html += diff2htmlHeaderText;

    html += "</head>\n";
    html += "\n";
    html += "<body>\n";
    html += "<title>Regression-Test Report</title>\n";
    html += "\n";

    for ( size_t dsIdx = 0; dsIdx < m_directorySets.size(); ++dsIdx )
    {
        std::vector<std::string> baseImageNames = getPngFilesInDirectory( m_directorySets[dsIdx].m_baseImageDir );

        html += "<table>\n";
        html += "  <tr>\n";
        html += "    <td colspan=\"3\" bgcolor=\"darkblue\" height=\"40\">  <b><font color=\"white\" size=\"3\"> " +
                m_directorySets[dsIdx].m_title + " </font></b> </td>\n";
        html += "  </tr>\n";

        std::string baseImageFolder = removeCommonStart( fileName, m_directorySets[dsIdx].m_baseImageDir );
        std::string genImageFolder  = removeCommonStart( fileName, m_directorySets[dsIdx].m_newImagesDir );
        std::string diffImageFolder = removeCommonStart( fileName, m_directorySets[dsIdx].m_diffImagesDir );

        for ( size_t fIdx = 0; fIdx < baseImageNames.size(); ++fIdx )
        {
            html += "  <tr>\n";
            html += "    <td colspan=\"3\" bgcolor=\"lightgray\"> " + baseImageNames[fIdx] + "</td>\n";
            html += "  </tr>\n";

            html += "  <tr>\n";
            if ( m_showOriginal )
            {
                html += "    <td>  <img src=\"" + baseImageFolder + "/" + baseImageNames[fIdx] +
                        "\" width=\"100%\" alt=\"" + baseImageFolder + "/" + baseImageNames[fIdx] + "\" >  </td>\n";
            }

            if ( m_showGenerated )
            {
                html += "    <td>  <img src=\"" + genImageFolder + "/" + baseImageNames[fIdx] +
                        "\" width=\"100%\" alt=\"" + genImageFolder + "/" + baseImageNames[fIdx] + "\" >  </td>\n";
            }

            if ( m_showInteractiveDiff )
            {
                html += "    <td> <div class = \"image-slider\"> <div> <img src=\"" + baseImageFolder + "/" +
                        baseImageNames[fIdx] + "\" > </div> <img src = \"" + genImageFolder + "/" +
                        baseImageNames[fIdx] + "\" > </div> </td>\n";
            }

            html += "    <td>  <img src=\"" + diffImageFolder + "/" + baseImageNames[fIdx] +
                    "\" width=\"100%\" alt=\"" + diffImageFolder + "/" + baseImageNames[fIdx] + "\" >  </td>\n";
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
///
//--------------------------------------------------------------------------------------------------
void RiaImageCompareReporter::showInteractiveOnly()
{
    m_showOriginal        = false;
    m_showGenerated       = false;
    m_showInteractiveDiff = true;
}

//--------------------------------------------------------------------------------------------------
/// Retuns the names of the *.png files in a directory. The names are without path, but with extension
//--------------------------------------------------------------------------------------------------

std::vector<std::string> RiaImageCompareReporter::getPngFilesInDirectory( const std::string& searchPath )
{
    QDir searchDir( QString::fromStdString( searchPath ) );

    searchDir.setFilter( QDir::Files );
    // QStringList filter;
    // filter.append("*.png");
    // searchDir.setNameFilters(filter);

    QStringList imageFiles = searchDir.entryList();

    std::vector<std::string> fileNames;
    for ( int i = 0; i < imageFiles.size(); ++i )
    {
        fileNames.push_back( imageFiles[i].toStdString() );
    }

    return fileNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaImageCompareReporter::cssString() const
{
    std::string html;

    html += "<style media=\"screen\" type=\"text/css\">";

    html += "";
    html += ".image-slider {";
    html += "position:relative;";
    html += "display: inline-block;";
    html += "line-height: 0;";
    html += "}";

    html += ".image-slider > div {";
    html += "position: absolute;";
    html += "top: 0; bottom: 0; left: 0;";
    html += "width: 25px;";
    html += "max-width: 100%;";
    html += "overflow: hidden;";
    html += "resize: horizontal;";
    html += "}";

    html += ".image-slider > div:before {";
    html += "content: '';";
    html += "position: absolute;";
    html += "right: 0; bottom: 0;";
    html += "width: 23px; height: 23px;";
    html += "padding: 5px;";
    html += "background: linear-gradient(-45deg, gray 50%, transparent 0);";
    html += "background-clip: content-box;";
    html += "cursor: ew-resize;";
    html += "-webkit-filter: drop-shadow(0 0 6px black);";
    html += "filter: drop-shadow(0 0 6px black);";
    html += "}";

    html += ".image-slider img {";
    html += "user-select: none;";
    html += "max-width: 1000px;";
    html += "}";

    html += "</style>";

    return html;
}
