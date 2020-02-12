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

#include "RiuFemResultTextBuilder.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"

#include "Rim2dIntersectionView.h"
#include "RimFormationNames.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

#include "RiuGeoMechXfTensorResultAccessor.h"

#include "RivExtrudedCurveIntersectionPartMgr.h"

#include "cafDisplayCoordTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuFemResultTextBuilder::RiuFemResultTextBuilder( RimGridView*                displayCoordView,
                                                  RimGeoMechResultDefinition* geomResDef,
                                                  int                         gridIndex,
                                                  int                         cellIndex,
                                                  int                         timeStepIndex )
    : m_isIntersectionTriangleSet( false )
{
    m_displayCoordView = displayCoordView;
    m_geomResDef       = geomResDef;
    m_gridIndex        = gridIndex;
    m_cellIndex        = cellIndex;
    m_timeStepIndex    = timeStepIndex;

    m_intersectionPointInDisplay = cvf::Vec3d::UNDEFINED;
    m_face                       = cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::setIntersectionPointInDisplay( cvf::Vec3d intersectionPointInDisplay )
{
    m_intersectionPointInDisplay = intersectionPointInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::setIntersectionTriangle( const std::array<cvf::Vec3f, 3>& triangle )
{
    m_intersectionTriangle      = triangle;
    m_isIntersectionTriangleSet = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::set2dIntersectionView( Rim2dIntersectionView* intersectionView )
{
    m_2dIntersectionView = intersectionView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::setFace( int face )
{
    m_face = face;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::mainResultText()
{
    QString text;

    text = closestNodeResultText( m_geomResDef );

    if ( !text.isEmpty() ) text += "\n";

    QString topoText = this->geometrySelectionText( "\n" );
    text += topoText;
    appendDetails( text, formationDetails() );
    text += "\n";

    if ( m_geomResDef->resultPositionType() != RIG_ELEMENT )
    {
        appendDetails( text, gridResultDetails() );
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::geometrySelectionText( QString itemSeparator )
{
    QString text;

    if ( m_geomResDef->geoMechCase() )
    {
        RigGeoMechCaseData* geomData = m_geomResDef->geoMechCase()->geoMechData();
        if ( geomData )
        {
            RigFemPart* femPart     = geomData->femParts()->part( m_gridIndex );
            int         elementId   = femPart->elmId( m_cellIndex );
            auto        elementType = femPart->elementType( m_cellIndex );

            text +=
                QString( "Element : Id[%1], Type[%2]" ).arg( elementId ).arg( RigFemTypes::elementTypeText( elementType ) );

            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if ( geomData->femParts()->part( m_gridIndex )->getOrCreateStructGrid()->ijkFromCellIndex( m_cellIndex, &i, &j, &k ) )
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;
                k++;

                text += QString( ", ijk[%1, %2, %3]" ).arg( i ).arg( j ).arg( k ) + itemSeparator;

                QString formattedText;
                if ( m_2dIntersectionView )
                {
                    formattedText.sprintf( "Horizontal length from well start: %.2f", m_intersectionPointInDisplay.x() );
                    text += formattedText + itemSeparator;

                    cvf::Mat4d t = m_2dIntersectionView->flatIntersectionPartMgr()->unflattenTransformMatrix(
                        m_intersectionPointInDisplay );
                    if ( !t.isZero() )
                    {
                        cvf::Vec3d intPt = m_intersectionPointInDisplay.getTransformedPoint( t );
                        formattedText.sprintf( "Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]",
                                               intPt.x(),
                                               intPt.y(),
                                               -intPt.z() );
                        text += formattedText;
                    }
                }
                else
                {
                    if ( m_displayCoordView )
                    {
                        cvf::ref<caf::DisplayCoordTransform> transForm = m_displayCoordView->displayCoordTransform();
                        cvf::Vec3d domainCoord = transForm->translateToDomainCoord( m_intersectionPointInDisplay );

                        formattedText.sprintf( "Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]",
                                               domainCoord.x(),
                                               domainCoord.y(),
                                               -domainCoord.z() );
                        text += formattedText;
                    }
                }
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::gridResultDetails()
{
    QString text;

    if ( m_geomResDef->geoMechCase() && m_geomResDef->geoMechCase()->geoMechData() )
    {
        RigGeoMechCaseData* eclipseCaseData = m_geomResDef->geoMechCase()->geoMechData();

        this->appendTextFromResultColors( eclipseCaseData, m_gridIndex, m_cellIndex, m_timeStepIndex, m_geomResDef, &text );

        if ( !text.isEmpty() )
        {
            text.prepend( "-- Element result details --\n" );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::formationDetails()
{
    QString  text;
    RimCase* rimCase = m_geomResDef->geoMechCase();
    if ( rimCase )
    {
        if ( rimCase->activeFormationNames() && rimCase->activeFormationNames()->formationNamesData() )
        {
            RigFormationNames* formNames = rimCase->activeFormationNames()->formationNamesData();

            size_t k = cvf::UNDEFINED_SIZE_T;
            {
                if ( m_geomResDef->geoMechCase() )
                {
                    RigGeoMechCaseData* geomData = m_geomResDef->geoMechCase()->geoMechData();
                    if ( geomData )
                    {
                        size_t i = 0;
                        size_t j = 0;
                        geomData->femParts()
                            ->part( m_gridIndex )
                            ->getOrCreateStructGrid()
                            ->ijkFromCellIndex( m_cellIndex, &i, &j, &k );
                    }
                }
            }

            if ( k != cvf::UNDEFINED_SIZE_T )
            {
                QString formName = formNames->formationNameFromKLayerIdx( k );
                if ( !formName.isEmpty() )
                {
                    // text += "-- Formation details --\n";

                    text += QString( "Formation Name: %1\n" ).arg( formName );
                }
            }
        }
    }
    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::appendTextFromResultColors( RigGeoMechCaseData*         geomData,
                                                          int                         gridIndex,
                                                          int                         cellIndex,
                                                          int                         timeStepIndex,
                                                          RimGeoMechResultDefinition* resultDefinition,
                                                          QString*                    resultInfoText )
{
    if ( !resultDefinition )
    {
        return;
    }

    if ( resultDefinition->hasResult() )
    {
        const std::vector<float>& scalarResults =
            geomData->femPartResults()->resultValues( resultDefinition->resultAddress(), gridIndex, timeStepIndex );
        if ( scalarResults.size() )
        {
            caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = resultDefinition->resultPositionType();
            resultInfoText->append( resPosAppEnum.uiText() + ", " );
            resultInfoText->append( resultDefinition->resultFieldUiName() + ", " );
            resultInfoText->append( resultDefinition->resultComponentUiName() + ":\n" );

            if ( resultDefinition->resultPositionType() != RIG_ELEMENT_NODAL_FACE )
            {
                RigFemPart*    femPart         = geomData->femParts()->part( gridIndex );
                RigElementType elmType         = femPart->elementType( cellIndex );
                const int*     elmentConn      = femPart->connectivities( cellIndex );
                int            elmNodeCount    = RigFemTypes::elmentNodeCount( elmType );
                const int*     lElmNodeToIpMap = RigFemTypes::localElmNodeToIntegrationPointMapping( elmType );

                for ( int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx )
                {
                    float scalarValue = std::numeric_limits<float>::infinity();
                    int   nodeIdx     = elmentConn[lNodeIdx];
                    if ( resultDefinition->resultPositionType() == RIG_NODAL ||
                         ( resultDefinition->resultPositionType() == RIG_DIFFERENTIALS &&
                           resultDefinition->resultFieldName() == "POR-Bar" ) )
                    {
                        scalarValue = scalarResults[nodeIdx];
                    }
                    else if ( resultDefinition->resultPositionType() == RIG_ELEMENT )
                    {
                        scalarValue = scalarResults[cellIndex];
                    }
                    else
                    {
                        size_t resIdx = femPart->elementNodeResultIdx( cellIndex, lNodeIdx );
                        scalarValue   = scalarResults[resIdx];
                    }

                    if ( resultDefinition->resultPositionType() == RIG_INTEGRATION_POINT )
                    {
                        resultInfoText->append( QString( "\tIP:%1 \t: %2 \tAss. Node: \t%3" )
                                                    .arg( lElmNodeToIpMap[lNodeIdx] + 1 )
                                                    .arg( scalarValue )
                                                    .arg( femPart->nodes().nodeIds[nodeIdx] ) );
                    }
                    else
                    {
                        resultInfoText->append(
                            QString( "\tN:%1 \t: %2" ).arg( femPart->nodes().nodeIds[nodeIdx] ).arg( scalarValue ) );
                    }

                    cvf::Vec3f nodeCoord = femPart->nodes().coordinates[nodeIdx];
                    resultInfoText->append(
                        QString( "\t( %3, %4, %5)\n" ).arg( nodeCoord[0] ).arg( nodeCoord[1] ).arg( nodeCoord[2] ) );
                }
            }
            else
            {
                int elmNodeFaceStartResIdx = cellIndex * 24;

                resultInfoText->append( QString( "Pos I Face:\n" ) );
                for ( int ptIdx = 0; ptIdx < 4; ++ptIdx )
                {
                    resultInfoText->append( QString( "\t%2\n" ).arg( scalarResults[elmNodeFaceStartResIdx + ptIdx] ) );
                }

                resultInfoText->append( QString( "Neg I Face:\n" ) );
                for ( int ptIdx = 4; ptIdx < 8; ++ptIdx )
                {
                    resultInfoText->append( QString( "\t%2\n" ).arg( scalarResults[elmNodeFaceStartResIdx + ptIdx] ) );
                }

                resultInfoText->append( QString( "Pos J Face:\n" ) );
                for ( int ptIdx = 8; ptIdx < 12; ++ptIdx )
                {
                    resultInfoText->append( QString( "\t%2\n" ).arg( scalarResults[elmNodeFaceStartResIdx + ptIdx] ) );
                }

                resultInfoText->append( QString( "Neg J Face:\n" ) );
                for ( int ptIdx = 12; ptIdx < 16; ++ptIdx )
                {
                    resultInfoText->append( QString( "\t%2\n" ).arg( scalarResults[elmNodeFaceStartResIdx + ptIdx] ) );
                }

                resultInfoText->append( QString( "Pos K Face:\n" ) );
                for ( int ptIdx = 16; ptIdx < 20; ++ptIdx )
                {
                    resultInfoText->append( QString( "\t%2\n" ).arg( scalarResults[elmNodeFaceStartResIdx + ptIdx] ) );
                }

                resultInfoText->append( QString( "Neg K Face:\n" ) );
                for ( int ptIdx = 20; ptIdx < 24; ++ptIdx )
                {
                    resultInfoText->append( QString( "\t%2\n" ).arg( scalarResults[elmNodeFaceStartResIdx + ptIdx] ) );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::appendDetails( QString& text, const QString& details )
{
    if ( !details.isEmpty() )
    {
        text += "\n";
        text += details;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::closestNodeResultText( RimGeoMechResultDefinition* resultColors )
{
    QString text;
    if ( !resultColors )
    {
        return text;
    }

    if ( resultColors->hasResult() )
    {
        if ( !( m_geomResDef->geoMechCase() && m_geomResDef->geoMechCase()->geoMechData() ) ) return text;

        RigGeoMechCaseData* geomData = m_geomResDef->geoMechCase()->geoMechData();

        const std::vector<float>& scalarResults =
            geomData->femPartResults()->resultValues( resultColors->resultAddress(), m_gridIndex, m_timeStepIndex );

        if ( scalarResults.size() && m_displayCoordView )
        {
            RigFemPart*         femPart              = geomData->femParts()->part( m_gridIndex );
            RigFemResultPosEnum activeResultPosition = resultColors->resultPositionType();

            cvf::Vec3d intersectionPointInDomain =
                m_displayCoordView->displayCoordTransform()->translateToDomainCoord( m_intersectionPointInDisplay );

            RigFemClosestResultIndexCalculator closestIndexCalc( femPart,
                                                                 activeResultPosition,
                                                                 m_cellIndex,
                                                                 m_face,
                                                                 intersectionPointInDomain );

            int resultIndex         = closestIndexCalc.resultIndexToClosestResult();
            int closestNodeId       = closestIndexCalc.closestNodeId();
            int closestElmNodResIdx = closestIndexCalc.closestElementNodeResIdx();

            float scalarValue = ( resultIndex >= 0 ) ? scalarResults[resultIndex] : std::numeric_limits<float>::infinity();

            if ( activeResultPosition == RIG_ELEMENT )
            {
                text.append( QString( "Element result: %1\n" ).arg( scalarValue ) );
            }
            else if ( activeResultPosition != RIG_ELEMENT_NODAL_FACE )
            {
                text.append( QString( "Closest result: N[%1], %2\n" ).arg( closestNodeId ).arg( scalarValue ) );
            }
            else if ( m_face != -1 )
            {
                text.append( QString( "Closest result: N[%1], on face: %2, %3\n" )
                                 .arg( closestNodeId )
                                 .arg( caf::AppEnum<cvf::StructGridInterface::FaceType>::textFromIndex( m_face ) )
                                 .arg( scalarValue ) );
            }
            else if ( m_isIntersectionTriangleSet && activeResultPosition == RIG_ELEMENT_NODAL_FACE )
            {
                RiuGeoMechXfTensorResultAccessor tensAccessor( geomData->femPartResults(),
                                                               resultColors->resultAddress(),
                                                               m_timeStepIndex );
                float tensValue = tensAccessor.calculateElmNodeValue( m_intersectionTriangle, closestElmNodResIdx );

                text.append( QString( "Closest result: N[%1], in Element[%2] transformed onto intersection: %3 \n" )
                                 .arg( closestNodeId )
                                 .arg( femPart->elmId( m_cellIndex ) )
                                 .arg( tensValue ) );
            }
        }
    }

    return text;
}
