/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGeoMechCase.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RicfCommandObject.h"
#include "RifOdbReader.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimFormationNames.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechContourMapViewCollection.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimIntersectionCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimTimeStepFilter.h"
#include "RimTools.h"
#include "RimWellLogPlotCollection.h"

#include "cafPdmFieldIOScriptability.h"
#include "cafPdmObjectScriptability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafUtils.h"

#include "cvfVector3.h"

#include <QFile>
#include <QIcon>

#include <array>

CAF_PDM_SOURCE_INIT( RimGeoMechCase, "ResInsightGeoMechCase" );

namespace caf
{
template <>
void caf::AppEnum<RimGeoMechCase::BiotCoefficientType>::setUp()
{
    addItem( RimGeoMechCase::BIOT_NONE, "BIOT_NONE", "None" );
    addItem( RimGeoMechCase::BIOT_FIXED, "BIOT_FIXED", "Fixed biot coefficient" );
    addItem( RimGeoMechCase::BIOT_PER_ELEMENT, "BIOT_PER_ELEMENT", "Biot coefficient from element properties" );
    setDefault( RimGeoMechCase::BIOT_NONE );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::RimGeoMechCase( void )
    : m_applyTimeFilter( false )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "GeoMechanical Case",
                                                    ":/GeoMechCase48x48.png",
                                                    "",
                                                    "The GeoMechanical Results Case",
                                                    "GeoMechCase",
                                                    "The Abaqus Based GeoMech Case" );

    CAF_PDM_InitScriptableFieldWithKeywordNoDefault( &geoMechViews,
                                                     "GeoMechViews",
                                                     "Views",
                                                     "",
                                                     "",
                                                     "",
                                                     "All GeoMech Views in the Case" );
    geoMechViews.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_cohesion, "CaseCohesion", 10.0, "Cohesion", "", "Used to calculate the SE:SFI result", "" );
    CAF_PDM_InitField( &m_frictionAngleDeg,
                       "FrctionAngleDeg",
                       30.0,
                       "Friction Angle [Deg]",
                       "",
                       "Used to calculate the SE:SFI result",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_elementPropertyFileNames, "ElementPropertyFileNames", "Element Property Files", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_elementPropertyFileNameIndexUiSelection,
                                "ElementPropertyFileNameIndexUiSelection",
                                "",
                                "",
                                "",
                                "" );
    m_elementPropertyFileNameIndexUiSelection.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_closeElementPropertyFileCommand, "closeElementPropertyFileCommad", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_closeElementPropertyFileCommand );

    CAF_PDM_InitField( &m_reloadElementPropertyFileCommand, "reloadElementPropertyFileCommand", false, "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_reloadElementPropertyFileCommand );

    caf::AppEnum<BiotCoefficientType> defaultBiotCoefficientType = RimGeoMechCase::BIOT_NONE;
    CAF_PDM_InitField( &m_biotCoefficientType, "BiotCoefficientType", defaultBiotCoefficientType, "Biot Coefficient", "", "", "" );
    CAF_PDM_InitField( &m_biotFixedCoefficient, "BiotFixedCoefficient", 1.0, "Fixed coefficient", "", "", "" );
    m_biotFixedCoefficient.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_biotResultAddress, "BiotResultAddress", QString( "" ), "Value", "", "", "" );
    m_biotResultAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_contourMapCollection, "ContourMaps", "2d Contour Maps", "", "", "" );
    m_contourMapCollection = new RimGeoMechContourMapViewCollection;
    m_contourMapCollection.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::~RimGeoMechCase( void )
{
    geoMechViews.deleteAllChildObjects();

    RimProject* project = RiaApplication::instance()->project();
    if ( project )
    {
        if ( project->mainPlotCollection() )
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if ( plotCollection )
            {
                plotCollection->removeExtractors( this->geoMechData() );
            }
        }
    }

    if ( this->geoMechData() )
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT( this->geoMechData()->refCount() == 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData* RimGeoMechCase::geoMechData()
{
    return m_geoMechCaseData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGeoMechCaseData* RimGeoMechCase::geoMechData() const
{
    return m_geoMechCaseData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::reloadDataAndUpdate()
{
    if ( this->geoMechData() )
    {
        m_geoMechCaseData = nullptr;
        std::string errMsg;
        if ( this->openGeoMechCase( &errMsg ) == CASE_OPEN_ERROR )
        {
            RiaLogging::error( QString::fromStdString( errMsg ) );
        }
        for ( auto v : geoMechViews() )
        {
            v->loadDataAndUpdate();
            v->setCurrentTimeStep( v->currentTimeStep() );
        }

        for ( RimGeoMechContourMapView* contourMap : m_contourMapCollection->views() )
        {
            CVF_ASSERT( contourMap );
            contourMap->loadDataAndUpdate();
            contourMap->updateGridBoxData();
            contourMap->updateAnnotationItems();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechCase::createAndAddReservoirView()
{
    RimGeoMechView* gmv = new RimGeoMechView();

    gmv->setGeoMechCase( this );

    geoMechViews.push_back( gmv );
    return gmv;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechCase::createCopyAndAddView( const RimGeoMechView* sourceView )
{
    RimGeoMechView* rimGeoMechView = dynamic_cast<RimGeoMechView*>(
        sourceView->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    CVF_ASSERT( rimGeoMechView );

    rimGeoMechView->setGeoMechCase( this );

    caf::PdmDocument::updateUiIconStateRecursively( rimGeoMechView );

    geoMechViews.push_back( rimGeoMechView );

    // Resolve references after reservoir view has been inserted into Rim structures
    rimGeoMechView->resolveReferencesRecursively();
    rimGeoMechView->initAfterReadRecursively();

    return rimGeoMechView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::CaseOpenStatus RimGeoMechCase::openGeoMechCase( std::string* errorMessage )
{
    // If read already, return
    if ( this->m_geoMechCaseData.notNull() ) return CASE_OPEN_OK;

    if ( !caf::Utils::fileExists( m_caseFileName().path() ) )
    {
        return CASE_OPEN_ERROR;
    }

    cvf::ref<RigGeoMechCaseData> geoMechCaseData = new RigGeoMechCaseData( m_caseFileName().path().toStdString() );
    bool                         fileOpenSuccess = geoMechCaseData->open( errorMessage );
    if ( !fileOpenSuccess )
    {
        return CASE_OPEN_ERROR;
    }

    std::vector<std::string> stepNames;
    if ( !geoMechCaseData->readTimeSteps( errorMessage, &stepNames ) )
    {
        return CASE_OPEN_ERROR;
    }

    std::vector<std::pair<QString, QDateTime>> timeSteps;
    for ( const std::string& timeStepStringStdString : stepNames )
    {
        QString timeStepString = QString::fromStdString( timeStepStringStdString );
        timeSteps.push_back( std::make_pair( timeStepString, dateTimeFromTimeStepString( timeStepString ) ) );
    }

    m_timeStepFilter->setTimeStepsFromFile( timeSteps );

    if ( m_applyTimeFilter )
    {
        m_applyTimeFilter = false; // Clear when we've done this once.

        caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                     m_timeStepFilter,
                                                     "Time Step Filter",
                                                     "",
                                                     QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        propertyDialog.resize( QSize( 400, 400 ) );

        // Push arrow cursor onto the cursor stack so it takes over from the wait cursor.
        QApplication::setOverrideCursor( QCursor( Qt::ArrowCursor ) );
        int propertyReturnValue = propertyDialog.exec();
        // Pop arrow cursor off the cursor stack so that the previous (wait) cursor takes over.
        QApplication::restoreOverrideCursor();
        if ( propertyReturnValue != QDialog::Accepted )
        {
            return CASE_OPEN_CANCELLED;
        }
        m_timeStepFilter->updateFilteredTimeStepsFromUi();
    }

    // Continue reading the open file
    if ( !geoMechCaseData->readFemParts( errorMessage, m_timeStepFilter->filteredTimeSteps() ) )
    {
        return CASE_OPEN_ERROR;
    }

    if ( activeFormationNames() )
    {
        geoMechCaseData->femPartResults()->setActiveFormationNames( activeFormationNames()->formationNamesData() );
    }
    else
    {
        geoMechCaseData->femPartResults()->setActiveFormationNames( nullptr );
    }

    std::vector<QString> fileNames;
    for ( const caf::FilePath& fileName : m_elementPropertyFileNames.v() )
    {
        fileNames.push_back( fileName.path() );
    }
    geoMechCaseData->femPartResults()->addElementPropertyFiles( fileNames );
    geoMechCaseData->femPartResults()->setCalculationParameters( m_cohesion, cvf::Math::toRadians( m_frictionAngleDeg() ) );

    m_geoMechCaseData = geoMechCaseData;

    return CASE_OPEN_OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    // No longer in use. Filepaths are now of type caf::FilePath, and updated in RimProject on load.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimGeoMechCase::allSpecialViews() const
{
    std::vector<Rim3dView*> views;
    for ( size_t vIdx = 0; vIdx < geoMechViews.size(); ++vIdx )
    {
        views.push_back( geoMechViews[vIdx] );
    }

    for ( RimGeoMechContourMapView* view : m_contourMapCollection->views() )
    {
        views.push_back( view );
    }

    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    std::vector<PdmObjectHandle*> children;
    geoMechViews.childObjects( &children );

    for ( auto child : children )
        uiTreeOrdering.add( child );

    if ( !m_2dIntersectionViewCollection->views().empty() )
    {
        uiTreeOrdering.add( &m_2dIntersectionViewCollection );
    }

    if ( !m_contourMapCollection->views().empty() )
    {
        uiTreeOrdering.add( &m_contourMapCollection );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechContourMapViewCollection* RimGeoMechCase::contourMapCollection()
{
    return m_contourMapCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimGeoMechCase::timeStepDates() const
{
    QStringList timeStrings = timeStepStrings();

    return RimGeoMechCase::vectorOfValidDateTimesFromTimeStepStrings( timeStrings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::initAfterRead()
{
    RimCase::initAfterRead();

    size_t j;
    for ( j = 0; j < geoMechViews().size(); j++ )
    {
        RimGeoMechView* riv = geoMechViews()[j];
        CVF_ASSERT( riv );

        riv->setGeoMechCase( this );
    }

    for ( RimGeoMechContourMapView* contourMap : m_contourMapCollection->views() )
    {
        contourMap->setGeoMechCase( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimGeoMechCase::timeStepStrings() const
{
    QStringList stringList;

    const RigGeoMechCaseData* rigCaseData = geoMechData();
    if ( rigCaseData && rigCaseData->femPartResults() )
    {
        std::vector<std::string> stepNames = rigCaseData->femPartResults()->filteredStepNames();
        for ( size_t i = 0; i < stepNames.size(); i++ )
        {
            stringList += QString::fromStdString( stepNames[i] );
        }
    }

    return stringList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::timeStepName( int frameIdx ) const
{
    const RigGeoMechCaseData* rigCaseData = geoMechData();
    if ( rigCaseData && rigCaseData->femPartResults() )
    {
        std::vector<std::string> stepNames = rigCaseData->femPartResults()->filteredStepNames();
        if ( frameIdx < static_cast<int>( stepNames.size() ) )
        {
            return QString::fromStdString( stepNames[frameIdx] );
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeoMechCase::reservoirBoundingBox()
{
    cvf::BoundingBox boundingBox;

    RigGeoMechCaseData* rigCaseData = this->geoMechData();
    if ( rigCaseData && rigCaseData->femPartResults() && rigCaseData->femParts()->part( 0 ) )
    {
        RigFemPart*           femPart     = rigCaseData->femParts()->part( 0 );
        const RigFemPartGrid* femPartGrid = femPart->getOrCreateStructGrid();

        RigFemResultAddress       porBarAddr( RigFemResultPosEnum::RIG_ELEMENT_NODAL, "POR-Bar", "" );
        const std::vector<float>& resultValues = rigCaseData->femPartResults()->resultValues( porBarAddr, 0, 0 );

        for ( int i = 0; i < femPart->elementCount(); ++i )
        {
            size_t resValueIdx = femPart->elementNodeResultIdx( (int)i, 0 );
            CVF_ASSERT( resValueIdx < resultValues.size() );
            double scalarValue   = resultValues[resValueIdx];
            bool   validPorValue = scalarValue != std::numeric_limits<double>::infinity();

            if ( validPorValue )
            {
                std::array<cvf::Vec3d, 8> hexCorners;
                femPartGrid->cellCornerVertices( i, hexCorners.data() );
                for ( size_t c = 0; c < 8; ++c )
                {
                    boundingBox.add( hexCorners[c] );
                }
            }
        }
    }
    return boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeoMechCase::activeCellsBoundingBox() const
{
    return allCellsBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimGeoMechCase::allCellsBoundingBox() const
{
    if ( m_geoMechCaseData.notNull() && m_geoMechCaseData->femParts() )
    {
        return m_geoMechCaseData->femParts()->boundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechCase::characteristicCellSize() const
{
    if ( geoMechData() && geoMechData()->femParts() )
    {
        double cellSize = geoMechData()->femParts()->characteristicElementSize();

        return cellSize;
    }

    return 10.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::addElementPropertyFiles( const std::vector<caf::FilePath>& fileNames )
{
    std::vector<QString> newFileNames;

    for ( const caf::FilePath& newFileNameToPossiblyAdd : fileNames )
    {
        bool fileAlreadyAdded = false;

        for ( const caf::FilePath& existingFileName : m_elementPropertyFileNames() )
        {
            if ( existingFileName == newFileNameToPossiblyAdd )
            {
                fileAlreadyAdded = true;
                break;
            }
        }
        if ( !fileAlreadyAdded )
        {
            newFileNames.push_back( newFileNameToPossiblyAdd.path() );
            m_elementPropertyFileNames.v().push_back( newFileNameToPossiblyAdd );
        }
    }

    this->updateConnectedEditors();

    if ( m_geoMechCaseData.notNull() )
    {
        geoMechData()->femPartResults()->addElementPropertyFiles( newFileNames );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechCase::cohesion() const
{
    return m_cohesion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechCase::frictionAngleDeg() const
{
    return m_frictionAngleDeg;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::BiotCoefficientType RimGeoMechCase::biotCoefficientType() const
{
    return m_biotCoefficientType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGeoMechCase::biotFixedCoefficient() const
{
    return m_biotFixedCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::biotResultAddress() const
{
    return m_biotResultAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::setApplyTimeFilter( bool applyTimeFilter )
{
    m_applyTimeFilter = applyTimeFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimGeoMechCase::displayModelOffset() const
{
    auto bb = this->allCellsBoundingBox();
    if ( bb.isValid() )
    {
        return this->allCellsBoundingBox().min();
    }

    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimGeoMechCase::vectorOfValidDateTimesFromTimeStepStrings( const QStringList& timeStepStrings )
{
    std::vector<QDateTime> dates;

    for ( const QString& timeStepString : timeStepStrings )
    {
        QDateTime dateTime = dateTimeFromTimeStepString( timeStepString );
        if ( dateTime.isValid() )
        {
            dates.push_back( dateTime );
        }
    }

    return dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimGeoMechCase::dateTimeFromTimeStepString( const QString& timeStepString )
{
    QString dateFormat = "yyyyMMdd";
    QString dateStr    = subStringOfDigits( timeStepString, dateFormat.size() );
    return QDateTime::fromString( dateStr, dateFormat );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                       const QVariant&            oldValue,
                                       const QVariant&            newValue )
{
    if ( changedField == &m_activeFormationNames )
    {
        updateFormationNamesData();
    }

    if ( changedField == &m_cohesion || changedField == &m_frictionAngleDeg )
    {
        RigGeoMechCaseData* rigCaseData = geoMechData();
        if ( rigCaseData && rigCaseData->femPartResults() )
        {
            rigCaseData->femPartResults()->setCalculationParameters( m_cohesion(),
                                                                     cvf::Math::toRadians( m_frictionAngleDeg() ) );
        }

        updateConnectedViews();
    }
    else if ( changedField == &m_biotFixedCoefficient || changedField == &m_biotCoefficientType ||
              changedField == &m_biotResultAddress )
    {
        RigGeoMechCaseData* rigCaseData = geoMechData();
        if ( rigCaseData && rigCaseData->femPartResults() )
        {
            if ( m_biotCoefficientType() == RimGeoMechCase::BIOT_NONE )
            {
                rigCaseData->femPartResults()->setBiotCoefficientParameters( 1.0, "" );
            }
            else if ( m_biotCoefficientType() == RimGeoMechCase::BIOT_FIXED )
            {
                rigCaseData->femPartResults()->setBiotCoefficientParameters( m_biotFixedCoefficient(), "" );
            }
            else if ( m_biotCoefficientType() == RimGeoMechCase::BIOT_PER_ELEMENT )
            {
                if ( changedField == &m_biotCoefficientType )
                {
                    // Show info message to user when selecting "from file" option before
                    // an element property has been imported
                    std::vector<std::string> elementProperties = possibleElementPropertyFieldNames();
                    if ( elementProperties.empty() )
                    {
                        QString importMessage =
                            QString( "Please import biot coefficients from file (typically called alpha.inp) by "
                                     "selecting 'Import Element Property Table' on the Geomechanical Model." );
                        RiaLogging::info( importMessage );
                        // Set back to default value
                        m_biotCoefficientType = RimGeoMechCase::BIOT_NONE;
                        return;
                    }
                }

                if ( biotResultAddress().isEmpty() )
                {
                    // Automatically select the first available property element if empty
                    std::vector<std::string> elementProperties = possibleElementPropertyFieldNames();
                    if ( !elementProperties.empty() )
                    {
                        m_biotResultAddress = QString::fromStdString( elementProperties[0] );
                    }
                }

                rigCaseData->femPartResults()->setBiotCoefficientParameters( 1.0, biotResultAddress() );
            }
        }

        updateConnectedViews();
    }
    else if ( changedField == &m_reloadElementPropertyFileCommand )
    {
        m_reloadElementPropertyFileCommand = false;
        reloadSelectedElementPropertyFiles();
        updateConnectedEditors();
    }
    else if ( changedField == &m_closeElementPropertyFileCommand )
    {
        m_closeElementPropertyFileCommand = false;
        closeSelectedElementPropertyFiles();
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::updateFormationNamesData()
{
    RigGeoMechCaseData* rigCaseData = geoMechData();
    if ( rigCaseData && rigCaseData->femPartResults() )
    {
        if ( activeFormationNames() )
        {
            rigCaseData->femPartResults()->setActiveFormationNames( activeFormationNames()->formationNamesData() );
        }
        else
        {
            rigCaseData->femPartResults()->setActiveFormationNames( nullptr );
        }

        std::vector<Rim3dView*> views = this->views();
        for ( Rim3dView* view : views )
        {
            RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>( view );

            if ( geomView && geomView->isUsingFormationNames() )
            {
                if ( !activeFormationNames() )
                {
                    if ( geomView->cellResult()->resultPositionType() == RIG_FORMATION_NAMES )
                    {
                        geomView->cellResult()->setResultAddress( RigFemResultAddress( RIG_FORMATION_NAMES, "", "" ) );
                        geomView->cellResult()->updateConnectedEditors();
                    }

                    RimGeoMechPropertyFilterCollection* eclFilColl = geomView->geoMechPropertyFilterCollection();
                    for ( RimGeoMechPropertyFilter* propFilter : eclFilColl->propertyFilters )
                    {
                        if ( propFilter->resultDefinition()->resultPositionType() == RIG_FORMATION_NAMES )
                        {
                            propFilter->resultDefinition()->setResultAddress(
                                RigFemResultAddress( RIG_FORMATION_NAMES, "", "" ) );
                        }
                    }
                }

                RimGeoMechPropertyFilterCollection* eclFilColl = geomView->geoMechPropertyFilterCollection();
                for ( RimGeoMechPropertyFilter* propFilter : eclFilColl->propertyFilters )
                {
                    if ( propFilter->resultDefinition->resultPositionType() == RIG_FORMATION_NAMES )
                    {
                        propFilter->setToDefaultValues();
                        propFilter->updateConnectedEditors();
                    }
                }

                geomView->cellResult()->updateConnectedEditors();

                view->scheduleGeometryRegen( PROPERTY_FILTERED );
                view->scheduleCreateDisplayModelAndRedraw();
                geomView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase::subStringOfDigits( const QString& inputString, int numberOfDigitsToFind )
{
    for ( int j = 0; j < inputString.size(); j++ )
    {
        if ( inputString.at( j ).isDigit() )
        {
            QString digitString;

            for ( int k = 0; k < numberOfDigitsToFind; k++ )
            {
                if ( j + k < inputString.size() && inputString.at( j + k ).isDigit() )
                {
                    digitString += inputString.at( j + k );
                }
            }

            if ( digitString.size() == numberOfDigitsToFind )
            {
                return digitString;
            }
        }
    }

    return "";
}

struct descendingComparator
{
    template <class T>
    bool operator()( T const& a, T const& b ) const
    {
        return a > b;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::closeSelectedElementPropertyFiles()
{
    std::sort( m_elementPropertyFileNameIndexUiSelection.v().begin(),
               m_elementPropertyFileNameIndexUiSelection.v().end(),
               descendingComparator() );

    std::vector<QString> filesToClose;

    for ( size_t idx : m_elementPropertyFileNameIndexUiSelection.v() )
    {
        filesToClose.push_back( m_elementPropertyFileNames.v().at( idx ).path() );
        m_elementPropertyFileNames.v().erase( m_elementPropertyFileNames.v().begin() + idx );
    }

    m_elementPropertyFileNameIndexUiSelection.v().clear();

    std::vector<RigFemResultAddress> addressesToDelete;

    if ( m_geoMechCaseData.notNull() )
    {
        addressesToDelete = geoMechData()->femPartResults()->removeElementPropertyFiles( filesToClose );
    }

    for ( RimGeoMechView* view : geoMechViews() )
    {
        for ( RigFemResultAddress address : addressesToDelete )
        {
            if ( address == view->cellResultResultDefinition()->resultAddress() )
            {
                view->cellResult()->setResultAddress( RigFemResultAddress() );
            }

            for ( RimGeoMechPropertyFilter* propertyFilter : view->geoMechPropertyFilterCollection()->propertyFilters() )
            {
                if ( address == propertyFilter->resultDefinition->resultAddress() )
                {
                    propertyFilter->resultDefinition->setResultAddress( RigFemResultAddress() );
                }
            }
        }

        view->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::reloadSelectedElementPropertyFiles()
{
    std::vector<QString> filesToReload;

    for ( size_t idx : m_elementPropertyFileNameIndexUiSelection.v() )
    {
        filesToReload.push_back( m_elementPropertyFileNames.v().at( idx ).path() );
    }

    m_elementPropertyFileNameIndexUiSelection.v().clear();

    if ( m_geoMechCaseData.notNull() )
    {
        geoMechData()->femPartResults()->removeElementPropertyFiles( filesToReload );
        geoMechData()->femPartResults()->addElementPropertyFiles( filesToReload );
    }

    for ( RimGeoMechView* view : geoMechViews() )
    {
        view->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &caseUserDescription );
    uiOrdering.add( &caseId );
    uiOrdering.add( &m_caseFileName );

    caf::PdmUiGroup* caseGroup = uiOrdering.addNewGroup( "Case Options" );
    caseGroup->add( &m_activeFormationNames );
    caseGroup->add( &m_cohesion );
    caseGroup->add( &m_frictionAngleDeg );

    caf::PdmUiGroup* elmPropGroup = uiOrdering.addNewGroup( "Element Properties" );
    elmPropGroup->add( &m_elementPropertyFileNameIndexUiSelection );
    elmPropGroup->add( &m_reloadElementPropertyFileCommand );
    elmPropGroup->add( &m_closeElementPropertyFileCommand );

    caf::PdmUiGroup* biotGroup = uiOrdering.addNewGroup( "Biot Coefficient" );
    biotGroup->add( &m_biotCoefficientType );
    biotGroup->add( &m_biotFixedCoefficient );
    biotGroup->add( &m_biotResultAddress );
    m_biotFixedCoefficient.uiCapability()->setUiHidden( m_biotCoefficientType != BIOT_FIXED );
    m_biotResultAddress.uiCapability()->setUiHidden( m_biotCoefficientType != BIOT_PER_ELEMENT );

    caf::PdmUiGroup* timeStepFilterGroup = uiOrdering.addNewGroup( "Time Step Filter" );
    timeStepFilterGroup->setCollapsedByDefault( true );
    m_timeStepFilter->uiOrdering( uiConfigName, *timeStepFilterGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                            QString                    uiConfigName,
                                            caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_reloadElementPropertyFileCommand )
    {
        dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute )->m_buttonText = "Reload Case(s)";
    }
    if ( field == &m_closeElementPropertyFileCommand )
    {
        dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute )->m_buttonText = "Close Case(s)";
    }

    if ( field == &m_biotFixedCoefficient )
    {
        auto uiDoubleValueEditorAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( uiDoubleValueEditorAttr )
        {
            uiDoubleValueEditorAttr->m_decimals  = 2;
            uiDoubleValueEditorAttr->m_validator = new QDoubleValidator( 0.0, 1.0, 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                     bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimCase::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_elementPropertyFileNameIndexUiSelection )
    {
        for ( size_t i = 0; i < m_elementPropertyFileNames.v().size(); i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( m_elementPropertyFileNames.v().at( i ).path(), (int)i, true ) );
        }
    }
    else if ( fieldNeedingOptions == &m_biotResultAddress )
    {
        std::vector<std::string> elementProperties = possibleElementPropertyFieldNames();
        for ( const std::string elementProperty : elementProperties )
        {
            QString result = QString::fromStdString( elementProperty );
            options.push_back( caf::PdmOptionItemInfo( result, result ) );
        }
    }

    return options;
}

void RimGeoMechCase::updateConnectedViews()
{
    std::vector<Rim3dView*> views = this->views();
    for ( Rim3dView* view : views )
    {
        if ( view )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

std::vector<std::string> RimGeoMechCase::possibleElementPropertyFieldNames()
{
    std::vector<std::string> fieldNames;

    if ( geoMechData() )
    {
        std::map<std::string, std::vector<std::string>> fieldWithComponentNames =
            geoMechData()->femPartResults()->scalarFieldAndComponentNames( RIG_ELEMENT );

        std::map<std::string, std::vector<std::string>>::const_iterator fieldIt;
        for ( fieldIt = fieldWithComponentNames.begin(); fieldIt != fieldWithComponentNames.end(); ++fieldIt )
        {
            fieldNames.push_back( fieldIt->first );
        }
    }
    return fieldNames;
}
