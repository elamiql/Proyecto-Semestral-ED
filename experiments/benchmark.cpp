#include "../include/benchmark.hpp"
#include "../include/Metrics.hpp"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

namespace Benchmark {

long long getMemoryUsageKB() {
#ifdef __linux__
    std::ifstream stream("/proc/self/status");
    std::string line;
    while (std::getline(stream, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            size_t i = 6;
            while (i < line.size() && !isdigit(line[i]))
                i++;
            size_t j = i;
            while (j < line.size() && isdigit(line[j]))
                j++;
            if (i < line.size())
                return std::stoll(line.substr(i, j - i));
        }
    }
#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<long long>(pmc.WorkingSetSize / 1024);
    }
#endif
    return -1;
}

MetricStats calculateStats(const std::vector<double> &times) {
    if (times.empty())
        return {0.0, 0.0};
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double mean = sum / times.size();

    double varianceSum = 0.0;
    for (double t : times) {
        varianceSum += (t - mean) * (t - mean);
    }
    double variance =
        (times.size() > 1) ? varianceSum / (times.size() - 1) : 0.0;
    return {mean, variance};
}

void runMetricsBenchmark(const std::string &datasetName, const Graph &graph) {
    std::cout << "\n==================================================\n";
    std::cout << " CRONOMETRANDO 7 MÉTRICAS: " << datasetName << "\n";
    std::cout << "==================================================\n";
    std::cout << "Nodos (V): " << graph.getNumVertices()
              << " | Aristas (E): " << graph.getNumEdges() << "\n\n";

    const int RUNS = 10;

    std::vector<std::pair<std::string, MetricStats>> finalReport;

    // 1. Degree Centrality (Rápida)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res =
                Metrics::degreeCentrality(graph, Metrics::DegreeMode::Total);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Degree Centrality", calculateStats(times)});
    }

    // 2. Betweenness Centrality (Lenta)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::betweennessCentrality(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back(
            {"Betweenness Centrality", calculateStats(times)});
    }

    // 3. Closeness Centrality (Lenta)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::closenessCentrality(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Closeness Centrality", calculateStats(times)});
    }

    // 4. PageRank (Rápida)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::pageRank(graph, 0.85, 100, 1e-6);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"PageRank", calculateStats(times)});
    }

    // 5. Average Shortest Path (Lenta)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            double res = Metrics::averageShortestPath(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Average Shortest Path", calculateStats(times)});
    }

    // 6. Local Clustering Coefficient (Rápida)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::localClusteringCoefficient(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back(
            {"Local Clustering Coeff.", calculateStats(times)});
    }

    // 7. Diámetro (Lenta)
    {
        std::vector<double> times;
        for (int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            double res = Metrics::diametro(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Diámetro de la Red", calculateStats(times)});
    }

    std::cout << "| Métrica | Tiempo Promedio (ms) | Varianza (ms²) |\n";
    std::cout << "| :--- | :---: | :---: |\n";
    for (const auto &entry : finalReport) {
        std::cout << "| " << std::left << std::setw(26) << entry.first << " | "
                  << std::fixed << std::setprecision(4) << std::setw(20)
                  << entry.second.mean << " | " << std::setw(14)
                  << entry.second.variance << " |\n";
    }
}

} // namespace Benchmark