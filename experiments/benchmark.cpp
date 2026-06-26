#include "../include/Graph.hpp"
#include "../include/GraphLoader.hpp"
#include "benchmark.hpp"
#include <iostream>
#include <chrono>

int main() {
    // Rutas relativas a tus dos datasets reales en formato Pajek (.net)
    const std::string tradePath   = "data/2018.net.csv";
    const std::string sciencePath = "data/NetScience.net.csv";

    std::cout << "==================================================\n";
    std::cout << " BENCHMARK INICIAL: VELOCIDAD DE CARGA Y MEMORIA   \n";
    std::cout << "==================================================\n";

    
    long long memBeforeTrade = Benchmark::getMemoryUsageKB();
    auto startTrade = std::chrono::high_resolution_clock::now();
    
    
    Graph g_trade = GraphLoader::loadPajekNet(tradePath, true);
    
    auto endTrade = std::chrono::high_resolution_clock::now();
    long long memAfterTrade = Benchmark::getMemoryUsageKB();
    double timeTrade = std::chrono::duration<double, std::milli>(endTrade - startTrade).count();

    std::cout << "Dataset: Comercio Internacional (2018)\n";
    std::cout << "  - Tiempo de construcción: " << timeTrade << " ms\n";
    if (memBeforeTrade != -1 && memAfterTrade != -1) {
        std::cout << "  - Asignación de RAM:      " << (memAfterTrade - memBeforeTrade) / 1024.0 << " MB\n";
    }
    std::cout << "--------------------------------------------------\n";

    
    long long memBeforeScience = Benchmark::getMemoryUsageKB();
    auto startScience = std::chrono::high_resolution_clock::now();
    
    Graph g_science = GraphLoader::loadPajekNet(sciencePath, false);
    
    auto endScience = std::chrono::high_resolution_clock::now();
    long long memAfterScience = Benchmark::getMemoryUsageKB();
    double timeScience = std::chrono::duration<double, std::milli>(endScience - startScience).count();

    std::cout << "Dataset: Red de Coautorías Científicas (NetScience)\n";
    std::cout << "  - Tiempo de construcción: " << timeScience << " ms\n";
    if (memBeforeScience != -1 && memAfterScience != -1) {
        std::cout << "  - Asignación de RAM:      " << (memAfterScience - memBeforeScience) / 1024.0 << " MB\n";
    }
    std::cout << "--------------------------------------------------\n";


    
    std::cout << "\n>>> ANALIZANDO GRAFO 1: COMERCIO INTERNACIONAL <<<\n";
    Benchmark::runMetricsBenchmark("Trade Network 2018", g_trade);
    Benchmark::runPerturbations("Trade Network 2018", g_trade);

    std::cout << "\n==================================================\n";

    
    std::cout << "\n>>> ANALIZANDO GRAFO 2: COLABORACIÓN CIENTÍFICA <<<\n";
    Benchmark::runMetricsBenchmark("NetScience Co-authorship", g_science);
    Benchmark::runPerturbations("NetScience Co-authorship", g_science);

    return 0;
}