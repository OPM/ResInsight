/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RivWellPathPartMgr.h"

#include "RiaGuiApplication.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellPath.h"

#include "Rim3dWellLogCurveCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimModeledWellPath.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"
#include "RimWellIASettings.h"
#include "RimWellIASettingsCollection.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementFilter.h"
#include "RimWellMeasurementInView.h"
#include "RimWellMeasurementInViewCollection.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"
#include "RimWellPathValve.h"

#include "Riv3dWellLogPlanePartMgr.h"
#include "RivAnnotationSourceInfo.h"
#include "RivBoxGeometryGenerator.h"
#include "RivDrawableSpheres.h"
#include "RivFishbonesSubsPartMgr.h"
#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPipeGeometryGenerator.h"
#include "RivSectionFlattener.h"
#include "RivTextLabelSourceInfo.h"
#include "RivWellConnectionFactorPartMgr.h"
#include "RivWellFracturePartMgr.h"
#include "RivWellPathPartMgr.h"
#include "RivWellPathSourceInfo.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfDrawableVectors.h"
#include "cvfFont.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
#include "cvfModelBasicList.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfPart.h"
#include "cvfScalarMapperContinuousLinear.h"
#include "cvfShaderProgram.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::RivWellPathPartMgr( RimWellPath* wellPath, Rim3dView* view )
{
    m_rimWellPath = wellPath;
    m_rimView     = view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivWellPathPartMgr::~RivWellPathPartMgr()
{
    clearAllBranchData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivWellPathPartMgr::isWellPathWithinBoundingBox( const cvf::BoundingBox& wellPathClipBoundingBox ) const
{
    if ( !m_rimWellPath->wellPathGeometry() ) return false;

    const std::vector<cvf::Vec3d>& wellpathCenterLine = m_rimWellPath->wellPathGeometry()->wellPathPoints();
    if ( wellpathCenterLine.size() < 2 ) return false;

    // Skip visualization if outside the domain of this case
    {
        cvf::Vec3d casemax = wellPathClipBoundingBox.max();
        cvf::Vec3d casemin = wellPathClipBoundingBox.min();
        cvf::Vec3d caseext = wellPathClipBoundingBox.extent();

        // Add up to the sealevel
        cvf::BoundingBox relevantWellpathBBox = wellPathClipBoundingBox;
        relevantWellpathBBox.add( cvf::Vec3d( casemax.x(), casemax.y(), 0.0 ) );

        // Add some sideways leeway

        cvf::Vec3d addSize = 3.0 * cvf::Vec3d( caseext.x(), caseext.y(), 0.0 );
        relevantWellpathBBox.add( casemax + addSize );
        relevantWellpathBBox.add( casemin - addSize );

        if ( !RigWellPath::isAnyPointInsideBoundingBox( wellpathCenterLine, relevantWellpathBBox ) )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivWellPathPartMgr::isWellPathEnabled( const cvf::BoundingBox& wellPathClipBoundingBox ) const
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if ( !wellPathCollection ) return false;

    if ( !wellPathCollection->isActive() ) return false;

    if ( wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF ) return false;

    if ( wellPathCollection->wellPathVisibility() == RimWellPathCollection::ALL_ON && !m_rimWellPath->showWellPath() ) return false;

    if ( !isWellPathWithinBoundingBox( wellPathClipBoundingBox ) ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendStaticFracturePartsToModel( cvf::ModelBasicList* model, const cvf::BoundingBox& wellPathClipBoundingBox )
{
    if ( m_rimView.isNull() ) return;

    const RimEclipseView* eclView = dynamic_cast<const RimEclipseView*>( m_rimView.p() );
    if ( !eclView ) return;

    if ( !m_rimWellPath || !m_rimWellPath->showWellPath() || !m_rimWellPath->fractureCollection()->isChecked() ) return;

    if ( !isWellPathWithinBoundingBox( wellPathClipBoundingBox ) ) return;

    for ( RimWellPathFracture* f : m_rimWellPath->fractureCollection()->activeFractures() )
    {
        CVF_ASSERT( f );

        f->fracturePartManager()->appendGeometryPartsToModel( model, *eclView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFishboneSubsPartsToModel( cvf::ModelBasicList*              model,
                                                         const caf::DisplayCoordTransform* displayCoordTransform,
                                                         double                            characteristicCellSize )
{
    if ( !m_rimWellPath || !m_rimWellPath->fishbonesCollection()->isChecked() ) return;

    for ( const auto& rimFishboneSubs : m_rimWellPath->fishbonesCollection()->activeFishbonesSubs() )
    {
        cvf::ref<RivFishbonesSubsPartMgr> fishbSubPartMgr = new RivFishbonesSubsPartMgr( rimFishboneSubs );
        fishbSubPartMgr->appendGeometryPartsToModel( model, displayCoordTransform, characteristicCellSize );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendWellPathAttributesToModel( cvf::ModelBasicList*              model,
                                                          const caf::DisplayCoordTransform* displayCoordTransform,
                                                          double                            characteristicCellSize )
{
    if ( !m_rimWellPath ) return;

    RivPipeGeometryGenerator           geoGenerator;
    std::vector<RimWellPathAttribute*> attributes = m_rimWellPath->attributeCollection()->attributes();

    for ( RimWellPathAttribute* attribute : attributes )
    {
        if ( attribute->isEnabled() )
        {
            if ( attribute->componentType() == RiaDefines::WellPathComponentType::CASING )
            {
                double wellPathRadius = this->wellPathRadius( characteristicCellSize, this->wellPathCollection() );
                double endMD          = attribute->endMD();
                double shoeLength     = wellPathRadius;
                double shoeStartMD    = endMD - shoeLength;

                std::vector<cvf::Vec3d> displayCoords;
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( shoeStartMD ) ) );
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMD ) ) );
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMD ) ) );

                std::vector<double> radii;
                radii.push_back( wellPathRadius );
                radii.push_back( wellPathRadius * 2.5 );
                radii.push_back( wellPathRadius * 1.1 );

                cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( attribute );

                cvf::Collection<cvf::Part> parts;
                RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth( &parts,
                                                                                   displayCoords,
                                                                                   radii,
                                                                                   attribute->defaultComponentColor() );
                for ( auto part : parts )
                {
                    part->setSourceInfo( objectSourceInfo.p() );
                    model->addPart( part.p() );
                }
            }
            else if ( attribute->componentType() == RiaDefines::WellPathComponentType::PACKER )
            {
                double wellPathRadius = this->wellPathRadius( characteristicCellSize, this->wellPathCollection() );
                double startMD        = attribute->startMD();
                double endMD          = attribute->endMD();

                std::vector<cvf::Vec3d> displayCoords;
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( startMD ) ) );
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( startMD ) ) );
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMD ) ) );
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMD ) ) );

                std::vector<double> radii;
                radii.push_back( wellPathRadius );
                radii.push_back( wellPathRadius * 1.5 );
                radii.push_back( wellPathRadius * 1.5 );
                radii.push_back( wellPathRadius );

                cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( attribute );

                cvf::Collection<cvf::Part> parts;
                RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth( &parts,
                                                                                   displayCoords,
                                                                                   radii,
                                                                                   attribute->defaultComponentColor() );
                for ( auto part : parts )
                {
                    part->setSourceInfo( objectSourceInfo.p() );
                    model->addPart( part.p() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendWellMeasurementsToModel( cvf::ModelBasicList*              model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        double                            characteristicCellSize )
{
    if ( !m_rimWellPath ) return;

    RimGridView* gridView = dynamic_cast<RimGridView*>( m_rimView.p() );
    if ( !gridView ) return;

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( !wellPathCollection ) return;

    RimWellMeasurementCollection* wellMeasurementCollection = wellPathCollection->measurementCollection();
    if ( !wellMeasurementCollection ) return;

    if ( !gridView->measurementCollection()->isChecked() ) return;

    for ( RimWellMeasurementInView* wellMeasurementInView :
          gridView->measurementCollection()->visibleMeasurementsForWellPath( m_rimWellPath->name() ) )
    {
        std::vector<QString> measurementKinds;
        measurementKinds.push_back( wellMeasurementInView->measurementKind() );

        double lowerBound = 0.0;
        double upperBound = 0.0;
        wellMeasurementInView->rangeValues( &lowerBound, &upperBound );
        std::vector<int> qualityFilter = wellMeasurementInView->qualityFilter();

        std::vector<RimWellMeasurement*> wellMeasurements =
            RimWellMeasurementFilter::filterMeasurements( wellMeasurementCollection->measurements(),
                                                          *wellPathCollection,
                                                          *m_rimWellPath,
                                                          measurementKinds,
                                                          lowerBound,
                                                          upperBound,
                                                          qualityFilter );

        RivPipeGeometryGenerator geoGenerator;
        for ( RimWellMeasurement* wellMeasurement : wellMeasurements )
        {
            double wellPathRadius = this->wellPathRadius( characteristicCellSize, this->wellPathCollection() );
            double startMD        = wellMeasurement->MD() - wellPathRadius * 0.5;
            double endMD          = wellMeasurement->MD() + wellPathRadius * 0.5;

            double wellMeasurementRadius =
                this->wellMeasurementRadius( characteristicCellSize, this->wellPathCollection(), wellMeasurementInView );

            std::vector<cvf::Vec3d> displayCoords;
            displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( startMD ) ) );
            displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( startMD ) ) );
            displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMD ) ) );
            displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( endMD ) ) );

            std::vector<double> radii;
            radii.push_back( std::min( wellPathRadius, wellMeasurementRadius ) );
            radii.push_back( wellMeasurementRadius );
            radii.push_back( wellMeasurementRadius );
            radii.push_back( std::min( wellPathRadius, wellMeasurementRadius ) );

            cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( wellMeasurement );

            cvf::Collection<cvf::Part> parts;

            // Use the view legend config to find color, if only one type of measurement is selected.
            cvf::Color3f color = cvf::Color3f( wellMeasurementInView->legendConfig()->scalarMapper()->mapToColor( wellMeasurement->value() ) );

            RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth( &parts, displayCoords, radii, color );
            for ( auto part : parts )
            {
                part->setSourceInfo( objectSourceInfo.p() );
                model->addPart( part.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendPerforationsToModel( cvf::ModelBasicList*              model,
                                                    size_t                            timeStepIndex,
                                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                                    double                            characteristicCellSize,
                                                    bool                              doFlatten )
{
    if ( !m_rimWellPath || !m_rimWellPath->perforationIntervalCollection()->isChecked() ) return;

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if ( !wellPathCollection ) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return;

    QDateTime currentTimeStamp;
    if ( m_rimView )
    {
        auto rimCase = m_rimView->ownerCase();

        if ( rimCase )
        {
            std::vector<QDateTime> timeStamps = rimCase->timeStepDates();
            if ( timeStepIndex < timeStamps.size() )
            {
                currentTimeStamp = timeStamps[timeStepIndex];
            }
        }
    }

    // Since we're using the index of measured depths to find the index of a point, ensure they're equal
    CVF_ASSERT( wellPathGeometry->measuredDepths().size() == wellPathGeometry->wellPathPoints().size() );

    double wellPathRadius    = this->wellPathRadius( characteristicCellSize, wellPathCollection );
    double perforationRadius = wellPathRadius * 1.1;

    std::vector<RimPerforationInterval*> perforations = m_rimWellPath->descendantsIncludingThisOfType<RimPerforationInterval>();
    for ( RimPerforationInterval* perforation : perforations )
    {
        using namespace std;

        if ( !perforation->isChecked() ) continue;
        if ( perforation->startMD() > perforation->endMD() ) continue;

        if ( !perforation->isActiveOnDate( currentTimeStamp ) ) continue;

        double             horizontalLengthAlongWellPath = 0.0;
        vector<cvf::Vec3d> perfIntervalCL;
        {
            pair<vector<cvf::Vec3d>, vector<double>> perfintervalCoordsAndMD =
                wellPathGeometry->clippedPointSubset( perforation->startMD(), perforation->endMD(), &horizontalLengthAlongWellPath );
            perfIntervalCL = perfintervalCoordsAndMD.first;
        }

        if ( perfIntervalCL.size() < 2 ) continue;

        vector<cvf::Vec3d> perfIntervalCLDiplayCS;
        if ( doFlatten )
        {
            cvf::Vec3d         dummy;
            vector<cvf::Mat4d> flatningCSs =
                RivSectionFlattener::calculateFlatteningCSsForPolyline( perfIntervalCL,
                                                                        cvf::Vec3d::Z_AXIS,
                                                                        { horizontalLengthAlongWellPath, 0.0, perfIntervalCL[0].z() },
                                                                        &dummy );

            for ( size_t cIdx = 0; cIdx < perfIntervalCL.size(); ++cIdx )
            {
                auto clpoint = perfIntervalCL[cIdx].getTransformedPoint( flatningCSs[cIdx] );
                perfIntervalCLDiplayCS.push_back( displayCoordTransform->scaleToDisplaySize( clpoint ) );
            }
        }
        else
        {
            for ( cvf::Vec3d& point : perfIntervalCL )
            {
                perfIntervalCLDiplayCS.push_back( displayCoordTransform->transformToDisplayCoord( point ) );
            }
        }
        {
            cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( perforation );

            cvf::Collection<cvf::Part> parts;
            RivPipeGeometryGenerator::cylinderWithCenterLineParts( &parts, perfIntervalCLDiplayCS, cvf::Color3f::GREEN, perforationRadius );
            for ( auto part : parts )
            {
                part->setSourceInfo( objectSourceInfo.p() );
                model->addPart( part.p() );
            }
        }

        appendPerforationValvesToModel( model, perforation, wellPathRadius, displayCoordTransform );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendPerforationValvesToModel( cvf::ModelBasicList*              model,
                                                         RimPerforationInterval*           perforation,
                                                         double                            wellPathRadius,
                                                         const caf::DisplayCoordTransform* displayCoordTransform )
{
    for ( RimWellPathValve* valve : perforation->valves() )
    {
        if ( !valve->isChecked() ) continue;

        std::vector<double> measuredDepthsRelativeToStartMD;
        std::vector<double> radii;
        cvf::Color3f        valveColor = valve->defaultComponentColor();
        if ( valve->componentType() == RiaDefines::WellPathComponentType::ICV )
        {
            measuredDepthsRelativeToStartMD = { 0.0, 1.0, 1.5, 4.0, 5.0, 5.5, 8.0, 9.0 };
            radii                           = { wellPathRadius,
                                                wellPathRadius * 1.8,
                                                wellPathRadius * 2.0,
                                                wellPathRadius * 2.0,
                                                wellPathRadius * 1.8,
                                                wellPathRadius * 1.7,
                                                wellPathRadius * 1.7,
                                                wellPathRadius };

            double                  startMD = valve->startMD();
            std::vector<cvf::Vec3d> displayCoords;
            for ( double mdRelativeToStart : measuredDepthsRelativeToStartMD )
            {
                displayCoords.push_back( displayCoordTransform->transformToDisplayCoord(
                    m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( mdRelativeToStart * 0.333 * wellPathRadius + startMD ) ) );
            }

            cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( valve );

            cvf::Collection<cvf::Part> parts;
            RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth( &parts, displayCoords, radii, valveColor );
            for ( auto part : parts )
            {
                part->setSourceInfo( objectSourceInfo.p() );
                model->addPart( part.p() );
            }
        }
        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICD ||
                  valve->componentType() == RiaDefines::WellPathComponentType::AICD )
        {
            std::vector<double> valveLocations = valve->valveLocations();
            for ( double startMD : valveLocations )
            {
                int size = 16;

                measuredDepthsRelativeToStartMD.resize( size );
                radii.resize( size );
                for ( int i = 0; i < size; i += 2 )
                {
                    measuredDepthsRelativeToStartMD[i]     = double( i / 2 );
                    measuredDepthsRelativeToStartMD[i + 1] = double( i / 2 + 0.5 );
                }
                radii[0]     = wellPathRadius;
                bool inner   = false;
                int  nInners = 0;
                for ( int i = 1; i < size - 1; i += 2 )
                {
                    if ( inner && valve->componentType() == RiaDefines::WellPathComponentType::AICD && nInners > 0 )
                    {
                        radii[i + 1] = radii[i] = wellPathRadius * 1.7;
                        nInners                 = 0;
                    }
                    else if ( inner )
                    {
                        radii[i + 1] = radii[i] = wellPathRadius * 1.9;
                        nInners++;
                    }
                    else
                    {
                        radii[i + 1] = radii[i] = wellPathRadius * 2.0;
                    }
                    inner = !inner;
                }
                radii[size - 1] = wellPathRadius;

                std::vector<cvf::Vec3d> displayCoords;
                for ( double mdRelativeToStart : measuredDepthsRelativeToStartMD )
                {
                    displayCoords.push_back(
                        displayCoordTransform->transformToDisplayCoord( m_rimWellPath->wellPathGeometry()->interpolatedPointAlongWellPath(
                            mdRelativeToStart * 0.333 * wellPathRadius + startMD ) ) );
                }

                cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( valve );

                cvf::Collection<cvf::Part> parts;
                RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth( &parts, displayCoords, radii, valveColor );
                for ( auto part : parts )
                {
                    part->setSourceInfo( objectSourceInfo.p() );
                    model->addPart( part.p() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendVirtualTransmissibilitiesToModel( cvf::ModelBasicList*              model,
                                                                 size_t                            timeStepIndex,
                                                                 const caf::DisplayCoordTransform* displayCoordTransform,
                                                                 double                            characteristicCellSize )
{
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( m_rimView.p() );
    if ( !eclView ) return;

    if ( !eclView->isVirtualConnectionFactorGeometryVisible() ) return;

    auto eclipseCase = eclView->firstAncestorOrThisOfType<RimEclipseCase>();
    if ( !eclipseCase ) return;

    const RigVirtualPerforationTransmissibilities* trans = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if ( trans )
    {
        m_wellConnectionFactorPartMgr = new RivWellConnectionFactorPartMgr( m_rimWellPath, eclView->virtualPerforationResult() );

        m_wellConnectionFactorPartMgr->appendDynamicGeometryPartsToModel( model, timeStepIndex );
    }
}

//--------------------------------------------------------------------------------------------------
/// The pipe geometry needs to be rebuilt on scale change to keep the pipes round
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::buildWellPathParts( const caf::DisplayCoordTransform* displayCoordTransform,
                                             double                            characteristicCellSize,
                                             const cvf::BoundingBox&           wellPathClipBoundingBox,
                                             bool                              doFlatten )
{
    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if ( !wellPathCollection ) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return;

    std::vector<cvf::Vec3d> wellpathCenterLine = wellPathGeometry->uniqueWellPathPoints();

    if ( wellpathCenterLine.size() < 2 ) return;

    clearAllBranchData();

    double wellPathRadius = this->wellPathRadius( characteristicCellSize, wellPathCollection );

    std::vector<cvf::Vec3d> clippedWellPathCenterLine;

    // Generate the well path geometry as a line and pipe structure

    m_pipeGeomGenerator = new RivPipeGeometryGenerator;

    m_pipeGeomGenerator->setRadius( wellPathRadius );
    m_pipeGeomGenerator->setCrossSectionVertexCount( wellPathCollection->wellPathCrossSectionVertexCount() );

    double horizontalLengthAlongWellToClipPoint = 0.0;
    double measuredDepthAtFirstClipPoint        = 0.0;
    size_t idxToFirstVisibleSegment             = 0;
    if ( wellPathCollection->wellPathClip )
    {
        double maxZClipHeight     = wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance;
        clippedWellPathCenterLine = RigWellPath::clipPolylineStartAboveZ( wellpathCenterLine,
                                                                          maxZClipHeight,
                                                                          horizontalLengthAlongWellToClipPoint,
                                                                          measuredDepthAtFirstClipPoint,
                                                                          idxToFirstVisibleSegment );
    }
    else
    {
        clippedWellPathCenterLine = wellpathCenterLine;
    }

    if ( clippedWellPathCenterLine.size() < 2 ) return;

    cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray( clippedWellPathCenterLine.size() );

    // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.

    if ( doFlatten )
    {
        cvf::Vec3d              dummy;
        std::vector<cvf::Mat4d> flatningCSs = RivSectionFlattener::calculateFlatteningCSsForPolyline( clippedWellPathCenterLine,
                                                                                                      cvf::Vec3d::Z_AXIS,
                                                                                                      { horizontalLengthAlongWellToClipPoint,
                                                                                                        0.0,
                                                                                                        clippedWellPathCenterLine[0].z() },
                                                                                                      &dummy );

        for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
        {
            auto clpoint         = clippedWellPathCenterLine[cIdx].getTransformedPoint( flatningCSs[cIdx] );
            ( *cvfCoords )[cIdx] = displayCoordTransform->scaleToDisplaySize( clpoint );
        }
    }
    else
    {
        for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
        {
            ( *cvfCoords )[cIdx] = displayCoordTransform->transformToDisplayCoord( clippedWellPathCenterLine[cIdx] );
        }
    }

    m_pipeGeomGenerator->setFirstVisibleSegmentIndex( idxToFirstVisibleSegment );
    m_pipeGeomGenerator->setPipeCenterCoords( cvfCoords.p() );
    m_surfaceDrawable    = m_pipeGeomGenerator->createPipeSurface();
    m_centerLineDrawable = m_pipeGeomGenerator->createCenterLine();

    if ( m_surfaceDrawable.notNull() )
    {
        m_surfacePart = new cvf::Part;
        m_surfacePart->setName( "RivWellPathPartMgr::surface" );
        m_surfacePart->setDrawable( m_surfaceDrawable.p() );

        RivWellPathSourceInfo* sourceInfo = new RivWellPathSourceInfo( m_rimWellPath, m_pipeGeomGenerator.p() );
        m_surfacePart->setSourceInfo( sourceInfo );

        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( m_rimWellPath->wellPathColor() ), caf::PO_1 );
        cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();

        m_surfacePart->setEffect( eff.p() );
    }

    if ( m_centerLineDrawable.notNull() )
    {
        m_centerLinePart = new cvf::Part;
        m_centerLinePart->setName( "RivWellPathPartMgr::centerLinePart" );
        m_centerLinePart->setDrawable( m_centerLineDrawable.p() );

        caf::MeshEffectGenerator gen( m_rimWellPath->wellPathColor() );
        cvf::ref<cvf::Effect>    eff = gen.generateCachedEffect();

        m_centerLinePart->setEffect( eff.p() );

        if ( m_rimWellPath->measuredDepthLabelInterval().has_value() && clippedWellPathCenterLine.size() > 2 )
        {
            const double distanceBetweenLabels = m_rimWellPath->measuredDepthLabelInterval().value();

            // Create a round number as start for measured depth label
            const double startMeasuredDepth =
                ( static_cast<int>( measuredDepthAtFirstClipPoint / distanceBetweenLabels ) + 1 ) * distanceBetweenLabels;

            std::vector<std::string> labelTexts;
            std::vector<cvf::Vec3d>  labelDisplayCoords;

            double measuredDepth = startMeasuredDepth;
            while ( measuredDepth < wellPathGeometry->measuredDepths().back() )
            {
                labelTexts.push_back( std::to_string( static_cast<int>( measuredDepth ) ) );
                auto domainCoord = wellPathGeometry->interpolatedPointAlongWellPath( measuredDepth );

                auto displayCoord = displayCoordTransform->transformToDisplayCoord( domainCoord );
                labelDisplayCoords.push_back( displayCoord );

                measuredDepth += distanceBetweenLabels;
            }

            m_centerLinePart->setSourceInfo( new RivAnnotationSourceInfo( labelTexts, labelDisplayCoords ) );
        }
    }

    // Generate label with well-path name at a position that is slightly offset towards the end of the well path
    // This is to avoid overlap between well path laterals.
    cvf::Vec3d textPosition = cvfCoords->get( 0 );
    cvf::Vec3d tangent      = ( cvfCoords->get( cvfCoords->size() - 1 ) - cvfCoords->get( 0 ) ).getNormalized();

    textPosition.z() += 2.2 * characteristicCellSize;
    textPosition += tangent * 2.2 * characteristicCellSize;

    if ( wellPathCollection->showWellPathLabel() && m_rimWellPath->showWellPathLabel() && !m_rimWellPath->name().isEmpty() )
    {
        cvf::Font* font = RiaGuiApplication::instance()->defaultWellLabelFont();

        auto drawableText = RivAnnotationTools::createDrawableTextNoBackground( font,
                                                                                wellPathCollection->wellPathLabelColor(),
                                                                                m_rimWellPath->name().toStdString(),
                                                                                cvf::Vec3f( textPosition ) );

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivWellHeadPartMgr: text " + m_rimWellPath->name().toStdString() );
        part->setDrawable( drawableText.p() );

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect( eff.p() );
        part->setPriority( RivPartPriority::Text );

        part->setSourceInfo( new RivTextLabelSourceInfo( m_rimWellPath, m_rimWellPath->name().toStdString(), cvf::Vec3f( textPosition ) ) );

        m_wellLabelPart = part;
    }

    auto modeledWellPath = dynamic_cast<RimModeledWellPath*>( m_rimWellPath.p() );
    if ( modeledWellPath )
    {
        bool showWellTargetSpheres = modeledWellPath->geometryDefinition()->showSpheres();

        if ( showWellTargetSpheres )
        {
            auto geoDef = modeledWellPath->geometryDefinition();

            auto   sphereColor        = geoDef->sphereColor();
            double sphereRadiusFactor = geoDef->sphereRadiusFactor();

            cvf::ref<cvf::Vec3fArray>   vertices = new cvf::Vec3fArray;
            cvf::ref<cvf::Vec3fArray>   vecRes   = new cvf::Vec3fArray;
            cvf::ref<cvf::Color3fArray> colors   = new cvf::Color3fArray;

            auto wellTargets = geoDef->activeWellTargets();

            size_t pointCount = wellTargets.size();
            vertices->reserve( pointCount );
            vecRes->reserve( pointCount );
            colors->reserve( pointCount );

            for ( const auto target : wellTargets )
            {
                auto domainCoord  = target->targetPointXYZ() + modeledWellPath->geometryDefinition()->anchorPointXyz();
                auto displayCoord = displayCoordTransform->transformToDisplayCoord( domainCoord );
                vertices->add( cvf::Vec3f( displayCoord ) );
                vecRes->add( cvf::Vec3f::X_AXIS );
                colors->add( sphereColor );
            }

            cvf::ref<RivDrawableSpheres> vectorDrawable;
            if ( RiaGuiApplication::instance()->useShaders() )
            {
                vectorDrawable = new RivDrawableSpheres( "u_transformationMatrix", "u_color" );
            }
            else
            {
                vectorDrawable = new RivDrawableSpheres();
            }

            vectorDrawable->setVectors( vertices.p(), vecRes.p() );
            vectorDrawable->setColors( colors.p() );

            double cellRadius  = 15.0;
            auto   eclipseView = dynamic_cast<RimEclipseView*>( m_rimView.p() );
            if ( eclipseView && eclipseView->mainGrid() )
            {
                double characteristicCellSize = eclipseView->mainGrid()->characteristicIJCellSize();
                cellRadius                    = sphereRadiusFactor * characteristicCellSize;
            }

            cvf::GeometryBuilderTriangles builder;
            cvf::GeometryUtils::createSphere( cellRadius, 15, 15, &builder );
            vectorDrawable->setGlyph( builder.trianglesUShort().p(), builder.vertices().p() );
            vectorDrawable->setRadius( cellRadius );
            vectorDrawable->setCenterCoords( vertices.p() );

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "RivWellPathPartMgr_WellTargetSpheres" );
            part->setDrawable( vectorDrawable.p() );

            part->setEffect( new cvf::Effect() );

            auto sourceInfo = new RivObjectSourceInfo( geoDef );
            part->setSourceInfo( sourceInfo );

            m_spherePart = part;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendStaticGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                                           double                            characteristicCellSize,
                                                           const cvf::BoundingBox&           wellPathClipBoundingBox )
{
    if ( !isWellPathEnabled( wellPathClipBoundingBox ) ) return;

    // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
    buildWellPathParts( displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox, false );

    if ( m_surfacePart.notNull() )
    {
        model->addPart( m_surfacePart.p() );
    }

    if ( m_centerLinePart.notNull() )
    {
        model->addPart( m_centerLinePart.p() );
    }

    if ( m_wellLabelPart.notNull() )
    {
        model->addPart( m_wellLabelPart.p() );
    }

    if ( m_spherePart.notNull() )
    {
        model->addPart( m_spherePart.p() );
    }

    appendFishboneSubsPartsToModel( model, displayCoordTransform, characteristicCellSize );
    appendWellPathAttributesToModel( model, displayCoordTransform, characteristicCellSize );
    appendWellIntegrityIntervalsToModel( model, displayCoordTransform, characteristicCellSize );

    RimGridView* gridView = dynamic_cast<RimGridView*>( m_rimView.p() );
    if ( gridView )
    {
        m_3dWellLogPlanePartMgr = new Riv3dWellLogPlanePartMgr( m_rimWellPath, gridView );
        m_3dWellLogPlanePartMgr->appendPlaneToModel( model, displayCoordTransform, wellPathClipBoundingBox, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFlattenedStaticGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                                                    double                            characteristicCellSize,
                                                                    const cvf::BoundingBox&           wellPathClipBoundingBox )
{
    if ( !wellPathClipBoundingBox.isValid() ) return;
    if ( !isWellPathWithinBoundingBox( wellPathClipBoundingBox ) ) return;

    // The pipe geometry needs to be rebuilt on scale change to keep the pipes round
    buildWellPathParts( displayCoordTransform, characteristicCellSize, wellPathClipBoundingBox, true );

    if ( m_surfacePart.notNull() )
    {
        model->addPart( m_surfacePart.p() );
    }

    if ( m_centerLinePart.notNull() )
    {
        model->addPart( m_centerLinePart.p() );
    }

    if ( m_wellLabelPart.notNull() )
    {
        model->addPart( m_wellLabelPart.p() );
    }

    /*
    // TODO: Currently not supported.
    // Require coordinate transformations of the spheres similar to RivWellPathPartMgr::buildWellPathParts
    // https://github.com/OPM/ResInsight/issues/7891
    //
        if ( m_spherePart.notNull() )
        {
            model->addPart( m_spherePart.p() );
        }
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                            size_t                            timeStepIndex,
                                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                                            double                            characteristicCellSize,
                                                            const cvf::BoundingBox&           wellPathClipBoundingBox )
{
    CVF_ASSERT( model );

    bool showWellPath = isWellPathEnabled( wellPathClipBoundingBox );

    if ( showWellPath )
    {
        // Only show perforations and virtual transmissibilities when well path is shown
        appendPerforationsToModel( model, timeStepIndex, displayCoordTransform, characteristicCellSize, false );
        appendVirtualTransmissibilitiesToModel( model, timeStepIndex, displayCoordTransform, characteristicCellSize );
    }

    // Well measurements visibility is independent of well path visibility
    appendWellMeasurementsToModel( model, displayCoordTransform, characteristicCellSize );

    if ( showWellPath )
    {
        RimGridView* gridView = dynamic_cast<RimGridView*>( m_rimView.p() );
        if ( gridView )
        {
            if ( m_3dWellLogPlanePartMgr.isNull() )
            {
                m_3dWellLogPlanePartMgr = new Riv3dWellLogPlanePartMgr( m_rimWellPath, gridView );
            }
            m_3dWellLogPlanePartMgr->appendPlaneToModel( model, displayCoordTransform, wellPathClipBoundingBox, false );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                     size_t                            timeStepIndex,
                                                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                                                     double                            characteristicCellSize,
                                                                     const cvf::BoundingBox&           wellPathClipBoundingBox )
{
    CVF_ASSERT( model );

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if ( !wellPathCollection ) return;

    if ( m_rimWellPath.isNull() ) return;

    if ( !isWellPathWithinBoundingBox( wellPathClipBoundingBox ) ) return;

    appendPerforationsToModel( model, timeStepIndex, displayCoordTransform, characteristicCellSize, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::clearAllBranchData()
{
    m_pipeGeomGenerator  = nullptr;
    m_surfacePart        = nullptr;
    m_surfaceDrawable    = nullptr;
    m_centerLinePart     = nullptr;
    m_centerLineDrawable = nullptr;
    m_wellLabelPart      = nullptr;
    m_spherePart         = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCollection* RivWellPathPartMgr::wellPathCollection() const
{
    if ( !m_rimWellPath ) return nullptr;

    return m_rimWellPath->firstAncestorOrThisOfType<RimWellPathCollection>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivWellPathPartMgr::wellPathRadius( double characteristicCellSize, RimWellPathCollection* wellPathCollection )
{
    return wellPathCollection->wellPathRadiusScaleFactor() * m_rimWellPath->wellPathRadiusScaleFactor() * characteristicCellSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivWellPathPartMgr::wellMeasurementRadius( double                          characteristicCellSize,
                                                  const RimWellPathCollection*    wellPathCollection,
                                                  const RimWellMeasurementInView* wellMeasurementInView )
{
    return wellPathCollection->wellPathRadiusScaleFactor() * wellMeasurementInView->radiusScaleFactor() * characteristicCellSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivWellPathPartMgr::appendWellIntegrityIntervalsToModel( cvf::ModelBasicList*              model,
                                                              const caf::DisplayCoordTransform* displayCoordTransform,
                                                              double                            characteristicCellSize )
{
    if ( !m_rimWellPath ) return;

    RimWellPathCollection* wellPathCollection = this->wellPathCollection();
    if ( !wellPathCollection ) return;

    RigWellPath* wellPathGeometry = m_rimWellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return;

    // Since we're using the index of measured depths to find the index of a point, ensure they're equal
    CVF_ASSERT( wellPathGeometry->measuredDepths().size() == wellPathGeometry->wellPathPoints().size() );

    double wellPathRadius    = this->wellPathRadius( characteristicCellSize, wellPathCollection );
    double wiaIntervalRadius = wellPathRadius * 1.15;

    for ( auto wiaModel : m_rimWellPath->wellIASettingsCollection()->settings() )
    {
        if ( !wiaModel->isChecked() ) continue;
        if ( wiaModel->startMD() > wiaModel->endMD() ) continue;

        double                  horizontalLengthAlongWellPath = 0.0;
        std::vector<cvf::Vec3d> intervalCL;
        {
            std::pair<std::vector<cvf::Vec3d>, std::vector<double>> intervalCoordsAndMD =
                wellPathGeometry->clippedPointSubset( wiaModel->startMD(), wiaModel->endMD(), &horizontalLengthAlongWellPath );
            intervalCL = intervalCoordsAndMD.first;
        }

        if ( intervalCL.size() < 2 ) continue;

        std::vector<cvf::Vec3d> intervalCLDisplayCS;

        for ( cvf::Vec3d& point : intervalCL )
        {
            intervalCLDisplayCS.push_back( displayCoordTransform->transformToDisplayCoord( point ) );
        }
        cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( wiaModel );

        cvf::Collection<cvf::Part> parts;
        RivPipeGeometryGenerator::cylinderWithCenterLineParts( &parts, intervalCLDisplayCS, cvf::Color3f::ORCHID, wiaIntervalRadius );
        for ( auto& part : parts )
        {
            part->setSourceInfo( objectSourceInfo.p() );
            model->addPart( part.p() );
        }

        if ( wiaModel->showBox() )
        {
            const auto& vertices = wiaModel->modelBoxVertices();

            std::vector<cvf::Vec3f> transformedVertices;

            for ( auto& v : vertices )
            {
                transformedVertices.push_back( cvf::Vec3f( displayCoordTransform->transformToDisplayCoord( v ) ) );
            }

            cvf::ref<cvf::Part> boxpart = RivBoxGeometryGenerator::createBoxFromVertices( transformedVertices, cvf::Color3f::ORCHID );
            boxpart->setSourceInfo( objectSourceInfo.p() );
            model->addPart( boxpart.p() );
        }
    }
}
