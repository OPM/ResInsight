#pragma once

#include "cafPdmFieldCapability.h"
#include <QString>
#include <vector>

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
    ~PdmXmlFieldHandle() override { }

    PdmFieldHandle* fieldHandle()                       { return m_owner; }
    const PdmFieldHandle* fieldHandle() const           { return m_owner; }

    bool            isIOReadable() const                { return m_isIOReadable; }
    bool            isIOWritable() const                { return m_isIOWritable; }
    bool            isCopyable()  const                 { return m_isCopyable;}
    
    virtual bool    isVectorField() const               { return false; }

    void            disableIO();
    void            setIOWritable(bool isWritable)      { m_isIOWritable = isWritable; }
    void            setIOReadable(bool isReadable)      { m_isIOReadable = isReadable; }
    void            setCopyable(bool isCopyable)        { m_isCopyable  = isCopyable; }

    QString         dataTypeName() const;

    virtual void    readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory)  = 0;
    virtual void    writeFieldData(QXmlStreamWriter& xmlStream) const = 0;

    virtual bool    resolveReferences() = 0;

    virtual QString referenceString() const             { return QString(); }

protected:
    bool            assertValid() const;
    QString         m_dataTypeName; ///< Must be set in constructor of derived XmlFieldHandle

private:
    bool                 m_isIOReadable;
    bool                 m_isIOWritable;
    bool                 m_isCopyable;

    PdmFieldHandle* m_owner;
};

} // End of namespace caf

#include "cafInternalPdmXmlFieldCapability.h"
