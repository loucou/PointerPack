PointerPack
===========

This is a header only library: simply copy and include PointerPack.h or SVOPointerPack in your project.

It proposes two data structure to store multiple pointers in a compressed manner:
- PointerPack contains a single pointer to a heap allocated compressed array containing your data
- SVOPointerPack (not finished) contains two pointers and uses Small Value Optimization to prevent most heap allocations.

The bit compression is bases on the observation that pointers often start with several zeroes, and that they often share a common prefix.


TODO
----
- write unit tests
- finish SVOPointerPack
