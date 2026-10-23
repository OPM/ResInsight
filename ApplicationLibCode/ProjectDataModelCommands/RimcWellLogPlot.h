/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimWellLogPlot.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimWellPath;
class RimWellLogTrack;

//==================================================================================================
///
//==================================================================================================
class RimcWellLogPlot_newWellLogTrack : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellLogPlot_newWellLogTrack( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

    static RimWellLogTrack*
        createWellLogTrack( RimWellLogPlot* wellLogPlot, RimEclipseCase* eclipseCase, RimWellPath* wellPath, const QString& title );

private:
    caf::PdmField<QString>            m_title;
    caf::PdmPtrField<RimEclipseCase*> m_case;
    caf::PdmPtrField<RimWellPath*>    m_wellPath;
};

//==================================================================================================
/// Export the curves of the well log plot to LAS files. Returns the exported file names.
//==================================================================================================
class RimWellLogPlot_exportDataAsLas : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogPlot_exportDataAsLas( caf::PdmObjectHandle* self );

    void setExportFolder( const QString& exportFolder );
    void setFilePrefix( const QString& filePrefix );
    void setExportTvdRkb( bool enable );
    void setCapitalizeFileNames( bool enable );
    void setResampleInterval( double interval );
    void setConvertToStandardUnits( bool enable );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_exportFolder;
    caf::PdmField<QString> m_filePrefix;
    caf::PdmField<bool>    m_exportTvdRkb;
    caf::PdmField<bool>    m_capitalizeFileNames;
    caf::PdmField<double>  m_resampleInterval;
    caf::PdmField<bool>    m_convertToStandardUnits;
};

//==================================================================================================
/// Export the curves of the well log plot to a single ASCII file. Returns the exported file name.
//==================================================================================================
class RimWellLogPlot_exportDataAsAscii : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogPlot_exportDataAsAscii( caf::PdmObjectHandle* self );

    void setExportFolder( const QString& exportFolder );
    void setFilePrefix( const QString& filePrefix );
    void setCapitalizeFileNames( bool enable );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_exportFolder;
    caf::PdmField<QString> m_filePrefix;
    caf::PdmField<bool>    m_capitalizeFileNames;
};
