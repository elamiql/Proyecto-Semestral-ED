# Proyecto semestral: Métricas de centralidad en redes

## Estructura del Proyecto

```
Proyecto_ED/
├── data/           # Datasets
├── docs/           # Informe y documentos
├── include/
│   └── Graph.hpp   # Interfaz del ADT Grafo
├── src/
│   ├── Graph.cpp   # Implementación del ADT Grafo
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
./proyecto_ed
make clean
```

o

```bash
mingw32-make
./proyecto_ed
mingw32-make clean
```

## Integrantes

- Ariel Cisternas
- Agustín Salgado
- Ignacio Silva
