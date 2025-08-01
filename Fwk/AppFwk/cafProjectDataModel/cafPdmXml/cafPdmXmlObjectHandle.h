#pragma once

#include "cafPdmDeprecation.h"
#include "cafPdmObjectCapability.h"

#include <QString>

#include <list>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{
class PdmXmlFieldHandle;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmReferenceHelper;
class PdmFieldHandle;

//==================================================================================================
//
//
//
//==================================================================================================
class PdmXmlObjectHandle : public PdmObjectCapability
{
public:
    PdmXmlObjectHandle( PdmObjectHandle* owner, bool giveOwnership );
    ~PdmXmlObjectHandle() override {}

    /// The classKeyword method is overridden in subclasses by the CAF_PDM_XML_HEADER_INIT macro
    virtual QString classKeyword() const                                     = 0;
    virtual bool    matchesClassKeyword( const QString& classKeyword ) const = 0;

    /// Convenience methods to serialize/de-serialize this particular object (with children)
    void    readObjectFromXmlString( const QString& xmlString, PdmObjectFactory* objectFactory );
    QString writeObjectToXmlString() const;
    static PdmObjectHandle*
        readUnknownObjectFromXmlString( const QString& xmlString, PdmObjectFactory* objectFactory, bool isCopyOperation );
    PdmObjectHandle* copyByXmlSerialization( PdmObjectFactory* objectFactory );
    PdmObjectHandle* copyAndCastByXmlSerialization( const QString&    destinationClassKeyword,
                                                    const QString&    sourceClassKeyword,
                                                    PdmObjectFactory* objectFactory );

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    std::vector<QString> readFields( QXmlStreamReader&                  inputStream,
                                     PdmObjectFactory*                  objectFactory,
                                     bool                               isCopyOperation,
                                     const std::vector<PdmDeprecation>& deprecations = {} );
    void                 writeFields( QXmlStreamWriter& outputStream ) const;

    /// Check if a string is a valid Xml element name
    static bool isValidXmlElementName( const QString& name );

    void initAfterReadRecursively();
    void setupBeforeSaveRecursively();

    // Never call resolveReferencesRecursively() from initAfterRead(), as the document is not fully imported and the
    // resolving might fail. The object needs to be fully inserted into the document before resolving references.
    void resolveReferencesRecursively( std::vector<PdmFieldHandle*>* fieldWithFailingResolve = nullptr );

    bool inheritsClassWithKeyword( const QString& testClassKeyword ) const;

    const std::list<QString>& classInheritanceStack() const;

protected: // Virtual
    /// Method gets called from PdmDocument after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead() {};
    /// Method gets called from PdmDocument before saving document.
    /// Re-implement to make sure your fields have correct data before saving
    virtual void setupBeforeSave() {};

    /// Override this method if you need to exclude or reorder fields for export to XML
    [[nodiscard]] virtual std::vector<PdmFieldHandle*> fieldsForExport() const;

    /// This method is intended to be used in macros to make compile time errors
    // if user uses them on wrong type of objects
    bool isInheritedFromPdmXmlSerializable() { return true; }

    void registerClassKeyword( const QString& registerKeyword );

private:
    static void initAfterReadRecursively( PdmObjectHandle* object );
    static void setupBeforeSaveRecursively( PdmObjectHandle* object );
    static void resolveReferencesRecursively( PdmObjectHandle*              object,
                                              std::vector<PdmFieldHandle*>* fieldWithFailingResolve );

private:
    friend class PdmObjectHandle; // Only temporary for void PdmObject::addFieldNoDefault( ) accessing findField

    std::list<QString> m_classInheritanceStack;
    PdmObjectHandle*   m_owner;
};

PdmXmlObjectHandle* xmlObj( PdmObjectHandle* obj );

} // End of namespace caf

#include "cafPdmXmlFieldHandle.h"
