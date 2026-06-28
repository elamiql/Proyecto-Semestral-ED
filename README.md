# Proyecto Semestral: Métricas de Centralidad en Redes

Implementación del ADT Grafo y cálculo de diversas métricas de centralidad para el análisis de redes complejas (grafos dirigidos y no dirigidos, con o sin peso). Desarrollado para la asignatura de Estructuras de Datos.

## Estructura del Proyecto

```text
Proyecto_ED/
├── data/                       # Datasets utilizados
│   ├── imdb_edgelist.csv
│   └── train_test_network.csv
├── docs/                       # Informe y documentos
│   └── Proyecto_Semestral_Métricas_de_Centralidad_en_Redes.pdf
├── experiments/                # Scripts de medición experimental
│   └── benchmark.cpp
├── include/                    # Archivos de cabecera (Interfaces)
│   ├── benchmark.hpp
│   ├── Graph.hpp
│   ├── GraphLoader.hpp
│   └── Metrics.hpp
├── src/                        # Código fuente (Implementaciones)
│   ├── Graph.cpp
│   ├── GraphLoader.cpp
│   ├── main.cpp
│   └── Metrics.cpp
├── .gitignore
├── Makefile
└── README.md
```

## Compilación y Ejecución

**Nota sobre el rendimiento:** El cálculo completo de las métricas (especialmente aquellas basadas en caminos mínimos como Betweenness y Closeness) sobre datasets extensos como IMDb Actors toma aproximadamente 40 minutos en completarse debido a la complejidad computacional.

### En Linux (Ubuntu / macOS)

Utiliza la herramienta `make` estándar:

```bash
make
./proyecto_ed
make clean
```

### En Windows (MinGW)

Si tienes el compilador GCC/MinGW instalado, utiliza:

```bash
mingw32-make
proyecto_ed.exe
mingw32-make clean
```

## Métricas Disponibles

- `degreeCentrality`
- `betweennessCentrality`
- `closenessCentrality`
- `pageRank`
- `averageShortestPath`
- `localClusteringCoefficient`
- `networkDiameter`

## Integrantes (Grupo 10)

- Ariel Cisternas
- Agustín Salgado
- Ignacio Silva