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
  dim1.setSpaceIds({"first", "second", "third"});

  EXPECT_EQ(dim1.size(), 3);

  dim1.setCurrentValue(0.1f);
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.2f);
  EXPECT_EQ(dim1.getCurrentId(), "second");
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.3f);
  EXPECT_EQ(dim1.getCurrentId(), "third");
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.3f);
  EXPECT_EQ(dim1.getCurrentId(), "third");
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.2f);
  EXPECT_EQ(dim1.getCurrentId(), "second");
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.1f);
  EXPECT_EQ(dim1.getCurrentId(), "first");
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 0.1f);
  EXPECT_EQ(dim1.getCurrentId(), "first");

  EXPECT_FLOAT_EQ(dim1.at(0), 0.1f);
  EXPECT_FLOAT_EQ(dim1.at(1), 0.2f);
  EXPECT_FLOAT_EQ(dim1.at(2), 0.3f);

  EXPECT_EQ(dim1.idAt(0), "first");
  EXPECT_EQ(dim1.idAt(1), "second");
  EXPECT_EQ(dim1.idAt(2), "third");

  dim1.clear();

  EXPECT_EQ(dim1.size(), 0);
}

TEST(Parameter, Int32) {

  ParameterSpaceDimension dim1("dim1", "",
                               ParameterSpaceDimension::Datatype::INT32);
  int32_t values[] = {10, 20, 30};
  dim1.setSpaceValues(values, 3);
  dim1.setSpaceIds({"first", "second", "third"});

  EXPECT_EQ(dim1.size(), 3);

  dim1.setCurrentValue(10);
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 20);
  EXPECT_EQ(dim1.getCurrentId(), "second");
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 30);
  EXPECT_EQ(dim1.getCurrentId(), "third");
  dim1.stepIncrement();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 30);
  EXPECT_EQ(dim1.getCurrentId(), "third");
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 20);
  EXPECT_EQ(dim1.getCurrentId(), "second");
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 10);
  EXPECT_EQ(dim1.getCurrentId(), "first");
  dim1.stepDecrease();
  EXPECT_FLOAT_EQ(dim1.getCurrentValue(), 10);
  EXPECT_EQ(dim1.getCurrentId(), "first");

  EXPECT_FLOAT_EQ(dim1.at(0), 10);
  EXPECT_FLOAT_EQ(dim1.at(1), 20);
  EXPECT_FLOAT_EQ(dim1.at(2), 30);

  EXPECT_EQ(dim1.idAt(0), "first");
  EXPECT_EQ(dim1.idAt(1), "second");
  EXPECT_EQ(dim1.idAt(2), "third");

  dim1.clear();

  EXPECT_EQ(dim1.size(), 0);
}
