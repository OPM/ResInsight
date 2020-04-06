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

#pragma once

#include "RimCase.h"

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfObject.h"

#include <QDateTime>

class RimGeoMechView;
class RigGeoMechCaseData;
class RifGeoMechReaderInterface;
class RimGeoMechContourMapView;
class RimGeoMechContourMapViewCollection;

//==================================================================================================
///
///
//==================================================================================================
class RimGeoMechCase : public RimCase
{
    CAF_PDM_HEADER_INIT;

public:
    enum CaseOpenStatus
    {
        CASE_OPEN_OK = 0,
        CASE_OPEN_CANCELLED,
        CASE_OPEN_ERROR
    };

    enum BiotCoefficientType
    {
        BIOT_NONE = 0,
        BIOT_FIXED,
        BIOT_PER_ELEMENT
    };

    RimGeoMechCase( void );
    ~RimGeoMechCase( void ) override;

    CaseOpenStatus openGeoMechCase( std::string* errorMessage );

    RigGeoMechCaseData*       geoMechData();
    const RigGeoMechCaseData* geoMechData() const;

    RimGeoMechContourMapViewCollection* contourMapCollection();

    void reloadDataAndUpdate();

    RimGeoMechView* createAndAddReservoirView();
    RimGeoMechView* createCopyAndAddView( const RimGeoMechView* sourceView );

    void updateFilePathsFromProjectPath( const QString& projectPath, const QString& oldProjectPath ) override;

    std::vector<QDateTime> timeStepDates() const override;
    QStringList            timeStepStrings() const override;
    QString                timeStepName( int frameIdx ) const override;

    cvf::BoundingBox reservoirBoundingBox() override;
    cvf::BoundingBox activeCellsBoundingBox() const override;
    cvf::BoundingBox allCellsBoundingBox() const override;

    double characteristicCellSize() const override;

    void addElementPropertyFiles( const std::vector<caf::FilePath>& filenames );

    double cohesion() const;
    double frictionAngleDeg() const;

    void setApplyTimeFilter( bool applyTimeFilter );
    // Fields:
    caf::PdmChildArrayField<RimGeoMechView*> geoMechViews;

    BiotCoefficientType biotCoefficientType() const;
    double              biotFixedCoefficient() const;
    QString             biotResultAddress() const;

private:
    cvf::Vec3d                    displayModelOffset() const override;
    static std::vector<QDateTime> vectorOfValidDateTimesFromTimeStepStrings( const QStringList& timeStepStrings );
    static QDateTime              dateTimeFromTimeStepString( const QString& timeStepString );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    QString findFileNameForElementProperty( const std::string&                   elementProperty,
                                            const std::map<std::string, QString> addressesInFiles ) const;

    void updateFormationNamesData() override;

    void           initAfterRead() override;
    static QString subStringOfDigits( const QString& timeStepString, int numberOfDigitsToFind );

    void                     importElementPropertyFile();
    void                     closeSelectedElementPropertyFiles();
    void                     reloadSelectedElementPropertyFiles();
    std::vector<Rim3dView*>  allSpecialViews() const override;
    void                     updateConnectedViews();
    std::vector<std::string> possibleElementPropertyFieldNames();

private:
    cvf::ref<RigGeoMechCaseData>              m_geoMechCaseData;
    caf::PdmField<double>                     m_cohesion;
    caf::PdmField<double>                     m_frictionAngleDeg;
    caf::PdmField<std::vector<caf::FilePath>> m_elementPropertyFileNames;
    caf::PdmField<std::vector<int>>           m_elementPropertyFileNameIndexUiSelection;
    caf::PdmField<bool>                       m_importElementPropertyFileCommand;
    caf::PdmField<bool>                       m_closeElementPropertyFileCommand;
    caf::PdmField<bool>                       m_reloadElementPropertyFileCommand;

    caf::PdmField<caf::AppEnum<BiotCoefficientType>> m_biotCoefficientType;
    caf::PdmField<double>                            m_biotFixedCoefficient;
    caf::PdmField<QString>                           m_biotResultAddress;

    caf::PdmChildField<RimGeoMechContourMapViewCollection*> m_contourMapCollection;

    bool m_applyTimeFilter;
};
