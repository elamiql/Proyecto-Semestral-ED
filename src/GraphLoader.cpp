#include "GraphLoader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace GraphLoader {

// Trim de espacios/tabs/\r
static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

//  loadYeast
//  Formato: "proteina1\tproteina2" o "proteina1 proteina2"
//  Lineas con '#' son comentarios. Grafo no dirigido.
Graph loadYeast(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("loadYeast: no se pudo abrir " + filepath);

    Graph g(false);
    std::string line;
    int linesRead = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string u, v;

        if (std::getline(ss, u, '\t') && std::getline(ss, v, '\t')) {
            u = trim(u);
            v = trim(v);
        } else {
            ss.clear();
            ss.str(line);
            ss >> u >> v;
        }

        if (!u.empty() && !v.empty()) {
            g.addEdge(u, v, 1.0);
            linesRead++;
        }
    }

    std::cout << "[Yeast] Cargado: " << g.getNumVertices() << " vertices, "
              << g.getNumEdges() << " aristas (" << linesRead << " lineas)\n";
    return g;
}

//  loadTrade
//  Formato CSV: exporter,importer,year,value
//  Primera linea es encabezado. Grafo dirigido con peso.
Graph loadTrade(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("loadTrade: no se pudo abrir " + filepath);

    Graph g(true);
    std::string line;
    int linesRead = 0;

    std::getline(file, line); // Saltar encabezado

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string exporter, importer, year, valueStr;

        if (!std::getline(ss, exporter, ','))
            continue;
        if (!std::getline(ss, importer, ','))
            continue;
        if (!std::getline(ss, year, ','))
            continue;
        if (!std::getline(ss, valueStr, ','))
            continue;

        exporter = trim(exporter);
        importer = trim(importer);
        if (exporter.empty() || importer.empty())
            continue;

        double weight = 1.0;
        try {
            weight = std::stod(trim(valueStr));
        } catch (...) {
        }

        g.addEdge(exporter, importer, weight);
        linesRead++;
    }

    std::cout << "[Trade] Cargado: " << g.getNumVertices() << " vertices, "
              << g.getNumEdges() << " aristas (" << linesRead << " lineas)\n";
    return g;
}

//  loadIoT
//  Formato CSV: src_ip,src_port,dst_ip,dst_port,...,duration,...
//  Detecta columnas por nombre en el encabezado automaticamente.
//  Grafo dirigido con peso (duration).
Graph loadIoT(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("loadIoT: no se pudo abrir " + filepath);

    Graph g(true);
    std::string line;

    // Leer encabezado y detectar indices de columnas
    std::getline(file, line);
    std::istringstream headerSS(line);
    std::string col;
    std::vector<std::string> headers;
    while (std::getline(headerSS, col, ','))
        headers.push_back(trim(col));

    int idxSrc = -1, idxDst = -1, idxDur = -1;
    for (int i = 0; i < (int)headers.size(); i++) {
        std::string h = headers[i];
        for (char &c : h)
            c = tolower(c);
        if (h == "src_ip" || h == "src" || h == "source_ip")
            idxSrc = i;
        if (h == "dst_ip" || h == "dst" || h == "dest_ip")
            idxDst = i;
        if (h == "duration" || h == "dur")
            idxDur = i;
    }

    // Fallback si no encontro por nombre
    if (idxSrc == -1)
        idxSrc = 0;
    if (idxDst == -1)
        idxDst = 2; // En el dataset real dst_ip esta en col 2

    std::cout << "[IoT] Columnas detectadas: src_ip=" << idxSrc
              << " dst_ip=" << idxDst << " duration=" << idxDur << "\n";

    int linesRead = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::vector<std::string> cols;
        std::string token;
        while (std::getline(ss, token, ','))
            cols.push_back(trim(token));

        if ((int)cols.size() <= std::max(idxSrc, idxDst))
            continue;

        std::string src = cols[idxSrc];
        std::string dst = cols[idxDst];
        if (src.empty() || dst.empty())
            continue;

        double weight = 1.0;
        if (idxDur >= 0 && idxDur < (int)cols.size()) {
            try {
                weight = std::stod(cols[idxDur]);
            } catch (...) {
            }
        }

        g.addEdge(src, dst, weight);
        linesRead++;
    }

    std::cout << "[IoT] Cargado: " << g.getNumVertices() << " vertices, "
              << g.getNumEdges() << " aristas (" << linesRead << " lineas)\n";
    return g;
}

//  loadIMDb
//  Formato CSV real: From,To,Strength
//  IDs de actores formato nmXXXXXXX. Grafo no dirigido con peso.
//  Strength = numero de peliculas en comun.
Graph loadIMDb(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("loadIMDb: no se pudo abrir " + filepath);

    Graph g(false);
    std::string line;
    int linesRead = 0;

    // Saltar encabezado (From,To,Strength)
    std::getline(file, line);

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string u, v, weightStr;

        if (!std::getline(ss, u, ','))
            continue;
        if (!std::getline(ss, v, ','))
            continue;
        std::getline(ss, weightStr, ',');

        u = trim(u);
        v = trim(v);
        if (u.empty() || v.empty())
            continue;

        double weight = 1.0;
        if (!weightStr.empty()) {
            try {
                weight = std::stod(trim(weightStr));
            } catch (...) {
            }
        }

        g.addEdge(u, v, weight);
        linesRead++;
    }

    std::cout << "[IMDb] Cargado: " << g.getNumVertices() << " vertices, "
              << g.getNumEdges() << " aristas (" << linesRead << " lineas)\n";
    return g;
}

//  loadGeneric
//  Parser generico configurable para cualquier formato simple.
Graph loadGeneric(const std::string &filepath, char sep, bool directed,
                  bool hasWeight, int skipLines) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("loadGeneric: no se pudo abrir " + filepath);

    Graph g(directed);
    std::string line;
    int linesRead = 0;

    for (int i = 0; i < skipLines; i++)
        std::getline(file, line);

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string u, v, weightStr;

        if (!std::getline(ss, u, sep))
            continue;
        if (!std::getline(ss, v, sep))
            continue;
        if (hasWeight)
            std::getline(ss, weightStr, sep);

        u = trim(u);
        v = trim(v);
        if (u.empty() || v.empty())
            continue;

        double weight = 1.0;
        if (hasWeight && !weightStr.empty()) {
            try {
                weight = std::stod(trim(weightStr));
            } catch (...) {
            }
        }

        g.addEdge(u, v, weight);
        linesRead++;
    }

    std::cout << "[Generic] Cargado: " << g.getNumVertices() << " vertices, "
              << g.getNumEdges() << " aristas (" << linesRead << " lineas)\n";
    return g;
}

Graph loadPajekNet(const std::string &filepath, bool directed) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("loadPajekNet: no se pudo abrir " + filepath);

    Graph g(directed);
    std::string line;
    
   
    std::vector<std::string> pajekIdToName;
    
    pajekIdToName.resize(2000, ""); 

    bool readingVertices = false;
    bool readingEdges = false;
    int linesRead = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

       
        if (line[0] == '*') {
            std::string lowerLine = line;
            for (char &c : lowerLine) c = tolower(c);
            
            if (lowerLine.find("vertices") != std::string::npos) {
                readingVertices = true;
                readingEdges = false;
            } else if (lowerLine.find("arcs") != std::string::npos || lowerLine.find("edges") != std::string::npos) {
                readingVertices = false;
                readingEdges = true;
            }
            continue;
        }

        
        if (readingVertices) {
            std::istringstream ss(line);
            int id;
            std::string name;
            if (ss >> id) {
                ss >> std::ws;
                
                if (ss.peek() == '"') {
                    ss.get(); 
                    std::getline(ss, name, '"');
                } else {
                    ss >> name;
                }
                
                
                if (id >= (int)pajekIdToName.size()) {
                    pajekIdToName.resize(id + 500, "");
                }
                
                pajekIdToName[id] = name;
                g.addVertex(name);
            }
        }
        
       
        else if (readingEdges) {
            std::istringstream ss(line);
            int uId, vId;
            double weight = 1.0;
            if (ss >> uId >> vId) {
                
                ss >> weight; 
                
                
                if (uId < (int)pajekIdToName.size() && vId < (int)pajekIdToName.size()) {
                    std::string srcName = pajekIdToName[uId];
                    std::string dstName = pajekIdToName[vId];
                    
                    if (!srcName.empty() && !dstName.empty()) {
                        g.addEdge(srcName, dstName, weight);
                        linesRead++;
                    }
                }
            }
        }
    }

    std::cout << "[Pajek] Cargado: " << g.getNumVertices() << " vertices, "
              << g.getNumEdges() << " aristas (" << linesRead << " lineas de red)\n";
    return g;
}

}