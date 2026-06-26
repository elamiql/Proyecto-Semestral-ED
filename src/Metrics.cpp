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

//  hasNegativeWeight
//  Detecta si el grafo contiene alguna arista con peso negativo.
bool hasNegativeWeight(const Graph &graph) {
    // Recorre todas las aristas y se detiene apenas encuentra un peso negativo.
    for (int source = 0; source < graph.getNumVertices(); ++source) {
        for (const Edge &edge : graph.getNeighbors(source)) {
            if (edge.weight < 0.0) {
                return true;
            }
        }
    }
    return false;
}

//  hasUnitWeights
//  Verifica si todas las aristas tienen peso 1.0 para usar BFS en vez de
//  Dijkstra.
bool hasUnitWeights(const Graph &graph) {
    // Verifica si todas las aristas pesan 1.0 para poder usar BFS.
    for (int source = 0; source < graph.getNumVertices(); ++source) {
        for (const Edge &edge : graph.getNeighbors(source)) {
            if (std::fabs(edge.weight - 1.0) > kEpsilon) {
                return false;
            }
        }
    }
    return true;
}

//  construirConjuntosDeVecinos
//  Construye conjuntos de vecinos para calcular clustering local sin repetir
//  nodos.
std::vector<std::unordered_set<int>>
construirConjuntosDeVecinos(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<std::unordered_set<int>> neighbors(vertexCount);

    // Carga vecinos salientes y, si el grafo es dirigido, también los
    // entrantes.
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

//  calcularDistanciasBFS
//  Calcula distancias mínimas en grafos con pesos unitarios.
std::vector<double> calcularDistanciasBFS(const Graph &graph, int source) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> distancias(vertexCount,
                                   std::numeric_limits<double>::infinity());
    std::queue<int> queue;

    // Marca el origen con distancia cero y empieza la expansión por capas.
    distancias[source] = 0.0;
    queue.push(source);

    while (!queue.empty()) {
        // Toma el siguiente vértice descubierto.
        const int vertex = queue.front();
        queue.pop();

        for (const Edge &edge : graph.getNeighbors(vertex)) {
            // Solo asigna distancia la primera vez que se visita el vecino.
            if (distancias[edge.destination] ==
                std::numeric_limits<double>::infinity()) {
                distancias[edge.destination] = distancias[vertex] + 1.0;
                queue.push(edge.destination);
            }
        }
    }

    return distancias;
}

//  calcularDistanciasDijkstra
//  Calcula distancias mínimas en grafos ponderados con Dijkstra.
std::vector<double> calcularDistanciasDijkstra(const Graph &graph, int source) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> distancias(vertexCount,
                                   std::numeric_limits<double>::infinity());
    using Item = std::pair<double, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;

    // Inicializa el origen y prioriza siempre el mejor camino conocido.
    distancias[source] = 0.0;
    queue.push({0.0, source});

    while (!queue.empty()) {
        // Si aparece una entrada vieja, se ignora.
        const auto [distanciaActual, vertex] = queue.top();
        queue.pop();

        if (distanciaActual > distancias[vertex] + kEpsilon) {
            continue;
        }

        // Relaja las aristas salientes con la nueva distancia acumulada.
        for (const Edge &edge : graph.getNeighbors(vertex)) {
            const double candidato = distanciaActual + edge.weight;
            if (candidato + kEpsilon < distancias[edge.destination]) {
                distancias[edge.destination] = candidato;
                queue.push({candidato, edge.destination});
            }
        }
    }

    return distancias;
}

} // namespace

//  degreeCentrality
//  Calcula la centralidad de grado de cada vértice según la modalidad indicada.
std::vector<double> degreeCentrality(const Graph &graph, DegreeMode mode) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> centrality(vertexCount, 0.0);
    if (vertexCount <= 1) {
        return centrality;
    }

    // Normaliza por V - 1 para que el valor quede en una escala comparable.
    const double normalization = 1.0 / static_cast<double>(vertexCount - 1);

    // Out usa el grado saliente almacenado por el ADT.
    if (mode == DegreeMode::Out) {
        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            centrality[vertex] =
                static_cast<double>(graph.degree(vertex)) * normalization;
        }
        return centrality;
    }

    // En grafos no dirigidos, Total coincide con el grado ya almacenado.
    if (mode == DegreeMode::Total && !graph.getIsDirected()) {
        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            centrality[vertex] =
                static_cast<double>(graph.degree(vertex)) * normalization;
        }
        return centrality;
    }

    // Para In y Total en grafos dirigidos, reconstruye el in-degree recorriendo
    // las aristas.
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

//  betweennessCentrality
//  Calcula la centralidad de intermediación con el algoritmo de Brandes.
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

    // Brandes (2001), weighted version using Dijkstra from each source.
    // Repite el proceso tomando cada vértice como origen de caminos mínimos.
    for (int source = 0; source < vertexCount; ++source) {
        std::vector<int> stack;
        stack.reserve(vertexCount);

        // predecessors guarda los predecesores en caminos mínimos.
        std::vector<std::vector<int>> predecessors(vertexCount);
        // sigma cuenta cuántos caminos mínimos llegan a cada vértice.
        std::vector<double> sigma(vertexCount, 0.0);
        // distance guarda la distancia mínima desde el origen actual.
        std::vector<double> distance(vertexCount,
                                     std::numeric_limits<double>::infinity());

        using Item = std::pair<double, int>;
        std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;

        sigma[source] = 1.0;
        distance[source] = 0.0;
        queue.push({0.0, source});

        while (!queue.empty()) {
            // Extrae el vértice con menor distancia pendiente.
            const auto [currentDistance, vertex] = queue.top();
            queue.pop();

            if (currentDistance > distance[vertex] + kEpsilon) {
                continue;
            }

            // Guarda el orden de finalización para la fase de acumulación.
            stack.push_back(vertex);

            // Relaja aristas y actualiza caminos mínimos y predecesores.
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

        // Recorre la pila al revés para propagar la dependencia hacia atrás.
        std::vector<double> dependency(vertexCount, 0.0);
        while (!stack.empty()) {
            const int vertex = stack.back();
            stack.pop_back();

            // Distribuye la dependencia entre los predecesores del vértice
            // actual.
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

    if (!graph.getIsDirected()) {
        for (double &value : centrality) {
            value *= 0.5;
        }
    }

    return centrality;
}

//  closenessCentrality
//  Calcula la centralidad de cercanía a partir de caminos mínimos desde cada
//  nodo.
std::vector<double> closenessCentrality(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> centrality(vertexCount, 0.0);
    if (vertexCount <= 1) {
        return centrality;
    }

    // Valida pesos negativos y decide BFS vs Dijkstra una sola vez (no en cada
    // vértice).
    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: closenessCentrality requiere pesos no negativos");
    }
    const bool useBFS = hasUnitWeights(graph);

    // Calcula la cercanía de cada vértice sumando sus distancias hacia los
    // demás.
    for (int source = 0; source < vertexCount; ++source) {
        const std::vector<double> distancias =
            useBFS ? calcularDistanciasBFS(graph, source)
                   : calcularDistanciasDijkstra(graph, source);
        double reachable = 0.0;
        double sum = 0.0;

        for (int target = 0; target < vertexCount; ++target) {
            // Ignora la distancia al propio nodo y solo acumula distancias
            // finitas.
            if (target == source) {
                continue;
            }
            if (std::isfinite(distancias[target])) {
                sum += distancias[target];
                reachable += 1.0;
            }
        }

        // La centralidad es el inverso del promedio de distancias alcanzables.
        if (sum > 0.0 && reachable > 0.0) {
            centrality[source] = reachable / sum;
        }
    }

    return centrality;
}

//  pageRank
//  Calcula PageRank por iteración hasta converger o alcanzar el máximo de
//  iteraciones.
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

    // Parte con una distribución uniforme entre todos los vértices.
    std::fill(rank.begin(), rank.end(), 1.0 / static_cast<double>(vertexCount));
    // La parte base evita que el rango desaparezca por completo en nodos
    // aislados.
    const double baseRank = (1.0 - damping) / static_cast<double>(vertexCount);

    // Calcula el out-degree de cada vértice para repartir el rango en cada
    // iteración.
    std::vector<int> outDegree(vertexCount, 0);
    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        outDegree[vertex] = graph.degree(vertex);
    }

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // Arranca la nueva iteración con el valor base en todos los vértices.
        std::vector<double> nextRank(vertexCount, baseRank);
        double danglingMass = 0.0;

        for (int vertex = 0; vertex < vertexCount; ++vertex) {
            // Los vértices sin salidas acumulan masa que luego se reparte
            // uniformemente.
            if (outDegree[vertex] == 0) {
                danglingMass += rank[vertex];
                continue;
            }

            // Distribuye el rango del vértice entre sus vecinos salientes.
            const double share =
                damping * rank[vertex] / static_cast<double>(outDegree[vertex]);
            for (const Edge &edge : graph.getNeighbors(vertex)) {
                nextRank[edge.destination] += share;
            }
        }

        // Reparte la masa colgante entre todos los nodos.
        const double danglingShare =
            damping * danglingMass / static_cast<double>(vertexCount);
        for (double &value : nextRank) {
            value += danglingShare;
        }

        // Mide cuánto cambió la distribución para detectar convergencia.
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

//  averageShortestPath
//  Calcula el promedio de distancias finitas entre todos los pares alcanzables.
double averageShortestPath(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    if (vertexCount <= 1) {
        return 0.0;
    }

    // Valida pesos negativos y decide BFS vs Dijkstra una sola vez (no en cada
    // vértice).
    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: averageShortestPath requiere pesos no negativos");
    }
    const bool useBFS = hasUnitWeights(graph);

    // Suma distancias finitas entre pares ordenados alcanzables.
    double totalDistance = 0.0;
    double pairCount = 0.0;

    for (int source = 0; source < vertexCount; ++source) {
        const std::vector<double> distancias =
            useBFS ? calcularDistanciasBFS(graph, source)
                   : calcularDistanciasDijkstra(graph, source);
        for (int target = 0; target < vertexCount; ++target) {
            // Ignora el propio vértice y cualquier destino no alcanzable.
            if (target == source) {
                continue;
            }
            if (std::isfinite(distancias[target])) {
                totalDistance += distancias[target];
                pairCount += 1.0;
            }
        }
    }

    // Devuelve el promedio global solo sobre las distancias válidas.
    if (pairCount == 0.0) {
        return 0.0;
    }

    return totalDistance / pairCount;
}

//  diametro
//  Calcula la distancia más larga entre pares conectados en la red.
double diametro(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    if (vertexCount <= 1) {
        return 0.0;
    }

    // Valida pesos negativos y decide BFS vs Dijkstra una sola vez (no en cada
    // vértice).
    if (hasNegativeWeight(graph)) {
        throw std::invalid_argument(
            "Metrics: diametro requiere pesos no negativos");
    }
    const bool useBFS = hasUnitWeights(graph);

    // Guarda la mayor distancia finita observada en toda la red.
    double maxDistance = 0.0;
    for (int source = 0; source < vertexCount; ++source) {
        const std::vector<double> distancias =
            useBFS ? calcularDistanciasBFS(graph, source)
                   : calcularDistanciasDijkstra(graph, source);
        for (double value : distancias) {
            // Solo toma en cuenta vértices alcanzables desde el origen actual.
            if (std::isfinite(value)) {
                maxDistance = std::max(maxDistance, value);
            }
        }
    }

    return maxDistance;
}

//  localClusteringCoefficient
//  Calcula el coeficiente de clustering local de cada vértice.
std::vector<double> localClusteringCoefficient(const Graph &graph) {
    const int vertexCount = graph.getNumVertices();
    std::vector<double> coefficient(vertexCount, 0.0);
    if (vertexCount == 0) {
        return coefficient;
    }

    // Construye el vecindario de cada vértice para detectar triángulos rápido.
    const std::vector<std::unordered_set<int>> vecinos =
        construirConjuntosDeVecinos(graph);

    for (int vertex = 0; vertex < vertexCount; ++vertex) {
        const auto &vecinosAdyacentes = vecinos[vertex];
        const int degree = static_cast<int>(vecinosAdyacentes.size());
        // Un vértice con menos de dos vecinos no puede formar triángulos.
        if (degree < 2) {
            continue;
        }

        // Recorre cada par de vecinos y cuenta los que sí están conectados
        // entre sí.
        std::vector<int> lista(vecinosAdyacentes.begin(),
                               vecinosAdyacentes.end());
        int links = 0;

        for (std::size_t i = 0; i < lista.size(); ++i) {
            for (std::size_t j = i + 1; j < lista.size(); ++j) {
                const int izquierdo = lista[i];
                const int derecho = lista[j];

                // Se considera conectado si aparece en cualquiera de las dos
                // direcciones.
                bool conectado = vecinos[izquierdo].count(derecho) > 0 ||
                                 vecinos[derecho].count(izquierdo) > 0;
                if (conectado) {
                    ++links;
                }
            }
        }

        // Aplica la fórmula: 2 * enlaces reales / enlaces posibles.
        coefficient[vertex] =
            (2.0 * static_cast<double>(links)) /
            (static_cast<double>(degree) * static_cast<double>(degree - 1));
    }

    return coefficient;
}

} // namespace Metrics