/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimFaultReactivationModel.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaPreferencesGeoMech.h"
#include "RiaQDateTimeTools.h"

#include "RifJsonEncodeDecode.h"
#include "RifParameterXmlReader.h"

#include "RigActiveCellInfo.h"
#include "RigBasicPlane.h"
#include "RigEclipseCaseData.h"
#include "RigFaultReactivationModel.h"
#include "RigFaultReactivationModelGenerator.h"
#include "RigPolyLinesData.h"

#include "WellPathCommands/PointTangentManipulator/RicPolyline3dEditor.h"
#include "WellPathCommands/RicPolylineTargetsPickEventHandler.h"

#include "RiuViewer.h"

#include "RivFaultReactivationModelPartMgr.h"

#include "Rim3dView.h"
#include "RimDoubleParameter.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFaultReactivationEnums.h"
#include "RimFaultReactivationTools.h"
#include "RimGeoMechCase.h"
#include "RimParameterGroup.h"
#include "RimPolylineTarget.h"
#include "RimTimeStepFilter.h"
#include "RimTools.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "cvfPlane.h"
#include "cvfTextureImage.h"

#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QVariant>

CAF_PDM_SOURCE_INIT( RimFaultReactivationModel, "FaultReactivationModel" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel::RimFaultReactivationModel()
    : m_pickTargetsEventHandler( new RicPolylineTargetsPickEventHandler( this ) )
{
    CAF_PDM_InitObject( "Fault Reactivation Model", ":/fault_react_24x24.png" );

    CAF_PDM_InitField( &m_userDescription, "UserDescription", QString( "Model" ), "Name" );
    CAF_PDM_InitFieldNoDefault( &m_geomechCase, "GeoMechCase", "Global GeoMech Model" );
    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDirectory", "Working Folder" );
    CAF_PDM_InitField( &m_modelThickness, "ModelThickness", 100.0, "Model Cell Thickness" );

    CAF_PDM_InitField( &m_modelExtentFromAnchor, "ModelExtentFromAnchor", 3000.0, "Horz. Extent from Anchor" );
    CAF_PDM_InitField( &m_modelMinZ, "ModelMinZ", 0.0, "Seabed Depth" );
    CAF_PDM_InitField( &m_modelBelowSize, "ModelBelowSize", 500.0, "Depth Below Fault" );

    CAF_PDM_InitFieldNoDefault( &m_startCellIndex, "StartCellIndex", "Start Cell Index" );
    CAF_PDM_InitFieldNoDefault( &m_startCellFace, "StartCellFace", "Start Cell Face" );
    m_startCellIndex = 0;
    m_startCellFace  = cvf::StructGridInterface::FaceType::NO_FACE;

    CAF_PDM_InitField( &m_faultExtendUpwards, "FaultExtendUpwards", 0.0, "Above Reservoir" );
    m_faultExtendUpwards.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_faultExtendDownwards, "FaultExtendDownwards", 0.0, "Below Reservoir" );
    m_faultExtendDownwards.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showModelPlane, "ShowModelPlane", true, "Show 2D Model" );

    CAF_PDM_InitFieldNoDefault( &m_fault, "Fault", "Fault" );
    m_fault.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_modelPart1Color, "ModelPart1Color", cvf::Color3f( cvf::Color3f::GREEN ), "Part 1 Color" );
    CAF_PDM_InitField( &m_modelPart2Color, "ModelPart2Color", cvf::Color3f( cvf::Color3f::BLUE ), "Part 2 Color" );

    CAF_PDM_InitField( &m_maxReservoirCellHeight, "MaxReservoirCellHeight", 20.0, "Max. Reservoir Cell Height" );
    CAF_PDM_InitField( &m_cellHeightGrowFactor, "CellHeightGrowFactor", 1.05, "Cell Height Grow Factor" );

    CAF_PDM_InitField( &m_minReservoirCellWidth, "MinReservoirCellWidth", 20.0, "Reservoir Cell Width" );
    CAF_PDM_InitField( &m_cellWidthGrowFactor, "CellWidthGrowFactor", 1.05, "Cell Width Grow Factor" );

    CAF_PDM_InitField( &m_useLocalCoordinates, "UseLocalCoordinates", false, "Export Using Local Coordinates" );

    // Time Step Selection
    CAF_PDM_InitFieldNoDefault( &m_timeStepFilter, "TimeStepFilter", "Available Time Steps" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Select Time Steps" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitField( &m_useGridPorePressure, "UseGridPorePressure", true, "Output Grid Pore Pressure" );
    CAF_PDM_InitField( &m_useGridVoidRatio, "UseGridVoidRatio", true, "Output Grid Void Ratio" );
    CAF_PDM_InitField( &m_useGridTemperature, "UseGridTemperature", true, "Output Grid Temperature" );
    CAF_PDM_InitField( &m_useGridDensity, "UseGridDensity", false, "Output Grid Density" );
    CAF_PDM_InitField( &m_useGridElasticProperties, "UseGridElasticProperties", false, "Output Grid Elastic Properties" );
    CAF_PDM_InitField( &m_useGridStress, "UseGridStress", false, "Output Grid Stress" );

    CAF_PDM_InitField( &m_waterDensity, "WaterDensity", 1030.0, "Water Density [kg/m3]" );

    CAF_PDM_InitFieldNoDefault( &m_targets, "Targets", "Targets" );
    m_targets.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_targets.uiCapability()->setUiTreeChildrenHidden( true );
    m_targets.uiCapability()->setUiTreeHidden( true );
    m_targets.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_targets.uiCapability()->setCustomContextMenuEnabled( false );

    CAF_PDM_InitFieldNoDefault( &m_materialParameters, "MaterialParameters", "Materials", ":/Bullet.png" );

    this->setUi3dEditorTypeName( RicPolyline3dEditor::uiEditorTypeName() );

    setDeletable( true );

    m_2Dmodel = new RigFaultReactivationModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationModel::~RimFaultReactivationModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::initSettings( QString& outErrmsg )
{
    RifParameterXmlReader basicreader( RiaPreferencesGeoMech::current()->geomechFRMDefaultXML() );
    if ( !basicreader.parseFile( outErrmsg ) ) return false;

    m_materialParameters.deleteChildren();
    for ( auto group : basicreader.parameterGroups() )
    {
        m_materialParameters.push_back( group );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::userDescription()
{
    return m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setUserDescription( QString description )
{
    m_userDescription = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFaultReactivationModel::userDescriptionField()
{
    return &m_userDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::string> RimFaultReactivationModel::validateModel() const
{
    if ( fault() == nullptr )
    {
        return std::make_pair( false, "A fault has not been selected. Please check your model settings." );
    }

    if ( selectedTimeSteps().size() < 2 )
    {
        return std::make_pair( false, "You need at least 2 selected timesteps. Please check your model settings." );
    }

    if ( selectedTimeSteps()[0] != m_availableTimeSteps[0] )
    {
        return std::make_pair( false, "The first available timestep must always be selected. Please check your model settings." );
    }

    return std::make_pair( true, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setFaultInformation( RimFaultInView* fault, size_t cellIndex, cvf::StructGridInterface::FaceType face )
{
    m_fault          = fault;
    m_startCellIndex = cellIndex;
    m_startCellFace  = face;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInView* RimFaultReactivationModel::fault() const
{
    return m_fault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setTargets( cvf::Vec3d target1, cvf::Vec3d target2 )
{
    m_targets.deleteChildren();

    RimPolylineTarget* planeCenter = new RimPolylineTarget();
    planeCenter->setAsPointXYZ( target1 );

    m_targets.push_back( planeCenter );

    RimPolylineTarget* steeringTarget = new RimPolylineTarget();
    steeringTarget->setAsPointXYZ( target2 );

    m_targets.push_back( steeringTarget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineTarget*> RimFaultReactivationModel::activeTargets() const
{
    return m_targets.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert )
{
    // do nothing, we should only have 2 predefined targets
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::deleteTarget( RimPolylineTarget* targetToDelete )
{
    // do nothing, we should always have 2 predefined targets
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::pickingEnabled() const
{
    // never pick, we only have our 2 predefined targets
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PickEventHandler* RimFaultReactivationModel::pickEventHandler() const
{
    return m_pickTargetsEventHandler.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::updateVisualization()
{
    auto view = firstAncestorOrThisOfType<Rim3dView>();
    if ( !view ) return;

    if ( m_startCellIndex() == 0 ) return;
    if ( m_startCellFace() == cvf::StructGridInterface::FaceType::NO_FACE ) return;
    if ( m_targets.size() < 2 ) return;

    auto normal = m_targets[1]->targetPointXYZ() - m_targets[0]->targetPointXYZ();
    normal.z()  = 0.0;
    if ( !normal.normalize() ) return;

    auto modelNormal = normal ^ cvf::Vec3d::Z_AXIS;
    modelNormal.normalize();

    auto generator = std::make_shared<RigFaultReactivationModelGenerator>( m_targets[0]->targetPointXYZ(), modelNormal );
    generator->setFault( m_fault()->faultGeometry() );
    generator->setGrid( eclipseCase()->mainGrid() );
    generator->setActiveCellInfo( eclipseCase()->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ) );
    generator->setModelSize( m_modelMinZ, m_modelBelowSize, m_modelExtentFromAnchor );
    generator->setFaultBufferDepth( m_faultExtendUpwards, m_faultExtendDownwards );
    generator->setModelThickness( m_modelThickness );
    generator->setModelGriddingOptions( m_maxReservoirCellHeight, m_cellHeightGrowFactor, m_minReservoirCellWidth, m_cellWidthGrowFactor );
    generator->setupLocalCoordinateTransform();
    generator->setUseLocalCoordinates( m_useLocalCoordinates );

    m_2Dmodel->setPartColors( m_modelPart1Color, m_modelPart2Color );
    m_2Dmodel->setGenerator( generator );
    m_2Dmodel->updateGeometry( m_startCellIndex, (cvf::StructGridInterface::FaceType)m_startCellFace() );

    view->scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::updateEditorsAndVisualization()
{
    updateConnectedEditors();
    updateVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimFaultReactivationModel::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;

    std::vector<cvf::Vec3d> line;
    for ( const RimPolylineTarget* target : m_targets )
    {
        line.push_back( target->targetPointXYZ() );
    }
    pld->setPolyLine( line );

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFaultReactivationModel::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_fault )
    {
        if ( m_fault() != nullptr )
        {
            auto coll = m_fault->firstAncestorOrThisOfType<RimFaultInViewCollection>();
            if ( coll != nullptr ) RimTools::faultOptionItems( &options, coll );
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        RimTimeStepFilter::timeStepOptions( options, &m_selectedTimeSteps, m_availableTimeSteps, m_selectedTimeSteps, m_timeStepFilter() );
    }
    else if ( fieldNeedingOptions == &m_geomechCase )
    {
        RimTools::geoMechCaseOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFaultReactivationModelPartMgr* RimFaultReactivationModel::partMgr()
{
    if ( m_partMgr.isNull() ) m_partMgr = new RivFaultReactivationModelPartMgr( this );

    return m_partMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigFaultReactivationModel> RimFaultReactivationModel::model() const
{
    return m_2Dmodel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::showModel() const
{
    return m_showModelPlane;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( &m_userDescription );
    genGrp->add( &m_fault );
    genGrp->add( &m_baseDir );
    genGrp->add( &m_geomechCase );

    auto modelGrp = uiOrdering.addNewGroup( "2D Model" );
    modelGrp->add( &m_showModelPlane );

    auto sizeModelGrp = modelGrp->addNewGroup( "Size" );
    sizeModelGrp->add( &m_modelExtentFromAnchor );
    sizeModelGrp->add( &m_modelMinZ );
    sizeModelGrp->add( &m_modelBelowSize );

    const bool hasGeoMechCase = ( m_geomechCase() != nullptr );

    if ( hasGeoMechCase )
    {
        m_modelMinZ.setValue( std::abs( m_geomechCase->allCellsBoundingBox().max().z() ) );
        m_modelMinZ.uiCapability()->setUiReadOnly( true );
    }
    else
    {
        m_modelMinZ.uiCapability()->setUiReadOnly( false );
    }

    auto faultGrp = modelGrp->addNewGroup( "Fault Extension" );
    faultGrp->add( &m_faultExtendUpwards );
    faultGrp->add( &m_faultExtendDownwards );

    auto gridModelGrp = modelGrp->addNewGroup( "Grid Definition" );
    gridModelGrp->add( &m_maxReservoirCellHeight );
    gridModelGrp->add( &m_cellHeightGrowFactor );

    gridModelGrp->add( &m_minReservoirCellWidth );
    gridModelGrp->add( &m_cellWidthGrowFactor );

    gridModelGrp->add( &m_modelThickness );

    auto appModelGrp = modelGrp->addNewGroup( "Appearance" );
    appModelGrp->setCollapsedByDefault();
    appModelGrp->add( &m_modelPart1Color );
    appModelGrp->add( &m_modelPart2Color );

    auto timeStepGrp = uiOrdering.addNewGroup( "Time Steps" );
    timeStepGrp->add( &m_timeStepFilter );
    timeStepGrp->add( &m_selectedTimeSteps );

    auto propertiesGrp = uiOrdering.addNewGroup( "Export" );
    propertiesGrp->add( &m_useLocalCoordinates );
    propertiesGrp->add( &m_useGridPorePressure );
    propertiesGrp->add( &m_useGridVoidRatio );
    propertiesGrp->add( &m_useGridTemperature );

    if ( hasGeoMechCase )
    {
        propertiesGrp->add( &m_useGridDensity );
        propertiesGrp->add( &m_useGridElasticProperties );
        propertiesGrp->add( &m_useGridStress );
        propertiesGrp->add( &m_waterDensity );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( ( changedField == &m_useGridPorePressure ) || ( changedField == &m_useGridVoidRatio ) || ( changedField == &m_useGridTemperature ) ||
         ( changedField == &m_useGridDensity ) || ( changedField == &m_useGridElasticProperties ) )
    {
        return; // do nothing
    }
    else if ( changedField == &m_userDescription )
    {
        updateConnectedEditors();
    }
    else
    {
        if ( changedField == &m_geomechCase )
        {
            if ( m_geomechCase() != nullptr )
            {
                m_modelMinZ.setValue( std::abs( m_geomechCase()->allCellsBoundingBox().max().z() ) );
            }
        }

        updateVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_targets )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;
        }
    }
    else if ( ( field == &m_faultExtendUpwards ) || ( field == &m_faultExtendDownwards ) )
    {
        auto* attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( attr )
        {
            auto eclCase = eclipseCase();
            if ( eclCase )
            {
                auto   bb       = eclCase->allCellsBoundingBox();
                double diff     = bb.max().z() - bb.min().z();
                attr->m_minimum = 0;
                attr->m_maximum = diff;
            }
            else
            {
                attr->m_minimum = 0;
                attr->m_maximum = 1000;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimFaultReactivationModel::eclipseCase() const
{
    auto eCase = firstAncestorOrThisOfType<RimEclipseCase>();

    if ( eCase != nullptr )
        return eCase;
    else
        return dynamic_cast<RimEclipseCase*>( RiaApplication::instance()->activeGridView()->ownerCase() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimFaultReactivationModel::geoMechCase() const
{
    return m_geomechCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::setBaseDir( QString path )
{
    m_baseDir = path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::baseDir() const
{
    return m_baseDir().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationModel::updateTimeSteps()
{
    m_availableTimeSteps.clear();
    const auto eCase = eclipseCase();
    if ( eCase != nullptr ) m_availableTimeSteps = eCase->timeStepDates();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimFaultReactivationModel::selectedTimeSteps() const
{
    std::vector<QDateTime> dates;
    for ( auto d : m_selectedTimeSteps() )
        dates.push_back( d );

    // selected dates might come in the order they were selected, sort them
    std::sort( dates.begin(), dates.end() );
    return dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimFaultReactivationModel::selectedTimeStepIndexes() const
{
    std::vector<size_t> selectedTimeStepIndexes;
    for ( auto& timeStep : selectedTimeSteps() )
    {
        auto idx = std::find( m_availableTimeSteps.begin(), m_availableTimeSteps.end(), timeStep );
        if ( idx == m_availableTimeSteps.end() ) return {};

        selectedTimeStepIndexes.push_back( idx - m_availableTimeSteps.begin() );
    }
    return selectedTimeStepIndexes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimFaultReactivationModel::commandParameters() const
{
    QStringList retlist;

    retlist << baseDir();
    retlist << inputFilename();
    retlist << outputOdbFilename();

    return retlist;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::outputOdbFilename() const
{
    QDir directory( baseDir() );
    return directory.absoluteFilePath( baseFilename() + ".odb" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::inputFilename() const
{
    QDir directory( baseDir() );
    return directory.absoluteFilePath( baseFilename() + ".inp" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::settingsFilename() const
{
    QDir directory( baseDir() );
    return directory.absoluteFilePath( baseFilename() + ".settings.json" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultReactivationModel::baseFilename() const
{
    QString tmp = m_userDescription();

    if ( tmp.isEmpty() ) return "faultReactivation";

    tmp.replace( ' ', '_' );
    tmp.replace( '/', '_' );
    tmp.replace( '\\', '_' );
    tmp.replace( ':', '_' );

    return tmp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, 3> RimFaultReactivationModel::materialParameters( ElementSets elementSet ) const
{
    std::array<double, 3>                     retVal   = { 0.0, 0.0, 0.0 };
    static std::map<ElementSets, std::string> groupMap = { { ElementSets::OverBurden, "material_overburden" },
                                                           { ElementSets::Reservoir, "material_reservoir" },
                                                           { ElementSets::IntraReservoir, "material_intrareservoir" },
                                                           { ElementSets::UnderBurden, "material_underburden" } };

    auto keyName = QString::fromStdString( groupMap[elementSet] );

    for ( auto& grp : m_materialParameters )
    {
        if ( grp->name() != keyName ) continue;

        retVal[0] = grp->parameterDoubleValue( "youngs_modulus", 0.0 );
        retVal[1] = grp->parameterDoubleValue( "poissons_number", 0.0 );
        retVal[2] = grp->parameterDoubleValue( "density", 0.0 );

        break;
    }

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::useGridVoidRatio() const
{
    return m_useGridVoidRatio();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::useGridPorePressure() const
{
    return m_useGridPorePressure();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::useGridTemperature() const
{
    return m_useGridTemperature();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::useGridDensity() const
{
    return m_useGridDensity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::useGridElasticProperties() const
{
    return m_useGridElasticProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationModel::useGridStress() const
{
    return m_useGridStress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationModel::seaBedDepth() const
{
    return m_modelMinZ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationModel::waterDensity() const
{
    return m_waterDensity;
}
