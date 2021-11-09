/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RifParameterXmlReader.h"

#include "RimDoubleParameter.h"
#include "RimGenericParameter.h"
#include "RimIntegerParameter.h"
#include "RimListParameter.h"
#include "RimParameterList.h"
#include "RimStringParameter.h"

#include "RimParameterGroup.h"

#include <QFile>
#include <QXmlStreamReader>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifParameterXmlReader::RifParameterXmlReader( QString filename )
    : m_filename( filename )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifParameterXmlReader::~RifParameterXmlReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericParameter* getParameterFromTypeStr( QString typestr )
{
    // check that we have a type we support
    if ( typestr == "float" )
    {
        return new RimDoubleParameter();
    }
    else if ( typestr == "int" )
    {
        return new RimIntegerParameter();
    }
    else if ( typestr == "string" )
    {
        return new RimStringParameter();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifParameterXmlReader::parseFile( QString& outErrorText )
{
    m_parameters.clear();

    outErrorText = "XML read error from file " + m_filename + " : ";

    QFile dataFile( m_filename );
    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        outErrorText += "Could not open file.";
        return false;
    }

    QXmlStreamReader xml;
    xml.setDevice( &dataFile );
    QString parameter;
    QString unit;

    RimParameterGroup* group = nullptr;

    std::list<QString> reqGroupAttrs = { QString( "name" ) };
    std::list<QString> reqListAttrs  = { QString( "name" ) };
    std::list<QString> reqParamAttrs = { QString( "name" ), QString( "label" ), QString( "type" ) };

    bool              bResult     = true;
    RimParameterList* currentList = nullptr;

    while ( !xml.atEnd() )
    {
        if ( xml.readNext() )
        {
            if ( xml.isStartElement() )
            {
                if ( xml.name() == "group" )
                {
                    // check that we have the required attributes
                    for ( auto& reqattr : reqGroupAttrs )
                    {
                        if ( !xml.attributes().hasAttribute( reqattr ) )
                        {
                            outErrorText += "Missing required attribute \"" + reqattr + "\" for a parameter group.";
                            bResult = false;
                            break;
                        }
                    }
                    if ( !bResult ) break;

                    group = new RimParameterGroup();
                    if ( xml.attributes().hasAttribute( "name" ) )
                    {
                        group->setName( xml.attributes().value( "name" ).toString() );
                    }
                    if ( xml.attributes().hasAttribute( "label" ) )
                    {
                        group->setLabel( xml.attributes().value( "label" ).toString() );
                    }
                    if ( xml.attributes().hasAttribute( "expanded" ) )
                    {
                        group->setExpanded( xml.attributes().value( "expanded" ).toString().toLower() == "true" );
                    }
                    if ( xml.attributes().hasAttribute( "comment" ) )
                    {
                        group->setComment( xml.attributes().value( "comment" ).toString() );
                    }
                    continue;
                }
                else if ( xml.name() == "parameter" )
                {
                    if ( group == nullptr ) continue;

                    // check that we have the required attributes
                    for ( auto& reqattr : reqParamAttrs )
                    {
                        if ( !xml.attributes().hasAttribute( reqattr ) )
                        {
                            outErrorText += "Missing required attribute \"" + reqattr + "\" for a parameter.";
                            bResult = false;
                            break;
                        }
                    }
                    if ( !bResult ) break;

                    // get a parameter of the required type
                    QString paramtypestr = xml.attributes().value( "type" ).toString().toLower();

                    RimGenericParameter* parameter = getParameterFromTypeStr( paramtypestr );
                    if ( parameter == nullptr )
                    {
                        outErrorText += "Unsupported parameter type found: " + paramtypestr;
                        bResult = false;
                        break;
                    }

                    parameter->setName( xml.attributes().value( "name" ).toString() );
                    parameter->setLabel( xml.attributes().value( "label" ).toString() );
                    parameter->setAdvanced( false );

                    if ( xml.attributes().hasAttribute( "advanced" ) )
                    {
                        if ( xml.attributes().value( "advanced" ).toString().toLower() == "true" )
                            parameter->setAdvanced( true );
                    }

                    if ( xml.attributes().hasAttribute( "description" ) )
                    {
                        parameter->setDescription( xml.attributes().value( "description" ).toString() );
                    }

                    parameter->setValue( xml.readElementText().trimmed() );
                    if ( !parameter->isValid() )
                    {
                        outErrorText += "Invalid parameter value found for parameter: " + parameter->name();
                        delete parameter;
                        bResult = false;
                        break;
                    }

                    group->addParameter( parameter );
                    if ( currentList )
                    {
                        currentList->addParameter( parameter->name() );
                    }
                }
                else if ( xml.name() == "list" )
                {
                    // check that we have the required attributes
                    for ( auto& reqattr : reqListAttrs )
                    {
                        if ( !xml.attributes().hasAttribute( reqattr ) )
                        {
                            outErrorText += "Missing required attribute \"" + reqattr + "\" for a list parameter.";
                            bResult = false;
                            break;
                        }
                    }
                    if ( !bResult ) break;

                    currentList = new RimParameterList();
                    currentList->setName( xml.attributes().value( "name" ).toString() );
                    currentList->setLabel( xml.attributes().value( "label" ).toString() );
                }
            }
            else if ( xml.isEndElement() )
            {
                if ( xml.name() == "group" )
                {
                    if ( group != nullptr )
                    {
                        m_parameters.push_back( group );
                        group = nullptr;
                    }
                }
                else if ( xml.name() == "list" )
                {
                    if ( group )
                    {
                        group->addList( currentList );
                    }
                    currentList = nullptr;
                }
            }
        }
    }

    dataFile.close();

    if ( bResult ) outErrorText = "";

    return bResult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<RimParameterGroup*>& RifParameterXmlReader::parameterGroups()
{
    return m_parameters;
}
