#include "gtest/gtest.h"

#include "RigTransmisibilityCondenser.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigTransmissibilityCondenser, BasicTest)
{
   
    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, 1.0);

        double condT = condenser.condensedTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 });

        EXPECT_DOUBLE_EQ(1.0, condT);
    }

    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 });

        EXPECT_DOUBLE_EQ(0.5, condT);
    }

    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 });

        EXPECT_DOUBLE_EQ(0.25, condT);
    }

    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 });

        EXPECT_DOUBLE_EQ(0.5, condT);
    }
   
    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 4 }, 0.5);
        double condT = condenser.condensedTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 1 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 4 });

        EXPECT_DOUBLE_EQ(0.25, condT); 

    }


    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 5 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 7 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 9 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 4 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 8 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, 0.5);

        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 1 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 2 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 4 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 3 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 4 }, 0.5);

        condenser.addNeighborTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 6 }, { false, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 4 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RigTransmissibilityCondenser::CellAddress::ECLIPSE, 5 }, { true, RigTransmissibilityCondenser::CellAddress::STIMPLAN, 6 });

        EXPECT_DOUBLE_EQ(0.083333333, condT); 

    }
}


