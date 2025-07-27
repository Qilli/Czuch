#pragma once
#include <atomic> // For thread-safe counters
#include <cstdlib> // For std::malloc and std::free
#include <iostream>
#include "Logging.h"

namespace Czuch
{
	extern std::atomic<long long> total_allocations_bytes;
	extern std::atomic<long long> current_allocated_bytes;
	extern std::atomic<int> num_allocations;
	extern std::atomic<int> num_deallocations;
}

void* operator new(std::size_t size);
void* operator new[](std::size_t size); 
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;