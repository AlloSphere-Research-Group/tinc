#include "gtest/gtest.h"

#include "tinc/DiskBufferJson.hpp"

using namespace tinc;

TEST(DiskBuffer, BasicJson) {
  DiskBufferJson db{"id", "name.ext", "path"};

  db.enableRoundRobin(5);
  auto filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_0.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_1.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_2.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_3.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_4.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_0.ext");
};

TEST(DiskBuffer, Documentation) {
  DiskBufferJson db{"id", "name.ext", "path"};
  db.setDocumentation("Hello");
  EXPECT_EQ(db.getDocumentation(), "Hello");
}

TEST(DiskBuffer, RoundRobin) {
  DiskBufferJson db{"id", "name.ext", "path"};

  db.enableRoundRobin(5);
  auto filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_0.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_1.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_2.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_3.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_4.ext");
  filename = db.getFilenameForWriting();
  EXPECT_EQ(filename, "name_0.ext");
};
