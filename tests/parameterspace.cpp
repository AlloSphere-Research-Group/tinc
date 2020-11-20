#include "gtest/gtest.h"

#include "tinc/ParameterSpace.hpp"
using namespace tinc;

TEST(ParameterSpace, DimensionValues) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  std::vector<float> values = {-0.25, -0.125, 0.0, 0.125, 0.25};
  dim1->setSpaceValues(values);

  EXPECT_EQ(dim1->size(), 5);
  auto setValues = dim1->getSpaceValues<float>();
  for (size_t i = 0; i < 5; i++) {

    EXPECT_EQ(setValues[i], values[i]);
  }

  // TODO verify dimension space setting for all types.
}

TEST(ParameterSpace, FilenameTemplate) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);

  float values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->setSpaceValues(values, 5, "xx");

  float dim3Values[6];
  std::vector<std::string> ids;
  for (int i = 0; i < 6; i++) {
    dim3Values[i] = i * 0.01;
    ids.push_back("id" + std::to_string(i));
  }
  dim3->setSpaceValues(dim3Values, 6);
  dim3->setSpaceIds(ids);

  dim1->setCurrentValue(0.5);
  dim2->setCurrentValue(0.2);
  dim3->setCurrentValue(0.02);

  auto name = ps.resolveFilename("file_%%dim1%%_%%dim2%%_%%dim3%%");
  EXPECT_EQ(name, "file_0.500000_1_id2");

  name = ps.resolveFilename("file_%%dim2:VALUE%%_%%dim3:VALUE%%");
  EXPECT_EQ(name, "file_0.200000_0.020000");

  name = ps.resolveFilename("file_%%dim2:ID%%_%%dim3:ID%%");
  EXPECT_EQ(name, "file_xx0.200000_id2");
}
