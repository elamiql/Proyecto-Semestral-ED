#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

//  Estructura de una arista
struct Edge {
    int destination; // ID numérico interno del nodo destino
    double weight;   // Peso de la arista (default 1.0)
};

//  ADT Grafo — Lista de Adyacencia
//
//  Soporta:
//    - Grafos dirigidos y no dirigidos
//    - Aristas con peso
//    - Nodos identificados por string (IPs, nombres, etc.)
//    - Añadir/quitar vértices y aristas
//
//  Complejidad espacial: O(V + E)
class Graph {
  private:
    // Traducción string <-> ID numérico interno
    std::unordered_map<std::string, int> nodeToIndex;
    std::vector<std::string> indexToNode;

    // Lista de adyacencia principal: adjList[u] = lista de aristas salientes de
    // u
    std::vector<std::vector<Edge>> adjList;

    bool isDirected;
    int edgeCount;

  public:
    // directed = false → grafo no dirigido
    // directed = true  → grafo dirigido
    explicit Graph(bool directed = false);

    // Modificación del grafo

    // Añade un vértice si no existe. Devuelve su ID interno.
    // Complejidad: O(1) amortizado
    int addVertex(const std::string &name);

    // Añade una arista de src a dest con el peso indicado.
    // Si el grafo es no dirigido, añade la arista inversa también.
    // Crea los vértices si no existen.
    // Complejidad: O(1) amortizado
    void addEdge(const std::string &src, const std::string &dest,
                 double weight = 1.0);

    // Elimina la arista entre src y dest.
    // Si es no dirigido, elimina las dos direcciones.
    // Complejidad: O(grado(src))
    bool removeEdge(const std::string &src, const std::string &dest);

    // Consultas

    // Devuelve el número de vértices
    // Complejidad: O(1)
    int getNumVertices() const;

    // Devuelve el número de aristas
    // Complejidad: O(1)
    int getNumEdges() const;

    // Devuelve true si el grafo es dirigido
    bool getIsDirected() const;

    // Devuelve la lista de aristas salientes de un nodo por ID
    // Complejidad: O(1)
    const std::vector<Edge> &getNeighbors(int nodeIndex) const;

    // Convierte nombre de nodo a ID interno (-1 si no existe)
    // Complejidad: O(1) amortizado
    int getIndex(const std::string &name) const;

    // Convierte ID interno a nombre de nodo
    // Complejidad: O(1)
    const std::string &getNodeName(int index) const;

    // Devuelve true si existe el vértice con ese nombre
    bool hasVertex(const std::string &name) const;

    // Devuelve true si existe arista de src a dest
    // Complejidad: O(grado(src))
    bool hasEdge(const std::string &src, const std::string &dest) const;

    // Devuelve el grado de un nodo (aristas salientes)
    // Para grafos no dirigidos, es el grado total
    // Complejidad: O(1)
    int degree(int nodeIndex) const;

    // Imprime el grafo en consola (útil para debugging)
    void print() const;
};