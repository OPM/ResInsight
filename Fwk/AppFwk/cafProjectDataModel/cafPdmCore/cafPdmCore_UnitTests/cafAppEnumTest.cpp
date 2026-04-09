#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"

// Define a test enum
enum class TestEnumType
{
    VALUE_A,
    VALUE_B,
    VALUE_C,
    VALUE_D,
    VALUE_E
};

namespace caf
{
template <>
void AppEnum<TestEnumType>::setUp()
{
    addItem( TestEnumType::VALUE_A, "VALUE_A", "Value A" );
    addItem( TestEnumType::VALUE_B, "VALUE_B", "Value B" );
    addItem( TestEnumType::VALUE_C, "VALUE_C", "Value C" );
    addItem( TestEnumType::VALUE_D, "VALUE_D", "Value D" );
    addItem( TestEnumType::VALUE_E, "VALUE_E", "Value E" );
    setDefault( TestEnumType::VALUE_A );
}
} // namespace caf

// First test object with an enum field
class TestObject1 : public caf::PdmObjectHandle
{
public:
    TestObject1()
    {
        this->addField( &m_enumField, "EnumField" );
        m_enumField.setOwnerClass( "TestObject1" );
        m_enumField = TestEnumType::VALUE_A;
    }

    caf::PdmDataValueField<caf::AppEnum<TestEnumType>> m_enumField;
};

// Second test object with an enum field using the same keyword
class TestObject2 : public caf::PdmObjectHandle
{
public:
    TestObject2()
    {
        this->addField( &m_enumField, "EnumField" );
        m_enumField.setOwnerClass( "TestObject2" );
        m_enumField = TestEnumType::VALUE_B;
    }

    caf::PdmDataValueField<caf::AppEnum<TestEnumType>> m_enumField;
};

// Third test object to test uninitialized subset
class TestObject3 : public caf::PdmObjectHandle
{
public:
    TestObject3()
    {
        this->addField( &m_enumField, "UniqueEnumField" );
        m_enumField.setOwnerClass( "TestObject3" );
        m_enumField = TestEnumType::VALUE_C;
    }

    caf::PdmDataValueField<caf::AppEnum<TestEnumType>> m_enumField;
};

//--------------------------------------------------------------------------------------------------
/// Test that two different objects can have the same field keyword with different enum subsets
//--------------------------------------------------------------------------------------------------
TEST( AppEnumTest, EnumSubsetNoCollision )
{
    TestObject1 obj1;
    TestObject2 obj2;

    // Set different subsets for the same field keyword in different objects
    std::vector<TestEnumType> subset1 = { TestEnumType::VALUE_A, TestEnumType::VALUE_B };
    std::vector<TestEnumType> subset2 = { TestEnumType::VALUE_C, TestEnumType::VALUE_D, TestEnumType::VALUE_E };

    caf::AppEnum<TestEnumType>::setEnumSubset( &obj1.m_enumField, subset1 );
    caf::AppEnum<TestEnumType>::setEnumSubset( &obj2.m_enumField, subset2 );

    // Retrieve the subsets
    auto retrievedSubset1 = caf::AppEnum<TestEnumType>::enumSubset( &obj1.m_enumField );
    auto retrievedSubset2 = caf::AppEnum<TestEnumType>::enumSubset( &obj2.m_enumField );

    // Verify that the subsets are correct and independent
    ASSERT_EQ( 2u, retrievedSubset1.size() );
    EXPECT_EQ( TestEnumType::VALUE_A, retrievedSubset1[0] );
    EXPECT_EQ( TestEnumType::VALUE_B, retrievedSubset1[1] );

    ASSERT_EQ( 3u, retrievedSubset2.size() );
    EXPECT_EQ( TestEnumType::VALUE_C, retrievedSubset2[0] );
    EXPECT_EQ( TestEnumType::VALUE_D, retrievedSubset2[1] );
    EXPECT_EQ( TestEnumType::VALUE_E, retrievedSubset2[2] );
}

//--------------------------------------------------------------------------------------------------
/// Test that multiple instances of the same class share the same subset
//--------------------------------------------------------------------------------------------------
TEST( AppEnumTest, EnumSubsetSameClass )
{
    TestObject1 obj1a;
    TestObject1 obj1b;

    // Set subset for first instance
    std::vector<TestEnumType> subset = { TestEnumType::VALUE_A, TestEnumType::VALUE_C };
    caf::AppEnum<TestEnumType>::setEnumSubset( &obj1a.m_enumField, subset );

    // Both instances should have the same subset since they have the same class name and field keyword
    auto retrievedSubset1 = caf::AppEnum<TestEnumType>::enumSubset( &obj1a.m_enumField );
    auto retrievedSubset2 = caf::AppEnum<TestEnumType>::enumSubset( &obj1b.m_enumField );

    ASSERT_EQ( 2u, retrievedSubset1.size() );
    ASSERT_EQ( 2u, retrievedSubset2.size() );
    EXPECT_EQ( TestEnumType::VALUE_A, retrievedSubset1[0] );
    EXPECT_EQ( TestEnumType::VALUE_C, retrievedSubset1[1] );
    EXPECT_EQ( TestEnumType::VALUE_A, retrievedSubset2[0] );
    EXPECT_EQ( TestEnumType::VALUE_C, retrievedSubset2[1] );
}

//--------------------------------------------------------------------------------------------------
/// Test that null field handle returns empty subset
//--------------------------------------------------------------------------------------------------
TEST( AppEnumTest, EnumSubsetNullHandle )
{
    auto retrievedSubset = caf::AppEnum<TestEnumType>::enumSubset( nullptr );
    EXPECT_TRUE( retrievedSubset.empty() );
}

//--------------------------------------------------------------------------------------------------
/// Test that requesting a subset that hasn't been set returns empty vector
//--------------------------------------------------------------------------------------------------
TEST( AppEnumTest, EnumSubsetNotSet )
{
    TestObject3 obj;
    auto        retrievedSubset = caf::AppEnum<TestEnumType>::enumSubset( &obj.m_enumField );
    EXPECT_TRUE( retrievedSubset.empty() );
}
