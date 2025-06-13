
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
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<double>> m_valveLocations;
};

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( InheritedDemoObj, InheritedDemoObj_appendValves, "AppendValves" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
InheritedDemoObj_appendValves::InheritedDemoObj_appendValves( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_VALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
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
QString InheritedDemoObj_appendValves::classKeywordReturnedType() const
{
    return InheritedDemoObj::classKeywordStatic();
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
