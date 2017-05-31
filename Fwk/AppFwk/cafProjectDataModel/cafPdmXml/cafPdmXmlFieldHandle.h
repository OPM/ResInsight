#pragma once

#include "cafPdmFieldCapability.h"
#include <QString>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{

class PdmFieldHandle;
class PdmObjectFactory;
class PdmReferenceHelper;


//==================================================================================================
//
// 
//
//==================================================================================================
class PdmXmlFieldHandle : public PdmFieldCapability
{
public:
    PdmXmlFieldHandle(PdmFieldHandle* owner , bool giveOwnership);
    virtual ~PdmXmlFieldHandle() { }

    PdmFieldHandle* fieldHandle()                       { return m_owner; }
    const PdmFieldHandle* fieldHandle() const           { return m_owner; }

    bool            isIOReadable() const                { return m_isIOReadable; }
    bool            isIOWritable() const                { return m_isIOWritable; }
    void            setIOWritable(bool isWritable)      { m_isIOWritable = isWritable; }
    void            setIOReadable(bool isReadable)      { m_isIOReadable = isReadable; }

    QString         childClassKeyword();

    virtual void    readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory)  = 0;
    virtual void    writeFieldData(QXmlStreamWriter& xmlStream) const = 0;

    virtual void    resolveReferences() { };

protected:
    bool            assertValid() const;
    QString         m_childClassKeyword; ///< Must be set in constructor of derived XmlFieldHandle

private:
    bool            m_isIOReadable;
    bool            m_isIOWritable;

    PdmFieldHandle* m_owner;
};

} // End of namespace caf

#include "cafInternalPdmXmlFieldCapability.h"
