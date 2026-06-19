#include "../include/Graph.hpp"
#include "../include/GraphLoader.hpp"
#include <iostream>

int testsPasados = 0;
int testsFallados = 0;

#define ASSERT(cond, msg)                                                      \
    if (cond) {                                                                \
        std::cout << "  [OK]   " << msg << "\n";                               \
        testsPasados++;                                                        \
    } else {                                                                   \
        std::cout << "  [FAIL] " << msg << "\n";                               \
        testsFallados++;                                                       \
    }

void testUndirectedGraph() {
    std::cout << "\n=== TEST 1: Grafo No Dirigido ===\n";
    Graph g(false);
    g.addEdge("192.168.1.1", "10.0.0.5", 15.5);
    g.addEdge("10.0.0.5", "172.16.0.2", 5.0);
    g.addEdge("172.16.0.2", "192.168.1.1", 8.0);
    g.addEdge("192.168.1.1", "172.16.0.2", 3.0);

    ASSERT(g.getNumVertices() == 3, "3 vertices");
    ASSERT(g.getNumEdges() == 4, "4 aristas");
    ASSERT(g.hasEdge("10.0.0.5", "192.168.1.1"), "arista inversa existe");
    ASSERT(g.degree(g.getIndex("192.168.1.1")) == 3, "grado == 3");

    bool r = g.removeEdge("192.168.1.1", "10.0.0.5");
    ASSERT(r == true, "removeEdge retorna true");
    ASSERT(!g.hasEdge("10.0.0.5", "192.168.1.1"), "arista inversa eliminada");
    ASSERT(g.getNumEdges() == 3, "quedan 3 aristas");
}

void testDirectedGraph() {
    std::cout << "\n=== TEST 2: Grafo Dirigido ===\n";
    Graph g(true);
    g.addEdge("Chile", "China", 5000.0);
    g.addEdge("Chile", "USA", 8000.0);
    g.addEdge("China", "USA", 15000.0);
    g.addEdge("USA", "Chile", 3000.0);

    ASSERT(g.getNumVertices() == 3, "3 vertices");
    ASSERT(g.getNumEdges() == 4, "4 aristas");
    ASSERT(!g.hasEdge("China", "Chile"), "China->Chile NO existe");
    ASSERT(g.hasEdge("Chile", "China"), "Chile->China SI existe");
    ASSERT(g.removeEdge("China", "Chile") == false,
           "removeEdge inexistente = false");
}

void testVertexOperations() {
    std::cout << "\n=== TEST 3: Operaciones de Vertices ===\n";
    Graph g(false);
    g.addVertex("A");
    g.addVertex("B");
    g.addVertex("C");
    g.addEdge("A", "B");
    g.addVertex("A");

    ASSERT(g.getNumVertices() == 3, "3 vertices sin duplicados");
    ASSERT(g.hasVertex("A"), "hasVertex A");
    ASSERT(!g.hasVertex("Z"), "hasVertex Z no existe");
    ASSERT(g.getIndex("Z") == -1, "getIndex inexistente == -1");
    int idA = g.getIndex("A");
    ASSERT(g.getNodeName(idA) == "A", "getNodeName(getIndex(A)) == A");
}

void testLoaders() {
    std::cout << "\n=== TEST 4: Parser IMDb (dataset real) ===\n";
    try {
        Graph g = GraphLoader::loadIMDb("data/imdb_edgelist.csv");
        // Validaciones con datos reales
        ASSERT(g.getNumVertices() > 0, "vertices cargados > 0");
        ASSERT(g.getNumEdges() > 0, "aristas cargadas > 0");
        ASSERT(!g.getIsDirected(), "grafo no dirigido");
        // Verificar que los IDs de IMDb tienen formato nm
        int id0 = 0;
        ASSERT(g.getNodeName(id0).substr(0, 2) == "nm",
               "IDs tienen formato nmXXXXXXX");
        std::cout << "  [INFO] " << g.getNumVertices() << " actores, "
                  << g.getNumEdges() << " colaboraciones\n";
    } catch (const std::exception &e) {
        std::cout << "  [FAIL] IMDb: " << e.what() << "\n";
        testsFallados++;
    }

    std::cout << "\n=== TEST 5: Parser IoT (dataset real) ===\n";
    try {
        Graph g = GraphLoader::loadIoT("data/train_test_network.csv");
        ASSERT(g.getNumVertices() > 0, "vertices cargados > 0");
        ASSERT(g.getNumEdges() > 0, "aristas cargadas > 0");
        ASSERT(g.getIsDirected(), "grafo dirigido");
        std::cout << "  [INFO] " << g.getNumVertices() << " IPs, "
                  << g.getNumEdges() << " conexiones\n";
    } catch (const std::exception &e) {
        std::cout << "  [FAIL] IoT: " << e.what() << "\n";
        testsFallados++;
    }
}

int main() {
    std::cout << "============================================\n";
    std::cout << "  ADT Grafo + Loaders — Suite de Tests\n";
    std::cout << "============================================\n";

    testUndirectedGraph();
    testDirectedGraph();
    testVertexOperations();
    testLoaders();

    std::cout << "\n============================================\n";
    std::cout << "  Resultado: " << testsPasados << " OK / "
              << (testsPasados + testsFallados) << " total\n";
    if (testsFallados == 0)
        std::cout << "  Todo listo\n";
    else
        std::cout << "  " << testsFallados << " test(s) fallando.\n";
    std::cout << "============================================\n";
    return testsFallados;
}