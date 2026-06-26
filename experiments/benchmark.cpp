// experiments/benchmark.cpp
#include "../include/Graph.hpp"
#include "../include/GraphLoader.hpp"
#include "benchmark.hpp"
#include <iostream>
#include <chrono>

int main() {
    const std::string imdbPath = "data/imdb_edgelist.csv";
    const std::string iotPath  = "data/train_test_network.csv";

    std::cout << "==================================================\n";
    std::cout << " BENCHMARK INICIAL: VELOCIDAD DE CARGA Y MEMORIA   \n";
    std::cout << "==================================================\n";

    // --- EXEC: IMDb Dataset ---
    long long memBeforeIMDb = Benchmark::getMemoryUsageKB();
    auto startIMDb = std::chrono::high_resolution_clock::now();
    
    Graph g_imdb = GraphLoader::loadIMDb(imdbPath);
    
    auto endIMDb = std::chrono::high_resolution_clock::now();
    long long memAfterIMDb = Benchmark::getMemoryUsageKB();
    double timeIMDb = std::chrono::duration<double, std::milli>(endIMDb - startIMDb).count();

    std::cout << "Dataset: IMDb Actors Network\n";
    std::cout << "  - Tiempo de construcción: " << timeIMDb << " ms\n";
    if (memBeforeIMDb != -1 && memAfterIMDb != -1) {
        std::cout << "  - Asignación de RAM:      " << (memAfterIMDb - memBeforeIMDb) / 1024.0 << " MB\n";
    }
    std::cout << "--------------------------------------------------\n";

    // --- EXEC: IoT Network ---
    long long memBeforeIoT = Benchmark::getMemoryUsageKB();
    auto startIoT = std::chrono::high_resolution_clock::now();
    
    Graph g_iot = GraphLoader::loadIoT(iotPath);
    
    auto endIoT = std::chrono::high_resolution_clock::now();
    long long memAfterIoT = Benchmark::getMemoryUsageKB();
    double timeIoT = std::chrono::duration<double, std::milli>(endIoT - startIoT).count();

    std::cout << "Dataset: IoT Network Connections\n";
    std::cout << "  - Tiempo de construcción: " << timeIoT << " ms\n";
    if (memBeforeIoT != -1 && memAfterIoT != -1) {
        std::cout << "  - Asignación de RAM:      " << (memAfterIoT - memBeforeIoT) / 1024.0 << " MB\n";
    }

    
    Benchmark::runMetricsBenchmark("IMDb Actores", g_imdb);
    Benchmark::runPerturbations("IMDb Actores", g_imdb);

    Benchmark::runMetricsBenchmark("IoT Traffic", g_iot);
    Benchmark::runPerturbations("IoT Traffic", g_iot);

    return 0;
}