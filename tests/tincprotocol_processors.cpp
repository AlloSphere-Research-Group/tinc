#include "gtest/gtest.h"

#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorScript.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(Processor, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ProcessorCpp proc1("proc1");
  proc1.setDocumentation("Hello");
  EXPECT_EQ(proc1.getDocumentation(), "Hello");

  ProcessorScript proc2("proc2");
  proc2.setDocumentation("World");

  tserver.registerProcessor(proc1);
  tserver.registerProcessor(proc2);

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(1.0); // FIXME there should be no need for this wait

  auto *proc1Client = tclient.getProcessor("proc1");
  auto *proc2Client = tclient.getProcessor("proc2");

  EXPECT_NE(proc1Client, nullptr);
  EXPECT_NE(proc2Client, nullptr);

  EXPECT_EQ(proc1Client->getDocumentation(), "Hello");
  EXPECT_EQ(proc2Client->getDocumentation(), "World");

  tclient.stop();
  tserver.stop();
}

TEST(ProcessorScript, ConstructorCopy) {
  ProcessorScript proc;
  proc.setInputFileNames({"_in_"});
  proc.setOutputFileNames({"_out_"});
  proc.getScriptFile("_script_");
  proc.setCommand("_command_");
  proc.setScriptName("_scriptname_");

  proc.setInputDirectory("_indir_");
  proc.setOutputDirectory("_outdir_");
  proc.setRunningDirectory("_rundir_");

  ProcessorScript procCopy = proc;

  EXPECT_EQ(procCopy.getInputFileNames().size(), 1);
  EXPECT_EQ(procCopy.getOutputFileNames().size(), 1);
  EXPECT_EQ(procCopy.getInputFileNames()[0], proc.getInputFileNames()[0]);
  EXPECT_EQ(procCopy.getScriptFile(), proc.getScriptFile());
  EXPECT_EQ(procCopy.getOutputFileNames()[0], proc.getOutputFileNames()[0]);
  EXPECT_EQ(procCopy.getCommand(), proc.getCommand());
  EXPECT_EQ(procCopy.getScriptName(), proc.getScriptName());
  EXPECT_EQ(procCopy.getInputDirectory(), proc.getInputDirectory());
  EXPECT_EQ(procCopy.getOutputDirectory(), proc.getOutputDirectory());
  EXPECT_EQ(procCopy.getRunningDirectory(), proc.getRunningDirectory());
}
