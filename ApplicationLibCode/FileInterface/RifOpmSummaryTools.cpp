/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RifOpmSummaryTools.h"

#include "RiaFilePathTools.h"
#include "RiaLogging.h"

#ifdef _MSC_VER
// Disable warning from external library to make sure treat warnings as error works
#pragma warning( disable : 4267 )
#endif
#include "opm/io/eclipse/ESmry.hpp"
#include "opm/io/eclipse/ExtESmry.hpp"

#include "QRegularExpression"
#include <QFile>
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<std::set<RifEclipseSummaryAddress>, std::map<RifEclipseSummaryAddress, size_t>, std::map<RifEclipseSummaryAddress, std::string>>
    RifOpmSummaryTools::buildAddressesSmspecAndKeywordMap( const Opm::EclIO::ESmry* summaryFile )
{
    std::set<RifEclipseSummaryAddress>              addresses;
    std::map<RifEclipseSummaryAddress, size_t>      addressToSmspecIndexMap;
    std::map<RifEclipseSummaryAddress, std::string> addressToKeywordMap;

    if ( summaryFile )
    {
        auto keywords = summaryFile->keywordList();
        for ( const auto& keyword : keywords )
        {
            auto eclAdr = RifEclipseSummaryAddress::fromEclipseTextAddress( keyword );
            if ( !eclAdr.isValid() )
            {
                // If a category is not found, use the MISC category
                eclAdr = RifEclipseSummaryAddress::miscAddress( keyword );
            }

            if ( eclAdr.isValid() )
            {
                addresses.insert( eclAdr );
                size_t smspecIndex              = summaryFile->getSmspecIndexForKeyword( keyword );
                addressToSmspecIndexMap[eclAdr] = smspecIndex;
                addressToKeywordMap[eclAdr]     = keyword;
            }
        }
    }

    return { addresses, addressToSmspecIndexMap, addressToKeywordMap };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::set<RifEclipseSummaryAddress>, std::map<RifEclipseSummaryAddress, std::string>>
    RifOpmSummaryTools::buildAddressesAndKeywordMap( const std::vector<std::string>& keywords )
{
    std::set<RifEclipseSummaryAddress>              addresses;
    std::map<RifEclipseSummaryAddress, std::string> addressToKeywordMap;

    std::vector<std::string> invalidKeywords;

#pragma omp parallel
    {
        std::vector<RifEclipseSummaryAddress>                         threadAddresses;
        std::vector<std::pair<RifEclipseSummaryAddress, std::string>> threadAddressToKeywordMap;
        std::vector<std::string>                                      threadInvalidKeywords;

#pragma omp for
        for ( int index = 0; index < (int)keywords.size(); index++ )
        {
            auto keyword = keywords[index];

            auto eclAdr = RifEclipseSummaryAddress::fromEclipseTextAddress( keyword );
            if ( !eclAdr.isValid() )
            {
                threadInvalidKeywords.push_back( keyword );

                // If a category is not found, use the MISC category
                eclAdr = RifEclipseSummaryAddress::miscAddress( keyword );
            }

            if ( eclAdr.isValid() )
            {
                threadAddresses.emplace_back( eclAdr );
                threadAddressToKeywordMap.emplace_back( std::make_pair( eclAdr, keyword ) );
            }
        }

#pragma omp critical
        {
            addresses.insert( threadAddresses.begin(), threadAddresses.end() );
            addressToKeywordMap.insert( threadAddressToKeywordMap.begin(), threadAddressToKeywordMap.end() );
            invalidKeywords.insert( invalidKeywords.end(), threadInvalidKeywords.begin(), threadInvalidKeywords.end() );
        }

        // DEBUG code
        // Used to print keywords not being categorized correctly
        /*
            for ( const auto& kw : invalidKeywords )
            {
                RiaLogging::warning( QString::fromStdString( kw ) );
            }
        */
    }

    return { addresses, addressToKeywordMap };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddressDefines::SummaryCategory RifOpmSummaryTools::categoryFromKeyword( const std::string& keyword )
{
    auto opmCategory = Opm::EclIO::SummaryNode::category_from_keyword( keyword );
    switch ( opmCategory )
    {
        case Opm::EclIO::SummaryNode::Category::Aquifer:
            return SummaryCategory::SUMMARY_AQUIFER;
        case Opm::EclIO::SummaryNode::Category::Block:
            return SummaryCategory::SUMMARY_BLOCK;
        case Opm::EclIO::SummaryNode::Category::Connection:
            return SummaryCategory::SUMMARY_WELL_CONNECTION;
        case Opm::EclIO::SummaryNode::Category::Completion:
            return SummaryCategory::SUMMARY_WELL_COMPLETION;
        case Opm::EclIO::SummaryNode::Category::Field:
            return SummaryCategory::SUMMARY_FIELD;
        case Opm::EclIO::SummaryNode::Category::Group:
            return SummaryCategory::SUMMARY_GROUP;
    }

    return SummaryCategory::SUMMARY_INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifOpmSummaryTools::enhancedSummaryFilename( const QString& fileName )
{
    QString s( fileName );
    return s.replace( ".SMSPEC", ".ESMRY" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifOpmSummaryTools::smspecSummaryFilename( const QString& fileName )
{
    QString s( fileName );
    return s.replace( ".ESMRY", ".SMSPEC" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmSummaryTools::isEsmryConversionRequired( const QString& fileName )
{
    auto candidateEsmryFileName = enhancedSummaryFilename( fileName );
    auto smspecFileName         = smspecSummaryFilename( fileName );

    if ( !QFile::exists( candidateEsmryFileName ) && QFile::exists( smspecFileName ) )
    {
        return true;
    }

    if ( RiaFilePathTools::isFirstOlderThanSecond( candidateEsmryFileName.toStdString(), smspecFileName.toStdString() ) )
    {
        QString root = QFileInfo( smspecFileName ).canonicalPath();

        const QString smspecFileNameShort = QFileInfo( smspecFileName ).fileName();
        const QString esmryFileNameShort  = QFileInfo( candidateEsmryFileName ).fileName();

        RiaLogging::debug(
            QString( " %3 : %1 is older than %2, recreating %1." ).arg( esmryFileNameShort ).arg( smspecFileNameShort ).arg( root ) );

        // Check if we have write permission in the folder
        QFileInfo info( smspecFileName );

        if ( !info.isWritable() )
        {
            QString txt = QString( "ESMRY is older than SMSPEC, but export to file %1 failed due to missing write permissions. "
                                   "Aborting operation." )
                              .arg( candidateEsmryFileName );
            RiaLogging::error( txt );

            return false;
        }

        if ( !std::filesystem::remove( candidateEsmryFileName.toStdString() ) )
        {
            RiaLogging::error( QString( "Failed to remove file: %1" ).arg( candidateEsmryFileName ) );
            return false;
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<int, QString> RifOpmSummaryTools::extractRealizationNumber( const QString& path )
{
    QRegularExpression      pattern( "realization-(\\d+)", QRegularExpression::CaseInsensitiveOption );
    QRegularExpressionMatch match = pattern.match( path );

    if ( match.hasMatch() )
    {
        bool ok;
        int  result = match.captured( 1 ).toInt( &ok );
        if ( ok )
        {
            return result;
        }
        return std::unexpected( QString( "Invalid realization number format in path: %1" ).arg( path ) );
    }

    return std::unexpected( QString( "Could not extract realization number from path: %1" ).arg( path ) );
}
