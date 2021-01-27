#include "gtest/gtest.h"
#include <dmx_universe_data.hpp>

using namespace sACNcpp;

TEST(DMXUniverseDataTests, testWrite8Bit) {    
    DMXUniverseData data;

    data.writeVariableResolutionValue(1.0, 1, 1);

    EXPECT_EQ (data[2],  0);
    EXPECT_EQ (data[1],  255); 
}

TEST(DMXUniverseDataTests, testRead8Bit) {    
    DMXUniverseData data;

    data.set(1,255);

    EXPECT_FLOAT_EQ (data.readVariableResolutionValue(1,1),  1);
}

TEST(DMXUniverseDataTests, testWrite16Bit) {    
    DMXUniverseData data;

    data.writeVariableResolutionValue(1.0, 1, 2);
    data.writeVariableResolutionValue(0.5, 10, 2);
    data.writeVariableResolutionValue(0.003051804, 12, 2);

    EXPECT_EQ (data[2],  255);
    EXPECT_EQ (data[1],  255); 
    EXPECT_EQ (data[10],  127); 
    EXPECT_EQ (data[11],  255); 
    EXPECT_EQ (data[12],  0); 
    EXPECT_NEAR (data[13],  200, 1); 
}

TEST(DMXUniverseDataTests, testRead16Bit) {    
    DMXUniverseData data;

    data.set(1,127);
    data.set(2,255);

    EXPECT_NEAR (data.readVariableResolutionValue(1,2),  0.5, 1e-5);

    data.set(1,0);
    data.set(2,200);   

    EXPECT_NEAR (data.readVariableResolutionValue(1,2),  0.003051804, 1e-5);

    data.set(1,255);
    data.set(2,255);   

    EXPECT_NEAR (data.readVariableResolutionValue(1,2),  1, 1e-5);
}

TEST(DMXUniverseDataTests, testWrite24Bit) {    
    DMXUniverseData data;

    data.writeVariableResolutionValue(1.0, 1, 3);
    data.writeVariableResolutionValue(0.5, 10, 3);
    data.writeVariableResolutionValue(0.003051804, 20, 3);

    EXPECT_EQ (data[1],  255);
    EXPECT_EQ (data[2],  255); 
    EXPECT_EQ (data[3],  255);

    EXPECT_EQ (data[10],  127); 
    EXPECT_EQ (data[11],  255); 
    EXPECT_EQ (data[12],  255);

    EXPECT_EQ (data[20],  0); 
    EXPECT_EQ (data[21],  200);
    EXPECT_EQ (data[22],  0); 
}

TEST(DMXUniverseDataTests, testRead24Bit) {    
    DMXUniverseData data;

    data.set(1,127);
    data.set(2,255);
    data.set(3,255);

    EXPECT_NEAR (data.readVariableResolutionValue(1,3),  0.5, 1e-5);

    data.set(1,0);
    data.set(2,200); 
    data.set(3,0);  

    EXPECT_NEAR (data.readVariableResolutionValue(1,3),  0.003051804, 1e-5);

    data.set(1,255);
    data.set(2,255); 
    data.set(3,255);    

    EXPECT_NEAR (data.readVariableResolutionValue(1,3),  1, 1e-5);
}
