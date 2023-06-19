/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023    Equinor ASA
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

#include "RimSEGYConvertOptions.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiLineEditor.h"

#include <QCoreApplication>

CAF_PDM_SOURCE_INIT( RimSEGYConvertOptions, "RimSEGYConvertOptions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSEGYConvertOptions::RimSEGYConvertOptions()
{
    CAF_PDM_InitObject( "SEG-Y convert options", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_inputFilename, "InputFilename", "Input SEG-Y File" );
    CAF_PDM_InitFieldNoDefault( &m_outputFilename, "OutputFilename", "Output VDS File" );

    CAF_PDM_InitField( &m_sampleStartOverride, "SampleStartOverride", std::make_pair( false, 0.0 ), "Depth (Z) Offset Override" );
    CAF_PDM_InitField( &m_sampleUnit, "SampleUnit", QString( "m" ), "Depth (Z) Unit" );

    CAF_PDM_InitFieldNoDefault( &m_headerFormatFilename, "HeaderFilename", "Header Definition File (optional)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSEGYConvertOptions::~RimSEGYConvertOptions()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSEGYConvertOptions::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSEGYConvertOptions::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto fileGrp = uiOrdering.addNewGroup( "Input/Output Files" );
    fileGrp->add( &m_inputFilename );
    fileGrp->add( &m_outputFilename );

    auto convGrp = uiOrdering.addNewGroup( "Convert Options" );
    convGrp->add( &m_sampleUnit );
    convGrp->add( &m_sampleStartOverride );
    convGrp->add( &m_headerFormatFilename );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSEGYConvertOptions::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_outputFilename )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSEGYConvertOptions::inputFilename() const
{
    return m_inputFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSEGYConvertOptions::setInputFilename( QString filename )
{
    m_inputFilename = filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSEGYConvertOptions::outputFilename() const
{
    return m_outputFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSEGYConvertOptions::setOutputFilename( QString filename )
{
    m_outputFilename = filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSEGYConvertOptions::headerFormatFilename() const
{
    return m_headerFormatFilename().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, double> RimSEGYConvertOptions::sampleStartOverride() const
{
    return m_sampleStartOverride();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSEGYConvertOptions::programDirectory() const
{
    return QCoreApplication::applicationDirPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSEGYConvertOptions::convertCommand() const
{
    return QString( "%1/%2" ).arg( programDirectory() ).arg( "SEGYImport" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimSEGYConvertOptions::convertCommandParameters() const
{
    QStringList retVal;

    retVal.append( m_inputFilename().path() );

    auto& [overrideSampleStart, overrideValue] = m_sampleStartOverride();
    if ( overrideSampleStart )
    {
        retVal.append( "--sample-start" );
        retVal.append( QString::number( overrideValue ) );
    }

    if ( !m_headerFormatFilename().path().isEmpty() )
    {
        retVal.append( "--header-format" );
        retVal.append( m_headerFormatFilename().path() );
    }

    if ( !m_sampleUnit().isEmpty() )
    {
        retVal.append( "--sample-unit" );
        retVal.append( m_sampleUnit() );
    }

    retVal.append( "--vdsfile" );
    retVal.append( m_outputFilename().path() );

    retVal.append( "--quiet" );

    return retVal;
}
