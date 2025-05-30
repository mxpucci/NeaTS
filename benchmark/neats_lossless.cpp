#include <iostream>
#include "../include/NeaTS.hpp"
#include <filesystem>
#include <chrono>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/dac_vector.hpp>
#include "gp_compressors_bench.hpp"
#include "../include/NeaTSL.hpp"
#include "../include/algorithms.hpp"
#include "AdaptiveApproximation.hpp"
#include <climits>
#include "st_compressors_bench.hpp"

using x_t = uint32_t;
using y_t = int64_t;

auto random_access_time(const auto &compressor, uint32_t num_runs = 10) {
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
        auto time = duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        dt += (time / num_queries);
    }
    return dt / num_runs;
};

double dac_scan_speed(const auto &compressor, uint32_t range) {
    auto seed_val = 1234;
    std::mt19937 mt1(seed_val);
    size_t dt = 0;
    size_t num_queries = 10000;
    // select query
    std::uniform_int_distribution<size_t> dist1(0, compressor.size() - range);
    std::vector<size_t> indexes(num_queries);
    for (auto i = 0; i < num_queries; ++i) {
        indexes[i] = (dist1(mt1));
    }

    //auto cnt = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (auto i : indexes) {
        std::vector<uint64_t> out(range);
        std::copy(compressor.begin() + i, compressor.begin() + i + range, out.begin());
        //cnt += compressor[i];
        do_not_optimize(out);

        /*
        // check if the scan is correct
        for (auto j = 0; j < range; ++j) {
            if (compressor[i + j] != out[j]) {
                std::cout << "Error in DAC scan" << std::endl;
                exit(1);
            }
        }
         */
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    auto time = duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    //dt = time / num_queries;
    //std::cout << "Time: " << dt << std::endl;
    auto speed = (((range * 8) * num_queries) / 1e6) / ((double) time / 1e9);
    //auto speed = (((range * 8) * num_queries) / 1e6) / ((double) time / 1e9);
    return speed;
};


template<typename T>
auto full_decompression_time(const auto &compressor, uint32_t num_runs = 1) {
    size_t res = 0;
    for (auto i = 0; i < num_runs; ++i) {
        std::vector<T> decompressed(compressor.size());
        auto t1 = std::chrono::high_resolution_clock::now();
        compressor.decompress(decompressed.begin(), decompressed.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        do_not_optimize(decompressed);
        auto ns_int = duration_cast<std::chrono::nanoseconds>(t2 - t1);
        res += ns_int.count();
    }
    return res / num_runs;
};

/*
template<auto bpc, typename poly, typename T1, typename T2>
void run(const auto &fn, bool header = false) {
    constexpr int64_t epsilon = BPC_TO_EPSILON(bpc);
    neats::lossless_compressor<x_t, y_t, bpc, poly, T1, T2> lc;
//
    auto data = fa::utils::read_data_binary<int64_t, int64_t>(fn);

    auto min_data = *std::min_element(data.begin(), data.end());
    auto max_data = *std::max_element(data.begin(), data.end());
    //std::cout << LOG2(max_data - min_data) << std::endl;
    min_data = min_data < 0 ? (min_data - 1) : -1;
//
    std::for_each(data.begin(), data.end(), [min_data, epsilon](auto &d) { d -= (min_data - epsilon); });
    auto start = std::chrono::high_resolution_clock::now();
    lc.partitioning(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

//
    typename std::decay<decltype(data)>::type decompressed(data.size());
    lc.decompress(decompressed.begin(), decompressed.end());

    for (auto i = 0; i < data.size(); ++i) {
        if (data[i] != decompressed[i]) {
            std::cout << i << ": " << data[i] << "!=" << decompressed[i] << std::endl;
            exit(1);
        }
    }

    auto uncompressed_bit_size = data.size() * sizeof(y_t) * 8;
    auto compressed_ra_bit_size = lc.size_in_bits();
    auto compressed_storage_bit_size = lc.storage_size_in_bits();
    auto cr_random_access = (long double) (compressed_ra_bit_size) / (long double) (uncompressed_bit_size);
    auto cr_only_storage = (long double) (compressed_storage_bit_size) / (long double) (uncompressed_bit_size);

    if (header) {
        std::cout << "compressor,dataset,max_bpc,uncompressed_bit_size,num_elements,"
                  << "compressed_random_access_bit_size,compressed_storage_bit_size,cr_random_access,cr_only_storage,compression_time(ns),"
                  << "RA_residuals_bit_size,RA_offset_residuals_bit_size,RA_coefficients_bit_size,RA_model_types_bit_size,RA_rank&select_bit_size,RA_starting_positions_bit_size,RA_bpc_bit_size,"
                  << "OS_residuals_bit_size,OS_coefficients_bit_size,OS_model_types_bit_size,OS_starting_positions_bit_size,OS_bpc_bit_size,"
                  << "random_access_time(ns),full_decompression_time(ns),SIMD_decompression_time(ns)"
                  << std::endl;
    }
    std::cout << "NeaTS," << fn << "," << bpc << "," << uncompressed_bit_size << "," << data.size() << ","
              << compressed_ra_bit_size << "," << compressed_storage_bit_size << "," << cr_random_access << ","
              << cr_only_storage << "," << ns << ",";
    lc.size_info();
    lc.storage_size_info();


    for (auto i = 0; i < data.size(); ++i) {
        if (lc[i] != data[i]) {
            std::cout << std::endl;
            std::cout << i << ": " << lc[i] << "!=" << decompressed[i] << std::endl;
            exit(1);
        }
    }

    size_t rat = random_access_time(lc);
    size_t fdt = full_decompression_time<int64_t>(lc);
    std::cout << rat << ","
              << fdt << ",";

    auto num_runs = 50;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (auto r = 0; r < num_runs; ++r) {
        std::vector<int64_t> decompressed_SIMD(data.size());
        lc.decompress_SIMD(decompressed_SIMD.data());
        do_not_optimize(decompressed_SIMD);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / num_runs;
    std::cout << time << std::endl;

    std::vector<int64_t> decompressed_SIMD(data.size());
    lc.decompress_SIMD(decompressed_SIMD.data());

    for (auto i = 0; i < data.size(); ++i) {
        if (data[i] != decompressed_SIMD[i]) {
            std::cout << "SIMD decompression error at index: ";
            std::cout << i << ": " << data[i] << "!=" << decompressed_SIMD[i] << std::endl;
            exit(1);
        }
    }

    std::ofstream out(fn + ".neats");
    //out.clear();
    size_t size_in_bytes = lc.serialize(out);
    out.close();
    //std::cout << "Size in bytes: " << size_in_bytes << std::endl;
    std::ifstream in(fn + ".neats");
    lc.load(in);
    in.close();
    for (auto i = 0; i < data.size(); ++i) {
        if (data[i] != lc[i]) {
            std::cout << "Loading error at index: ";
            std::cout << i << ": " << data[i] << "!=" << decompressed_SIMD[i] << std::endl;
            exit(1);
        }
    }

}

template<typename poly = double, typename T1 = float, typename T2 = double>
void inline run_bpcs(const auto &fn) {
    run<8, poly, T1, T2>(fn);
    run<9, poly, T1, T2>(fn);
    run<10, poly, T1, T2>(fn);
    run<11, poly, T1, T2>(fn);
    run<12, poly, T1, T2>(fn);
    run<13, poly, T1, T2>(fn);
    run<14, poly, T1, T2>(fn);
    run<15, poly, T1, T2>(fn);
    run<16, poly, T1, T2>(fn);
    run<17, poly, T1, T2>(fn);
    run<18, poly, T1, T2>(fn);
    run<19, poly, T1, T2>(fn);
    run<20, poly, T1, T2>(fn);
}
*/

/* Given a directory f returns a vector of all the files in the directory */
std::vector<std::string> get_files(const std::string &path) {
    std::vector<std::string> files;
    for (const auto &entry: std::filesystem::directory_iterator(path)) {
        //put only files with the extension .bin
        if (entry.path().extension() == ".bin")
            files.push_back(entry.path());
    }
    return files;
}

void run_all(const auto &path) {
    auto fns = get_files(path);

    for (const auto &fn: fns) {
        if ((fn.compare(path + std::string("geolife-lat.bin")) == 0) ||
            (fn.compare(path + std::string("geolife-lon.bin")) == 0)) {
            run_bpcs<long double, double, double>(fn);
        } else {
            run_bpcs<double, float, double>(fn);
        }
    }
    //std::cout << "DONE" << std::endl;
}

void dac_compression_full(const std::string& filename, std::ostream &out) {
    using T = int64_t;
    out << "filename,compressor,#values,uncompressed_bit_size,compressed_bit_size,compression_ratio,decompression_time_ns,compression_time_ns,random_access_time_ns" << std::endl;
    //auto fns = get_files(path);
    //for (auto filename: fns) {
    const auto data = pfa::algorithm::io::read_data_binary<T, T>(filename, false);
    //auto min_element = *std::min_element(data.begin(), data.end());
    auto min_data = *std::min_element(data.begin(), data.end());
    min_data = min_data < 0 ? (min_data - 1) : -1;

    std::vector<uint64_t> u_data(data.size());
    std::transform(data.begin(), data.end(), u_data.begin(),
                   [&](const auto &x) { return static_cast<uint64_t>(x - min_data); });

    auto t1 = std::chrono::high_resolution_clock::now();
    const auto dac_vector = sdsl::dac_vector_dp(u_data);
    auto t2 = std::chrono::high_resolution_clock::now();
    do_not_optimize(dac_vector);
    auto compression_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    std::vector<uint64_t> u_decompressed(data.size());
    for (auto i = 0; i < data.size(); ++i) {
        u_decompressed[i] = dac_vector[i];
    }
    t2 = std::chrono::high_resolution_clock::now();
    do_not_optimize(u_decompressed);
    auto decompression_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    auto compressed_bit_size = sdsl::size_in_bytes(dac_vector) * 8;

    out << filename << "," << "DAC" << "," << data.size() << "," << data.size() * sizeof(T) * 8 << ","
        << compressed_bit_size << ","
        << (long double) (compressed_bit_size) / (long double) (data.size() * sizeof(T) * 8) << ","
        << decompression_time << "," << compression_time << ",";

    for (auto j = 0; j < data.size(); ++j) {
        if (u_data[j] != u_decompressed[j]) {
            std::cerr << "Error during decompression" << std::endl;
            exit(1);
        }
    }

    out << random_access_time(dac_vector) << ",";

    for (auto i = 10; i <= 1000000; i *= 2) {
        auto ssp = dac_scan_speed(dac_vector, i);
        std::cout << ssp << ",";
    }

    //double scan_speed = dac_scan_speed(dac_vector, 20);
    //std::cout << "DAC scan speed: " << scan_speed << std::endl;
}

/*
template<int64_t error_bound, typename T0, typename T1, typename T2>
void lossy_compression(const auto &fn, std::ostream &out) {
    neats::lossy_compressor<x_t, y_t, error_bound, T0, T1, T2> lc;
    auto data = pfa::utils::read_data_binary<y_t, y_t>(fn);
    auto range = *std::max_element(data.begin(), data.end()) - *std::min_element(data.begin(), data.end());
    auto min_data = *std::min_element(data.begin(), data.end());
    min_data = min_data < 0 ? (min_data - 1) : -1;
    std::for_each(data.begin(), data.end(), [min_data](auto &d) { d -= (min_data - error_bound); });
    lc.partitioning(data.begin(), data.end());
    std::vector<y_t> decompressed(data.size());
    lc.decompress(decompressed.begin(), decompressed.end());

    for (auto i = 0; i < data.size(); ++i) {
        auto err = data[i] - decompressed[i];
        if (err > error_bound || err < -(error_bound + 1)) {
            std::cout << i << ": " << data[i] << "!=" << decompressed[i] << std::endl;
            exit(1);
        }
    }

    auto uncompressed_bit_size = data.size() * sizeof(y_t) * 8;
    auto compressed_bit_size = lc.size_in_bits();
    auto cr = (long double) (compressed_bit_size) / (long double) (uncompressed_bit_size);

    out << "NeaTS-Lossy," << fn << "," << error_bound << "," << uncompressed_bit_size << "," << data.size() << ","
        << compressed_bit_size << "," << cr << "," << range << std::endl;
}

template<int64_t error_bound, typename T0, typename T1, typename T2>
void pla_compression(const auto &fn, std::ostream &out) {
    using poa_t = fa::pfa::piecewise_optimal_approximation<x_t, y_t, T0, T1, T2>;
    auto data = fa::utils::read_data_binary<y_t, y_t>(fn);
    auto range = *std::max_element(data.begin(), data.end()) - *std::min_element(data.begin(), data.end());
    //auto min_data = *std::min_element(data.begin(), data.end());
    //min_data = min_data < 0 ? (min_data - 1) : -1;
    //std::for_each(data.begin(), data.end(), [min_data](auto &d) { d -= min_data; });
    auto res = fa::algorithm::make_pla<typename poa_t::pla_t, error_bound>(data.begin(), data.end());

    auto uncompressed_bit_size = data.size() * sizeof(y_t) * 8;
    size_t compressed_bit_size = 0;
    compressed_bit_size += res.size() * sizeof(x_t) * 8; // starting positions
    compressed_bit_size += res.size() * sizeof(T1) * 8; // slope
    compressed_bit_size += res.size() * sizeof(T2) * 8; // intercept

    auto cr = (long double) (compressed_bit_size) / (long double) (uncompressed_bit_size);

    out << "PLA," << fn << "," << error_bound << "," << uncompressed_bit_size << "," << data.size() << ","
        << compressed_bit_size << "," << cr << "," << range << std::endl;
}

template<int64_t error_bound, typename T0, typename T1, typename T2>
void adaptive_approximation_full(const auto &fn, std::ostream &out) {
    auto data = fa::utils::read_data_binary<y_t, y_t>(fn);
    auto range = *std::max_element(data.begin(), data.end()) - *std::min_element(data.begin(), data.end());
    auto min_data = *std::min_element(data.begin(), data.end());
    min_data = min_data < 0 ? (min_data - 1) : -1;
    std::for_each(data.begin(), data.end(), [min_data](auto &d) { d -= min_data; });

    auto AA = adaptive_approximation<T0, x_t, y_t, T1, T2>(data.begin(), data.end(), error_bound);
    AA.check_partitions(data.begin(), data.end());

    auto uncompressed_bit_size = data.size() * sizeof(y_t) * 8;
    auto compressed_bit_size = AA.size_in_bits();
    auto cr = (long double) (compressed_bit_size) / (long double) (uncompressed_bit_size);

    out << "AA," << fn << "," << error_bound << "," << uncompressed_bit_size << "," << data.size() << ","
        << compressed_bit_size << "," << cr << "," << range << std::endl;
}
*/

/*
void lossy_compression_full() {
    std::string path = "../data/its/";
    //auto fn = path + std::string(argv[1]);
    std::cout
            << "compressor,dataset,error_bound,uncompressed_bit_size,num_elements,compressed_bit_size,compression_ratio,range"
            << std::endl;

    lossy_compression<9, double, float, double>(path + std::string("dust.bin"), std::cout);
    pla_compression<9, double, float, double>(path + std::string("dust.bin"), std::cout);
    adaptive_approximation_full<9, double, float, double>(path + std::string("dust.bin"), std::cout);

    lossy_compression<62, double, float, double>(path + std::string("city_temperature.bin"), std::cout);
    pla_compression<62, double, float, double>(path + std::string("city_temperature.bin"), std::cout);
    adaptive_approximation_full<62, double, float, double>(path + std::string("city_temperature.bin"), std::cout);

    lossy_compression<68, double, float, double>(path + std::string("dew-point-temp.bin"), std::cout);
    pla_compression<68, double, float, double>(path + std::string("dew-point-temp.bin"), std::cout);
    adaptive_approximation_full<68, double, float, double>(path + std::string("dew-point-temp.bin"), std::cout);

    lossy_compression<27, double, float, double>(path + std::string("germany.bin"), std::cout);
    pla_compression<27, double, float, double>(path + std::string("germany.bin"), std::cout);
    adaptive_approximation_full<27, double, float, double>(path + std::string("germany.bin"), std::cout);

    lossy_compression<9, double, float, double>(path + std::string("uk.bin"), std::cout);
    pla_compression<9, double, float, double>(path + std::string("uk.bin"), std::cout);
    adaptive_approximation_full<9, double, float, double>(path + std::string("uk.bin"), std::cout);

    lossy_compression<5, double, float, double>(path + std::string("usa.bin"), std::cout);
    pla_compression<5, double, float, double>(path + std::string("usa.bin"), std::cout);
    adaptive_approximation_full<5, double, float, double>(path + std::string("usa.bin"), std::cout);

    lossy_compression<2289, double, float, double>(path + std::string("wind-dir.bin"), std::cout);
    pla_compression<2289, double, float, double>(path + std::string("wind-dir.bin"), std::cout);
    adaptive_approximation_full<2289, double, float, double>(path + std::string("wind-dir.bin"), std::cout);

    lossy_compression<334, double, float, double>(path + std::string("air-pressure.bin"), std::cout);
    pla_compression<334, double, float, double>(path + std::string("air-pressure.bin"), std::cout);
    adaptive_approximation_full<334, double, float, double>(path + std::string("air-pressure.bin"), std::cout);

    lossy_compression<23, double, float, double>(path + std::string("IR-temp.bin"), std::cout);
    pla_compression<23, double, float, double>(path + std::string("IR-temp.bin"), std::cout);
    adaptive_approximation_full<23, double, float, double>(path + std::string("IR-temp.bin"), std::cout);

    lossy_compression<285, long double, double, double>(path + std::string("geolife-lat.bin"), std::cout);
    pla_compression<285, long double, double, double>(path + std::string("geolife-lat.bin"), std::cout);
    adaptive_approximation_full<285, long double, double, double>(path + std::string("geolife-lat.bin"), std::cout);

    lossy_compression<50, long double, double, double>(path + std::string("geolife-lon.bin"), std::cout);
    pla_compression<50, long double, double, double>(path + std::string("geolife-lon.bin"), std::cout);
    adaptive_approximation_full<50, long double, double, double>(path + std::string("geolife-lon.bin"), std::cout);

    lossy_compression<238820000, long double, double, double>(path + std::string("basel-temp.bin"), std::cout);
    pla_compression<238820000, long double, double, double>(path + std::string("basel-temp.bin"), std::cout);
    adaptive_approximation_full<238820000, long double, double, double>(path + std::string("basel-temp.bin"),
                                                                        std::cout);

    lossy_compression<12790000, long double, double, double>(path + std::string("basel-wind.bin"), std::cout);
    pla_compression<12790000, long double, double, double>(path + std::string("basel-wind.bin"), std::cout);
    adaptive_approximation_full<12790000, long double, double, double>(path + std::string("basel-wind.bin"), std::cout);

    lossy_compression<266000, double, float, double>(path + std::string("bitcoin-price.bin"), std::cout);
    pla_compression<266000, double, float, double>(path + std::string("bitcoin-price.bin"), std::cout);
    adaptive_approximation_full<266000, double, float, double>(path + std::string("bitcoin-price.bin"), std::cout);

    lossy_compression<900, double, float, double>(path + std::string("bird-migration.bin"), std::cout);
    pla_compression<900, double, float, double>(path + std::string("bird-migration.bin"), std::cout);
    adaptive_approximation_full<900, double, float, double>(path + std::string("bird-migration.bin"), std::cout);
}
*/

void squash_block_compression(const auto &compressor, int level = -1) {
    std::string path = "../data/its/";

    auto fns = get_files(path);
    //auto compressors = {"lz4"};
    auto blocks = {512, 1024, 2048, 4096, 8192, 16384};
    for (const auto &fn: fns) {
        for (const auto &block: blocks) {
            squash_full(compressor, fn, std::cout, block, level);
            std::cout << ",";
            squash_random_access(compressor, fn, std::cout, block, level);
            //std::cout << std::endl;
        }
    }
}

/*
template<typename TypeIn, typename TypeOut = y_t, int64_t bpc, typename poly = double, typename T1 = float, typename T2 = double>
void test_run(const std::string &full_fn, bool header = false) {
    static_assert(std::is_unsigned_v<TypeIn> && std::is_signed_v<TypeOut>);
    pfa::utils::check_data<bpc, TypeIn, TypeOut>(full_fn);
    constexpr int64_t epsilon = BPC_TO_EPSILON(bpc);
    neats::lossless_compressor<x_t, y_t, bpc, poly, T1, T2> lc;
    auto processed_data = fa::utils::preprocess_data<bpc, TypeIn, TypeOut>(full_fn);
    //processed_data.resize(1000);

    auto start = std::chrono::high_resolution_clock::now();
    lc.partitioning(processed_data.begin(), processed_data.end());
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

//
    std::vector<TypeOut> decompressed(processed_data.size());
    lc.decompress(decompressed.begin(), decompressed.end());

    for (auto i = 0; i < processed_data.size(); ++i) {
        if (processed_data[i] != decompressed[i]) {
            std::cout << i << ": " << processed_data[i] << "!=" << decompressed[i] << std::endl;
            //exit(1);
        }
    }

    auto uncompressed_bit_size = processed_data.size() * sizeof(TypeIn) * 8;
    auto compressed_ra_bit_size = lc.size_in_bits();
    auto compressed_storage_bit_size = lc.storage_size_in_bits();
    auto cr_random_access = (long double) (compressed_ra_bit_size) / (long double) (uncompressed_bit_size);
    auto cr_only_storage = (long double) (compressed_storage_bit_size) / (long double) (uncompressed_bit_size);

    if (header) {
        std::cout << "compressor,dataset,max_bpc,uncompressed_bit_size,num_elements,"
                  << "compressed_random_access_bit_size,compressed_storage_bit_size,cr_random_access,cr_only_storage,compression_time(ns),"
                  << "RA_residuals_bit_size,RA_offset_residuals_bit_size,RA_coefficients_bit_size,RA_model_types_bit_size,RA_rank&select_bit_size,RA_starting_positions_bit_size,RA_bpc_bit_size,"
                  << "OS_residuals_bit_size,OS_coefficients_bit_size,OS_model_types_bit_size,OS_starting_positions_bit_size,OS_bpc_bit_size,"
                  << "random_access_time(ns),full_decompression_time(ns),SIMD_decompression_time(ns)"
                  << std::endl;
    }
    std::cout << "NeaTS," << full_fn << "," << bpc << "," << uncompressed_bit_size << "," << processed_data.size() << ","
              << compressed_ra_bit_size << "," << compressed_storage_bit_size << "," << cr_random_access << ","
              << cr_only_storage << "," << ns << ",";
    lc.size_info();
    lc.storage_size_info();

    size_t rat = random_access_time(lc);
    size_t fdt = full_decompression_time<int64_t>(lc);
    std::cout << rat << ","
              << fdt << ",";

    auto num_runs = 50;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (auto r = 0; r < num_runs; ++r) {
        std::vector<int64_t> decompressed_SIMD(processed_data.size());
        lc.decompress_SIMD(decompressed_SIMD.data());
        do_not_optimize(decompressed);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / num_runs;
    std::cout << time << std::endl;

    std::vector<int64_t> decompressed_SIMD(processed_data.size());
    lc.decompress_SIMD(decompressed_SIMD.data());


    std::ofstream out(full_fn + ".neats");
    //out.clear();
    size_t size_in_bytes = lc.serialize(out);
    out.close();
    //std::cout << "Size in bytes: " << size_in_bytes << std::endl;
    std::ifstream in(full_fn + ".neats");
    lc.load(in);
    in.close();
    for (auto i = 0; i < processed_data.size(); ++i) {
        if (processed_data[i] != lc[i]) {
            std::cout << "Loading error at index: ";
            std::cout << i << ": " << processed_data[i] << "!=" << decompressed_SIMD[i] << std::endl;
            exit(1);
        }
    }

}
*/

void streaming_compressors_full(const std::string &fn, size_t block_size = 1000) {
    std::ofstream out(fn);
    out << "filename,compressor,block_size,#values,bits_per_value,compression_speed(MB/s),decompression_speed(MB/s)"
        << std::endl;

    auto path_double_datasets = "";
    //const auto fn_fulls = get_files(path_double_datasets);
    const auto fn_full = fn;
    //for (const auto &fn_full: fn_fulls) {
        out << fn_full << ",chimp," + std::to_string(block_size) + ",";
        chimp_compression(fn_full, out);
        out << fn_full << ",chimp128," + std::to_string(block_size) + ",";
        chimp128_compression(fn_full, out);
        out << fn_full << ",tsxor," + std::to_string(block_size) + ",";
        tsxor_compression(fn_full, out);
        out << fn_full << ",gorilla," + std::to_string(block_size) + ",";
        gorilla_compression<double>(fn_full, out);
    //}

    out.close();
}

void streaming_compressors_random_access(const std::string &fn_out, size_t block_size = 1000) {
    std::ofstream out(fn_out);
    out << "filename,compressor,bits_per_value,random_access_speed(MB/s)"
        << std::endl;

    const auto full_fns = get_files("");
    for (const auto &full_fn: full_fns) {
        //auto fn = clear(full_fn);
        out << full_fn << ",chimp,";
        streaming_compressors_random_access<CompressorChimp<double>, DecompressorChimp<double>>(
                full_fn, out, block_size);
        out << full_fn << ",chimp128,";
        streaming_compressors_random_access<CompressorChimp128<double>, DecompressorChimp128<double>>(
                full_fn, out, block_size);
        out << full_fn << ",gorilla,";
        streaming_compressors_random_access<CompressorGorilla<double>, DecompressorGorilla<double>>(
                full_fn, out, block_size);
        out << full_fn << ",tsxor,";
        streaming_compressors_random_access<CompressorTSXor<double>, DecompressorTSXor<double>>(
                full_fn, out, block_size);
    }
}

/*
void neats_compression_full() {
    std::string path = "../data/its/";

    run<17, double, float, double>(path + std::string("dust.bin"), true);
    run<10, double, float, double>(path + std::string("city_temperature.bin"));
    run<12, double, float, double>(path + std::string("dew-point-temp.bin"));

    run<15, double, float, double>(path + std::string("air-pressure.bin"));
    run<16, double, float, double>(path + std::string("wind-dir.bin"));
    run<13, double, float, double>(path + std::string("germany.bin"));
    run<12, double, float, double>(path + std::string("uk.bin"));
    run<10, double, float, double>(path + std::string("usa.bin"));
    run<12, double, float, double>(path + std::string("IR-temp.bin"));

    run<20, long double, double, double>(path + std::string("geolife-lat.bin"));
    run<20, long double, double, double>(path + std::string("geolife-lon.bin"));

    run<22, double, float, double>(path + std::string("bird-migration.bin"));
    run<24, double, float, double>(path + std::string("bitcoin-price.bin"));
    run<37, long double, double, double>(path + std::string("basel-temp.bin"));
    run<37, long double, double, double>(path + std::string("basel-wind.bin"));
}
*/

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <full_fn>" << std::endl;
        return 1;
    }

    auto full_fn = std::string(argv[1]);
    //dac_compression_full(full_fn, std::cout);
    squash_scan("lz4", full_fn, std::cout, 1000, -1, false);

    /*
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        exit(1);
    }

    const std::filesystem::path full_filename(argv[1]);
    */

    //auto block_size = 1000;
    //const auto fn_full = "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.double_bin";
    /*
    std::cout << fn_full << ",chimp,";
    streaming_compressors_random_access<CompressorChimp<double>, DecompressorChimp<double>>(
            fn_full, std::cout, block_size);
    std::cout << fn_full << ",chimp128,";
    streaming_compressors_random_access<CompressorChimp128<double>, DecompressorChimp128<double>>(
            fn_full, std::cout, block_size);
    std::cout << fn_full << ",gorilla,";
    streaming_compressors_random_access<CompressorGorilla<double>, DecompressorGorilla<double>>(
            fn_full, std::cout, block_size);
    std::cout << fn_full << ",tsxor,";
    streaming_compressors_random_access<CompressorTSXor<double>, DecompressorTSXor<double>>(
            fn_full, std::cout, block_size);
    */
    //squash_full("brotli", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    //std::cout << ',';
    //squash_random_access("brotli", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    //std::cout << ',';
    //std::cout << "XZ" << std::endl;
    //squash_random_access("xz", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    //std::cout << std::endl;//for (const auto &fn_full: fn_fulls) {

    //std::cout << fn_full << ",chimp," + std::to_string(block_size) + ",";
    //chimp_compression(fn_full, std::cout);
    /*
    std::cout << fn_full << ",chimp128," + std::to_string(block_size) + ",";
    chimp128_compression(fn_full, std::cout);
    std::cout << fn_full << ",tsxor," + std::to_string(block_size) + ",";
    tsxor_compression(fn_full, std::cout);
    std::cout << fn_full << ",gorilla," + std::to_string(block_size) + ",";
    gorilla_compression<double>(fn_full, std::cout);
    */
    //streaming_compressors_full("/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.double_bin", 1000);
    //squash_full("zstd", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    //squash_full("xz", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    //squash_full("brotli", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    /*
    squash_full("lz4", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << ',';
    squash_random_access("lz4", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << std::endl;

    squash_full("snappy", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << ',';
    squash_random_access("snappy", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << std::endl;

    squash_full("brotli", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << ',';
    squash_random_access("brotli", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << std::endl;

    squash_full("xz", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << ',';
    squash_random_access("xz", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << std::endl;

    squash_full("zstd", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << ',';
    squash_random_access("zstd", "/data/citypost/neat_datasets/a-large-scale-12-lead-electrocardiogram-database-for-arrhythmia-study-1.0.0/I.bin_int64", std::cout, 1000, -1, false);
    std::cout << std::endl;
    */


    return 0;
}