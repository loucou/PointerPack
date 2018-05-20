#pragma once

#include <cstdint>
#include <climits>

template<typename T, size_t N>
class PointerPack
{
public:
    PointerPack(T** const pointerArray = nullptr)
        : _buffer(nullptr)
    {
        static_assert(CHAR_BIT == 8, "Expecting 8 bits words");
        static_assert(sizeof(uint8_t) == 1, "Expecting 1 word for uint8_t");
        static_assert(sizeof(T*) == sizeof(uintptr_t), "Expecting uintptr_t and void* to be the same size");
        static_assert(sizeof(T*) * CHAR_BIT < UCHAR_MAX, "Limitation on pointer size");
        Set(pointerArray);
    }

    PointerPack(const PointerPack& other)
        : _buffer(nullptr)
    {
        (*this) = other;
    }

    PointerPack(PointerPack&& other)
    {
        (*this) = std::move(other);
    }

    PointerPack& operator=(const PointerPack& other)
    {
        delete[] _buffer;
        size_t size = other.AllocatedSize();
        _buffer = new uint8_t[size];
        for( size_t i = 0; i < size; i++ )
            _buffer[i] = other._buffer[i];
    }

    PointerPack& operator=(PointerPack&& other)
    {
        delete[] _buffer;
        _buffer = other._buffer;
        other._buffer = nullptr;
    }

    ~PointerPack()
    {
        delete[] _buffer;
        _buffer = nullptr;
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
        delete[] _buffer;
        _buffer = nullptr;

        if( !pointerArray )
            return;

        bool allNullptr = true;
        for( size_t i = 0; i < N && allNullptr; i++ )
            if( pointerArray[i] )
                allNullptr = false;

        if( allNullptr )
            return;

        uintptr_t* ptrs = (uintptr_t*) pointerArray;

        uintptr_t ones = 0u;
        uintptr_t diff = 0u;
        uintptr_t ref = ptrs[0];
        for( size_t i = 0; i < N; i++ )
        {
            ones |= ptrs[i]; // each bit is 1 where any ptrs[i] is 1
            diff |= (ref ^ ptrs[i]); // each bit is 1 where ref and ptrs[i] differ
        }

        uint8_t PointerBits = CHAR_BIT * sizeof(T*);

        uint8_t numZeroBits = PointerBits;
        for( ; ones; ones >>= 1u )
            numZeroBits--;

        uint8_t numSharedBits = PointerBits;
        for( ; diff; diff >>= 1u )
            numSharedBits--;

        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        uint8_t numSpecificBits = PointerBits - numSharedBits;
        size_t numBits = 2 * CHAR_BIT + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (CHAR_BIT - 1)) / CHAR_BIT;

        _buffer = new uint8_t[numBytes];

        _buffer[0] = numZeroBits;

        _buffer[1] = numSharedBits;

        size_t writeOffset = 2 * CHAR_BIT;

        for( size_t b = 0; b < numSharedNonZeroBits; b++ )
        {
            uint8_t bitValue = readBit((uint8_t*) &ptrs[0], numSpecificBits + b);
            writeBit(_buffer, writeOffset++, bitValue);
        }

        for( size_t pointerIndex = 0; pointerIndex < N; pointerIndex++ )
        {
            for( size_t b = 0; b < numSpecificBits; b++ )
            {
                uint8_t bitValue = readBit((uint8_t*) &ptrs[pointerIndex], b);
                writeBit(_buffer, writeOffset++, bitValue);
            }
        }
    }

    T* const Get(size_t index) const
    {
        if( !_buffer )
            return nullptr;

        uint8_t PointerBits = CHAR_BIT * sizeof(T*);
        uint8_t numZeroBits = _buffer[0];
        uint8_t numSharedBits = _buffer[1];
        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        uint8_t numSpecificBits = PointerBits - numSharedBits;

        uintptr_t res = 0u;

        for( size_t b = 0; b < numSharedNonZeroBits; b++ )
        {
            uint8_t bitValue = readBit(_buffer, 2 * CHAR_BIT + b);
            writeBit((uint8_t*) &res, numSpecificBits + b, bitValue);
        }

        for( size_t b = 0; b < numSpecificBits; b++ )
        {
            size_t readOffset = 2 * CHAR_BIT + numSharedNonZeroBits + index * numSpecificBits + b;
            uint8_t bitValue = readBit(_buffer, readOffset);
            writeBit((uint8_t*) &res, b, bitValue);
        }

        return (T*) res;
    }

    size_t AllocatedSize() const
    {
        if( !_buffer )
            return 0;

        uint8_t numZeroBits = _buffer[0];
        uint8_t numSharedBits = _buffer[1];

        size_t numBits = 2 * CHAR_BIT + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (CHAR_BIT - 1)) / CHAR_BIT;

        return numBytes;
    }

private:
    uint8_t * _buffer;

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
