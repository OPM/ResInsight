//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfBase64.h"
#include "cvfArray.h"


#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Base64Test, EncodeDecode)
{
    // Create binary data buffer to be encoded
    static const size_t binaryDataSize = 4096;
    UByteArray binaryData;
    binaryData.resize(binaryDataSize);

    // Populate the binary data buffer
    size_t i;
    for (i = 0; i < binaryDataSize; i++)
    {
        binaryData.set(i, static_cast<const ubyte>(i%256));
    }

    // Encode binary data
    std::string encodedData = Base64::encode(binaryData);
    EXPECT_GT(encodedData.size(), binaryDataSize);

    // Decode
    ref<UByteArray> decodedData = Base64::decode(encodedData);
    ASSERT_TRUE(decodedData.notNull());
    ASSERT_EQ(binaryDataSize, decodedData->size());

    // Verify that decoded data is identical with the original binary data
    for (i = 0; i < binaryDataSize; i++)
    {
        const ubyte decodedByte = decodedData->get(i);
        ASSERT_EQ(binaryData[i], decodedByte);
    }
}

