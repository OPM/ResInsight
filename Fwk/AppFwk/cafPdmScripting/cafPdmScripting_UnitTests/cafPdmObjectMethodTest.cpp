
#include "gtest/gtest.h"

#include "cafMockObjects.h"

namespace caf
{

//==================================================================================================
///
//==================================================================================================
class InheritedDemoObj_appendValves : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    InheritedDemoObj_appendValves( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    bool                             isNullptrValidResult() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmField<std::vector<double>> m_valveLocations;
};

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( InheritedDemoObj, InheritedDemoObj_appendValves, "AppendValves" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
InheritedDemoObj_appendValves::InheritedDemoObj_appendValves( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Append Valves", "", "", "Append Valves" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_valveLocations, "ValveLocations", "ValveLocations" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* InheritedDemoObj_appendValves::execute()
{
    auto obj = self<InheritedDemoObj>();

    auto* child      = new SimpleObj;
    child->m_numbers = m_valveLocations();
    obj->m_simpleObjectsField().push_back( child );
    return obj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool InheritedDemoObj_appendValves::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool InheritedDemoObj_appendValves::isNullptrValidResult() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> InheritedDemoObj_appendValves::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new InheritedDemoObj );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmObjectMethodTest, GeneratePythonCode )
{
    caf::PdmPythonGenerator generator;
    std::vector<QString>    logMessages;
    auto                    generatedText = generator.generate( caf::PdmDefaultObjectFactory::instance(), logMessages );
    auto                    string        = generatedText.toStdString();
}

} // namespace caf
