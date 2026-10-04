#include "gtest/gtest.h"

#include "RimWorkflowVec3Binding.h"

#include <QJsonObject>

//--------------------------------------------------------------------------------------------------
/// taskmaestro_resinsight.models.Vec3 is a plain pydantic model with x/y/z number fields, so both
/// the schema default and config_values represent it as a JSON object, not an array.
//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowVec3Binding, AppliesObjectDefault )
{
    RimWorkflowVec3Binding binding;

    const QJsonObject schema{ { "name", "reference_point" },
                              { "description", "Reference point" },
                              { "required", true },
                              { "default", QJsonObject{ { "x", 457196.0 }, { "y", 7322270.0 }, { "z", 2742.0 } } } };
    binding.applySchema( schema );

    EXPECT_EQ( "{x: 457196, y: 7322270, z: 2742}", binding.toYamlValue() );
}

//--------------------------------------------------------------------------------------------------
TEST( RimWorkflowVec3Binding, NoDefaultYieldsNullYaml )
{
    RimWorkflowVec3Binding binding;

    const QJsonObject schema{ { "name", "reference_point" }, { "required", true } };
    binding.applySchema( schema );

    EXPECT_EQ( "null", binding.toYamlValue() );
}
