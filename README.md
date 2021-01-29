[![Allosphere-Research-Group](https://circleci.com/gh/AlloSphere-Research-Group/tinc/tree/devel.svg?style=shield)](https://circleci.com/gh/AlloSphere-Research-Group/tinc) [![codecov](https://codecov.io/gh/AlloSphere-Research-Group/tinc/branch/devel/graph/badge.svg?token=0Y3PBQORDU)](https://codecov.io/gh/AlloSphere-Research-Group/tinc)

# TINC (Toolkit for Interactive Computation)
The Toolkit for Interactive Computation (TINC) provides a set of C++ and python
classes to assist in the interactive exploration of large datasets by 
managing parameter spaces, interactive computation and caching of data.

TINC allows exposing C++ application controls to the network. This simplifies
the development of distributed applications as well as creating applications
that can be controlled trhrough python without having the application itself
depend on python. A great use case is by interacting with the C++ application
through a jupyter notebook.

TINC can also be used standalone in python to assist exploration of complex
datasets that are spread out through the filesystem through interactive
computation prototyping.

Here are some videos demonstrating the functionality and API of TINC

[TINC: ParameterSpaces in Python](https://youtu.be/QWwW9XPRAdM)
[TINC: Client in Python interacting with C++](https://youtu.be/xdXmISo4DeY)

# Using TINC

We have set up to repositories to assist learning and using TINC. The
[tinc-playground](https://github.com/AlloSphere-Research-Group/tinc-playground) 
is a quick way to build and explore the examples included with TINC. It
provides a convenience run script to easily build single file applications, or 
build all the examples. The [tinc-template]
(https://github.com/AlloSphere-Research-Group/tinc-template) 
repo shows how to set up a more complex project using TINC as a submodule 
through CMake. This template is useful to build larger applications made from
multiple files or with multiple target that might require additional
dependencies.

# Installation

## Dependencies

TINC depends on NetCDF4. You can use a binary installation and set the
NETCDF4_INSTALL_ROOT cmake variable using ccmake or on the command line
with:

    -DNETCDF4_INSTALL_ROOT="C:/Program Files/netCDF 4.7.4/"
    
If you use a binary installation, make sure you download a netCDF version that
includes *hdf5 AND DAP* support.

Or you can use the *./build_deps.sh script on *nix platforms. You will need
ninja and curl installed for these scripts to work.

## Acknowledgements

TINC is funded by NSF grant # OAC 2004693: "Elements: Cyber-infrastructure for
Interactive Computation and Display of Materials Datasets."
