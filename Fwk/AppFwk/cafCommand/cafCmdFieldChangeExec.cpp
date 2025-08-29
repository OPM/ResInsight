//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafCmdFieldChangeExec.h"

#include "cafNotificationCenter.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiOrdering.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString CmdFieldChangeExec::name()
{
    if ( !m_commandData->m_pathToFields.empty() )
    {
        PdmFieldHandle* field =
            PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToFields.front() );
        if ( field )
        {
            QString fieldText;

            PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
            if ( uiFieldHandle )
            {
                fieldText = QString( "Change field '%1'" ).arg( uiFieldHandle->uiName() );
            }

            if ( field->ownerObject() )
            {
                PdmUiObjectHandle* uiObjHandle = uiObj( field->ownerObject() );
                if ( uiObjHandle )
                {
                    fieldText += QString( " in '%1'" ).arg( uiObjHandle->uiName() );
                }
            }
            return fieldText;
        }
    }

    return "Field Changed";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::redo()
{
    m_commandData->m_undoFieldValueSerialized.resize( m_commandData->m_pathToFields.size() );

    std::vector<caf::PdmObjectHandle*> objs;

    for ( size_t i = 0; i < m_commandData->m_pathToFields.size(); i++ )
    {
        auto fieldTextPath = m_commandData->m_pathToFields[i];

        PdmFieldHandle* field = PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, fieldTextPath );
        if ( !field ) continue;

        bool objectNotificationFlag = false;

        if ( auto obj = field->ownerObject() )
        {
            objs.push_back( obj );

            if ( auto uiObjectHandle = uiObj( obj ) )
            {
                objectNotificationFlag = uiObjectHandle->notifyAllFieldsInMultiFieldChangedEvents();

                // Make sure that uiOrdering has been called on all objects, as some object do some state
                // initialization. This is relevant for data source stepping objects. This operation could be made into
                // a virtual function.
                caf::PdmUiOrdering ordering;
                uiObjectHandle->uiOrdering( "", ordering );
            }
        }

        PdmUiFieldHandle*  uiFieldHandle  = field->uiCapability();
        PdmXmlFieldHandle* xmlFieldHandle = field->xmlCapability();
        if ( uiFieldHandle && xmlFieldHandle )
        {
            // In multi field update operations, a single fieldChanged() notification is sufficient for many use cases
            // where properties like color/size are modified. For control operations like changing the data source
            // for multiple sub objects, we need to issue fieldChanged() for all modified fields. This behaviour can be
            // forced both on object level and field level

            bool isLastField                  = ( i == m_commandData->m_pathToFields.size() - 1 );
            bool sendFieldChangedNotification = isLastField;
            if ( objectNotificationFlag ) sendFieldChangedNotification = true;
            if ( uiFieldHandle->notifyAllFieldsInMultiFieldChangedEvents() ) sendFieldChangedNotification = true;

            if ( m_commandData->m_undoFieldValueSerialized[i].isEmpty() )
            {
                // We end up here only when the user actually has done something in the actual living Gui editor.
                {
                    QXmlStreamWriter xmlStream( &m_commandData->m_undoFieldValueSerialized[i] );
                    writeFieldDataToValidXmlDocument( xmlStream, xmlFieldHandle );
                }

                // This function will notify field change, no need to explicitly call notification
                // The ui value might be an index into the option entry cache, so we need to set the value
                // and be aware of the option entries, and then serialize the actual field value we ended up with.

                uiFieldHandle->setValueFromUiEditor( m_commandData->m_newUiValue, sendFieldChangedNotification );

                if ( m_commandData->m_redoFieldValueSerialized.isEmpty() )
                {
                    QXmlStreamWriter xmlStream( &m_commandData->m_redoFieldValueSerialized );
                    writeFieldDataToValidXmlDocument( xmlStream, xmlFieldHandle );
                }
            }
            else
            {
                QVariant oldFieldData = uiFieldHandle->toUiBasedQVariant();

                QXmlStreamReader xmlStream( m_commandData->m_redoFieldValueSerialized );

                readFieldValueFromValidXmlDocument( xmlStream, xmlFieldHandle );

                QVariant newFieldData = uiFieldHandle->toUiBasedQVariant();

                // New data is present in field, notify data changed
                if ( sendFieldChangedNotification )
                {
                    uiFieldHandle->notifyFieldChanged( oldFieldData, newFieldData );
                    if ( m_notificationCenter )
                        m_notificationCenter->notifyObserversOfDataChange( field->ownerObject() );
                }
            }
        }
    }

    if ( m_commandData->m_ownerOfChildArrayField && m_commandData->m_childArrayFieldHandle )
    {
        m_commandData->m_ownerOfChildArrayField->onChildrenUpdated( m_commandData->m_childArrayFieldHandle, objs );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::undo()
{
    for ( size_t i = 0; i < m_commandData->m_pathToFields.size(); i++ )
    {
        auto fieldTextPath = m_commandData->m_pathToFields[i];

        PdmFieldHandle* field = PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, fieldTextPath );
        if ( !field ) continue;

        PdmUiFieldHandle*  uiFieldHandle  = field->uiCapability();
        PdmXmlFieldHandle* xmlFieldHandle = field->xmlCapability();
        if ( uiFieldHandle && xmlFieldHandle )
        {
            bool isLastField = ( i == m_commandData->m_pathToFields.size() - 1 );

            QXmlStreamReader xmlStream( m_commandData->m_undoFieldValueSerialized[i] );
            QVariant         oldFieldData = uiFieldHandle->toUiBasedQVariant();

            readFieldValueFromValidXmlDocument( xmlStream, xmlFieldHandle );

            QVariant newFieldData = uiFieldHandle->toUiBasedQVariant();

            // New data is present in field, notify data changed
            if ( isLastField )
            {
                uiFieldHandle->notifyFieldChanged( oldFieldData, newFieldData );
                if ( m_notificationCenter ) m_notificationCenter->notifyObserversOfDataChange( field->ownerObject() );
            }
        }
    }

    if ( m_commandData->m_ownerOfChildArrayField && m_commandData->m_childArrayFieldHandle )
    {
        std::vector<caf::PdmObjectHandle*> objs;
        m_commandData->m_ownerOfChildArrayField->onChildrenUpdated( m_commandData->m_childArrayFieldHandle, objs );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFieldChangeExec::CmdFieldChangeExec( NotificationCenter* notificationCenter )
    : CmdExecuteCommand( notificationCenter )
{
    m_commandData = new CmdFieldChangeExecData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFieldChangeExec::~CmdFieldChangeExec()
{
    delete m_commandData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFieldChangeExecData* CmdFieldChangeExec::commandData()
{
    return m_commandData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::writeFieldDataToValidXmlDocument( QXmlStreamWriter& xmlStream, PdmXmlFieldHandle* xmlFieldHandle )
{
    xmlStream.setAutoFormatting( true );
    xmlStream.writeStartDocument();
    xmlStream.writeStartElement( "", "d" );
    xmlFieldHandle->writeFieldData( xmlStream );
    xmlStream.writeEndElement();
    xmlStream.writeEndDocument();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::readFieldValueFromValidXmlDocument( QXmlStreamReader& xmlStream, PdmXmlFieldHandle* xmlFieldHandle )
{
    // See PdmObject::readFields and friends to match token count for reading field values
    // The stream is supposed to be pointing at the first token of field content when calling readFieldData()
    int tokenCount = 3;
    for ( int i = 0; i < tokenCount; i++ )
    {
        xmlStream.readNext();
    }
    xmlFieldHandle->readFieldData( xmlStream, PdmDefaultObjectFactory::instance() );
    xmlFieldHandle->resolveReferences();
}

} // end namespace caf
