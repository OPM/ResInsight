/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigWellLogExtractor.h"

#include "RiaLogging.h"
#include "RigWellPath.h"

#include "cvfTrace.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogExtractor::RigWellLogExtractor( gsl::not_null<const RigWellPath*> wellpath,
                                          const std::string&                wellCaseErrorMsgName )
    : m_wellPathGeometry( wellpath )
    , m_wellCaseErrorMsgName( wellCaseErrorMsgName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogExtractor::~RigWellLogExtractor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogExtractor::cellIntersectionMDs() const
{
    return m_intersectionMeasuredDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogExtractor::cellIntersectionTVDs() const
{
    return m_intersectionTVDs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RigWellLogExtractor::cellIntersectionInfosAlongWellPath() const
{
    std::vector<WellPathCellIntersectionInfo> infoVector;
    if ( m_intersectedCellsGlobIdx.empty() ) return infoVector;

    for ( size_t i = 0; i < m_intersectedCellsGlobIdx.size() - 1; i = i + 2 )
    {
        CVF_ASSERT( m_intersectedCellsGlobIdx[i] == m_intersectedCellsGlobIdx[i + 1] );

        WellPathCellIntersectionInfo cellInfo;

        cellInfo.globCellIndex = m_intersectedCellsGlobIdx[i];
        cellInfo.startPoint    = m_intersections[i];
        cellInfo.endPoint      = m_intersections[i + 1];
        cellInfo.startMD       = m_intersectionMeasuredDepths[i];
        cellInfo.endMD         = m_intersectionMeasuredDepths[i + 1];

        cellInfo.intersectedCellFaceIn  = m_intersectedCellFaces[i];
        cellInfo.intersectedCellFaceOut = m_intersectedCellFaces[i + 1];

        cellInfo.intersectionLengthsInCellCS =
            this->calculateLengthInCell( cellInfo.globCellIndex, cellInfo.startPoint, cellInfo.endPoint );

        infoVector.push_back( cellInfo );
    }

    return infoVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigWellLogExtractor::intersectedCellsGlobIdx() const
{
    return m_intersectedCellsGlobIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* RigWellLogExtractor::wellPathGeometry() const
{
    return m_wellPathGeometry.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogExtractor::insertIntersectionsInMap( const std::vector<HexIntersectionInfo>& intersections,
                                                    cvf::Vec3d                              p1,
                                                    double                                  md1,
                                                    cvf::Vec3d                              p2,
                                                    double                                  md2,
                                                    double                                  tolerance,
                                                    std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo>* uniqueIntersections )
{
    for ( const auto& intersection : intersections )
    {
        double lengthAlongLineSegment1 = ( intersection.m_intersectionPoint - p1 ).length();
        double lengthAlongLineSegment2 = ( p2 - intersection.m_intersectionPoint ).length();
        double measuredDepthDiff       = md2 - md1;
        double lineLength              = lengthAlongLineSegment1 + lengthAlongLineSegment2;
        double measuredDepthOfPoint    = 0.0;

        if ( lineLength > 0.00001 )
        {
            measuredDepthOfPoint = md1 + measuredDepthDiff * lengthAlongLineSegment1 / ( lineLength );
        }
        else
        {
            measuredDepthOfPoint = md1;
        }

        uniqueIntersections->insert( std::make_pair( RigMDCellIdxEnterLeaveKey( measuredDepthOfPoint,
                                                                                intersection.m_hexIndex,
                                                                                intersection.m_isIntersectionEntering,
                                                                                tolerance ),
                                                     intersection ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogExtractor::populateReturnArrays( std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo>& uniqueIntersections )
{
    QStringList errorMessages;

    // For same MD and same cell, remove enter/leave pairs, as they only touches the wellpath, and should not contribute.
    {
        auto it1 = uniqueIntersections.begin();
        auto it2 = uniqueIntersections.begin();

        std::vector<std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo>::iterator> iteratorsToIntersectonsToErase;

        while ( it2 != uniqueIntersections.end() )
        {
            ++it2;
            if ( it2 != uniqueIntersections.end() )
            {
                if ( RigWellLogExtractionTools::isEqualDepth( it1->first.measuredDepth,
                                                              it2->first.measuredDepth,
                                                              it1->first.tolerance ) )
                {
                    if ( it1->first.hexIndex == it2->first.hexIndex )
                    {
                        // Remove the two from the map, as they just are a touch of the cell surface
                        CVF_TIGHT_ASSERT( !it1->first.isEnteringCell && it2->first.isEnteringCell );

                        iteratorsToIntersectonsToErase.push_back( it1 );
                        iteratorsToIntersectonsToErase.push_back( it2 );
                    }
                }
            }
            ++it1;
        }

        // Erase all the intersections that is not needed
        for ( auto erItIdx : iteratorsToIntersectonsToErase )
        {
            uniqueIntersections.erase( erItIdx );
        }
    }

    // Make sure the enter/leave flag is set correct. For thin cells with very twisted geometry, the intersection point
    // of the bottom face can be intersected before the top cell causing inverted isEnteringCell flag
    // NB! The operation must be performed here, as it is required to have all intersections from all well path segments
    // in place.
    // https://github.com/OPM/ResInsight/issues/9622

    {
        auto it1 = uniqueIntersections.begin();
        auto it2 = uniqueIntersections.begin();

        std::vector<RigMDCellIdxEnterLeaveKey> toBeSwitched;

        while ( it2 != uniqueIntersections.end() )
        {
            ++it2;

            // The intersections are ordered by increasing measured depth. Identify a pair of enter/leave intersections
            // with inverted flag setting for isEnteringCell
            if ( it2 != uniqueIntersections.end() && ( it1->first.hexIndex == it2->first.hexIndex ) &&
                 ( it1->first.isLeavingCell() && it2->first.isEnteringCell ) )
            {
                toBeSwitched.push_back( it1->first );
                toBeSwitched.push_back( it2->first );
            }
            ++it1;
        }

        // Change the object used as key in a map based on
        // https://stackoverflow.com/questions/5743545/what-is-the-fastest-way-to-change-a-key-of-an-element-inside-stdmap

        for ( auto& obj : toBeSwitched )
        {
            auto objToBeSwitched      = uniqueIntersections.extract( obj );
            auto updatedObj           = obj;
            updatedObj.isEnteringCell = !updatedObj.isEnteringCell;
            objToBeSwitched.key()     = updatedObj;
            uniqueIntersections.insert( std::move( objToBeSwitched ) );
        }
    }

    // Copy the map into a different sorting regime, with enter leave more significant than cell index

    std::map<RigMDEnterLeaveCellIdxKey, HexIntersectionInfo> sortedUniqueIntersections;
    {
        auto it = uniqueIntersections.begin();
        while ( it != uniqueIntersections.end() )
        {
            sortedUniqueIntersections.insert( std::make_pair( RigMDEnterLeaveCellIdxKey( it->first.measuredDepth,
                                                                                         it->first.isEnteringCell,
                                                                                         it->first.hexIndex,
                                                                                         it->first.tolerance ),
                                                              it->second ) );
            ++it;
        }
    }

    // Add points for the endpoint of the wellpath, if it starts/ends inside a cell
    {
        // Add an intersection for the well startpoint that is inside the first cell
        auto it = sortedUniqueIntersections.begin();
        if ( it != sortedUniqueIntersections.end() && !it->first.isEnteringCell ) // Leaving a cell as first
                                                                                  // intersection. Well starts inside a
                                                                                  // cell.
        {
            // Needs wellpath start point in front
            HexIntersectionInfo firstLeavingPoint      = it->second;
            firstLeavingPoint.m_intersectionPoint      = m_wellPathGeometry->wellPathPoints()[0];
            firstLeavingPoint.m_face                   = cvf::StructGridInterface::NO_FACE;
            firstLeavingPoint.m_isIntersectionEntering = true;

            sortedUniqueIntersections.insert(
                std::make_pair( RigMDEnterLeaveCellIdxKey( m_wellPathGeometry->measuredDepths()[0],
                                                           true,
                                                           firstLeavingPoint.m_hexIndex,
                                                           it->first.tolerance ),
                                firstLeavingPoint ) );
        }

        // Add an intersection for the well endpoint possibly inside the last cell.
        auto rit = sortedUniqueIntersections.rbegin();
        if ( rit != sortedUniqueIntersections.rend() && rit->first.isEnteringCell ) // Entering a cell as last
                                                                                    // intersection. Well ends inside a
                                                                                    // cell.
        {
            // Needs wellpath end point at end
            HexIntersectionInfo lastEnterPoint      = rit->second;
            lastEnterPoint.m_intersectionPoint      = m_wellPathGeometry->wellPathPoints().back();
            lastEnterPoint.m_isIntersectionEntering = false;
            lastEnterPoint.m_face                   = cvf::StructGridInterface::NO_FACE;

            sortedUniqueIntersections.insert(
                std::make_pair( RigMDEnterLeaveCellIdxKey( m_wellPathGeometry->measuredDepths().back(),
                                                           false,
                                                           lastEnterPoint.m_hexIndex,
                                                           rit->first.tolerance ),
                                lastEnterPoint ) );
        }
    }

    // Filter and store the intersections pairwise as cell enter-leave pairs
    // Discard points that does not have a match .
    {
        auto                                                               it1 = sortedUniqueIntersections.begin();
        std::map<RigMDEnterLeaveCellIdxKey, HexIntersectionInfo>::iterator it2;

        while ( it1 != sortedUniqueIntersections.end() )
        {
            it2 = it1;
            ++it2;

            if ( it2 == sortedUniqueIntersections.end() ) break;

            if ( RigMDEnterLeaveCellIdxKey::isProperCellEnterLeavePair( it1->first, it2->first ) )
            {
                appendIntersectionToArrays( it1->first.measuredDepth, it1->second, &errorMessages );
                ++it1;
                appendIntersectionToArrays( it1->first.measuredDepth, it1->second, &errorMessages );
                ++it1;
            }
            else
            {
                // If we haven't a proper pair, try our best to recover these variants:
                // 1-2 3 4 5 6 7 8 9 10 11-12
                //     +---+
                //       +---+
                //             +---+

                auto it11 = it1;
                auto it21 = it2;

                // Check if we have overlapping cells (typically at a fault)
                ++it21;
                if ( it21 != sortedUniqueIntersections.end() &&
                     RigMDEnterLeaveCellIdxKey::isProperCellEnterLeavePair( it11->first, it21->first ) )
                {
                    // Found 3 to 5 connection
                    appendIntersectionToArrays( it11->first.measuredDepth, it11->second, &errorMessages );
                    appendIntersectionToArrays( it21->first.measuredDepth, it21->second, &errorMessages );

                    ++it11;
                    ++it21;
                    if ( it21 != sortedUniqueIntersections.end() &&
                         RigMDEnterLeaveCellIdxKey::isProperCellEnterLeavePair( it11->first, it21->first ) )
                    {
                        // Found a 4 to 6 connection
                        appendIntersectionToArrays( it11->first.measuredDepth, it11->second, &errorMessages );
                        appendIntersectionToArrays( it21->first.measuredDepth, it21->second, &errorMessages );

                        it1 = it21;
                        ++it1;
                        continue;
                    }

                    errorMessages += QString( "Well Log Extraction : " ) +
                                     QString::fromStdString( m_wellCaseErrorMsgName ) + ( " Discards a point at MD:  " ) +
                                     QString::number( (double)( it1->first.measuredDepth ) );

                    // Found that 8 to 10 is not connected, after finding 7 to 9
                    it1 = it21; // Discard 8 by Jumping to 10
                    continue;
                }
                else
                {
                    errorMessages += QString( "Well Log Extraction : " ) +
                                     QString::fromStdString( m_wellCaseErrorMsgName ) + ( " Discards a point at MD:  " ) +
                                     QString::number( (double)( it1->first.measuredDepth ) );

                    // Found that 10 to 11 is not connected, and not 10 to 12 either
                    ++it1; // Discard 10 and jump to 11 and hope that recovers us
                    continue;
                }
            }
        }
    }

    bool reportErrorMessages = false;
    if ( reportErrorMessages )
    {
        errorMessages.removeDuplicates();
        for ( const auto& message : errorMessages )
        {
            RiaLogging::warning( message );
        }
    }
}

void RigWellLogExtractor::appendIntersectionToArrays( double                      measuredDepth,
                                                      const HexIntersectionInfo&  intersection,
                                                      gsl::not_null<QStringList*> errorMessages )
{
    QString errorMessage; // This error message is currently not displayed anywhere, as the output caused noise in
                          // several situations https://github.com/OPM/ResInsight/issues/7126

    if ( !m_intersectionMeasuredDepths.empty() && measuredDepth < m_intersectionMeasuredDepths.back() )
    {
        const double warningLimit = 0.01;
        double       diff         = std::fabs( measuredDepth - m_intersectionMeasuredDepths.back() );
        if ( diff > warningLimit )
        {
            errorMessage +=
                QString( "Well Log Extraction : %1 does not have a monotonically increasing measured depth." )
                    .arg( QString::fromStdString( m_wellCaseErrorMsgName ) );
        }

        // Allow alterations of up to 0.1 percent as long as we keep the measured depth monotonically increasing.
        const double tolerance = std::max( 1.0, measuredDepth ) * 1.0e-3;
        if ( RigWellLogExtractionTools::isEqualDepth( measuredDepth, m_intersectionMeasuredDepths.back(), tolerance ) )
        {
            if ( diff > warningLimit )
            {
                errorMessage += "The well path has been slightly adjusted";
            }
            measuredDepth = m_intersectionMeasuredDepths.back();
        }
    }

    m_intersectionMeasuredDepths.push_back( measuredDepth );
    m_intersectionTVDs.push_back( fabs( intersection.m_intersectionPoint[2] ) );
    m_intersections.push_back( intersection.m_intersectionPoint );
    m_intersectedCellsGlobIdx.push_back( intersection.m_hexIndex );
    m_intersectedCellFaces.push_back( intersection.m_face );
}
