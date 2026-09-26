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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"

#include <QString>

#include <memory>

class Rim3dView;
class RimIdenticalGridCaseGroup;
class RimFormationNames;
class RimCase;

//==================================================================================================
///
//==================================================================================================
class RimProject_importSummaryCase : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_importSummaryCase( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_fileName;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_summaryCase : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_summaryCase( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<int> m_caseId;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_surfaceFolder : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_surfaceFolder( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_folderName;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_createGridFromKeyValues : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_createGridFromKeyValues( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<int>     m_nx;
    caf::PdmField<int>     m_ny;
    caf::PdmField<int>     m_nz;
    caf::PdmField<QString> m_coordKey;
    caf::PdmField<QString> m_zcornKey;
    caf::PdmField<QString> m_actnumKey;
};

//==================================================================================================
/// Create a generic (case-less) 3D view for showing surfaces, polygons and well paths without a grid case.
//==================================================================================================
class RimProject_createGenericView : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_createGenericView( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_name;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_wellPathCollection : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_wellPathCollection( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_valveTemplates : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_valveTemplates( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_tileViews : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_tileViews( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_linkViews : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_linkViews( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrArrayField<Rim3dView*> m_views;
};

//==================================================================================================
///
//==================================================================================================
class RimProject_unlinkViews : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_unlinkViews( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrArrayField<Rim3dView*> m_views;
};

//==================================================================================================
/// Export snapshots of all 3D views and/or plots in the project.
//==================================================================================================
class RimProject_exportSnapshots : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_exportSnapshots( caf::PdmObjectHandle* self );

    void setContentType( RiaDefines::SnapshotContentType contentType );
    void setExportFolder( const QString& exportFolder );
    void setPrefix( const QString& prefix );
    void setWidth( int width );
    void setHeight( int height );
    void setPlotFileFormat( RiaDefines::SnapshotFileFormat fileFormat );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<caf::AppEnum<RiaDefines::SnapshotContentType>> m_contentType;
    caf::PdmField<QString>                                       m_exportFolder;
    caf::PdmField<QString>                                       m_prefix;
    caf::PdmField<int>                                           m_width;
    caf::PdmField<int>                                           m_height;
    caf::PdmField<caf::AppEnum<RiaDefines::SnapshotFileFormat>>  m_plotFileFormat;
};

//==================================================================================================
/// Load a grid case (EGRID, GRID, GRDECL, ROFF) from file and add it to the project.
//==================================================================================================
class RimProject_loadCase : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_loadCase( caf::PdmObjectHandle* self );

    void setPath( const QString& path );
    void setGridOnly( bool gridOnly );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_path;
    caf::PdmField<bool>    m_gridOnly;
};

//==================================================================================================
/// Create a grid case group (for statistics) from a list of grid files with identical grids.
//==================================================================================================
class RimProject_createGridCaseGroup : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_createGridCaseGroup( caf::PdmObjectHandle* self );

    void setCasePaths( const std::vector<QString>& casePaths );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<QString>> m_casePaths;
};

//==================================================================================================
/// Import formation names from one or more files (.lyr, .fmu, ...). Returns the created formation names
/// object, which can be assigned to cases with Case.set_formation_names().
//==================================================================================================
class RimProject_importFormationNames : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_importFormationNames( caf::PdmObjectHandle* self );

    void setFormationFiles( const std::vector<QString>& formationFiles );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<QString>> m_formationFiles;
};

//==================================================================================================
/// Run an Octave script for a set of cases. The script is run once per case with the case set as
/// current. Empty case list means all Eclipse cases in the project.
//==================================================================================================
class RimProject_runOctaveScript : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimProject_runOctaveScript( caf::PdmObjectHandle* self );

    void setPath( const QString& path );
    void setCases( const std::vector<RimCase*>& cases );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString>          m_path;
    caf::PdmPtrArrayField<RimCase*> m_cases;
};
