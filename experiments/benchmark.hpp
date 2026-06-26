
#pragma once

#include "../include/Graph.hpp"
#include "../include/Metrics.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <string>

#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#endif

namespace Benchmark {

// Captura nativa de RAM (RSS) leyendo el descriptor de estado del proceso en Linux
inline long long getMemoryUsageKB() {
#ifdef __linux__
    std::ifstream stream("/proc/self/status");
    std::string line;
    while (std::getline(stream, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            size_t i = 6;
            while (i < line.size() && !isdigit(line[i])) i++;
            size_t j = i;
            while (j < line.size() && isdigit(line[j])) j++;
            if (i < line.size()) return std::stoll(line.substr(i, j - i));
        }
    }
#endif
    return -1;
}

struct MetricStats {
    double mean;
    double variance;
};

// Calcula Media y Varianza 
inline MetricStats calculateStats(const std::vector<double>& times) {
    if (times.empty()) return {0.0, 0.0};
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double mean = sum / times.size();
    
    double varianceSum = 0.0;
    for (double t : times) {
        varianceSum += (t - mean) * (t - mean);
    }
    double variance = (times.size() > 1) ? varianceSum / (times.size() - 1) : 0.0;
    return {mean, variance};
}

// Ejecuta las 7 métricas secuencialmente respetando los 10 ciclos por cada una
inline void runMetricsBenchmark(const std::string& datasetName, const Graph& graph) {
    std::cout << "\n==================================================\n";
    std::cout << " CRONOMETRANDO 7 MÉTRICAS (10 ITERACIONES): " << datasetName << "\n";
    std::cout << "==================================================\n";
    std::cout << "Nodos (V): " << graph.getNumVertices() << " | Aristas (E): " << graph.getNumEdges() << "\n\n";

    const int RUNS = 10;
    std::vector<std::pair<std::string, MetricStats>> finalReport;

    // 1. Degree Centrality
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::degreeCentrality(graph, Metrics::DegreeMode::Total);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Degree Centrality", calculateStats(times)});
    }

    // 2. Betweenness Centrality (Brandes)
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::betweennessCentrality(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Betweenness Centrality", calculateStats(times)});
    }

    // 3. Closeness Centrality
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::closenessCentrality(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Closeness Centrality", calculateStats(times)});
    }

    // 4. PageRank
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::pageRank(graph, 0.85, 100, 1e-6);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"PageRank", calculateStats(times)});
    }

    // 5. Average Shortest Path
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            double res = Metrics::averageShortestPath(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Average Shortest Path", calculateStats(times)});
    }

    // 6. Local Clustering Coefficient
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = Metrics::localClusteringCoefficient(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Local Clustering Coeff.", calculateStats(times)});
    }

    // 7. Diámetro
    {
        std::vector<double> times;
        for(int i = 0; i < RUNS; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            double res = Metrics::diametro(graph);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        finalReport.push_back({"Diámetro de la Red", calculateStats(times)});
    }

    
    std::cout << "| Métrica | Tiempo Promedio (ms) | Varianza (ms²) |\n";
    std::cout << "| :--- | :---: | :---: |\n";
    for (const auto& entry : finalReport) {
        std::cout << "| " << std::left << std::setw(26) << entry.first 
                  << " | " << std::fixed << std::setprecision(4) << std::setw(20) << entry.second.mean 
                  << " | " << std::setw(14) << entry.second.variance << " |\n";
    }
}

// Lógica de inyección y remoción de aristas
inline void runPerturbations(const std::string& datasetName, Graph &graph) {
    std::cout << "\n==================================================\n";
    std::cout << " EXPERIMENTOS DE PERTURBACIÓN TOPOLÓGICA: " << datasetName << "\n";
    std::cout << "==================================================\n";

    if (graph.getNumVertices() < 3) return;

    double baseASP = Metrics::averageShortestPath(graph);
    double baseDiam = Metrics::diametro(graph);
    std::cout << "[Base] ASP Inicial: " << baseASP << " | Diámetro Inicial: " << baseDiam << "\n\n";

    // 1. Buscar dinámicamente el nodo con mayor grado 
    std::string hubNode = "";
    int maxDegree = -1;
    for (int i = 0; i < graph.getNumVertices(); ++i) {
        if (graph.degree(i) > maxDegree && !graph.getNeighbors(i).empty()) {
            maxDegree = graph.degree(i);
            hubNode = graph.getNodeName(i);
        }
    }

    if (!hubNode.empty()) {
        int hubIndex = graph.getIndex(hubNode);
        std::string targetNeighbor = graph.getNodeName(graph.getNeighbors(hubIndex)[0].destination);
        
        std::cout << "-> Acción: Quitando enlace de alta conectividad [" << hubNode << "] -> [" << targetNeighbor << "]\n";
        if (graph.removeEdge(hubNode, targetNeighbor)) {
            double diffASP = Metrics::averageShortestPath(graph) - baseASP;
            double diffDiam = Metrics::diametro(graph) - baseDiam;
            std::cout << "   [Resultado] Delta ASP:      " << (diffASP >= 0 ? "+" : "") << diffASP << "\n";
            std::cout << "   [Resultado] Delta Diámetro: " << (diffDiam >= 0 ? "+" : "") << diffDiam << "\n";
            graph.addEdge(hubNode, targetNeighbor, 1.0); // Restaurar topología
        }
    }

    // Conectar artificialmente el primer y último nodo del diccionario
    std::string nodeA = graph.getNodeName(0);
    std::string nodeB = graph.getNodeName(graph.getNumVertices() - 1);
    
    std::cout << "\n-> Acción: Inyectando puente artificial ('Shortcut') entre [" << nodeA << "] y [" << nodeB << "]\n";
    graph.addEdge(nodeA, nodeB, 1.0);
    double diffAddASP = Metrics::averageShortestPath(graph) - baseASP;
    double diffAddDiam = Metrics::diametro(graph) - baseDiam;
    std::cout << "   [Resultado] Delta ASP:      " << diffAddASP << "\n";
    std::cout << "   [Resultado] Delta Diámetro: " << diffAddDiam << "\n";
    graph.removeEdge(nodeA, nodeB); 

} 
}