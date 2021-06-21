/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////
#include "RiaGrpcServiceInterface.h"

#include "RimCase.h"
#include "RimProject.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmObjectScriptingCapabilityRegister.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmScriptIOMessages.h"
#include "cafPdmXmlFieldHandle.h"

#include <grpcpp/grpcpp.h>

#include <PdmObject.pb.h>
#include <QXmlStreamReader>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RiaGrpcServiceInterface::findCase( int caseId )
{
    std::vector<RimCase*> cases;
    RimProject::current()->allCases( cases );

    for ( RimCase* rimCase : cases )
    {
        if ( caseId == rimCase->caseId() )
        {
            return rimCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Find the number of data items that will fit in the given bytes.
/// The default argument for numBytesWantedInPackage is meant to be a sensible size for GRPC.
//--------------------------------------------------------------------------------------------------
size_t RiaGrpcServiceInterface::numberOfDataUnitsInPackage( size_t dataUnitSize, size_t packageByteCount /*= 64 * 1024u*/ )
{
    size_t dataUnitCount = packageByteCount / dataUnitSize;
    return dataUnitCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServiceInterface::copyPdmObjectFromCafToRips( const caf::PdmObjectHandle* source, rips::PdmObject* destination )
{
    CAF_ASSERT( source && destination && source->xmlCapability() );

    QString classKeyword = source->xmlCapability()->classKeyword();
    QString scriptName   = caf::PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( classKeyword );
    destination->set_class_keyword( scriptName.toStdString() );
    destination->set_address( reinterpret_cast<uint64_t>( source ) );

    bool visible = true;
    if ( source->uiCapability() && source->uiCapability()->objectToggleField() )
    {
        const caf::PdmField<bool>* boolField =
            dynamic_cast<const caf::PdmField<bool>*>( source->uiCapability()->objectToggleField() );
        if ( boolField )
        {
            visible = boolField->value();
        }
    }
    destination->set_visible( visible );

    std::vector<caf::PdmFieldHandle*> fields;
    source->fields( fields );

    auto parametersMap = destination->mutable_parameters();
    for ( auto field : fields )
    {
        auto pdmValueField = dynamic_cast<const caf::PdmValueField*>( field );
        if ( pdmValueField )
        {
            QString keyword    = pdmValueField->keyword();
            auto    ricfHandle = field->template capability<caf::PdmAbstractFieldScriptingCapability>();
            if ( ricfHandle != nullptr )
            {
                auto pdmProxyField = dynamic_cast<const caf::PdmProxyFieldHandle*>( field );
                if ( !( pdmProxyField && pdmProxyField->isStreamingField() ) )
                {
                    QString     text;
                    QTextStream outStream( &text );
                    ricfHandle->readFromField( outStream, false );
                    ( *parametersMap )[ricfHandle->scriptFieldName().toStdString()] = text.toStdString();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServiceInterface::copyPdmObjectFromRipsToCaf( const rips::PdmObject* source, caf::PdmObjectHandle* destination )
{
    CAF_ASSERT( source && destination && destination->xmlCapability() );

    if ( destination->uiCapability() && destination->uiCapability()->objectToggleField() )
    {
        caf::PdmField<bool>* boolField =
            dynamic_cast<caf::PdmField<bool>*>( destination->uiCapability()->objectToggleField() );
        if ( boolField )
        {
            QVariant oldValue = boolField->toQVariant();
            boolField->setValue( source->visible() );
            QVariant newValue = boolField->toQVariant();
            destination->uiCapability()->fieldChangedByUi( boolField, oldValue, newValue );
        }
    }

    std::vector<caf::PdmFieldHandle*> fields;
    destination->fields( fields );

    auto parametersMap = source->parameters();
    for ( auto field : fields )
    {
        auto scriptability = field->template capability<caf::PdmAbstractFieldScriptingCapability>();
        if ( scriptability )
        {
            if ( !dynamic_cast<caf::PdmValueField*>( field ) )
            {
                // Recursive object update is not supported
                // https://github.com/OPM/ResInsight/issues/7794
                continue;
            }
            QString keyword = scriptability->scriptFieldName();
            QString value   = QString::fromStdString( parametersMap[keyword.toStdString()] );

            QVariant oldValue, newValue;
            if ( assignFieldValue( value, field, &oldValue, &newValue ) )
            {
                destination->uiCapability()->fieldChangedByUi( field, oldValue, newValue );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGrpcServiceInterface::assignFieldValue( const QString&       stringValue,
                                                caf::PdmFieldHandle* field,
                                                QVariant*            oldValue,
                                                QVariant*            newValue )
{
    CAF_ASSERT( oldValue && newValue );

    auto scriptability = field->template capability<caf::PdmAbstractFieldScriptingCapability>();
    if ( field && scriptability != nullptr )
    {
        caf::PdmValueField*      valueField = dynamic_cast<caf::PdmValueField*>( field );
        QTextStream              stream( stringValue.toLatin1() );
        caf::PdmScriptIOMessages messages;
        if ( valueField ) *oldValue = valueField->toQVariant();
        scriptability->writeToField( stream, nullptr, &messages, false, RimProject::current() );
        if ( valueField ) *newValue = valueField->toQVariant();
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RiaGrpcServiceInterface::emplaceChildField( caf::PdmObject* parent,
                                                                  const QString&  fieldKeyword,
                                                                  const QString&  keywordForClassToCreate )
{
    std::vector<caf::PdmFieldHandle*> fields;
    parent->fields( fields );

    for ( auto field : fields )
    {
        auto pdmChildArrayField = dynamic_cast<caf::PdmChildArrayFieldHandle*>( field );
        auto pdmChildField      = dynamic_cast<caf::PdmChildFieldHandle*>( field );
        if ( pdmChildArrayField )
        {
            bool isMatching = false;
            if ( fieldKeyword.isEmpty() )
            {
                // Use first child array field if no fieldKeyword is specified
                isMatching = true;
            }
            else
            {
                isMatching = ( pdmChildArrayField->keyword() == fieldKeyword );
            }

            if ( isMatching )
            {
                auto objectCreated = emplaceChildArrayField( pdmChildArrayField, keywordForClassToCreate );

                // Notify parent object that a new object has been created
                parent->onChildAdded( pdmChildArrayField );

                return objectCreated;
            }
        }
        else if ( pdmChildField && pdmChildField->keyword() == fieldKeyword )
        {
            return emplaceChildField( pdmChildField, keywordForClassToCreate );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RiaGrpcServiceInterface::emplaceChildField( caf::PdmChildFieldHandle* childField,
                                                                  const QString&            keywordForClassToCreate )
{
    QString childClassKeyword;
    if ( keywordForClassToCreate.isEmpty() )
    {
        childClassKeyword = childField->xmlCapability()->dataTypeName();
    }
    else
    {
        childClassKeyword = keywordForClassToCreate;
    }

    auto pdmObjectHandle = caf::PdmDefaultObjectFactory::instance()->create( childClassKeyword );
    CAF_ASSERT( pdmObjectHandle );
    childField->setChildObject( pdmObjectHandle );
    return pdmObjectHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RiaGrpcServiceInterface::emplaceChildArrayField( caf::PdmChildArrayFieldHandle* childArrayField,
                                                                       const QString& keywordForClassToCreate )
{
    QString childClassKeyword;
    if ( keywordForClassToCreate.isEmpty() )
    {
        childClassKeyword = childArrayField->xmlCapability()->dataTypeName();
    }
    else
    {
        childClassKeyword = keywordForClassToCreate;
    }

    auto pdmObjectHandle = caf::PdmDefaultObjectFactory::instance()->create( childClassKeyword );
    if ( !pdmObjectHandle ) return nullptr;

    childArrayField->insertAt( -1, pdmObjectHandle );

    return pdmObjectHandle;
}
