#include "gtest/gtest.h"

#include "tinc/ProcessorScript.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(Processor, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  // TODO create processors of different types (perhaps in separate unit tests

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.5); // Give time to connect

  tclient.requestDiskBuffers();

  al::al_sleep(0.5); // Give time to connect

  // TODO check that the processor details are correct

  tclient.stop();
  tserver.stop();
}

TEST(ProcessorScript, ConstructorCopy) {
  ProcessorScript proc;
  proc.inputFile("_in_");
  proc.outputFile("_out_");
  proc.scriptFile("_script_");
  proc.setCommand("_command_");
  proc.setScriptName("_scriptname_");

  proc.setInputDirectory("_indir_");
  proc.setOutputDirectory("_outdir_");
  proc.setRunningDirectory("_rundir_");

  ProcessorScript procCopy = proc;

  EXPECT_EQ(procCopy.inputFile(), proc.inputFile());
  EXPECT_EQ(procCopy.scriptFile(), proc.scriptFile());
  EXPECT_EQ(procCopy.outputFile(), proc.outputFile());
  EXPECT_EQ(procCopy.getCommand(), proc.getCommand());
  EXPECT_EQ(procCopy.getScriptName(), proc.getScriptName());
  EXPECT_EQ(procCopy.getInputDirectory(), proc.getInputDirectory());
  EXPECT_EQ(procCopy.getOutputDirectory(), proc.getOutputDirectory());
  EXPECT_EQ(procCopy.getRunningDirectory(), proc.getRunningDirectory());
}