#include "czpch.h"
#include "MemoryControl.h"

namespace Czuch
{
    std::atomic<long long> total_allocations_bytes(0);
    std::atomic<long long> current_allocated_bytes(0);
    std::atomic<int> num_allocations(0);
    std::atomic<int> num_deallocations(0);
}

void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);

    if (!ptr) 
    {
        throw std::bad_alloc();
    }

    Czuch::total_allocations_bytes += size;
    Czuch::current_allocated_bytes += size;
    Czuch::num_allocations++;
  /*  if (Czuch::num_allocations.load() % 1000 == 0)
    {
        std::cout << "Allocated " << size << " bytes, total allocations: "
            << Czuch::total_allocations_bytes.load() << " bytes, current allocated: "
            << Czuch::current_allocated_bytes.load() << " bytes, allocations count: "
            << Czuch::num_allocations.load() << std::endl;
    }*/
    return ptr;
}

void* operator new[](std::size_t size) {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    if (ptr) 
    {
        Czuch::num_deallocations++;
        std::free(ptr);
    }
}

void operator delete[](void* ptr) noexcept {
    return operator delete(ptr); 
}
