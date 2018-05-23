#pragma once

#include <cstdint>

template<typename T, size_t N>
class SVOPointerPack
{
    constexpr static uint8_t kBitsPerByte = 8u;
    constexpr static uint8_t kBytesPerPointer = sizeof(void*);
    constexpr static uint8_t kBitsPerPointer = kBitsPerByte * kBytesPerPointer;

public:
    SVOPointerPack(T** const pointerArray = nullptr)
        : _buffer{ 0u, (uintptr_t) nullptr }
    {
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
        Clear(); // TODO: reuse allocated buffer if possible
        
        uint8_t* buf = (uint8_t*) _buffer;
        const uint8_t* otherbuf = (const uint8_t*) other._buffer;
        
        uint8_t numZeroBits, numSpecificBits, numSharedNonZeroBits;
        size_t numBytes = 0u;
        info(otherbuf, numZeroBits, numSpecificBits, numSharedNonZeroBits, numBytes);
        bool usesHeap = numBytes > 2 * sizeof(uintptr_t);

        if( usesHeap )
        {
            size_t nAllocBytes = numBytes - sizeof(uintptr_t);
            _buffer[1] = (uintptr_t) new uint8_t[nAllocBytes];
            std::memcpy((void*) _buffer[1], (const void*) other._buffer[1], nAllocBytes);
        }

        _buffer[0] = other._buffer[0];

        return *this;
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

        
        uint8_t numZeroBits = buf[0];
        uint8_t numSpecificBits = buf[1];
        uint8_t numSharedBits = kBitsPerPointer - numSpecificBits;
        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        size_t numBits = 2 * kBitsPerByte + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (kBitsPerByte - 1)) / kBitsPerByte;

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
        Clear(); // TODO: reuse allocated buffer if possible

        uint8_t numZeroBits, numSharedBits;
        measure(pointerArray, numZeroBits, numSharedBits);

        uintptr_t* ptrs = (uintptr_t*) pointerArray;

        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        uint8_t numSpecificBits = kBitsPerPointer - numSharedBits;
        size_t numBits = 2 * kBitsPerByte + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (kBitsPerByte - 1)) / kBitsPerByte;

        bool usesHeap = numBytes > 2 * sizeof(uintptr_t);

        if( usesHeap )
            _buffer[1] = (uintptr_t) new uint8_t[numBytes - sizeof(uintptr_t)];

        uint8_t* buf = (uint8_t*) _buffer;

        buf[0] = numZeroBits;

        buf[1] = numSpecificBits;

        size_t writeOffset = 2 * kBitsPerByte;

        for( size_t b = 0; b < numSharedNonZeroBits; b++ )
        {
            uint8_t bitValue = readBit((uint8_t*) &ptrs[0], numSpecificBits + b);
            writeBitToSVOBuffer(usesHeap, writeOffset++, bitValue);
        }

        for( size_t pointerIndex = 0; pointerIndex < N; pointerIndex++ )
        {
            for( size_t b = 0; b < numSpecificBits; b++ )
            {
                uint8_t bitValue = readBit((uint8_t*) &ptrs[pointerIndex], b);
                writeBitToSVOBuffer(usesHeap, writeOffset++, bitValue);
            }
        }
    }

    T* const Get(size_t index) const
    {
        uint8_t* buf = (uint8_t*) _buffer;

        uint8_t numZeroBits = buf[0];
        if( numZeroBits == kBitsPerPointer )
            return nullptr;

        uint8_t numSpecificBits = buf[1];
        uint8_t numSharedBits = kBitsPerPointer - numSpecificBits;
        uint8_t numSharedNonZeroBits = numSharedBits - numZeroBits;
        size_t numBits = 2 * kBitsPerByte + numSharedNonZeroBits + N * numSpecificBits;
        size_t numBytes = (numBits + (kBitsPerByte - 1)) / kBitsPerByte;

        bool usesHeap = numBytes > 2 * sizeof(uintptr_t);

        uintptr_t res = 0u;

        for( size_t b = 0; b < numSharedNonZeroBits; b++ )
        {
            uint8_t bitValue = readBitFromSVOBuffer(usesHeap, 2 * kBitsPerByte + b);
            writeBit((uint8_t*) &res, numSpecificBits + b, bitValue);
        }

        for( size_t b = 0; b < numSpecificBits; b++ )
        {
            size_t readOffset = 2 * kBitsPerByte + numSharedNonZeroBits + index * numSpecificBits + b;
            uint8_t bitValue = readBitFromSVOBuffer(usesHeap, readOffset);
            writeBit((uint8_t*) &res, b, bitValue);
        }

        return (T*) res;
    }

    void Clear()
    {
        uint8_t* buf = (uint8_t*) _buffer;

        uint8_t numZeroBits, numSpecificBits, numSharedNonZeroBits;
        size_t numBytes = 0u;
        info(buf, numZeroBits, numSpecificBits, numSharedNonZeroBits, numBytes);
        bool usesHeap = numBytes > 2 * sizeof(uintptr_t);

        if( usesHeap )
        {
            uint8_t* heapbuf = (uint8_t*) _buffer[1];
            delete[] heapbuf;
        }

        buf[0] = kBitsPerPointer;
        buf[1] = 0;
        _buffer[1] = (uintptr_t) nullptr;
    }

private:
    uintptr_t _buffer[2];

    static void measure(T** const pointerArray, uint8_t& numZeroBits, uint8_t& numSharedBits)
    {
        uintptr_t* ptrs = (uintptr_t*) pointerArray;

        uintptr_t ones = 0u;
        uintptr_t diff = 0u;
        uintptr_t ref = ptrs[0];
        for( size_t i = 0; i < N; i++ )
        {
            ones |= ptrs[i]; // each bit is 1 where any ptrs[i] is 1
            diff |= (ref ^ ptrs[i]); // each bit is 1 where ref and ptrs[i] differ
        }

        numZeroBits = kBitsPerPointer;
        for( ; ones; ones >>= 1u )
            numZeroBits--;

        numSharedBits = kBitsPerPointer;
        for( ; diff; diff >>= 1u )
            numSharedBits--;
    }

    static void info(const uint8_t* buffer, uint8_t& numZeroBits, uint8_t& numSpecificBits, uint8_t& numSharedNonZeroBits, size_t numBytes)
    {
        numZeroBits = buffer[0];
        numSpecificBits = buffer[1];
        uint8_t numSharedBits = kBitsPerPointer - numSpecificBits;
        numSharedNonZeroBits = numSharedBits - numZeroBits;
        size_t numBits = 2 * kBitsPerByte + numSharedNonZeroBits + N * numSpecificBits;
        numBytes = (numBits + (kBitsPerByte - 1)) / kBitsPerByte;
    }

    const uint8_t* const getSVOBuffer(bool usesHeap, size_t bitIndex) const
    {
        const uint8_t* pbuffer = ( usesHeap && bitIndex >= kBitsPerPointer ) 
            ? (const uint8_t*) _buffer[1] 
            : (const uint8_t*) &_buffer;
        return pbuffer;
    }

    uint8_t readBitFromSVOBuffer(bool usesHeap, size_t bitIndex) const
    {
        const uint8_t* const pbuffer = getSVOBuffer(usesHeap, bitIndex);
        return readBit(pbuffer, bitIndex);
    }

    void writeBitToSVOBuffer(bool usesHeap, size_t bitIndex, uint8_t bitValue)
    {
        uint8_t* const pbuffer = (uint8_t* const) getSVOBuffer(usesHeap, bitIndex);
        writeBit(pbuffer, bitIndex, bitValue);
    }

    static uint8_t readBit(const uint8_t* const buffer, size_t bitIndex)
    {
        size_t byteIndex = bitIndex / kBitsPerByte;
        size_t bitOffset = bitIndex % kBitsPerByte;
        return (buffer[byteIndex] >> bitOffset) & 1u;
    }

    static void writeBit(uint8_t* const buffer, size_t bitIndex, uint8_t bitValue)
    {
        size_t byteIndex = bitIndex / kBitsPerByte;
        size_t bitOffset = bitIndex % kBitsPerByte;

        uint8_t mask = ~(1u << bitOffset);
        buffer[byteIndex] &= mask;

        mask = (bitValue & 1u) << bitOffset;
        buffer[byteIndex] |= mask;
    }


};
