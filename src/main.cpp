// main.cpp — Benchmark completo Proyecto Redes
// Datasets: IMDb actors network + IoT Network (train_test_network)

#include "../include/Graph.hpp"
#include "../include/GraphLoader.hpp"
#include "../include/Metrics.hpp"
#include "../include/benchmark.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

// Helpers locales
static void printSeparator(char c = '-', int width = 52) {
    std::cout << std::string(width, c) << "\n";
}

static void section(const std::string &title) {
    printSeparator('=');
    std::cout << " " << title << "\n";
    printSeparator('=');
}

// Mide tiempo + RAM de construcción
struct LoadResult {
    Graph graph;
    double timeMs;
    long long memDeltaKB;
};

template <typename LoadFn>
static LoadResult measureLoad(const std::string &label, LoadFn fn) {
    long long memBefore = Benchmark::getMemoryUsageKB();
    auto t0 = std::chrono::high_resolution_clock::now();

    Graph g = fn();

    auto t1 = std::chrono::high_resolution_clock::now();
    long long memAfter = Benchmark::getMemoryUsageKB();

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    long long delta =
        (memBefore != -1 && memAfter != -1) ? (memAfter - memBefore) : -1;

    std::cout << "Dataset : " << label << "\n";
    std::cout << "  Vertices : " << g.getNumVertices() << "\n";
    std::cout << "  Aristas  : " << g.getNumEdges() << "\n";
    std::cout << "  Tiempo   : " << std::fixed << std::setprecision(3) << ms
              << " ms\n";
    if (delta >= 0)
        std::cout << "  RAM      : " << std::fixed << std::setprecision(2)
                  << delta / 1024.0 << " MB\n";
    else
        std::cout << "  RAM      : (no disponible en esta plataforma)\n";
    printSeparator();

    return {std::move(g), ms, delta};
}

// ============================================================================
// PERTURBACIONES OPTIMIZADAS (Salvavidas de CPU)
// Solo calcula métricas rápidas (O(V+E)) para no colgar el pc.
// ============================================================================

struct NetSnapshot {
    double avgDegree;
    double avgPagerank;
    double avgClustering;
};

static NetSnapshot takeSnapshot(const Graph &g) {
    auto deg = Metrics::degreeCentrality(g, Metrics::DegreeMode::Total);
    auto pr = Metrics::pageRank(g, 0.85, 100, 1e-6);
    auto cc = Metrics::localClusteringCoefficient(g);

    double sumDeg = 0, sumPr = 0, sumCc = 0;
    int n = g.getNumVertices();
    for (int i = 0; i < n; i++) {
        sumDeg += deg[i];
        sumPr += pr[i];
        sumCc += cc[i];
    }

    return {n > 0 ? sumDeg / n : 0.0, n > 0 ? sumPr / n : 0.0,
            n > 0 ? sumCc / n : 0.0};
}

static void printDelta(const std::string &action, const NetSnapshot &base,
                       const NetSnapshot &after) {
    auto d = [](double a, double b) -> std::string {
        double diff = b - a;
        std::ostringstream os;
        os << std::fixed << std::setprecision(6) << (diff >= 0 ? "+" : "")
           << diff;
        return os.str();
    };
    std::cout << "  [" << action << "]\n";
    std::cout << "    ΔDeg (mean)   : " << d(base.avgDegree, after.avgDegree)
              << "\n";
    std::cout << "    ΔPageRank     : "
              << d(base.avgPagerank, after.avgPagerank) << "\n";
    std::cout << "    ΔClustering   : "
              << d(base.avgClustering, after.avgClustering) << "\n";
}

static void runExtendedPerturbations(const std::string &datasetName, Graph &g) {
    section("PERTURBACIONES: " + datasetName);

    int n = g.getNumVertices();
    if (n < 3) {
        std::cout << "  Grafo muy pequeño, experimento omitido.\n";
        return;
    }

    int hubIdx = 0, periIdx = 0, midIdx = n / 2;
    int maxDeg = -1, minDeg = INT_MAX;

    for (int i = 0; i < n; i++) {
        int d = g.degree(i);
        if (d > maxDeg) {
            maxDeg = d;
            hubIdx = i;
        }
        if (d < minDeg) {
            minDeg = d;
            periIdx = i;
        }
    }

    std::string hub = g.getNodeName(hubIdx);
    std::string peri = g.getNodeName(periIdx);
    std::string mid = g.getNodeName(midIdx);

    std::cout << "  Hub       : " << hub << " (grado " << maxDeg << ")\n";
    std::cout << "  Periferia : " << peri << " (grado " << minDeg << ")\n";
    std::cout << "  Medio     : " << mid << " (grado " << g.degree(midIdx)
              << ")\n";
    printSeparator();

    NetSnapshot base = takeSnapshot(g);
    std::cout << "  [Base] Snapshot capturado (Métricas rápidas)\n\n";

    // 1. Remover arista desde hub
    if (!g.getNeighbors(hubIdx).empty()) {
        std::string neigh =
            g.getNodeName(g.getNeighbors(hubIdx)[0].destination);
        double w = g.getNeighbors(hubIdx)[0].weight;
        if (g.removeEdge(hub, neigh)) {
            printDelta("REMOVE hub→" + neigh, base, takeSnapshot(g));
            g.addEdge(hub, neigh, w);
        }
    }

    // 2. Remover arista desde nodo periférico
    if (!g.getNeighbors(periIdx).empty()) {
        std::string neigh =
            g.getNodeName(g.getNeighbors(periIdx)[0].destination);
        double w = g.getNeighbors(periIdx)[0].weight;
        if (g.removeEdge(peri, neigh)) {
            printDelta("REMOVE peri→" + neigh, base, takeSnapshot(g));
            g.addEdge(peri, neigh, w);
        }
    }

    // 3. Agregar shortcut hub ↔ periferia
    if (!g.hasEdge(hub, peri)) {
        g.addEdge(hub, peri, 1.0);
        printDelta("ADD   hub↔peri (shortcut)", base, takeSnapshot(g));
        g.removeEdge(hub, peri);
        if (!g.getIsDirected())
            g.removeEdge(peri, hub);
    }

    printSeparator();
}

// Pipeline completo
static void analyzeDataset(const std::string &label, Graph &g) {
    Benchmark::runMetricsBenchmark(label, g);
    runExtendedPerturbations(label, g);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    const std::string imdbFile = "data/imdb_edgelist.csv";
    const std::string iotFile = "data/train_test_network.csv";

    section("CONSTRUCCION DEL GRAFO Y USO DE MEMORIA");

    auto imdbResult = measureLoad("IMDb Actors Network (no dirigido)", [&]() {
        return GraphLoader::loadIMDb(imdbFile);
    });

    auto iotResult = measureLoad("IoT Network / Train-Test (dirigido)", [&]() {
        return GraphLoader::loadIoT(iotFile);
    });

    section("ANALISIS COMPLETO: IoT NETWORK (train_test_network)");
    analyzeDataset("IoT Network", iotResult.graph);

    section("ANALISIS COMPLETO: IMDb ACTORS NETWORK");
    analyzeDataset("IMDb Actors Network", imdbResult.graph);

    section("FIN DEL BENCHMARK");
    return 0;
}