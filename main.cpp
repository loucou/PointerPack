#include "PointerPack.h"
#include "SVOPointerPack.h"
#include <iostream>

void check()
{
    //TODO
}

void testPointerPack()
{
    float* ptr[8] = {
        (float*) 0b101'001,
        (float*) 0b101'100,
        (float*) 0b101'100,
        (float*) 0b101'001
    };

    PointerPack<float, 4> pack(ptr);

    pack.Set(1, (float*) 0b101'010);

    std::cout << "size: " << pack.AllocatedSize() << std::endl;

    for( size_t i = 0; i < 4; i++ )
    {
        float* p = pack.Get(i);
        std::cout << (uint64_t) ptr[i] << " -> " << (uint64_t) p;
        if( p != ptr[i] )
            std::cout << " !!!";
        std::cout << std::endl;
    }

    PointerPack<float, 4> pack2(pack);
    float* p = pack.Get(0);
    std::cout << (uint64_t) ptr[0] << " -> " << (uint64_t) p;
    if( p != ptr[0] )
        std::cout << " !!!";
    std::cout << std::endl;
}

void testSVOPointerPack()
{
    float* ptr[8] = {
        (float*) 0b101'001,
        (float*) 0b101'100,
        (float*) 0b101'100,
        (float*) 0b101'001
    };

    SVOPointerPack<float, 4> pack(ptr);

    pack.Set(1, (float*) 0b101'010);

    std::cout << "size: " << pack.AllocatedSize() << std::endl;

    for( size_t i = 0; i < 4; i++ )
    {
        float* p = pack.Get(i);
        std::cout << (uint64_t) ptr[i] << " -> " << (uint64_t) p;
        if( p != ptr[i] )
            std::cout << " !!!";
        std::cout << std::endl;
    }

    SVOPointerPack<float, 4> pack2(pack);
    float* p = pack.Get(0);
    std::cout << (uint64_t) ptr[0] << " -> " << (uint64_t) p;
    if( p != ptr[0] )
        std::cout << " !!!";
    std::cout << std::endl;
}

int main()
{
    //testPointerPack();
    testSVOPointerPack();
}
