#include "cafPdmXmlObjectHandle.h"

#include "cafAssert.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmXmlFieldHandle.h"

#include "cafPdmFieldHandle.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cassert>
#include <iostream>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmXmlObjectHandle::PdmXmlObjectHandle( PdmObjectHandle* owner, bool giveOwnership )
{
    m_owner = owner;
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmXmlObjectHandle* xmlObj( PdmObjectHandle* obj )
{
    if ( !obj ) return nullptr;
    PdmXmlObjectHandle* xmlObject = obj->capability<PdmXmlObjectHandle>();
    CAF_ASSERT( xmlObject );
    return xmlObject;
}

//--------------------------------------------------------------------------------------------------
/// Reads all the fields into this PdmObject
/// Assumes xmlStream points to the start element token of the PdmObject for which to read fields.
/// ( and not first token of object content)
/// This makes attribute based field storage possible.
/// Leaves the xmlStream pointing to the EndElement of the PdmObject.
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmXmlObjectHandle::readFields( QXmlStreamReader&                  xmlStream,
                                                     PdmObjectFactory*                  objectFactory,
                                                     bool                               isCopyOperation,
                                                     const std::vector<PdmDeprecation>& deprecations )
{
    std::vector<QString>        deprecationMessages;
    bool                        isObjectFinished = false;
    QXmlStreamReader::TokenType type;
    while ( !isObjectFinished )
    {
        type = xmlStream.readNext();

        switch ( type )
        {
            case QXmlStreamReader::StartElement:
            {
                QString name = xmlStream.name().toString();

                PdmFieldHandle* fieldHandle = m_owner->findField( name );
                if ( fieldHandle && fieldHandle->xmlCapability() )
                {
                    auto xmlAttributes = xmlStream.attributes();
                    if ( !xmlAttributes.isEmpty() )
                    {
                        std::vector<std::pair<QString, QString>> fieldAttributes;

                        for ( const auto& xmlAttr : xmlAttributes )
                        {
                            fieldAttributes.emplace_back( xmlAttr.name().toString(), xmlAttr.value().toString() );
                        }

                        for ( auto capability : fieldHandle->capabilities() )
                        {
                            capability->setAttributes( fieldAttributes );
                        }
                    }

                    PdmXmlFieldHandle* xmlFieldHandle = fieldHandle->xmlCapability();
                    bool               readable       = xmlFieldHandle->isIOReadable();
                    if ( isCopyOperation && !xmlFieldHandle->isCopyable() )
                    {
                        readable = false;
                    }
                    if ( readable )
                    {
                        // readFieldData assumes that the xmlStream points to first token of field content.
                        // After reading, the xmlStream is supposed to point to the first token after the field
                        // content. (typically an "endElement")
                        xmlStream.readNext();
                        std::vector<QString> deprecationMessagesForField =
                            xmlFieldHandle->readFieldData( xmlStream, objectFactory, deprecations );
                        deprecationMessages.insert( deprecationMessages.end(),
                                                    deprecationMessagesForField.begin(),
                                                    deprecationMessagesForField.end() );
                    }
                    else
                    {
                        xmlStream.skipCurrentElement();
                    }
                }
                else
                {
                    // Debug text is commented out, as this code is relatively often reached. Consider a new logging
                    // concept to receive this information
                    //
                    // std::cout << "Line " << xmlStream.lineNumber() << ": Warning: Could not find a field with name "
                    //           << name.toLatin1().data()
                    //           << " in the current object : " << classKeyword().toLatin1().data() << std::endl;

                    auto findDeprecation = []( const std::vector<PdmDeprecation>& deprecations,
                                               const QString&                     objectKeyword,
                                               const QString& fieldKeyword ) -> std::optional<PdmDeprecation>
                    {
                        for ( const PdmDeprecation& deprecation : deprecations )
                        {
                            if ( deprecation.fieldKeyword == fieldKeyword && deprecation.objectKeyword == objectKeyword )
                                return deprecation;
                        }

                        return {};
                    };

                    auto deprecation = findDeprecation( deprecations, classKeyword(), name );
                    if ( deprecation )
                    {
                        deprecationMessages.push_back( deprecation.value().message );
                    }

                    xmlStream.skipCurrentElement();
                }
                break;
            }
            break;
            case QXmlStreamReader::EndElement:
            {
                // End of object.
                isObjectFinished = true;
            }
            break;
            case QXmlStreamReader::EndDocument:
            {
                // End of object.
                isObjectFinished = true;
            }
            break;
            default:
            {
                // Just read on
                // Todo: Error handling
            }
            break;
        }
    }

    return deprecationMessages;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::writeFields( QXmlStreamWriter& xmlStream ) const
{
    for ( const auto& fieldHandle : fieldsForExport() )
    {
        const PdmXmlFieldHandle* field = fieldHandle->xmlCapability();
        if ( field && field->isIOWritable() )
        {
            QString keyword = field->fieldHandle()->keyword();
            CAF_ASSERT( PdmXmlObjectHandle::isValidXmlElementName( keyword ) );

            xmlStream.writeStartElement( "", keyword );

            for ( auto cap : fieldHandle->capabilities() )
            {
                auto attributes = cap->attributes();
                for ( const auto& [key, value] : attributes )
                {
                    xmlStream.writeAttribute( key, value );
                }
            }

            field->writeFieldData( xmlStream );

            xmlStream.writeEndElement();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::readObjectFromXmlString( const QString& xmlString, PdmObjectFactory* objectFactory )
{
    // A valid XML is used to store field data of the object. The format of the XML is like this

    /*
        <classKeyword>
            <fieldKeywordA>value</fieldKeywordA>
            <fieldKeywordB>value</fieldKeywordB>
        </classKeyword>
    */

    QXmlStreamReader inputStream( xmlString );

    inputStream.readNext(); // Start of document
    inputStream.readNext();
    QString classKeyword = inputStream.name().toString();
    CAF_ASSERT( classKeyword == this->classKeyword() );

    this->readFields( inputStream, objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmXmlObjectHandle::readUnknownObjectFromXmlString( const QString&    xmlString,
                                                                     PdmObjectFactory* objectFactory,
                                                                     bool              isCopyOperation )
{
    QXmlStreamReader inputStream( xmlString );

    inputStream.readNext(); // Start of document
    inputStream.readNext();
    QString          classKeyword = inputStream.name().toString();
    PdmObjectHandle* newObject    = objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    xmlObj( newObject )->readFields( inputStream, objectFactory, isCopyOperation );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmXmlObjectHandle::copyByXmlSerialization( PdmObjectFactory* objectFactory )
{
    this->setupBeforeSaveRecursively();

    QString xmlString = this->writeObjectToXmlString();

    PdmObjectHandle* objectCopy = PdmXmlObjectHandle::readUnknownObjectFromXmlString( xmlString, objectFactory, true );
    if ( !objectCopy ) return nullptr;

    objectCopy->xmlCapability()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* PdmXmlObjectHandle::copyAndCastByXmlSerialization( const QString&    destinationClassKeyword,
                                                                         const QString&    sourceClassKeyword,
                                                                         PdmObjectFactory* objectFactory )
{
    this->setupBeforeSaveRecursively();

    QString xmlString = this->writeObjectToXmlString();

    PdmObjectHandle* upgradedObject = objectFactory->create( destinationClassKeyword );
    if ( !upgradedObject ) return nullptr;

    QXmlStreamReader inputStream( xmlString );

    inputStream.readNext(); // Start of document
    inputStream.readNext();
    QString classKeyword = inputStream.name().toString();
    CAF_ASSERT( classKeyword == sourceClassKeyword );

    xmlObj( upgradedObject )->readFields( inputStream, objectFactory, true );

    xmlObj( upgradedObject )->initAfterReadRecursively();

    return upgradedObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmXmlObjectHandle::writeObjectToXmlString() const
{
    QString          xmlString;
    QXmlStreamWriter outputStream( &xmlString );
    outputStream.setAutoFormatting( true );

    outputStream.writeStartElement( "", this->classKeyword() );
    this->writeFields( outputStream );
    outputStream.writeEndElement();
    return xmlString;
}

//--------------------------------------------------------------------------------------------------
/// Check if a string is a valid Xml element name
//
/// http://www.w3schools.com/xml/xml_elements.asp
///
/// XML elements must follow these naming rules:
///   Names can contain letters, numbers, and other characters
///   Names cannot start with a number or punctuation character
///   Names cannot start with the letters xml (or XML, or Xml, etc)
///   Names cannot contain spaces
//--------------------------------------------------------------------------------------------------
bool PdmXmlObjectHandle::isValidXmlElementName( const QString& name )
{
    if ( name.isEmpty() )
    {
        return false;
    }

    if ( name.size() > 0 )
    {
        QChar firstChar = name[0];
        if ( firstChar.isDigit() || firstChar == '.' )
        {
            return false;
        }
    }

    if ( name.size() >= 3 )
    {
        if ( name.left( 3 ).compare( "xml", Qt::CaseInsensitive ) == 0 )
        {
            return false;
        }
    }

    if ( name.contains( ' ' ) )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> PdmXmlObjectHandle::fieldsForExport() const
{
    if ( !m_owner ) return {};

    return m_owner->fields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::registerClassKeyword( const QString& registerKeyword )
{
    m_classInheritanceStack.push_back( registerKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmXmlObjectHandle::inheritsClassWithKeyword( const QString& testClassKeyword ) const
{
    return std::find( m_classInheritanceStack.begin(), m_classInheritanceStack.end(), testClassKeyword ) !=
           m_classInheritanceStack.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<QString>& PdmXmlObjectHandle::classInheritanceStack() const
{
    return m_classInheritanceStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::initAfterReadRecursively( PdmObjectHandle* object )
{
    if ( object == nullptr ) return;

    // Set flag to be able to detect if resolveReferencesRecursively() is called from initAfterRead()
    object->m_isInsideInitAfterRead = true;

    std::vector<PdmFieldHandle*> fields = object->fields();

    std::vector<PdmObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        if ( fields[fIdx] )
        {
            std::vector<PdmObjectHandle*> fieldChildren = fields[fIdx]->children();
            children.insert( children.end(), fieldChildren.begin(), fieldChildren.end() );
        }
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        initAfterReadRecursively( children[cIdx] );
    }

    PdmXmlObjectHandle* xmlObject = xmlObj( object );
    if ( xmlObject )
    {
        xmlObject->initAfterRead();
    }

    object->m_isInsideInitAfterRead = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::initAfterReadRecursively()
{
    initAfterReadRecursively( this->m_owner );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::resolveReferencesRecursively( PdmObjectHandle*              object,
                                                       std::vector<PdmFieldHandle*>* fieldWithFailingResolve )
{
    if ( object == nullptr ) return;

    assert( !object->m_isInsideInitAfterRead );
    if ( object->m_isInsideInitAfterRead )
    {
        // In release, the assert above is not triggered, but the resolving might still fail.
        // In RelWithDebInfo, the following message will be available from the debug output window

        QString text =
            "Calling resolveReferencesRecursively() from initAfterRead() is not supported, "
            "as the object model might not be complete. If the object model is incomplete, the resolving might "
            "fail and assign a null pointer to the pointer field.";

        qDebug() << text;

        std::vector<PdmFieldHandle*> fields = object->fields();
        if ( !fields.empty() )
        {
            text = "    Field keywords: ";

            for ( auto f : fields )
            {
                text += f->keyword() + ", ";
            }
        }
        qDebug() << text;

        return;
    }

    std::vector<PdmFieldHandle*> fields = object->fields();

    std::vector<PdmObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        PdmFieldHandle* field = fields[fIdx];
        if ( field )
        {
            std::vector<PdmObjectHandle*> fieldChildren = field->children();

            bool resolvedOk = field->xmlCapability()->resolveReferences();
            if ( fieldWithFailingResolve && !resolvedOk )
            {
                fieldWithFailingResolve->push_back( field );
            }

            children.insert( children.end(), fieldChildren.begin(), fieldChildren.end() );
        }
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        resolveReferencesRecursively( children[cIdx], fieldWithFailingResolve );
    }
}

//--------------------------------------------------------------------------------------------------
/// Never call resolveReferencesRecursively() from initAfterRead(), as the document is not fully imported and the
// resolving might fail. The object needs to be fully inserted into the document before resolving references.
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::resolveReferencesRecursively(
    std::vector<PdmFieldHandle*>* fieldWithFailingResolve /*= nullptr*/ )
{
    std::vector<PdmFieldHandle*> tempFields;
    resolveReferencesRecursively( this->m_owner, &tempFields );

    if ( fieldWithFailingResolve )
    {
        for ( auto f : tempFields )
        {
            fieldWithFailingResolve->push_back( f );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::setupBeforeSaveRecursively( PdmObjectHandle* object )
{
    if ( object == nullptr ) return;

    std::vector<PdmFieldHandle*> fields = object->fields();

    std::vector<PdmObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        if ( fields[fIdx] )
        {
            std::vector<PdmObjectHandle*> fieldChildren = fields[fIdx]->children();
            children.insert( children.end(), fieldChildren.begin(), fieldChildren.end() );
        }
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        setupBeforeSaveRecursively( children[cIdx] );
    }

    PdmXmlObjectHandle* xmlObject = xmlObj( object );
    if ( xmlObject )
    {
        xmlObject->setupBeforeSave();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmXmlObjectHandle::setupBeforeSaveRecursively()
{
    setupBeforeSaveRecursively( this->m_owner );
}

//--------------------------------------------------------------------------------------------------
/// Implementation of xmlCapability() defined in cafPdmObjectHandle.h
//--------------------------------------------------------------------------------------------------
PdmXmlObjectHandle* PdmObjectHandle::xmlCapability() const
{
    PdmXmlObjectHandle* xmlField = capability<PdmXmlObjectHandle>();
    CAF_ASSERT( xmlField );

    return xmlField;
}

} // end namespace caf
