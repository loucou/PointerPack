PointerPack
===========

This is a header only library: simply include the headers in your project.

Uses these data structures to store multiple pointers in a compressed manner:
- PointerPack stores a single pointer to a heap allocated compressed array containing your data
- SVOPointerPack stores two pointers and uses Small Value Optimization to reduce heap allocations.

The bit compression is bases on the observation that pointers often start with several zeroes, and that they often share a common prefix.


TODO
----
- write unit tests
- finish SVOPointerPack
- implement heap allocated buffer re-use in both structures
- save 2 bits in 64bit mode and 4 in 32bit mode by compressing num zeroes and num specific
