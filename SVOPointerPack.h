#pragma once

#include <cstdint>
#include <climits>

template<typename T, size_t N>
class SVOPointerPack
{
public:
    SVOPointerPack(T** const pointerArray = nullptr)
        : _buffer{ 0u, 0u }
    {
        static_assert(CHAR_BIT == 8, "Expecting 8 bits words");
        static_assert(sizeof(uint8_t) == 1, "Expecting 1 word for uint8_t");
        static_assert(sizeof(T*) == sizeof(uintptr_t), "Expecting uintptr_t and void* to be the same size");
        static_assert(sizeof(T*) * CHAR_BIT < UCHAR_MAX, "Limitation on pointer size");
        Set(pointerArray);
    }

    SVOPointerPack(const SVOPointerPack& other)
        : _buffer{ 0u, 0u }
    {
        (*this) = other;
    }

    SVOPointerPack(SVOPointerPack&& other)
    {
        (*this) = std::move(other);
    }

    SVOPointerPack& operator=(const SVOPointerPack& other)
    {
        //delete[] _buffer;
        //size_t size = other.AllocatedSize();
        //_buffer = new uint8_t[size];
        //for( size_t i = 0; i < size; i++ )
        //    _buffer[i] = other._buffer[i];
        //return *this;
        throw std::exception("not implemented");
    }

    SVOPointerPack& operator=(SVOPointerPack&& other)
    {
        //delete[] _buffer;
        //_buffer = other._buffer;
        //other._buffer = nullptr;
        //return *this;
        throw std::exception("not implemented");
    }

    ~SVOPointerPack()
    {
        uint8_t* buf = (uint8_t*) _buffer;

        uint8_t PointerBits = CHAR_BIT * sizeof(T*);
        uint8_t numZeroBits = buf[0];
        uint8_t numSpecificBits = buf[1];
        uint8_t numSharedBits = PointerBits - numSpecificBits;
        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        size_t numBits = 2 * CHAR_BIT + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (CHAR_BIT - 1)) / CHAR_BIT;

        bool usesHeap = numBytes > N * sizeof(T*);

        if( usesHeap )
            delete[] (uint8_t*) _buffer[1];

        _buffer[0] = 0u;
        _buffer[1] = 0u;
    }

    void Set(size_t index, T* const pointer)
    {
        if( index >= N )
            throw std::invalid_argument("Index exceeds size");

        T* pointers[N];

        for( size_t i = 0; i < N; i++ )
            pointers[i] = i != index ? Get(i) : pointer;

        Set(pointers);
    }

    void Set(T** const pointerArray)
    {
        //delete[] _buffer;
        //_buffer = nullptr;

        //if( !pointerArray )
        //    return;

        //bool allNullptr = true;
        //for( size_t i = 0; i < N && allNullptr; i++ )
        //    if( pointerArray[i] )
        //        allNullptr = false;

        //if( allNullptr )
        //    return;

        //uintptr_t* ptrs = (uintptr_t*) pointerArray;

        //uintptr_t ones = 0u;
        //uintptr_t diff = 0u;
        //uintptr_t ref = ptrs[0];
        //for( size_t i = 0; i < N; i++ )
        //{
        //    ones |= ptrs[i]; // each bit is 1 where any ptrs[i] is 1
        //    diff |= (ref ^ ptrs[i]); // each bit is 1 where ref and ptrs[i] differ
        //}

        //uint8_t PointerBits = CHAR_BIT * sizeof(T*);

        //uint8_t numZeroBits = PointerBits;
        //for( ; ones; ones >>= 1u )
        //    numZeroBits--;

        //uint8_t numSharedBits = PointerBits;
        //for( ; diff; diff >>= 1u )
        //    numSharedBits--;

        //uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        //uint8_t numSpecificBits = PointerBits - numSharedBits;
        //size_t numBits = 2 * CHAR_BIT + numSharedNonZeroBits + N * numSpecificBits;
        //size_t numBytes = (numBits + (CHAR_BIT - 1)) / CHAR_BIT;

        //_buffer = new uint8_t[numBytes];

        //_buffer[0] = numZeroBits;

        //_buffer[1] = numSharedBits;

        //size_t writeOffset = 2 * CHAR_BIT;

        //for( size_t b = 0; b < numSharedNonZeroBits; b++ )
        //{
        //    uint8_t bitValue = readBit((uint8_t*) &ptrs[0], numSpecificBits + b);
        //    writeBit(_buffer, writeOffset++, bitValue);
        //}

        //for( size_t pointerIndex = 0; pointerIndex < N; pointerIndex++ )
        //{
        //    for( size_t b = 0; b < numSpecificBits; b++ )
        //    {
        //        uint8_t bitValue = readBit((uint8_t*) &ptrs[pointerIndex], b);
        //        writeBit(_buffer, writeOffset++, bitValue);
        //    }
        //}
        throw std::exception("not implemented");
    }

    T* const Get(size_t index) const
    {
        //if( _buffer[0] == 0u )
        //    return nullptr;

        uint8_t* buf = (uint8_t*) _buffer;

        uint8_t PointerBits = CHAR_BIT * sizeof(T*);
        uint8_t numZeroBits = buf[0];
        uint8_t numSpecificBits = buf[1];
        uint8_t numSharedBits = PointerBits - numSpecificBits;
        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        size_t numBits = 2 * CHAR_BIT + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (CHAR_BIT - 1)) / CHAR_BIT;

        bool usesHeap = numBytes > N * sizeof(T*);

        uintptr_t res = 0u;

        for( size_t b = 0; b < numSharedNonZeroBits; b++ )
        {
            uint8_t bitValue = readBitFromSVOBuffer(usesHeap, 2 * CHAR_BIT + b);
            writeBit((uint8_t*) &res, numSpecificBits + b, bitValue);
        }

        for( size_t b = 0; b < numSpecificBits; b++ )
        {
            size_t readOffset = 2 * CHAR_BIT + numSharedNonZeroBits + index * numSpecificBits + b;
            uint8_t bitValue = readBitFromSVOBuffer(usesHeap, readOffset);
            writeBit((uint8_t*) &res, b, bitValue);
        }

        return (T*) res;
    }

    size_t AllocatedSize() const
    {
        //if( !_buffer )
        //    return 0;

        //uint8_t PointerBits = CHAR_BIT * sizeof(T*);
        //uint8_t numZeroBits = _buffer[0];
        //uint8_t numSharedBits = _buffer[1];
        //uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        //uint8_t numSpecificBits = PointerBits - numSharedBits;

        //size_t numBits = 2 * CHAR_BIT + numSharedNonZeroBits + N * numSpecificBits;
        //size_t numBytes = (numBits + (CHAR_BIT - 1)) / CHAR_BIT;

        //return numBytes;
        throw std::exception("not implemented");
    }

private:
    uintptr_t _buffer[2];

    uint8_t readBitFromSVOBuffer(bool usesHeap, size_t bitIndex) const
    {
        const uint8_t* pbuffer = (!usesHeap || bitIndex < sizeof(T*) * CHAR_BIT) 
            ? (const uint8_t*) &_buffer 
            : (const uint8_t*) _buffer[1];
        return readBit(pbuffer, bitIndex);
    }

    void writeBitFromSVOBuffer(bool usesHeap, size_t bitIndex, uint8_t bitValue)
    {
        const uint8_t* pbuffer = (!usesHeap || bitIndex < sizeof(T*) * CHAR_BIT) ? &_buffer : _buffer[1];
        writeBit(pbuffer, bitIndex, bitValue);
    }

    static uint8_t readBit(const uint8_t* const buffer, size_t bitIndex)
    {
        size_t byteIndex = bitIndex / CHAR_BIT;
        size_t bitOffset = bitIndex % CHAR_BIT;
        return (buffer[byteIndex] >> bitOffset) & 1u;
    }

    static void writeBit(uint8_t* const buffer, size_t bitIndex, uint8_t bitValue)
    {
        size_t byteIndex = bitIndex / CHAR_BIT;
        size_t bitOffset = bitIndex % CHAR_BIT;

        uint8_t mask = ~(1u << bitOffset);
        buffer[byteIndex] &= mask;

        mask = (bitValue & 1u) << bitOffset;
        buffer[byteIndex] |= mask;
    }
};
