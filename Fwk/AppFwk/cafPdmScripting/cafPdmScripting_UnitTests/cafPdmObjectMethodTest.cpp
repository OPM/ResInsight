
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

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    bool                                          isNullptrValidResult_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

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
std::expected<caf::PdmObjectHandle*, QString> InheritedDemoObj_appendValves::execute()
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
bool InheritedDemoObj_appendValves::resultIsPersistent_obsolete() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool InheritedDemoObj_appendValves::isNullptrValidResult_obsolete() const
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
