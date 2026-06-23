#pragma once

#include "Graph.hpp"
#include <vector>

namespace Metrics {

// Tipo de centralidad de grado para grafos dirigidos.
enum class DegreeMode { Out, In, Total };

// Calcula la centralidad de grado de cada vértice.
// Out usa aristas salientes, In usa aristas entrantes y Total combina ambas
// según la dirección del grafo.
std::vector<double> degreeCentrality(const Graph &graph,
                                     DegreeMode mode = DegreeMode::Out);

// Calcula la centralidad de intermediación usando el algoritmo de Brandes.
std::vector<double> betweennessCentrality(const Graph &graph);

// Calcula la centralidad de cercanía a partir de los caminos mínimos.
std::vector<double> closenessCentrality(const Graph &graph);

// Calcula PageRank con factor de amortiguamiento y límite de iteraciones.
std::vector<double> pageRank(const Graph &graph, double damping = 0.85,
                             int maxIterations = 100,
                             double tolerance = 1e-6);

// Calcula el largo promedio de los caminos mínimos alcanzables en la red.
double averageShortestPath(const Graph &graph);

// Calcula el coeficiente de clustering local para cada vértice.
std::vector<double> localClusteringCoefficient(const Graph &graph);

// Calcula el diámetro de la red considerando distancias finitas.
double diametro(const Graph &graph);

}