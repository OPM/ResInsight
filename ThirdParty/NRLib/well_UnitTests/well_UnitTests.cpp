
#include "gtest/gtest.h"
#include <stdio.h>
#include <iostream>
#include <string>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int main(int argc, char **argv) 
{
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    char text[5];
    std::cin.getline(text, 5);

    return result;
}
