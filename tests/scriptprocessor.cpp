#include "gtest/gtest.h"

#include "tinc/ScriptProcessor.hpp"
using namespace tinc;

TEST(ScriptProcessor, ConstructorCopy) {
  ScriptProcessor proc;
  proc.inputFile("_in_");
  proc.outputFile("_out_");
  proc.scriptFile("_script_");
  proc.setCommand("_command_");
  proc.setScriptName("_scriptname_");

  proc.setInputDirectory("_indir_");
  proc.setOutputDirectory("_outdir_");
  proc.setRunningDirectory("_rundir_");

  ScriptProcessor procCopy = proc;

  EXPECT_EQ(procCopy.inputFile(), proc.inputFile());
  EXPECT_EQ(procCopy.scriptFile(), proc.scriptFile());
  EXPECT_EQ(procCopy.outputFile(), proc.outputFile());
  EXPECT_EQ(procCopy.getCommand(), proc.getCommand());
  EXPECT_EQ(procCopy.getScriptName(), proc.getScriptName());
  EXPECT_EQ(procCopy.getInputDirectory(), proc.getInputDirectory());
  EXPECT_EQ(procCopy.getOutputDirectory(), proc.getOutputDirectory());
  EXPECT_EQ(procCopy.getRunningDirectory(), proc.getRunningDirectory());
}
