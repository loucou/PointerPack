PointerPack
===========

This is a header only library: simply include the headers in your project.

Use these data structures to store multiple pointers in a compressed manner:
- PointerPack stores a single pointer to a heap allocated compressed array containing your data
- SVOPointerPack stores two pointers and uses Small Value Optimization to reduce heap allocations.

The bit compression is based on these observations:
- pointers often start with several zeroes
- they often share a common prefix
- the last bits are often the same (alignement)

TODO
----
- finish SVOPointerPack
- implement heap allocated buffer re-use in both structures
- check memory leaks
- write proper github CI unit tests
- save 2 bits in 64bit mode and 4 in 32bit mode by compressing num zeroes and num specific
- save up to 3 alignment bits
- benchmark memory and speed, compare with simply zipping / unzipping the pointer array
