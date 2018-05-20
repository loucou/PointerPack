PointerPack
===========

This is a header only library.

It proposes a data structure which uses a bit packing technique in order to reduce the memory required to store multiple pointers.

It uses the observation that pointers often start with several zeroes, and that they often share a common prefix.
