#include "gtest/gtest.h"

#include "tinc/ParameterSpace.hpp"
using namespace tinc;

TEST(ParameterSpace, FilenameTemplate) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);

  float values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->append(values, 5, "xx");

  for (int i = 0; i < 6; i++) {
    dim3->push_back(i * 0.01, "id" + std::to_string(i));
  }
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
