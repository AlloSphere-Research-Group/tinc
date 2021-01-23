#ifndef PYTHON_COMMON_H
#define PYTHON_COMMON_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

#ifdef _WINDOWS
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#endif

using json = nlohmann::json;

class PythonTester {
public:
  std::string pythonExecutable = "python";

  std::string pythonModulePath = "";

  std::string pythonCodeStart = R"(
import time
import sys
#sys.path.append('../../tinc-python')

def parameter_to_dict(p):
    d = p.__dict__.copy()
    d["_values"] = list(d["_values"])
    d["_ids"] = list(d["_ids"])
    try:
        del d["_interactive_widget"]
        del d["_data_type"]
        del d["tinc_client"]
    except:
        pass
    return d

def parameter_space_to_dict(ps):
    d = ps.__dict__.copy()
    d["_parameters"] = [parameter_to_dict(p) for p in ps.get_parameters()]

    try:
        del d["tinc_client"]
    except:
        pass
    return d

from tinc_client import *
tclient = TincClient()

time.sleep(0.5)
test_output = {}
)";

  std::string pythonCodeEnd = R"(
import json

with open('python_test_out.json', 'w') as outfile:
    try:
        json.dump(test_output, outfile)
    except TypeError as e:
        print(test_output)
        raise e

tclient.stop()
)";
  void runPython(std::string pythonCode) {

    std::ofstream f("test_python.py");
    f << pythonCodeStart << pythonCode << pythonCodeEnd;
    f.close();
    std::string command = pythonExecutable + " test_python.py";
    if (pythonModulePath.size() > 0) {
      std::cout << "Setting PYTHONPATH to " << pythonModulePath << std::endl;
#ifdef _WINDOWS
      if (!SetEnvironmentVariable("PYTHONPATH", pythonModulePath.c_str())) {
        printf("SetEnvironmentVariable failed (%d)\n", GetLastError());
      }
#else
      setenv("PYTHONPATH", pythonModulePath.c_str(), 1); // overwrite
#endif
    }
    if (std::system(command.c_str()) != 0) {
      std::cerr << "ERROR executing python" << std::endl;
    }
  }

  json readResults() {
    std::ifstream i("python_test_out.json");
    json j;
    i >> j;
    return j;
  }
};

#endif // PYTHON_COMMON_H
