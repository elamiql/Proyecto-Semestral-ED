#pragma once

#include "Graph.hpp"
#include <string>

//  GraphLoader — Parsers para cargar distintos datasets al ADT Grafo
//
//  Cada función recibe la ruta al archivo y devuelve un Graph listo para usar.
//  Si el archivo no se puede abrir, lanza std::runtime_error.
namespace GraphLoader {

// Yeast Protein-Protein Interaction Network
// Formato esperado (tab o espacio separado, una arista por línea):
//   proteina1  proteina2
//   YAL001C    YAL002W
// Grafo NO dirigido, sin peso (peso default = 1.0)
// Ignora líneas que empiecen con '#' (comentarios)
Graph loadYeast(const std::string &filepath);

// Trade Network
// Formato esperado (CSV con encabezado):
//   exporter,importer,year,value
//   Chile,China,2020,5000.0
// Grafo DIRIGIDO con peso (valor del comercio)
// Ignora la primera línea (encabezado)
Graph loadTrade(const std::string &filepath);

// IoT Network Dataset
// Formato esperado (CSV con encabezado):
//   src_ip,dst_ip,...,duration,...
// Grafo DIRIGIDO con peso (duración de la conexión)
// Solo toma src_ip, dst_ip y duration — descarta el resto
// Ignora la primera línea (encabezado)
Graph loadIoT(const std::string &filepath);

// IMDb Actors Network
// Formato esperado (TSV con encabezado):
//   actor1\tactor2\tpeso
//   Tom_Hanks\tMeg_Ryan\t3
// Grafo NO dirigido con peso (número de películas en común)
// Ignora líneas que empiecen con '#'
Graph loadIMDb(const std::string &filepath);

// Parser genérico
// Para cualquier archivo con formato simple:
//   nodo1 <sep> nodo2 [<sep> peso]
// Útil para datasets desconocidos o de formato simple.
// sep: carácter separador (ej. ' ', ',', '\t')
// directed: true si el grafo es dirigido
// hasWeight: true si la tercera columna es el peso
// skipLines: cuántas líneas saltar al inicio (encabezados)
Graph loadGeneric(const std::string &filepath, char sep, bool directed,
                  bool hasWeight, int skipLines = 0);

}