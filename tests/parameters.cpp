#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(Parameter, Float) {

  ParameterSpaceDimension dim1("dim1");
  float values[] = {0.1f, 0.2f, 0.3f};
  dim1.setSpaceValues(values, 3);

  dim1.setCurrentValue(0.1f);
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.2f);
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.3f);
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.3f);
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.2f);
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.1f);
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.1f);
}
