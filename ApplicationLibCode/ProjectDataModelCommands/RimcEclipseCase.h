/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimCellFilter;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_importProperties : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_importProperties( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

    QString classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<QString>> m_fileNames;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_exportValuesInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_exportValuesInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_coordinateX;
    caf::PdmField<QString> m_coordinateY;
    caf::PdmField<QString> m_coordinateZ;
    caf::PdmField<QString> m_propertyType;
    caf::PdmField<QString> m_propertyName;
    caf::PdmField<int>     m_timeStep;
    caf::PdmField<QString> m_porosityModel;
    caf::PdmField<QString> m_resultKey;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_exportCornerPointGridInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_exportCornerPointGridInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_zcornKey;
    caf::PdmField<QString> m_coordKey;
    caf::PdmField<QString> m_actnumKey;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_addResultAlias : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_addResultAlias( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_resultName;
    caf::PdmField<QString> m_aliasName;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_clearResultAliases : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_clearResultAliases( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};

//==================================================================================================
/// Return the data type (one of caf::AppEnum<RiaDefines::ResultDataType>) of a grid property.
/// Inverse of the data_type flag passed to set_active_cell_property / set_grid_property.
//==================================================================================================
class RimcEclipseCase_propertyDataType : public caf::PdmEnumObjectMethod<RiaDefines::ResultDataType>
{
    CAF_PDM_HEADER_INIT;

public:
    explicit RimcEclipseCase_propertyDataType( caf::PdmObjectHandle* self );

    std::expected<RiaDefines::ResultDataType, QString> executeEnum() override;
    QString                                            returnEnumScriptName() const override { return "PropertyDataType"; }

private:
    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>>     m_propertyType;
    caf::PdmField<QString>                                     m_propertyName;
    caf::PdmField<caf::AppEnum<RiaDefines::PorosityModelType>> m_porosityModel;
};

//==================================================================================================
/// Apply a cell filter to this case for a given grid + time step and write a per-cell 0/1 mask
/// (1 = passes the filter, 0 = filtered out) to the key-value store under m_maskKey. The vector
/// length and ordering match case.grid_property(..., grid_index) so the two can be combined
/// element-wise.
//==================================================================================================
class RimcEclipseCase_filteredCellsInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_filteredCellsInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<RimCellFilter*> m_filter;
    caf::PdmField<QString>           m_maskKey;
    caf::PdmField<int>               m_timeStep;
    caf::PdmField<int>               m_gridIndex;
};

//==================================================================================================
/// Export a cell property (static or dynamic, matrix model) of the case to a GRDECL style text file.
//==================================================================================================
class RimEclipseCase_exportProperty : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseCase_exportProperty( caf::PdmObjectHandle* self );

    void setTimeStep( int timeStep );
    void setPropertyName( const QString& propertyName );
    void setEclipseKeyword( const QString& eclipseKeyword );
    void setUndefinedValue( double undefinedValue );
    void setExportFile( const QString& exportFile );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<int>     m_timeStep;
    caf::PdmField<QString> m_propertyName;
    caf::PdmField<QString> m_eclipseKeyword;
    caf::PdmField<double>  m_undefinedValue;
    caf::PdmField<QString> m_exportFile;
};

//==================================================================================================
/// Export completion data (COMPDAT, WELSPECS, MSW keywords etc.) for well paths in this case.
//==================================================================================================
class RimEclipseCase_exportCompletions : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseCase_exportCompletions( caf::PdmObjectHandle* self );

    void setWellPaths( const std::vector<RimWellPath*>& wellPaths );
    void setTimeStep( int timeStep );
    void setExportFolder( const QString& exportFolder );
    void setCustomFileName( const QString& customFileName );
    void setFileSplit( RicExportCompletionDataSettingsUi::ExportSplit fileSplit );
    void setCompdatExport( RicExportCompletionDataSettingsUi::CompdatExport compdatExport );
    void setIncludeMsw( bool enable );
    void setUseNtgHorizontally( bool enable );
    void setIncludePerforations( bool enable );
    void setIncludeFishbones( bool enable );
    void setIncludeFractures( bool enable );
    void setExcludeMainBoreForFishbones( bool enable );
    void setPerformTransScaling( bool enable );
    void setTransScalingTimeStep( int timeStep );
    void setTransScalingWbhpSource( RicExportFractureCompletionsImpl::PressureDepletionWBHPSource source );
    void setTransScalingWbhp( double wbhp );
    void setExportComments( bool enable );
    void setExportWelspec( bool enable );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrArrayField<RimWellPath*> m_wellPaths;
    caf::PdmField<int>                  m_timeStep;
    caf::PdmField<QString>              m_exportFolder;
    caf::PdmField<QString>              m_customFileName;

    caf::PdmField<RicExportCompletionDataSettingsUi::ExportSplitType>   m_fileSplit;
    caf::PdmField<RicExportCompletionDataSettingsUi::CompdatExportType> m_compdatExport;

    caf::PdmField<bool> m_includeMsw;
    caf::PdmField<bool> m_useNtgHorizontally;
    caf::PdmField<bool> m_includePerforations;
    caf::PdmField<bool> m_includeFishbones;
    caf::PdmField<bool> m_includeFractures;
    caf::PdmField<bool> m_excludeMainBoreForFishbones;

    caf::PdmField<bool>                                                      m_performTransScaling;
    caf::PdmField<int>                                                       m_transScalingTimeStep;
    caf::PdmField<RicExportCompletionDataSettingsUi::TransScalingWBHPSource> m_transScalingWbhpSource;
    caf::PdmField<double>                                                    m_transScalingWbhp;

    caf::PdmField<bool> m_exportComments;
    caf::PdmField<bool> m_exportWelspec;
};

//==================================================================================================
/// Export the Multi Segment Well (MSW) model keywords (WELSEGS, COMPSEGS, ...) for well paths in this case.
//==================================================================================================
class RimEclipseCase_exportMswCompletions : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseCase_exportMswCompletions( caf::PdmObjectHandle* self );

    void setWellPaths( const std::vector<RimWellPath*>& wellPaths );
    void setExportFolder( const QString& exportFolder );
    void setFileSplit( RicExportCompletionDataSettingsUi::ExportSplit fileSplit );
    void setIncludePerforations( bool enable );
    void setIncludeFishbones( bool enable );
    void setIncludeFractures( bool enable );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrArrayField<RimWellPath*>                               m_wellPaths;
    caf::PdmField<QString>                                            m_exportFolder;
    caf::PdmField<RicExportCompletionDataSettingsUi::ExportSplitType> m_fileSplit;
    caf::PdmField<bool>                                               m_includePerforations;
    caf::PdmField<bool>                                               m_includeFishbones;
    caf::PdmField<bool>                                               m_includeFractures;
};
