#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "NeaTS.hpp"

/**
 * @brief Accesses the element at the specified index within the NeaTS compressor.
 * * This function provides a wrapper around the compressor's operator[].
 * * @tparam KeyType The original key type used when setting up the compressor (e.g., uint32_t).
 * @tparam ValueType The original value type that was compressed (e.g., int64_t). This is the type returned.
 * @param compressor A constant reference to the initialized pfa::neats::compressor object.
 * @param index The 0-based index of the element to retrieve.
 * @return ValueType The decompressed value at the specified index.
 * * @note Asserts that the index is within the valid range [0, compressor.size()).
 */
template <typename KeyType, typename ValueType>
ValueType random_access(const pfa::neats::compressor<KeyType, ValueType>& compressor, size_t index) {
    // Optional: Add boundary check in debug mode
    assert(index < compressor.size() && "random_access: Index out of bounds"); 
    
    // Use the compressor's built-in random access operator
    return compressor[index]; 
}


// Function to prevent compiler optimization
template<typename T>
void do_not_optimize(T const& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

// Function to measure random access time
auto random_access_time(const pfa::neats::compressor<uint32_t, int64_t> &compressor, uint32_t num_runs = 10) {
    auto seed_val = 2323;
    std::mt19937 mt1(seed_val);
    size_t dt = 0;
    for (auto j = 0; j < num_runs; ++j) {
        size_t num_queries = 1e+6;
        // select query
        std::uniform_int_distribution<size_t> dist1(1, compressor.size() - 1);
        std::vector<size_t> indexes(num_queries);
        for (auto i = 0; i < num_queries; ++i) {
            indexes[i] = (dist1(mt1));
        }

        auto cnt = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (auto it = indexes.begin(); it < indexes.end(); ++it) {
            cnt += compressor[*it];
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        do_not_optimize(cnt);
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        dt += (time / num_queries);
    }
    return dt / num_runs;
};

int main() {
    // Parameters
    const uint32_t n = 1000;  // number of points
    const uint32_t num_random_accesses = 100;  // number of random access tests
    
    // Generate random data
    std::vector<std::pair<uint32_t, int64_t>> data;
    data.reserve(n);
    
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> dis(-1000, 1000);
    std::uniform_int_distribution<uint32_t> index_dis(0, n-1);
    
    // Generate data points
    for (uint32_t x = 1; x <= n; ++x) {
        int64_t y = dis(gen);
        data.emplace_back(x, y);
    }
    
    // Create NeaTS compressor with max_bpc = 32 for lossless compression
    pfa::neats::compressor<uint32_t, int64_t> compressor(32);
    
    // Extract y values for compression
    std::vector<int64_t> y_values;
    y_values.reserve(n);
    for (const auto& point : data) {
        y_values.push_back(point.second);
    }
    
    // Compress the data
    compressor.partitioning(y_values.begin(), y_values.end());
    
    
    // Test 3: Measure random access performance
    std::cout << "\nMeasuring random access performance..." << std::endl;
    auto avg_access_time = random_access_time(compressor);
    std::cout << "Average random access time: " << avg_access_time << " ns" << std::endl;
    

    // --- Example Usage of the new function ---
    if (compressor.size() > 0) {
    size_t test_index = compressor.size() / 2; // Example index
    if (test_index < compressor.size()) { // Ensure index is valid
         int64_t value_at_index = random_access(compressor, test_index);
         std::cout << "\nTesting random_access function:" << std::endl;
         std::cout << "Value at index " << test_index << ": " << value_at_index << std::endl;
         // You can compare it with the original value if needed:
         std::cout << "Original value at index " << test_index << ": " << y_values[test_index] << std::endl;
         assert(value_at_index == y_values[test_index]); // Verify correctness
    } else {
         std::cout << "\nCannot test random_access: Compressor is empty or index calculation failed." << std::endl;
    }

}

    // Calculate sizes
    size_t bits = compressor.size_in_bits();
    size_t bytes = (bits + 7) / 8;  // Round up to nearest byte
    
    // Print statistics
    std::cout << "\nCompression Statistics:" << std::endl;
    std::cout << "Number of points: " << n << std::endl;
    std::cout << "Size in bits: " << bits << std::endl;
    std::cout << "Size in bytes: " << bytes << std::endl;
    std::cout << "Compression ratio: " << std::fixed << std::setprecision(2) 
              << (double)(n * sizeof(int64_t)) / bytes << "x" << std::endl;
    
    return 0;
}