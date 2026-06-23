# Proyecto semestral: Métricas de centralidad en redes

## Estructura del Proyecto

```
Proyecto_ED/
├── data/           # Datasets
├── docs/           # Informe y documentos
├── experiments/
│   └── benchmark.cpp # Experimento para las métricas
├── include/
│   ├── Graph.hpp   # Interfaz del ADT Grafo
│   └── GraphLoader.hpp # Parsers para datasets
├── src/
│   ├── Graph.cpp   # Implementación del ADT Grafo
│   ├── GraphLoader.cpp # Implementacion de parsers
│   ├── Metrics.cpp # Las 7 medidas de centralidad
│   └── main.cpp    # Carga de datasets y tests
├── experiments/
│   └── benchmark.cpp # Scripts de medición experimental
├── Makefile
└── README.md
```

## Compilar y ejecutar

```bash
make
proyecto_ed.exe
make clean
```

o

```bash
make
./proyecto_ed
make clean
```

o

```bash
mingw32-make
proyecto_ed.exe
mingw32-make clean
```

o

```bash
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src\Graph.cpp src\GraphLoader.cpp src\main.cpp -o proyecto_ed.exe
proyecto_ed.exe
```

## Métricas disponibles

- `degreeCentrality`
- `betweennessCentrality`
- `closenessCentrality`
- `pageRank`
- `averageShortestPath`
- `localClusteringCoefficient`
- `diametro`

## Integrantes

- Ariel Cisternas
- Agustín Salgado
- Ignacio Silva
