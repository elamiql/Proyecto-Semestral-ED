#include "../include/Graph.hpp"
#include <algorithm>
#include <iostream>

// Constructor
Graph::Graph(bool directed) : isDirected(directed), edgeCount(0) {}

// addVertex
// Si el nodo ya existe, devuelve su ID sin duplicar.
// Si es nuevo, le asigna el próximo ID disponible (tamaño actual del vector).
// Complejidad: O(1) amortizado (unordered_map + vector push_back)
int Graph::addVertex(const std::string &name) {
    auto it = nodeToIndex.find(name);
    if (it != nodeToIndex.end()) {
        return it->second; // Ya existe, devolver ID
    }
    int newIndex = (int)indexToNode.size();
    nodeToIndex[name] = newIndex;
    indexToNode.push_back(name);
    adjList.emplace_back(); // Lista de vecinos vacía para el nuevo nodo
    return newIndex;
}

// addEdge
// Crea los vértices si no existen y agrega la arista.
// Para no dirigidos, agrega la arista en ambas direcciones.
// Complejidad: O(1) amortizado
void Graph::addEdge(const std::string &src, const std::string &dest,
                    double weight) {
    int u = addVertex(src);
    int v = addVertex(dest);

    adjList[u].push_back({v, weight});
    edgeCount++;

    if (!isDirected && src != dest) {
        adjList[v].push_back({u, weight});
    }
}

// removeEdge
// Busca y elimina la arista de src a dest.
// Para no dirigidos, también elimina la arista inversa.
// Devuelve true si encontró y eliminó la arista, false si no existía.
// Complejidad: O(grado(src)) — tiene que recorrer la lista para encontrarla
bool Graph::removeEdge(const std::string &src, const std::string &dest) {
    int u = getIndex(src);
    int v = getIndex(dest);

    if (u == -1 || v == -1)
        return false; // Alguno de los nodos no existe

    // Buscar y eliminar arista u -> v
    auto &neighborsU = adjList[u];
    auto beforeSize = neighborsU.size();
    neighborsU.erase(
        std::remove_if(neighborsU.begin(), neighborsU.end(),
                       [v](const Edge &e) { return e.destination == v; }),
        neighborsU.end());

    bool removed = (neighborsU.size() < beforeSize);

    if (removed) {
        edgeCount--;
        // Si es no dirigido, eliminar también la arista inversa v -> u
        if (!isDirected) {
            auto &neighborsV = adjList[v];
            neighborsV.erase(std::remove_if(neighborsV.begin(),
                                            neighborsV.end(),
                                            [u](const Edge &e) {
                                                return e.destination == u;
                                            }),
                             neighborsV.end());
        }
    }
    return removed;
}

// Getters básicos

int Graph::getNumVertices() const { return (int)indexToNode.size(); }

int Graph::getNumEdges() const { return edgeCount; }

bool Graph::getIsDirected() const { return isDirected; }

const std::vector<Edge> &Graph::getNeighbors(int nodeIndex) const {
    if (nodeIndex < 0 || nodeIndex >= (int)adjList.size()) {
        throw std::out_of_range("Graph::getNeighbors — índice fuera de rango");
    }
    return adjList[nodeIndex];
}

int Graph::getIndex(const std::string &name) const {
    auto it = nodeToIndex.find(name);
    return (it != nodeToIndex.end()) ? it->second : -1;
}

const std::string &Graph::getNodeName(int index) const {
    if (index < 0 || index >= (int)indexToNode.size()) {
        throw std::out_of_range("Graph::getNodeName — índice fuera de rango");
    }
    return indexToNode[index];
}

bool Graph::hasVertex(const std::string &name) const {
    return nodeToIndex.count(name) > 0;
}

bool Graph::hasEdge(const std::string &src, const std::string &dest) const {
    int u = getIndex(src);
    int v = getIndex(dest);
    if (u == -1 || v == -1)
        return false;

    for (const Edge &e : adjList[u]) {
        if (e.destination == v)
            return true;
    }
    return false;
}

int Graph::degree(int nodeIndex) const {
    if (nodeIndex < 0 || nodeIndex >= (int)adjList.size()) {
        throw std::out_of_range("Graph::degree — índice fuera de rango");
    }
    return (int)adjList[nodeIndex].size();
}

// Para debugging: imprime cada nodo y sus vecinos con peso
void Graph::print() const {
    std::cout << "Grafo (" << (isDirected ? "Dirigido" : "No dirigido")
              << ")\n";
    std::cout << "Vértices: " << getNumVertices() << "  Aristas: " << edgeCount
              << "\n";
    std::cout << "─────────────────────────────────\n";
    for (int i = 0; i < (int)indexToNode.size(); i++) {
        std::cout << "[" << i << "] " << indexToNode[i] << " → ";
        for (const Edge &e : adjList[i]) {
            std::cout << indexToNode[e.destination] << "(w=" << e.weight
                      << ") ";
        }
        std::cout << "\n";
    }
}