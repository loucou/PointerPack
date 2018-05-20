PointerPack
===========

This is a header only library.

It proposes a data structure which stores pointers in a compressed manner, using bit packing.

It uses the observation that pointers often start with several zeroes, and that they often share a common prefix.
