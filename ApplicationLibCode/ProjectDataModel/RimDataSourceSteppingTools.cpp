/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RimDataSourceSteppingTools.h"

#include "RiaSummaryAddressAnalyzer.h"

#include "RifEclipseSummaryAddress.h"
#include "cafPdmUiFieldHandle.h"

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataSourceSteppingTools::modifyCurrentIndex( caf::PdmValueField*                  valueField,
                                                     const QList<caf::PdmOptionItemInfo>& options,
                                                     int                                  indexOffset,
                                                     bool                                 notifyChange )
{
    if ( valueField && !options.isEmpty() )
    {
        QVariant                              currentValue  = valueField->toQVariant();
        caf::PdmPointer<caf::PdmObjectHandle> currentHandle = currentValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
        int                                   currentIndex  = -1;
        for ( int i = 0; i < options.size(); i++ )
        {
            QVariant optionValue = options[i].value();
            // First try pointer variety. They are not supported by QVariant::operator==
            caf::PdmPointer<caf::PdmObjectHandle> optionHandle = optionValue.value<caf::PdmPointer<caf::PdmObjectHandle>>();
            if ( optionHandle )
            {
                if ( currentHandle == optionHandle )
                {
                    currentIndex = i;
                }
            }
            else if ( currentValue == optionValue )
            {
                currentIndex = i;
            }
        }

        if ( currentIndex == -1 )
        {
            currentIndex = 0;
        }

        int nextIndex = currentIndex + indexOffset;
        if ( nextIndex < options.size() && nextIndex > -1 )
        {
            QVariant newValue = options[nextIndex].value();
            valueField->setFromQVariant( newValue );
            if ( notifyChange ) valueField->uiCapability()->notifyFieldChanged( currentValue, newValue );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataSourceSteppingTools::updateAddressIfMatching( const QVariant&                              oldValue,
                                                          const QVariant&                              newValue,
                                                          RifEclipseSummaryAddress::SummaryVarCategory category,
                                                          RifEclipseSummaryAddress*                    adr )
{
    if ( !adr ) return false;

    if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();

        if ( adr->regionNumber() == oldInt )
        {
            adr->setRegion( newInt );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();

        if ( adr->aquiferNumber() == oldInt )
        {
            adr->setAquiferNumber( newInt );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();

        if ( adr->groupName() == oldString )
        {
            adr->setGroupName( newString );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();

        if ( adr->wellName() == oldString )
        {
            adr->setWellName( newString );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_BLOCK || category == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();
        if ( adr->blockAsString() == oldString )
        {
            adr->setCellIjk( newString );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION )
    {
        std::string oldString = oldValue.toString().toStdString();
        std::string newString = newValue.toString().toStdString();
        if ( adr->formatUiTextRegionToRegion() == oldString )
        {
            auto [region1, region2] = RifEclipseSummaryAddress::regionToRegionPairFromUiText( newString );
            adr->setRegion( region1 );
            adr->setRegion2( region2 );

            return true;
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_SEGMENT )
    {
        int oldInt = oldValue.toInt();
        int newInt = newValue.toInt();
        if ( adr->wellSegmentNumber() == oldInt )
        {
            adr->setWellSegmentNumber( newInt );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataSourceSteppingTools::updateHistoryAndSummaryQuantityIfMatching( const QVariant&           oldValue,
                                                                            const QVariant&           newValue,
                                                                            RifEclipseSummaryAddress* adr )
{
    if ( !adr ) return false;

    std::string oldString = oldValue.toString().toStdString();
    std::string newString = newValue.toString().toStdString();

    if ( adr->vectorName() == oldString )
    {
        adr->setVectorName( newString );

        return true;
    }

    std::string correspondingOldString = RiaSummaryAddressAnalyzer::correspondingHistorySummaryCurveName( oldString );
    std::string correspondingNewString = RiaSummaryAddressAnalyzer::correspondingHistorySummaryCurveName( newString );

    if ( adr->vectorName() == correspondingOldString )
    {
        adr->setVectorName( correspondingNewString );

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataSourceSteppingTools::updateQuantityIfMatching( const QVariant& oldValue, const QVariant& newValue, RifEclipseSummaryAddress* adr )
{
    if ( !adr ) return false;

    std::string oldString = oldValue.toString().toStdString();
    std::string newString = newValue.toString().toStdString();

    if ( adr->vectorName() == oldString )
    {
        adr->setVectorName( newString );

        return true;
    }

    return false;
}
