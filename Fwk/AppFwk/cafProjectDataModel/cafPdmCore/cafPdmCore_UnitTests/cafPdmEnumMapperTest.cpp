
#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafAppEnumMapper.h"
#include "cafTypeNameHelper.h"

enum class TestEnum
{
    Value1 = 5,
    Value2,
    Value3,
    Value4
};

enum class TestEnum2
{
    ValueA,
    ValueB,
    ValueC
};

namespace caf
{
template <>
void caf::AppEnum<TestEnum>::setUp()
{
    addItem( TestEnum::Value1, "VALUE_1", "Val 1" );
    addItem( TestEnum::Value2, "VALUE_2", "Val 2" );
    addItem( TestEnum::Value3, "VALUE_3", "Val 3" );
    addItem( TestEnum::Value4, "VALUE_4", "Val 4" );
    setDefault( TestEnum::Value2 );
}

template <>
void caf::AppEnum<TestEnum2>::setUp()
{
    addItem( TestEnum2::ValueA, "VALUE_A", "Val A" );
    addItem( TestEnum2::ValueB, "VALUE_B", "Val B" );
    addItem( TestEnum2::ValueC, "VALUE_C", "Val C" );
    setDefault( TestEnum2::ValueC );
}

} //namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmEnumMapperTest, CreateInstance )
{
    caf::AppEnumMapper* instance = caf::AppEnumMapper::instance();
    EXPECT_NE( nullptr, instance );

    auto TestEnum_key  = caf::cafTypeName<TestEnum>();
    auto TestEnum2_key = caf::cafTypeName<TestEnum2>();

    {
        instance->addItem( TestEnum_key, caf::convertToInteger<TestEnum>( TestEnum::Value1 ), "VALUE_1", "Val 1" );
        instance->addItem( TestEnum_key, caf::convertToInteger<TestEnum>( TestEnum::Value2 ), "VALUE_2", "Val 2" );
        instance->addItem( TestEnum_key, caf::convertToInteger<TestEnum>( TestEnum::Value3 ), "VALUE_3", "Val 3" );
        instance->addItem( TestEnum_key, caf::convertToInteger<TestEnum>( TestEnum::Value4 ), "VALUE_4", "Val 4" );
    }

    {
        instance->addItem( TestEnum2_key, caf::convertToInteger<TestEnum2>( TestEnum2::ValueA ), "VALUE_A", "Val A" );
        instance->addItem( TestEnum2_key, caf::convertToInteger<TestEnum2>( TestEnum2::ValueB ), "VALUE_B", "Val B" );
        instance->addItem( TestEnum2_key, caf::convertToInteger<TestEnum2>( TestEnum2::ValueC ), "VALUE_C", "Val C" );
    }

    EXPECT_EQ( 4, instance->size( TestEnum_key ) );
    EXPECT_EQ( 3, instance->size( TestEnum2_key ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmEnumMapperTest, ConverEnumToInteger )
{
    caf::AppEnumMapper* instance = caf::AppEnumMapper::instance();

    {
        auto sourceEnumValue = TestEnum::Value1;
        auto intValue        = caf::convertToInteger( sourceEnumValue );
        auto enumValue       = caf::convertToEnum<TestEnum>( intValue );
        EXPECT_EQ( enumValue, sourceEnumValue );
    }

    {
        auto sourceEnumValue = TestEnum2::ValueC;
        auto intValue        = caf::convertToInteger( sourceEnumValue );
        auto enumValue       = caf::convertToEnum<TestEnum2>( intValue );
        EXPECT_EQ( enumValue, sourceEnumValue );
    }
}
