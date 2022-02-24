/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RimRftTools.h"

#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaRftDefines.h"

#include "RifReaderRftInterface.h"

#include "cafPdmUiItem.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTools::wellLogChannelsOptions( RifReaderRftInterface* readerRft, const QString& wellName )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( readerRft )
    {
        for ( const RifEclipseRftAddress::RftWellLogChannelType& channelName :
              readerRft->availableWellLogChannels( wellName ) )
        {
            options.push_back(
                caf::PdmOptionItemInfo( caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText( channelName ),
                                        channelName ) );
        }
    }

    if ( options.empty() )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RifEclipseRftAddress::RftWellLogChannelType>::uiText(
                                                       RifEclipseRftAddress::RftWellLogChannelType::NONE ),
                                                   RifEclipseRftAddress::RftWellLogChannelType::NONE ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTools::wellNameOptions( RifReaderRftInterface* readerRft )
{
    QList<caf::PdmOptionItemInfo> options;

    options.push_back( caf::PdmOptionItemInfo( "None", "" ) );
    if ( readerRft )
    {
        std::set<QString> wellNames = readerRft->wellNames();
        for ( const QString& name : wellNames )
        {
            options.push_back( caf::PdmOptionItemInfo( name, name, false, caf::IconProvider( ":/Well.svg" ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTools::timeStepOptions( RifReaderRftInterface*                      readerRft,
                                                            const QString&                              wellName,
                                                            RifEclipseRftAddress::RftWellLogChannelType channelType )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( readerRft )
    {
        QString             dateFormat = "dd MMM yyyy";
        std::set<QDateTime> timeStamps = readerRft->availableTimeSteps( wellName, channelType );
        for ( const QDateTime& dt : timeStamps )
        {
            QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale( dt, dateFormat );

            options.push_back( caf::PdmOptionItemInfo( dateString, dt ) );
        }
    }

    options.push_back( caf::PdmOptionItemInfo( "None", QDateTime() ) );

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTools::segmentTimeStepOptions( RifReaderRftInterface* readerRft, const QString& wellName )
{
    return timeStepOptions( readerRft, wellName, RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTools::segmentResultNameOptions( RifReaderRftInterface* readerRft,
                                                                     const QString&         wellName,
                                                                     const QDateTime&       timeStep )
{
    QList<caf::PdmOptionItemInfo> options;

    options.push_front(
        caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), RiaResultNames::undefinedResultName() ) );

    if ( readerRft )
    {
        options.push_back(
            caf::PdmOptionItemInfo( RiaDefines::segmentNumberResultName(), RiaDefines::segmentNumberResultName() ) );

        for ( const auto& resultAdr : readerRft->eclipseRftAddresses( wellName, timeStep ) )
        {
            if ( resultAdr.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
            {
                options.push_back( caf::PdmOptionItemInfo( resultAdr.segmentResultName(), resultAdr.segmentResultName() ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimRftTools::segmentBranchIdOptions( RifReaderRftInterface* readerRft,
                                                                   const QString&         wellName,
                                                                   const QDateTime&       timeStep )
{
    QList<caf::PdmOptionItemInfo> options;

    options.push_front( caf::PdmOptionItemInfo( RiaDefines::allBranches(), RiaDefines::allBranches() ) );

    if ( readerRft )
    {
        std::vector<double> values;

        auto adr =
            RifEclipseRftAddress::createSegmentResult( wellName, timeStep, RiaDefines::segmentBranchNumberResultName() );

        readerRft->values( adr, &values );
        for ( const auto& v : values )
        {
            int  intValue = v;
            auto txt      = QString::number( intValue );
            options.push_back( caf::PdmOptionItemInfo( txt, txt ) );
        }
    }

    return options;
}
