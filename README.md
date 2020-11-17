[![Allosphere-Research-Group](https://circleci.com/gh/AlloSphere-Research-Group/tinc.svg?style=shield)](https://circleci.com/gh/AlloSphere-Research-Group/tinc)

# TINC (Toolkit for Interactive Computation)
This repository contains the TINC library.

# Using TINC

We have set up to repositories to assist learning and using TINC. The [tinc-playground](https://github.com/AlloSphere-Research-Group/tinc-playground) is a quick way to build and explore the examples included with TINC. It provides a convenience run script to easily build single file applications, or build all the examples. The [tinc-template](https://github.com/AlloSphere-Research-Group/tinc-template) repo shows how to set up a more complex project using TINC as a submodule through CMake. This template is useful to build larger applications made from multiple files or with multiple target that might require additional dependencies.

# Installation

## Dependencies

TINC depends on NetCDF4. You can use a binary installation and set the
NETCDF4_INSTALL_ROOT cmake variable using ccmake or on the command line
with:

    -DNETCDF4_INSTALL_ROOT="C:/Program Files/netCDF 4.7.4/"
    
If you use a binary installation, make sure you download a netCDF version that includes *hdf5 AND DAP* support.

Or you can use the *./build_deps.sh script on *nix platforms.


## Acknowledgements

TINC is funded by NSF grant # OAC 2004693: "Elements: Cyber-infrastructure for Interactive Computation and Display of Materials Datasets."
