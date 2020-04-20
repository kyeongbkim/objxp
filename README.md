Object eXpress, “ObjXp” in short, is an object exchange/synchronization framework between multiple processes in a distributed environment.

Client programs put, get and delete objects through simple APIs similar to the REST, and also provide the ability to automatically receive notification when specific objects are changed. In addition, it provides the ability to treat multiple objects in a single transaction when they are changed together, allowing it to control the race condition that may occur in multi-writer situations.

Currently, ObjXp's main code is written in C++ and supports the Python interface through the ‘swig’ utility. It will also support interfaces for Java and other programming languages in conjunction with Google's Protocol Buffers in the future.

For more information about ObjXp, please find a couple of slides in the "docs" folder.
