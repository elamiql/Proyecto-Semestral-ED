#include "../include/Metrics.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_set>

namespace Metrics {

namespace {

constexpr double kEpsilon = 1e-12;

bool hasNegativeWeight(const Graph &graph) {
    for (int source = 0; source < graph.getNumVertices(); ++source) {
        for (const Edge &edge : graph.getNeighbors(source)) {
            if (edge.weight < 0.0) {
                return true;
            }
        }
    }
    return false;
}

bool hasUnitWeights(const Graph &graph) {
    for (int source = 0; source < graph.getNumVertices(); ++source) {
        for (const Edge &edge : graph.getNeighbors(source)) {
            if (std::fabs(edge.weight - 1.0) > kEpsilon) {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::unordered_set<int>>
construirConjuntosDeVecinos(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<std::unordered_set<int>> neighbors(vertexCount);

    for (int source = 0; source < vertexCount; ++source) {
        for (const Edge &edge : graph.getNeighbors(source)) {
            if (edge.destination != source) {
                neighbors[source].insert(edge.destination);
                if (graph.getIsDirected()) {
                    neighbors[edge.destination].insert(source);
                }
            }
        }
    }

    return neighbors;
}

// ------------------------------------------------------------------
// OPTIMIZACIÓN: estas dos funciones ahora reciben el vector de salida
// `distancias` POR REFERENCIA en vez de crearlo y devolverlo por valor.
// El caller (closenessCentrality / averageShortestPath / diametro) lo
// declara UNA SOLA VEZ fuera del loop de nodos fuente, y aquí solo se
// resetea con std::fill (que no libera/realoca memoria) en cada llamada.
// Esto elimina V mallocs+frees redundantes (uno por cada nodo fuente)
// sin cambiar el algoritmo ni el resultado. Sigue siendo 100% secuencial.
// ------------------------------------------------------------------

void calcularDistanciasBFS(const Graph &graph, int source,
                           std::vector<double> &distancias) {
    std::fill(distancias.begin(), distancias.end(),
              std::numeric_limits<double>::infinity());

    std::queue<int> queue;
    distancias[source] = 0.0;
    queue.push(source);

    while (!queue.empty()) {
        const int vertex = queue.front();
        queue.pop();

        for (const Edge &edge : graph.getNeighbors(vertex)) {
            if (distancias[edge.destination] ==
                std::numeric_limits<double>::infinity()) {
                distancias[edge.destination] = distancias[vertex] + 1.0;
                queue.push(edge.destination);
            }
        }
    }
}

void calcularDistanciasDijkstra(const Graph &graph, int source,
                                std::vector<double> &distancias) {
    std::fill(distancias.begin(), distancias.end(),
              std::numeric_limits<double>::infinity());

    using Item = std::pair<double, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;

    distancias[source] = 0.0;
    queue.push({0.0, source});

    while (!queue.empty()) {
        const auto [distanciaActual, vertex] = queue.top();
        queue.pop();

        if (distanciaActual > distancias[vertex] + kEpsilon) {
            continue;
        }

        for (const Edge &edge : graph.getNeighbors(vertex)) {
            const double candidato = distanciaActual + edge.weight;
            if (candidato + kEpsilon < distancias[edge.destination]) {
                distancias[edge.destination] = candidato;
                queue.push({candidato, edge.destination});
            }
        }
    }
}

// ------------------------------------------------------------------
// OPTIMIZACIÓN: betweennessBFS y betweennessDijkstra ahora reciben
// stack/predecessors/sigma/distance/dependency POR REFERENCIA.
// El caller (betweennessCentrality) los declara UNA SOLA VEZ fuera del
// loop de 33k+ nodos fuente. Aquí adentro se resetean con .clear()/
// std::fill en vez de reconstruirse — .clear() en un std::vector NO
// libera la capacidad reservada, así que tras las primeras iteraciones
// casi no hay más reallocations. Mismo algoritmo de Brandes, mismo
// resultado, sin hilos, sin paralelismo: 100% secuencial.
// ------------------------------------------------------------------

void betweennessBFS(const Graph &graph, int source,
                    std::vector<double> &centrality, std::vector<int> &stack,
                    std::vector<std::vector<int>> &predecessors,
                    std::vector<double> &sigma, std::vector<int> &distance,
                    std::vector<double> &dependency) {
    // Reset de buffers reutilizados (sin liberar memoria reservada)
    stack.clear();
    for (auto &p : predecessors) {
        p.clear();
    }
    std::fill(sigma.begin(), sigma.end(), 0.0);
    std::fill(distance.begin(), distance.end(), -1);
    std::fill(dependency.begin(), dependency.end(), 0.0);

    sigma[source] = 1.0;
    distance[source] = 0;

    std::queue<int> queue;
    queue.push(source);

    while (!queue.empty()) {
        const int vertex = queue.front();
        queue.pop();
        stack.push_back(vertex);

        for (const Edge &edge : graph.getNeighbors(vertex)) {
            const int w = edge.destination;
            // Primera vez que se visita w
            if (distance[w] < 0) {
                queue.push(w);
                distance[w] = distance[vertex] + 1;
            }
            // Camino mínimo encontrado
            if (distance[w] == distance[vertex] + 1) {
                sigma[w] += sigma[vertex];
                predecessors[w].push_back(vertex);
            }
        }
    }

    while (!stack.empty()) {
        const int vertex = stack.back();
        stack.pop_back();

        for (int predecessor : predecessors[vertex]) {
            if (sigma[vertex] > 0.0) {
                dependency[predecessor] +=
                    (sigma[predecessor] / sigma[vertex]) *
                    (1.0 + dependency[vertex]);
            }
        }

        if (vertex != source) {
            centrality[vertex] += dependency[vertex];
        }
    }
}

void betweennessDijkstra(const Graph &graph, int source,
                         std::vector<double> &centrality,
                         std::vector<int> &stack,
                         std::vector<std::vector<int>> &predecessors,
                         std::vector<double> &sigma,
                         std::vector<double> &distance,
                         std::vector<double> &dependency) {
    // Reset de buffers reutilizados (sin liberar memoria reservada)
    stack.clear();
    for (auto &p : predecessors) {
        p.clear();
    }
    std::fill(sigma.begin(), sigma.end(), 0.0);
    std::fill(distance.begin(), distance.end(),
              std::numeric_limits<double>::infinity());
    std::fill(dependency.begin(), dependency.end(), 0.0);

    using Item = std::pair<double, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;

    sigma[source] = 1.0;
    distance[source] = 0.0;
    queue.push({0.0, source});

    while (!queue.empty()) {
        const auto [currentDistance, vertex] = queue.top();
        queue.pop();

        if (currentDistance > distance[vertex] + kEpsilon) {
            continue;
        }

        stack.push_back(vertex);

        for (const Edge &edge : graph.getNeighbors(vertex)) {
            const double candidate = distance[vertex] + edge.weight;

            if (candidate + kEpsilon < distance[edge.destination]) {
                distance[edge.destination] = candidate;
                queue.push({candidate, edge.destination});
                sigma[edge.destination] = sigma[vertex];
                predecessors[edge.destination].clear();
                predecessors[edge.destination].push_back(vertex);
            } else if (std::fabs(candidate - distance[edge.destination]) <=
                       kEpsilon) {
                sigma[edge.destination] += sigma[vertex];
                predecessors[edge.destination].push_back(vertex);
            }
        }
    }

    while (!stack.empty()) {
        const int vertex = stack.back();
        stack.pop_back();

        for (int predecessor : predecessors[vertex]) {
            if (sigma[vertex] > 0.0) {
                dependency[predecessor] +=
                    (sigma[predecessor] / sigma[vertex]) *
                    (1.0 + dependency[vertex]);
            }
        }

        if (vertex != source) {
            centrality[vertex] += dependency[vertex];
        }
    }
}

} // namespace

std::vector<double> degreeCentrality(const Graph &graph, DegreeMode mode) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> centrality(vertexCount, 0.0);
    if (vertexCount <= 1) {
        return centrality;
    }

    const double normalization = 1.0 / static_cast<double>(vertexCount - 1);

    if (mode == DegreeMode::Out) {
        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            centrality[vertex] =
                static_cast<double>(graph.degree(vertex)) * normalization;
        }
        return centrality;
    }

    if (mode == DegreeMode::Total && !graph.getIsDirected()) {
        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            centrality[vertex] =
                static_cast<double>(graph.degree(vertex)) * normalization;
        }
        return centrality;
    }

    std::vector<int> inDegree(vertexCount, 0);
    for (int source = 0; source < vertexCount; ++source) {
        for (const Edge &edge : graph.getNeighbors(source)) {
            ++inDegree[edge.destination];
        }
    }

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        if (mode == DegreeMode::In) {
            centrality[vertex] =
                static_cast<double>(inDegree[vertex]) * normalization;
        } else {
            centrality[vertex] =
                static_cast<double>(graph.degree(vertex) + inDegree[vertex]) *
                normalization;
        }
    }

    return centrality;
}

std::vector<double> betweennessCentrality(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> centrality(vertexCount, 0.0);
    if (vertexCount == 0) {
        return centrality;
    }

    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: betweennessCentrality requiere pesos no negativos");
    }

    // Usa BFS si todos los pesos son 1 (más rápido), Dijkstra si no.
    const bool useBFS = hasUnitWeights(graph);

    // Buffers reutilizados entre las V llamadas (declarados UNA vez aquí).
    std::vector<int> stack;
    stack.reserve(vertexCount);
    std::vector<std::vector<int>> predecessors(vertexCount);
    std::vector<double> sigma(vertexCount, 0.0);
    std::vector<double> dependency(vertexCount, 0.0);

    if (useBFS) {
        std::vector<int> distanceBFS(vertexCount, -1);
        for (int source = 0; source < vertexCount; ++source) {
            betweennessBFS(graph, source, centrality, stack, predecessors,
                           sigma, distanceBFS, dependency);
        }
    } else {
        std::vector<double> distanceDijkstra(
            vertexCount, std::numeric_limits<double>::infinity());
        for (int source = 0; source < vertexCount; ++source) {
            betweennessDijkstra(graph, source, centrality, stack, predecessors,
                                sigma, distanceDijkstra, dependency);
        }
    }

    if (!graph.getIsDirected()) {
        for (double &value : centrality) {
            value *= 0.5;
        }
    }

    return centrality;
}

std::vector<double> closenessCentrality(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> centrality(vertexCount, 0.0);
    if (vertexCount <= 1) {
        return centrality;
    }

    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: closenessCentrality requiere pesos no negativos");
    }
    const bool useBFS = hasUnitWeights(graph);

    // Buffer reutilizado entre las V llamadas.
    std::vector<double> distancias(vertexCount);

    for (int source = 0; source < vertexCount; ++source) {
        if (useBFS) {
            calcularDistanciasBFS(graph, source, distancias);
        } else {
            calcularDistanciasDijkstra(graph, source, distancias);
        }

        double reachable = 0.0;
        double sum = 0.0;

        for (int target = 0; target < vertexCount; ++target) {
            if (target == source) {
                continue;
            }
            if (std::isfinite(distancias[target])) {
                sum += distancias[target];
                reachable += 1.0;
            }
        }

        if (sum > 0.0 && reachable > 0.0) {
            centrality[source] = reachable / sum;
        }
    }

    return centrality;
}

std::vector<double> pageRank(const Graph &graph, double damping,
                             int maxIterations, double tolerance) {
    if (damping < 0.0 || damping > 1.0) {
        throw std::invalid_argument(
            "Metrics::pageRank: damping debe estar entre [0, 1]");
    }
    if (maxIterations <= 0) {
        throw std::invalid_argument(
            "Metrics::pageRank: maxIterations debe ser positivo");
    }

    const int vertexCount = graph.getNumVertices();
    std::vector<double> rank(vertexCount, 0.0);
    if (vertexCount == 0) {
        return rank;
    }

    std::fill(rank.begin(), rank.end(), 1.0 / static_cast<double>(vertexCount));
    const double baseRank = (1.0 - damping) / static_cast<double>(vertexCount);

    std::vector<int> outDegree(vertexCount, 0);
    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        outDegree[vertex] = graph.degree(vertex);
    }

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        std::vector<double> nextRank(vertexCount, baseRank);
        double danglingMass = 0.0;

        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            if (outDegree[vertex] == 0) {
                danglingMass += rank[vertex];
                continue;
            }

            const double share =
                damping * rank[vertex] / static_cast<double>(outDegree[vertex]);
            for (const Edge &edge : graph.getNeighbors(vertex)) {
                nextRank[edge.destination] += share;
            }
        }

        const double danglingShare =
            damping * danglingMass / static_cast<double>(vertexCount);
        for (double &value : nextRank) {
            value += danglingShare;
        }

        double delta = 0.0;
        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            delta += std::fabs(nextRank[vertex] - rank[vertex]);
        }

        rank.swap(nextRank);
        if (delta < tolerance) {
            break;
        }
    }

    return rank;
}

double averageShortestPath(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    if (vertexCount <= 1) {
        return 0.0;
    }

    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: averageShortestPath requiere pesos no negativos");
    }
    const bool useBFS = hasUnitWeights(graph);

    // Buffer reutilizado entre las V llamadas.
    std::vector<double> distancias(vertexCount);

    double totalDistance = 0.0;
    double pairCount = 0.0;

    for (int source = 0; source < vertexCount; ++source) {
        if (useBFS) {
            calcularDistanciasBFS(graph, source, distancias);
        } else {
            calcularDistanciasDijkstra(graph, source, distancias);
        }

        for (int target = 0; target < vertexCount; ++target) {
            if (target == source) {
                continue;
            }
            if (std::isfinite(distancias[target])) {
                totalDistance += distancias[target];
                pairCount += 1.0;
            }
        }
    }

    if (pairCount == 0.0) {
        return 0.0;
    }

    return totalDistance / pairCount;
}

double diametro(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    if (vertexCount <= 1) {
        return 0.0;
    }

    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: diametro requiere pesos no negativos");
    }
    const bool useBFS = hasUnitWeights(graph);

    // Buffer reutilizado entre las V llamadas.
    std::vector<double> distancias(vertexCount);

    double maxDistance = 0.0;
    for (int source = 0; source < vertexCount; ++source) {
        if (useBFS) {
            calcularDistanciasBFS(graph, source, distancias);
        } else {
            calcularDistanciasDijkstra(graph, source, distancias);
        }

        for (double value : distancias) {
            if (std::isfinite(value)) {
                maxDistance = std::max(maxDistance, value);
            }
        }
    }

    return maxDistance;
}

std::vector<double> localClusteringCoefficient(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> coefficient(vertexCount, 0.0);
    if (vertexCount == 0) {
        return coefficient;
    }

    const std::vector<std::unordered_set<int>> vecinos =
        construirConjuntosDeVecinos(graph);

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        const auto &vecinosAdyacentes = vecinos[vertex];
        const int degree = static_cast<int>(vecinosAdyacentes.size());
        if (degree < 2) {
            continue;
        }

        std::vector<int> lista(vecinosAdyacentes.begin(),
                               vecinosAdyacentes.end());
        int links = 0;

        for (std::size_t i = 0; i < lista.size(); ++i) {
            for (std::size_t j = i + 1; j < lista.size(); ++j) {
                const int izquierdo = lista[i];
                const int derecho = lista[j];

                bool conectado = vecinos[izquierdo].count(derecho) > 0 ||
                                 vecinos[derecho].count(izquierdo) > 0;
                if (conectado) {
                    ++links;
                }
            }
        }

        coefficient[vertex] =
            (2.0 * static_cast<double>(links)) /
            (static_cast<double>(degree) * static_cast<double>(degree - 1));
    }

    return coefficient;
}

} // namespace Metrics