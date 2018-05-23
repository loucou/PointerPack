#include "PointerPack.h"
#include "SVOPointerPack.h"
#include <iostream>

template<typename PackType>
void check(const PackType& pack, void** ref, size_t n)
{
    for( size_t i = 0; i < n; i++ )
        if( pack.Get(i) != ref[i] )
            throw std::exception("Pointer check error");
}

void testPointerPack()
{
    float* ptr[4] = {
        (float*) 0b101'001,
        (float*) 0b101'100,
        (float*) 0b101'100,
        (float*) 0b101'001
    };

    PointerPack<float, 4> pack(ptr);

    check(pack, (void**) ptr, 4);

    pack.Set(1, (float*) 0b101'010);
    ptr[1] = (float*) 0b101'010;
    
    check(pack, (void**) ptr, 4);

    PointerPack<float, 4> pack2(pack);

    check(pack, (void**) ptr, 4);
    check(pack2, (void**) ptr, 4);
}

void testSVOPointerPack()
{
    float* ptr[4] = {
        (float*) 0b101'001,
        (float*) 0b101'100,
        (float*) 0b101'100,
        (float*) 0b101'001
    };

    SVOPointerPack<float, 4> pack(ptr);

    check(pack, (void**) ptr, 4);

    pack.Set(1, (float*) 0b101'010);
    ptr[1] = (float*) 0b101'010;

    check(pack, (void**) ptr, 4);

    SVOPointerPack<float, 4> pack2(pack);

    check(pack, (void**) ptr, 4);
    check(pack2, (void**) ptr, 4);
}

int main()
{
    bool success = true;

    try
    {
        testPointerPack();
        testSVOPointerPack();
    }
    catch( const std::exception& e )
    {
        success = false;
        std::cout << "Error: " << e.what() << std::endl;
    }

    if( success )
        std::cout << "All tests OK" << std::endl;     
}
