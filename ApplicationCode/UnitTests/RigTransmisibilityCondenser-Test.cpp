#include "gtest/gtest.h"

#include "RigTransmisibilityCondenser.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigTransmissibilityCondenser, BasicTest)
{
    using RiCa = RigTransmissibilityCondenser::CellAddress;
    #if 1
    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 1 }, 1.0);

        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 1 });

        EXPECT_DOUBLE_EQ(1.0, condT);
    }

    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 1 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 1 });

        EXPECT_DOUBLE_EQ(0.5, condT);
    }

    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { false, RiCa::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { true, RiCa::STIMPLAN, 2 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 2 });

        EXPECT_DOUBLE_EQ(0.25, condT);
    }

    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { false, RiCa::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { false, RiCa::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { true, RiCa::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, { true, RiCa::STIMPLAN, 3 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 3 });

        EXPECT_DOUBLE_EQ(0.5, condT);
    }
    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { false, RiCa::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 1 }, { false, RiCa::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, { false, RiCa::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 3 }, { true, RiCa::STIMPLAN, 4 }, 0.5);
        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1 }, { true, RiCa::STIMPLAN, 4 });

        EXPECT_DOUBLE_EQ(0.25, condT); 

    }
    #endif


    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 5 }, { false, RiCa::STIMPLAN, 1 }, 0.5);

        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, { false, RiCa::STIMPLAN, 4 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 3 }, { false, RiCa::STIMPLAN, 4 }, 0.5);

        condenser.addNeighborTransmisibility({ true, RiCa::STIMPLAN, 6 }, { false, RiCa::STIMPLAN, 4 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 5 }, { true, RiCa::STIMPLAN, 6 });

        EXPECT_NEAR(0.1666666667, condT, 1e-7); 
    }
    #if 1
    {
        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 5 }, { false, RiCa::STIMPLAN, 1 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 7 }, { false, RiCa::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 8 }, { false, RiCa::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ true, RiCa::ECLIPSE, 9 }, { false, RiCa::STIMPLAN, 4 }, 0.5);

        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN, 2 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN, 3 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, { false, RiCa::STIMPLAN, 4 }, 0.5);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 3 }, { false, RiCa::STIMPLAN, 4 }, 0.5);

        condenser.addNeighborTransmisibility({ true, RiCa::STIMPLAN, 6 }, { false, RiCa::STIMPLAN, 4 }, 0.5);

        double condT = condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 5 }, { true, RiCa::STIMPLAN, 6 });

        EXPECT_NEAR(0.045454545, condT, 1e-7); 
    }
    #endif
    #if 1
    {
        // Test example from Matlab scripts, that match the iterative solution

        RigTransmissibilityCondenser condenser;

        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN,   2 }, 0.01127000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, { false, RiCa::STIMPLAN,   4 }, 1.12700000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 1 }, {  true, RiCa::ECLIPSE,    7 }, 0.09016000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, { false, RiCa::STIMPLAN,   3 }, 0.01127000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, { false, RiCa::STIMPLAN,   5 }, 1.12700000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 2 }, {  true, RiCa::ECLIPSE,    8 }, 0.09016000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 3 }, { false, RiCa::STIMPLAN,   6 }, 1.50266667);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 3 }, {  true, RiCa::ECLIPSE,    9 }, 0.09016000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 4 }, { false, RiCa::STIMPLAN,   5 }, 0.01127000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 4 }, {  true, RiCa::ECLIPSE,   10 }, 0.09016000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 5 }, { false, RiCa::STIMPLAN,   6 }, 0.01502667);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 5 }, {  true, RiCa::ECLIPSE,   11 }, 0.09016000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 6 }, {  true, RiCa::ECLIPSE,   12 }, 0.09016000);
        condenser.addNeighborTransmisibility({ false, RiCa::STIMPLAN, 6 }, {  true, RiCa::STIMPLAN,  13 }, 0.19219491);

        EXPECT_NEAR(0.00402732, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1+6 }, { true, RiCa::ECLIPSE,   2+6 }), 1e-6);
        EXPECT_NEAR(0.00027347, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1+6 }, { true, RiCa::ECLIPSE,   3+6 }), 1e-6);
        EXPECT_NEAR(0.03879174, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1+6 }, { true, RiCa::ECLIPSE,   4+6 }), 1e-6);
        EXPECT_NEAR(0.00400489, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1+6 }, { true, RiCa::ECLIPSE,   5+6 }), 1e-6);
        EXPECT_NEAR(0.00026172, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1+6 }, { true, RiCa::ECLIPSE,   6+6 }), 1e-6);
        EXPECT_NEAR(0.00055791, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 1+6 }, { true, RiCa::STIMPLAN,  7+6 }), 1e-6);
        EXPECT_NEAR(0.00245697, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 2+6 }, { true, RiCa::ECLIPSE,   3+6 }), 1e-6);
        EXPECT_NEAR(0.00401064, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 2+6 }, { true, RiCa::ECLIPSE,   4+6 }), 1e-6);
        EXPECT_NEAR(0.03442773, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 2+6 }, { true, RiCa::ECLIPSE,   5+6 }), 1e-6);
        EXPECT_NEAR(0.00233846, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 2+6 }, { true, RiCa::ECLIPSE,   6+6 }), 1e-6);
        EXPECT_NEAR(0.00498491, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 2+6 }, { true, RiCa::STIMPLAN,  7+6 }), 1e-6);
        EXPECT_NEAR(0.00027351, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 3+6 }, { true, RiCa::ECLIPSE,   4+6 }), 1e-6);
        EXPECT_NEAR(0.00246579, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 3+6 }, { true, RiCa::ECLIPSE,   5+6 }), 1e-6);
        EXPECT_NEAR(0.01956640, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 3+6 }, { true, RiCa::ECLIPSE,   6+6 }), 1e-6);
        EXPECT_NEAR(0.04170988, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 3+6 }, { true, RiCa::STIMPLAN,  7+6 }), 1e-6);
        EXPECT_NEAR(0.00402105, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 4+6 }, { true, RiCa::ECLIPSE,   5+6 }), 1e-6);
        EXPECT_NEAR(0.00026189, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 4+6 }, { true, RiCa::ECLIPSE,   6+6 }), 1e-6);
        EXPECT_NEAR(0.00055827, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 4+6 }, { true, RiCa::STIMPLAN,  7+6 }), 1e-6);
        EXPECT_NEAR(0.00237402, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 5+6 }, { true, RiCa::ECLIPSE,   6+6 }), 1e-6);
        EXPECT_NEAR(0.00506073, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 5+6 }, { true, RiCa::STIMPLAN,  7+6 }), 1e-6);
        EXPECT_NEAR(0.04448791, condenser.condensedTransmisibility({ true, RiCa::ECLIPSE, 6+6 }, { true, RiCa::STIMPLAN,  7+6 }), 1e-6);
    }
    #endif
}


