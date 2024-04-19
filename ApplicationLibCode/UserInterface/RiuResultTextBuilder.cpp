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

#include "RiuResultTextBuilder.h"

#include "RigAllanDiagramData.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigWellResultFrame.h"
#include "RigWellResultPoint.h"

#include "Rim2dIntersectionView.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimFormationNames.h"
#include "RimIntersectionResultDefinition.h"
#include "RimMultipleEclipseResults.h"
#include "RimRegularLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimViewLinker.h"

#include "RivExtrudedCurveIntersectionPartMgr.h"

#include "cafDisplayCoordTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuResultTextBuilder::RiuResultTextBuilder( RimGridView*                settingsView,
                                            RimEclipseResultDefinition* eclResDef,
                                            size_t                      gridIndex,
                                            size_t                      cellIndex,
                                            size_t                      timeStepIndex )
{
    CVF_ASSERT( eclResDef );

    m_displayCoordView = settingsView;
    m_eclipseView      = dynamic_cast<RimEclipseView*>( settingsView );
    m_eclResDef        = eclResDef;
    m_gridIndex        = gridIndex;
    m_cellIndex        = cellIndex;
    m_timeStepIndex    = timeStepIndex;

    m_nncIndex                   = cvf::UNDEFINED_SIZE_T;
    m_intersectionPointInDisplay = cvf::Vec3d::UNDEFINED;
    m_face                       = cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuResultTextBuilder::RiuResultTextBuilder( RimGridView*                settingsView,
                                            RimEclipseResultDefinition* eclResDef,
                                            size_t                      reservoirCellIndex,
                                            size_t                      timeStepIndex )
{
    CVF_ASSERT( eclResDef );

    m_displayCoordView = settingsView;
    m_eclipseView      = dynamic_cast<RimEclipseView*>( settingsView );
    m_eclResDef        = eclResDef;
    m_gridIndex        = 0;
    m_cellIndex        = 0;
    m_timeStepIndex    = timeStepIndex;

    RimEclipseCase* eclipseCase = eclResDef->eclipseCase();
    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();
        RigMainGrid*        mainGrid = caseData->mainGrid();

        const RigCell& cell = caseData->mainGrid()->globalCellArray()[reservoirCellIndex];

        for ( size_t i = 0; i < mainGrid->gridCount(); i++ )
        {
            if ( mainGrid->gridByIndex( i ) == cell.hostGrid() )
            {
                m_gridIndex = i;
                m_cellIndex = cell.gridLocalCellIndex();
            }
        }
    }

    m_nncIndex                   = cvf::UNDEFINED_SIZE_T;
    m_intersectionPointInDisplay = cvf::Vec3d::UNDEFINED;
    m_face                       = cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::setNncIndex( size_t nncIndex )
{
    m_nncIndex = nncIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::setIntersectionPointInDisplay( cvf::Vec3d intersectionPointInDisplay )
{
    m_intersectionPointInDisplay = intersectionPointInDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::set2dIntersectionView( Rim2dIntersectionView* intersectionView )
{
    m_2dIntersectionView = intersectionView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuResultTextBuilder::setFace( cvf::StructGridInterface::FaceType face )
{
    m_face = face;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::mainResultText()
{
    QString text;

    // Produce result text for all variants
    // Priority defined as follows :  NNC, Fault, Grid
    {
        QString nncText = nncResultText();
        if ( !nncText.isEmpty() )
        {
            text = "NNC : " + nncText;
        }
        else if ( m_cellIndex != cvf::UNDEFINED_SIZE_T )
        {
            QString faultText = faultResultText();

            if ( !faultResultText().isEmpty() )
            {
                text = "Fault : " + faultText;
            }
            else
            {
                text = "Grid cell : " + gridResultText();
            }
        }

        text += "\n";
    }

    QString topoText = geometrySelectionText( "\n" );
    text += topoText;
    appendDetails( text, formationDetails() );
    text += "\n";

    appendDetails( text, nncDetails() );

    if ( m_cellIndex != cvf::UNDEFINED_SIZE_T )
    {
        appendDetails( text, faultResultDetails() );
        appendDetails( text, cellEdgeResultDetails() );
        appendDetails( text, gridResultDetails() );
        appendDetails( text, wellResultText() );
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::geometrySelectionText( const QString& itemSeparator )
{
    QString text;

    if ( m_eclResDef && m_eclResDef->eclipseCase() )
    {
        const RigEclipseCaseData* eclipseCase = m_eclResDef->eclipseCase()->eclipseCaseData();
        if ( eclipseCase )
        {
            if ( m_cellIndex != cvf::UNDEFINED_SIZE_T )
            {
                size_t i = 0;
                size_t j = 0;
                size_t k = 0;

                const RigGridBase* grid = eclipseCase->grid( m_gridIndex );
                if ( grid->ijkFromCellIndex( m_cellIndex, &i, &j, &k ) )
                {
                    // Adjust to 1-based Eclipse indexing
                    i++;
                    j++;
                    k++;

                    if ( m_face != cvf::StructGridInterface::NO_FACE )
                    {
                        cvf::StructGridInterface::FaceEnum faceEnum( m_face );

                        QString faceText = faceEnum.text();

                        text += QString( "Face : %1" ).arg( faceText ) + itemSeparator;
                    }

                    QString gridName = QString::fromStdString( grid->gridName() );
                    text += QString( "Grid : %1 [%2]" ).arg( gridName ).arg( m_gridIndex ) + itemSeparator;

                    text += QString( "Cell : [%1, %2, %3]" ).arg( i ).arg( j ).arg( k ) + itemSeparator;

                    size_t globalCellIndex = grid->reservoirCellIndex( m_cellIndex );
                    text += QString( "Global Cell Index : %4" ).arg( globalCellIndex ) + itemSeparator;

                    text += coordinatesText( grid, globalCellIndex, itemSeparator );
                }
            }

            if ( m_intersectionPointInDisplay != cvf::Vec3d::UNDEFINED )
            {
                QString formattedText;
                if ( m_2dIntersectionView )
                {
                    formattedText = QString( "Horizontal length from well start: %1" ).arg( m_intersectionPointInDisplay.x(), 5, 'f', 2 );
                    text += formattedText + itemSeparator;

                    cvf::Mat4d t = m_2dIntersectionView->flatIntersectionPartMgr()->unflattenTransformMatrix( m_intersectionPointInDisplay );
                    if ( !t.isZero() )
                    {
                        cvf::Vec3d intPt = m_intersectionPointInDisplay.getTransformedPoint( t );
                        formattedText    = createTextFromDomainCoordinate( "Intersection point : [E: %1, N: %2, Depth: %3]", intPt );
                        text += formattedText;
                    }
                }
                else
                {
                    if ( m_displayCoordView )
                    {
                        cvf::ref<caf::DisplayCoordTransform> transForm = m_displayCoordView->displayCoordTransform();
                        cvf::Vec3d domainCoord                         = transForm->translateToDomainCoord( m_intersectionPointInDisplay );

                        formattedText = createTextFromDomainCoordinate( "Intersection point : [E: %1, N: %2, Depth: %3]", domainCoord );
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
QString RiuResultTextBuilder::coordinatesText( const RigGridBase* grid, size_t globalCellIndex, const QString& itemSeparator )
{
    QString text;

    bool showCenter = false;
    bool showCorner = false;

    if ( m_eclipseView )
    {
        std::vector<RimMultipleEclipseResults*> additionalResultSettings = m_eclipseView->descendantsOfType<RimMultipleEclipseResults>();
        if ( !additionalResultSettings.empty() )
        {
            showCenter = additionalResultSettings[0]->showCenterCoordinates();
            showCorner = additionalResultSettings[0]->showCornerCoordinates();
        }
    }

    if ( showCenter )
    {
        auto       mainGrid = grid->mainGrid();
        auto       cell     = mainGrid->cell( globalCellIndex );
        const auto center   = cell.center();

        text += createTextFromDomainCoordinate( "Cell Center : [%1, %2, %3]", center ) + itemSeparator;
    }

    if ( showCorner )
    {
        auto mainGrid = grid->mainGrid();
        auto cell     = mainGrid->cell( globalCellIndex );
        auto indices  = cell.cornerIndices();

        text += QString( "Cell Corners" ) + itemSeparator;

        const std::vector<std::pair<int, std::string>> riNodeOrder{ { 0, "i- j- k+" },
                                                                    { 1, "i+ j- k+" },
                                                                    { 2, "i+ j+ k+" },
                                                                    { 3, "i- j+ k+" },
                                                                    { 4, "i- j- k-" },
                                                                    { 5, "i+ j- k-" },
                                                                    { 6, "i+ j+ k-" },
                                                                    { 7, "i- j+ k-" } };

        for ( int i = 0; i < 8; i++ )
        {
            const auto& [nodeIndex, nodeText] = riNodeOrder[i];
            auto v                            = mainGrid->nodes()[indices[nodeIndex]];

            text += createTextFromDomainCoordinate( QString::fromStdString( nodeText ) + " : [%1, %2, %3]" + itemSeparator, v );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::createTextFromDomainCoordinate( const QString& formatString, const cvf::Vec3d& domainCoord )
{
    return formatString.arg( domainCoord.x(), 5, 'f', 2 ).arg( domainCoord.y(), 5, 'f', 2 ).arg( -domainCoord.z(), 5, 'f', 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::gridResultDetails()
{
    std::vector<RimEclipseResultDefinition*> resultDefinitions;

    std::vector<std::unique_ptr<RimEclipseResultDefinition>> tmp;

    QStringList additionalCellResultText;
    resultDefinitions.push_back( m_eclResDef );
    if ( m_eclipseView )
    {
        std::vector<RigEclipseResultAddress> resultAddresses = m_eclipseView->additionalResultsForResultInfo();

        for ( const auto& result : resultAddresses )
        {
            auto myResDef = std::make_unique<RimEclipseResultDefinition>();
            myResDef->setEclipseCase( m_eclResDef->eclipseCase() );
            myResDef->simpleCopy( m_eclResDef );
            myResDef->setFromEclipseResultAddress( result );
            myResDef->loadResult();

            resultDefinitions.push_back( myResDef.get() );
            tmp.push_back( std::move( myResDef ) );
        }
    }

    const auto [hasMultipleCases, linkedViewText] = resultTextFromLinkedViews();
    QString text                                  = cellResultText( resultDefinitions, hasMultipleCases );

    for ( const auto& txt : additionalCellResultText )
    {
        text += "\n" + txt;
    }

    for ( const auto& txt : linkedViewText )
    {
        text += "\n" + txt;
    }

    if ( !text.isEmpty() )
    {
        text.prepend( "-- Grid cell result details --\n" );
    }
    text += "\n";

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::faultResultDetails()
{
    QString text;

    if ( m_eclResDef->eclipseCase() && m_eclResDef->eclipseCase()->eclipseCaseData() )
    {
        RigEclipseCaseData* eclipseCaseData = m_eclResDef->eclipseCase()->eclipseCaseData();
        RigGridBase*        grid            = eclipseCaseData->grid( m_gridIndex );
        RigMainGrid*        mainGrid        = grid->mainGrid();

        const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace( m_cellIndex, m_face );
        if ( fault )
        {
            text += "-- Fault result details --\n";

            text += QString( "Fault Name: %1\n" ).arg( fault->name() );

            cvf::StructGridInterface::FaceEnum faceHelper( m_face );
            text += "Fault Face : " + faceHelper.text() + "\n";

            if ( m_eclipseView && m_eclipseView->faultResultSettings()->hasValidCustomResult() )
            {
                if ( m_eclipseView->faultResultSettings()->customFaultResult()->resultType() != RiaDefines::ResultCatType::ALLAN_DIAGRAMS )
                {
                    text += "Fault result data:\n";
                    appendTextFromResultColors( eclipseCaseData,
                                                m_gridIndex,
                                                m_cellIndex,
                                                m_timeStepIndex,
                                                m_eclipseView->currentFaultResultColors(),
                                                &text );
                }
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::formationDetails()
{
    QString  text;
    RimCase* rimCase = m_eclResDef->eclipseCase();
    if ( rimCase )
    {
        if ( rimCase->activeFormationNames() && rimCase->activeFormationNames()->formationNamesData() )
        {
            RigFormationNames* formNames = rimCase->activeFormationNames()->formationNamesData();

            size_t k = cvf::UNDEFINED_SIZE_T;
            {
                const RigEclipseCaseData* eclipseData = m_eclResDef->eclipseCase()->eclipseCaseData();
                if ( eclipseData )
                {
                    if ( m_cellIndex != cvf::UNDEFINED_SIZE_T )
                    {
                        size_t i = cvf::UNDEFINED_SIZE_T;
                        size_t j = cvf::UNDEFINED_SIZE_T;

                        eclipseData->grid( m_gridIndex )->ijkFromCellIndex( m_cellIndex, &i, &j, &k );
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
QString RiuResultTextBuilder::gridResultText()
{
    QString text = cellResultText( { m_eclResDef } );
    text.replace( "\n", " " );

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::faultResultText()
{
    QString text;

    if ( m_eclResDef->eclipseCase() && m_eclResDef->eclipseCase()->eclipseCaseData() )
    {
        RigEclipseCaseData* eclipseCaseData = m_eclResDef->eclipseCase()->eclipseCaseData();

        RigGridBase* grid     = eclipseCaseData->grid( m_gridIndex );
        RigMainGrid* mainGrid = grid->mainGrid();

        const RigFault* fault = mainGrid->findFaultFromCellIndexAndCellFace( m_cellIndex, m_face );

        if ( fault )
        {
            cvf::StructGridInterface::FaceEnum faceHelper( m_face );
            if ( m_eclipseView && m_eclipseView->faultResultSettings()->hasValidCustomResult() )
            {
                text = cellResultText( { m_eclipseView->currentFaultResultColors() } );
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::nncResultText()
{
    QString text;

    if ( m_nncIndex != cvf::UNDEFINED_SIZE_T )
    {
        if ( m_eclResDef.notNull() && m_eclResDef->eclipseCase() )
        {
            RigEclipseCaseData* eclipseCase = m_eclResDef->eclipseCase()->eclipseCaseData();

            RigMainGrid* grid = eclipseCase->mainGrid();
            CVF_ASSERT( grid );

            RigNNCData* nncData = grid->nncData();
            CVF_ASSERT( nncData );

            if ( nncData && m_nncIndex < nncData->allConnections().size() )
            {
                const RigConnection& conn = nncData->allConnections()[m_nncIndex];

                cvf::StructGridInterface::FaceEnum face( conn.face() );

                if ( m_eclipseView && m_eclipseView->currentFaultResultColors() )
                {
                    RigEclipseResultAddress   eclipseResultAddress = m_eclipseView->currentFaultResultColors()->eclipseResultAddress();
                    RiaDefines::ResultCatType resultType           = m_eclipseView->currentFaultResultColors()->resultType();

                    const std::vector<double>* nncValues = nullptr;

                    if ( resultType == RiaDefines::ResultCatType::STATIC_NATIVE )
                    {
                        nncValues = nncData->staticConnectionScalarResult( eclipseResultAddress );
                    }
                    else if ( resultType == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
                    {
                        if ( m_eclResDef.notNull() && m_eclResDef->eclipseCase() )
                        {
                            size_t nativeTimeStep = m_eclResDef->eclipseCase()->uiToNativeTimeStepIndex( m_timeStepIndex );
                            nncValues             = nncData->dynamicConnectionScalarResult( eclipseResultAddress, nativeTimeStep );
                        }
                    }

                    if ( nncValues && ( m_nncIndex < nncValues->size() ) )
                    {
                        QString resultVar   = m_eclipseView->currentFaultResultColors()->resultVariableUiName();
                        double  scalarValue = ( *nncValues )[m_nncIndex];

                        text = QString( "%1 : %2" ).arg( resultVar ).arg( scalarValue );
                    }

                    if ( resultType == RiaDefines::ResultCatType::ALLAN_DIAGRAMS )
                    {
                        nncValues = nncData->staticConnectionScalarResult( eclipseResultAddress );
                        QString resultValueText;

                        if ( m_eclipseView->currentFaultResultColors()->resultVariable() == RiaResultNames::formationAllanResultName() )
                        {
                            std::pair<int, int> fmIndexPair =
                                eclipseCase->allanDiagramData()->formationIndexCombinationFromCategory( ( *nncValues )[m_nncIndex] );

                            std::vector<QString> fmNames = eclipseCase->formationNames();
                            // clang-format off
                            if ( fmIndexPair.first >= 0 && 
                                 fmIndexPair.second >= 0 &&
                                 static_cast<int>(fmNames.size()) > fmIndexPair.first &&
                                 static_cast<int>(fmNames.size()) > fmIndexPair.second )
                            {
                                resultValueText = fmNames[fmIndexPair.first] + " - " +
                                                  fmNames[fmIndexPair.second];
                            }
                            // clang-format on
                        }
                        else if ( m_eclipseView->currentFaultResultColors()->resultVariable() ==
                                  RiaResultNames::formationBinaryAllanResultName() )
                        {
                            resultValueText = ( *nncValues )[m_nncIndex] == 0 ? "Same formation" : "Different formation";
                        }

                        QString resultVar = m_eclipseView->currentFaultResultColors()->resultVariableUiName();
                        text              = QString( "%1 : %2" ).arg( resultVar ).arg( resultValueText );
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
void RiuResultTextBuilder::appendTextFromResultColors( RigEclipseCaseData*         eclipseCase,
                                                       size_t                      gridIndex,
                                                       size_t                      cellIndex,
                                                       size_t                      timeStepIndex,
                                                       RimEclipseResultDefinition* resultColors,
                                                       QString*                    resultInfoText )
{
    if ( !resultColors )
    {
        return;
    }

    RiaDefines::PorosityModelType porosityModel = resultColors->porosityModel();
    if ( resultColors->isTernarySaturationSelected() )
    {
        RigCaseCellResultsData* gridCellResults = resultColors->currentGridCellResults();
        if ( gridCellResults )
        {
            gridCellResults->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) );
            gridCellResults->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) );
            gridCellResults->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) );

            cvf::ref<RigResultAccessor> dataAccessObjectX =
                RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                   gridIndex,
                                                                   porosityModel,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            RiaResultNames::soil() ) );
            cvf::ref<RigResultAccessor> dataAccessObjectY =
                RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                   gridIndex,
                                                                   porosityModel,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            RiaResultNames::sgas() ) );
            cvf::ref<RigResultAccessor> dataAccessObjectZ =
                RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                   gridIndex,
                                                                   porosityModel,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            RiaResultNames::swat() ) );

            double scalarValue = 0.0;

            if ( dataAccessObjectX.notNull() )
                scalarValue = dataAccessObjectX->cellScalar( cellIndex );
            else
                scalarValue = 0.0;
            resultInfoText->append( QString( "SOIL : %1\n" ).arg( scalarValue ) );

            if ( dataAccessObjectY.notNull() )
                scalarValue = dataAccessObjectY->cellScalar( cellIndex );
            else
                scalarValue = 0.0;
            resultInfoText->append( QString( "SGAS : %1\n" ).arg( scalarValue ) );

            if ( dataAccessObjectZ.notNull() )
                scalarValue = dataAccessObjectZ->cellScalar( cellIndex );
            else
                scalarValue = 0.0;
            resultInfoText->append( QString( "SWAT : %1\n" ).arg( scalarValue ) );
        }

        return;
    }
    else if ( resultColors->hasResult() )
    {
        if ( resultColors->hasStaticResult() )
        {
            if ( resultColors->resultVariable().compare( RiaResultNames::combinedTransmissibilityResultName(), Qt::CaseInsensitive ) == 0 )
            {
                cvf::ref<RigResultAccessor> transResultAccessor =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                       gridIndex,
                                                                       porosityModel,
                                                                       0,
                                                                       RigEclipseResultAddress(
                                                                           RiaResultNames::combinedTransmissibilityResultName() ) );
                {
                    double scalarValue = transResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_I );
                    resultInfoText->append( QString( "Tran X : %1\n" ).arg( scalarValue ) );

                    scalarValue = transResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_J );
                    resultInfoText->append( QString( "Tran Y : %1\n" ).arg( scalarValue ) );

                    scalarValue = transResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_K );
                    resultInfoText->append( QString( "Tran Z : %1\n" ).arg( scalarValue ) );
                }

                return;
            }
            else if ( resultColors->resultVariable().compare( RiaResultNames::combinedMultResultName(), Qt::CaseInsensitive ) == 0 )
            {
                cvf::ref<RigResultAccessor> multResultAccessor =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                       gridIndex,
                                                                       porosityModel,
                                                                       0,
                                                                       RigEclipseResultAddress( RiaResultNames::combinedMultResultName() ) );
                {
                    double scalarValue = multResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_I );
                    resultInfoText->append( QString( "MULTX : %1\n" ).arg( scalarValue ) );
                    scalarValue = multResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::NEG_I );
                    resultInfoText->append( QString( "MULTX- : %1\n" ).arg( scalarValue ) );

                    scalarValue = multResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_J );
                    resultInfoText->append( QString( "MULTY : %1\n" ).arg( scalarValue ) );
                    scalarValue = multResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::NEG_J );
                    resultInfoText->append( QString( "MULTY- : %1\n" ).arg( scalarValue ) );

                    scalarValue = multResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_K );
                    resultInfoText->append( QString( "MULTZ : %1\n" ).arg( scalarValue ) );
                    scalarValue = multResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::NEG_K );
                    resultInfoText->append( QString( "MULTZ- : %1\n" ).arg( scalarValue ) );
                }

                return;
            }
            else if ( resultColors->resultVariable().compare( RiaResultNames::combinedRiTranResultName(), Qt::CaseInsensitive ) == 0 )
            {
                cvf::ref<RigResultAccessor> transResultAccessor =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                       gridIndex,
                                                                       porosityModel,
                                                                       0,
                                                                       RigEclipseResultAddress( RiaResultNames::combinedRiTranResultName() ) );
                {
                    double scalarValue = transResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_I );
                    resultInfoText->append( QString( "riTran X : %1\n" ).arg( scalarValue ) );

                    scalarValue = transResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_J );
                    resultInfoText->append( QString( "riTran Y : %1\n" ).arg( scalarValue ) );

                    scalarValue = transResultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_K );
                    resultInfoText->append( QString( "riTran Z : %1\n" ).arg( scalarValue ) );
                }

                return;
            }
            else if ( resultColors->resultVariable().compare( RiaResultNames::combinedRiMultResultName(), Qt::CaseInsensitive ) == 0 )
            {
                cvf::ref<RigResultAccessor> resultAccessor =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                       gridIndex,
                                                                       porosityModel,
                                                                       0,
                                                                       RigEclipseResultAddress( RiaResultNames::combinedRiMultResultName() ) );
                {
                    double scalarValue = resultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_I );
                    resultInfoText->append( QString( "riMult X : %1\n" ).arg( scalarValue ) );

                    scalarValue = resultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_J );
                    resultInfoText->append( QString( "riMult Y : %1\n" ).arg( scalarValue ) );

                    scalarValue = resultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_K );
                    resultInfoText->append( QString( "riMult Z : %1\n" ).arg( scalarValue ) );
                }
                return;
            }
            else if ( resultColors->resultVariable().compare( RiaResultNames::combinedRiAreaNormTranResultName(), Qt::CaseInsensitive ) == 0 )
            {
                cvf::ref<RigResultAccessor> resultAccessor =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCase,
                                                                       gridIndex,
                                                                       porosityModel,
                                                                       0,
                                                                       RigEclipseResultAddress(
                                                                           RiaResultNames::combinedRiAreaNormTranResultName() ) );
                {
                    double scalarValue = resultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_I );
                    resultInfoText->append( QString( "riTransByArea X : %1\n" ).arg( scalarValue ) );

                    scalarValue = resultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_J );
                    resultInfoText->append( QString( "riTransByArea Y : %1\n" ).arg( scalarValue ) );

                    scalarValue = resultAccessor->cellFaceScalar( cellIndex, cvf::StructGridInterface::POS_K );
                    resultInfoText->append( QString( "riTransByArea Z : %1\n" ).arg( scalarValue ) );
                }

                return;
            }
        }
    }

    resultInfoText->append( cellResultText( { resultColors } ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QStringList> RiuResultTextBuilder::resultTextFromLinkedViews() const
{
    if ( !m_eclipseView || !m_eclipseView->assosiatedViewLinker() ) return {};

    QStringList     additionalCellResultText;
    bool            hasMultipleCases   = false;
    RimEclipseCase* primaryEclipseCase = m_eclipseView->eclipseCase();

    auto views = m_eclipseView->assosiatedViewLinker()->allViews();
    for ( auto view : views )
    {
        auto eclView = dynamic_cast<RimEclipseView*>( view );
        if ( !eclView || eclView == m_eclipseView ) continue;

        // Match on IJK size, as the cell index is used to identify the grid cell to extract the result from
        auto otherEclipseCase = eclView->eclipseCase();
        if ( !primaryEclipseCase->isGridSizeEqualTo( otherEclipseCase ) ) continue;

        RiuResultTextBuilder textBuilder( eclView, eclView->cellResult(), m_cellIndex, m_timeStepIndex );
        auto                 text = textBuilder.gridResultText();

        if ( primaryEclipseCase != otherEclipseCase )
        {
            hasMultipleCases = true;

            auto caseName = otherEclipseCase->caseUserDescription();
            text += "( " + caseName + " )";
        }

        additionalCellResultText.push_back( text );
    }

    return { hasMultipleCases, additionalCellResultText };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::cellEdgeResultDetails()
{
    QString text;

    if ( m_eclipseView && m_eclipseView->cellEdgeResult()->showTextResult() )
    {
        m_eclipseView->cellEdgeResult()->loadResult();

        text += "-- Cell edge result data --\n";

        std::vector<RimCellEdgeMetaData> metaData;
        m_eclipseView->cellEdgeResult()->cellEdgeMetaData( &metaData );

        std::set<RigEclipseResultAddress> uniqueResultAddresses;

        for ( int idx = 0; idx < 6; idx++ )
        {
            RigEclipseResultAddress resultAddr = metaData[idx].m_eclipseResultAddress;
            if ( !resultAddr.isValid() ) continue;

            if ( uniqueResultAddresses.find( resultAddr ) != uniqueResultAddresses.end() ) continue;

            size_t adjustedTimeStep = m_timeStepIndex;
            if ( metaData[idx].m_isStatic )
            {
                adjustedTimeStep = 0;
            }

            RiaDefines::PorosityModelType porosityModel = m_eclResDef->porosityModel();
            cvf::ref<RigResultAccessor>   resultAccessor =
                RigResultAccessorFactory::createFromResultAddress( m_eclResDef->eclipseCase()->eclipseCaseData(),
                                                                   m_gridIndex,
                                                                   porosityModel,
                                                                   adjustedTimeStep,
                                                                   resultAddr );
            if ( resultAccessor.notNull() )
            {
                double scalarValue = resultAccessor->cellScalar( m_cellIndex );
                text.append( QString( "%1 : %2\n" ).arg( metaData[idx].m_resultVariable ).arg( scalarValue ) );

                uniqueResultAddresses.insert( resultAddr );
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::nncDetails()
{
    QString text;

    if ( m_nncIndex != cvf::UNDEFINED_SIZE_T )
    {
        if ( m_eclResDef.notNull() && m_eclResDef->eclipseCase() )
        {
            RigEclipseCaseData* eclipseCase = m_eclResDef->eclipseCase()->eclipseCaseData();

            RigMainGrid* grid = eclipseCase->mainGrid();
            CVF_ASSERT( grid );

            RigNNCData* nncData = grid->nncData();
            CVF_ASSERT( nncData );

            if ( nncData && m_nncIndex < nncData->allConnections().size() )
            {
                text += "-- NNC details --\n";
                {
                    const RigConnection&               conn = nncData->allConnections()[m_nncIndex];
                    cvf::StructGridInterface::FaceEnum face( conn.face() );

                    // First cell of NNC
                    {
                        CVF_ASSERT( conn.c1GlobIdx() < grid->globalCellArray().size() );
                        const RigCell& cell = grid->globalCellArray()[conn.c1GlobIdx()];

                        RigGridBase* hostGrid           = cell.hostGrid();
                        size_t       gridLocalCellIndex = cell.gridLocalCellIndex();

                        size_t i, j, k;
                        if ( hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
                        {
                            // Adjust to 1-based Eclipse indexing
                            i++;
                            j++;
                            k++;

                            QString gridName = QString::fromStdString( hostGrid->gridName() );
                            text.append(
                                QString( "NNC 1 : cell [%1, %2, %3] face %4 (%5)\n" ).arg( i ).arg( j ).arg( k ).arg( face.text() ).arg( gridName ) );
                        }
                    }

                    // Second cell of NNC
                    {
                        CVF_ASSERT( conn.c2GlobIdx() < grid->globalCellArray().size() );
                        const RigCell& cell = grid->globalCellArray()[conn.c2GlobIdx()];

                        RigGridBase* hostGrid           = cell.hostGrid();
                        size_t       gridLocalCellIndex = cell.gridLocalCellIndex();

                        size_t i, j, k;
                        if ( hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
                        {
                            // Adjust to 1-based Eclipse indexing
                            i++;
                            j++;
                            k++;

                            QString                            gridName = QString::fromStdString( hostGrid->gridName() );
                            cvf::StructGridInterface::FaceEnum oppositeFaceEnum( cvf::StructGridInterface::oppositeFace( face ) );
                            QString                            faceText = oppositeFaceEnum.text();

                            text.append(
                                QString( "NNC 2 : cell [%1, %2, %3] face %4 (%5)\n" ).arg( i ).arg( j ).arg( k ).arg( faceText ).arg( gridName ) );
                        }
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
void RiuResultTextBuilder::appendDetails( QString& text, const QString& details )
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
QString RiuResultTextBuilder::cellResultText( const std::vector<RimEclipseResultDefinition*>& resultDefinitions, bool appendCaseName )
{
    std::map<QString, QString> keyValues;

    int maxKeyLength = 0;
    for ( const auto& resDef : resultDefinitions )
    {
        auto resultTextAndValues = cellResultTextAndValueText( resDef );
        for ( const auto& [key, value] : resultTextAndValues )
        {
            maxKeyLength   = std::max( maxKeyLength, (int)key.length() );
            keyValues[key] = value;
        }
    }

    QString text;
    for ( const auto& [key, value] : keyValues )
    {
        if ( !text.isEmpty() ) text += "\n";
        text += QString( "%1 : %2" ).arg( key, -maxKeyLength ).arg( value );

        if ( appendCaseName && m_eclipseView && m_eclipseView->eclipseCase() )
        {
            auto caseName = m_eclipseView->eclipseCase()->caseUserDescription();
            text += "( " + caseName + " )";
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RiuResultTextBuilder::cellResultTextAndValueText( RimEclipseResultDefinition* eclResDef )
{
    if ( !eclResDef ) return {};

    std::map<QString, QString> keyValues;

    if ( m_eclResDef->eclipseCase() && m_eclResDef->eclipseCase()->eclipseCaseData() )
    {
        RigEclipseCaseData* eclipseCaseData = m_eclResDef->eclipseCase()->eclipseCaseData();

        if ( eclResDef->isTernarySaturationSelected() )
        {
            RigCaseCellResultsData* gridCellResults = m_eclResDef->currentGridCellResults();
            if ( gridCellResults )
            {
                gridCellResults->ensureKnownResultLoaded(
                    RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) );
                gridCellResults->ensureKnownResultLoaded(
                    RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) );
                gridCellResults->ensureKnownResultLoaded(
                    RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) );

                RiaDefines::PorosityModelType porosityModel = eclResDef->porosityModel();

                cvf::ref<RigResultAccessor> dataAccessObjectX =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                       m_gridIndex,
                                                                       porosityModel,
                                                                       m_timeStepIndex,
                                                                       RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                                RiaResultNames::soil() ) );
                cvf::ref<RigResultAccessor> dataAccessObjectY =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                       m_gridIndex,
                                                                       porosityModel,
                                                                       m_timeStepIndex,
                                                                       RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                                RiaResultNames::sgas() ) );
                cvf::ref<RigResultAccessor> dataAccessObjectZ =
                    RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                       m_gridIndex,
                                                                       porosityModel,
                                                                       m_timeStepIndex,
                                                                       RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                                RiaResultNames::swat() ) );

                double scalarValue = 0.0;

                if ( dataAccessObjectX.notNull() )
                    scalarValue = dataAccessObjectX->cellScalar( m_cellIndex );
                else
                    scalarValue = 0.0;

                keyValues["SOIL"] = QString( "%1" ).arg( scalarValue );

                if ( dataAccessObjectY.notNull() )
                    scalarValue = dataAccessObjectY->cellScalar( m_cellIndex );
                else
                    scalarValue = 0.0;

                keyValues["SGAS"] = QString( "%1" ).arg( scalarValue );

                if ( dataAccessObjectZ.notNull() )
                    scalarValue = dataAccessObjectZ->cellScalar( m_cellIndex );
                else
                    scalarValue = 0.0;
                keyValues["SWAT"] = QString( "%1" ).arg( scalarValue );
            }
        }
        else
        {
            size_t adjustedTimeStep = m_timeStepIndex;
            if ( eclResDef->hasStaticResult() )
            {
                adjustedTimeStep = 0;
            }

            cvf::ref<RigResultAccessor> resultAccessor =
                RigResultAccessorFactory::createFromResultDefinition( eclipseCaseData, m_gridIndex, adjustedTimeStep, eclResDef );
            if ( resultAccessor.notNull() )
            {
                double  scalarValue           = resultAccessor->cellFaceScalar( m_cellIndex, m_face );
                QString resultDescriptionText = eclResDef->resultVariableUiName();

                QString resultValueText;
                if ( eclResDef->hasCategoryResult() )
                {
                    auto resColorDef = dynamic_cast<RimEclipseCellColors*>( eclResDef );

                    RimRegularLegendConfig* legendConfig = nullptr;

                    if ( resColorDef )
                    {
                        legendConfig = resColorDef->legendConfig();
                    }
                    else
                    {
                        auto interResDef = eclResDef->firstAncestorOrThisOfType<RimIntersectionResultDefinition>();
                        if ( interResDef )
                        {
                            legendConfig = interResDef->regularLegendConfig();
                        }
                    }

                    if ( legendConfig )
                        resultValueText += legendConfig->categoryNameFromCategoryValue( scalarValue );
                    else
                        resultValueText += QString( "%1" ).arg( scalarValue );
                }
                else
                {
                    resultValueText = QString( "%1" ).arg( scalarValue );
                }

                keyValues[resultDescriptionText] = resultValueText;
            }
        }
    }

    return keyValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuResultTextBuilder::wellResultText()
{
    QString text;

    if ( m_eclResDef->eclipseCase() && m_eclResDef->eclipseCase()->eclipseCaseData() )
    {
        cvf::Collection<RigSimWellData> simWellData = m_eclResDef->eclipseCase()->eclipseCaseData()->wellResults();
        for ( const auto& singleWellResultData : simWellData )
        {
            if ( !singleWellResultData->hasWellResult( m_timeStepIndex ) )
            {
                continue;
            }

            const RigWellResultFrame* wellResultFrame = singleWellResultData->wellResultFrame( m_timeStepIndex );
            if ( !wellResultFrame )
            {
                continue;
            }

            const RigWellResultPoint wellResultCell = wellResultFrame->findResultCellWellHeadIncluded( m_gridIndex, m_cellIndex );
            if ( wellResultCell.isValid() )
            {
                const int branchId        = wellResultCell.branchId();
                const int segmentId       = wellResultCell.segmentId();
                const int outletBranchId  = wellResultCell.outletBranchId();
                const int outletSegmentId = wellResultCell.outletSegmentId();

                text += QString( "-- Well-cell connection info --\n Well Name: %1\n Branch Id: %2\n Segment "
                                 "Id: %3\n Outlet Branch Id: %4\n Outlet Segment Id: %5\n" )
                            .arg( singleWellResultData->m_wellName )
                            .arg( branchId )
                            .arg( segmentId )
                            .arg( outletBranchId )
                            .arg( outletSegmentId );
            }
        }
    }

    return text;
}
