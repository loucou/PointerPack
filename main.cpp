#include "PointerPack.h"
#include <iostream>

int main()
{
    float* ptr[8] = {
        (float*) 0b101'001,
        (float*) 0b101'100,
        (float*) 0b101'100,
        (float*) 0b101'001
    };

    PointerPack<float, 4> pack(ptr);

    pack.Set(1, (float*) 0b101'010);

    for( size_t i = 0; i < 4; i++ )
    {
        float* p = pack.Get(i);
        std::cout << (uint64_t) ptr[i] << " -> " << (uint64_t) p;
        if( p != ptr[i] )
            std::cout << " !!!";
        std::cout << std::endl;
    }

}
