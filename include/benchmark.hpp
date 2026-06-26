#pragma once

#include "../include/Graph.hpp"
#include <string>
#include <vector>

namespace Benchmark {

struct MetricStats {
    double mean;
    double variance;
};

// Captura nativa de RAM (RSS)
long long getMemoryUsageKB();

// Calcula Media y Varianza
MetricStats calculateStats(const std::vector<double> &times);

// Ejecuta las 7 métricas secuencialmente
void runMetricsBenchmark(const std::string &datasetName, const Graph &graph);

} // namespace Benchmark