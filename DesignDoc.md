
# Tinc Design Document (Toolkit for Interactive Computation)

This document is for enumerating the purpose and details of the Tinc software toolkit and its underlying modules and functionality. This document should serve as a guide and scaffold for the design of Tinc and its iterative development. Here we can iterate on intended functionality, structure, naming conventions, protocols, and apis.

## Tinc Functionality 
- Enable distributed interactive computation, through execution of scripts and building reactive computation chains to process data, caching computation data over interactive multi-dimensional parameter spaces, providing python scripting interface to distributed c++ audiovisual computation system.
- Data Processing pipelines through building computation chains via c++ processors and / python script processors
- Interactive and iteratable multi dimensional parameter spaces and their mappings to data sources on disk
- Caching local processed data based on Parameter spaces for rapid exploration
- Client / Server distributed architecture for communicating and synchronizing interactive visualizations and computations over the network
- Python interface for communicating and processing data interactively with the Tinc Server

## Tinc Architecture / Modules / Naming Conventions

Tinc is built on top of the allolib software platform and provides additional modules:

- Data Processing Tools:
  - Processor.hpp
  - CppProcessor.hpp
  - ScriptProcessor.hpp
  - ProcessorAsyncWrapper.hpp
  - ComputationChain.hpp

- Interactive Parameter Spaces:
  - ParameterSpace.hpp
  - ParameterSpaceDimension.hpp
  - ...

- Distributed Client Server Model:
  - TincServer.hpp
  - TincClient.hpp
  - TincProtocol.hpp
  - ...

- Utils:
  -


## Tinc Communication Protocols

Tinc uses the protobuf framework for defining its network protocol see:
- src/tinc_protocol.proto


## Tinc APIs

...
