#pragma once
#include <atomic> // For thread-safe counters
#include <cstdlib> // For std::malloc and std::free
#include <iostream>
#include"Logging.h"

namespace Czuch
{
	std::atomic<long long> total_allocations_bytes(0);
	std::atomic<long long> current_allocated_bytes(0);
	std::atomic<int> num_allocations(0);
	std::atomic<int> num_deallocations(0);

    void* operator new(std::size_t size) {
        total_allocations_bytes += size;
        current_allocated_bytes += size;
        num_allocations++;
        void* ptr = std::malloc(size);
        std::cout << "Allocated " << size << " bytes at " << ptr << "\n"; // Too verbose for large runs
        return ptr;
    }

    void operator delete(void* ptr) noexcept {
        if (ptr) {
            num_deallocations++;
            std::cout << "Deallocated at " << ptr << "\n";
            std::free(ptr); // Use std::free for actual deallocation
        }
    }

}