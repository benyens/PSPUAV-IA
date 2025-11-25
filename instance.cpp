#include "instance.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <unordered_map>

// La idea de este archivo es la lectura y formateo de las instancias entregadas

// Helpers para validación y lectura
static inline void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Error al cargar instancia: " + message);
    }
}

// Borra espacios al inicio y fin
static inline std::string trim(std::string s) {
    auto ns = [](int ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), ns));
    while (!s.empty() && std::isspace(s.back())) s.pop_back();
    return s;
}

// Lee una línea con formato "KEY VALUE" ignorando comentarios y espacios
static int readInst(std::ifstream& in, const std::string& key_expected) {
    std::string line;
    while(std::getline(in, line)) {
        auto p1 = line.find('#'); if (p1 != std::string::npos) line = line.substr(0, p1);
        auto p2 = line.find("//"); if (p2 != std::string::npos) line = line.substr(0, p2);
        line = trim(line);
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string key; int val;
        ss >> key >> val;
        expect(!ss.fail(), "línea mal formada para " + key_expected);
        expect(key == key_expected, "se esperaba clave '" + key_expected + "', se encontró '" + key + "'");
        return val;
    }
    throw std::runtime_error("No se encontró la clave '" + key_expected + "'");
}

// Carga una instancia desde archivo
Instance loadInstance(const std::string& ruta_instancia) {
    std::ifstream in(ruta_instancia);
    if (!in) throw std::runtime_error("No se pudo abrir el archivo: " + ruta_instancia);

    // Leer dimensiones de la grilla
    Instance instancia;
    instancia.rows = readInst(in, "GRID_ROWS");
    instancia.cols = readInst(in, "GRID_COLS");
    expect(instancia.rows > 0 && instancia.cols > 0, "GRID_ROWS/COLS deben ser >0");

    // Inicializar grillas
    instancia.obstacle.resize(instancia.rows, std::vector<bool>(instancia.cols, false));
    instancia.urgency.resize(instancia.rows, std::vector<double>(instancia.cols, 0.0));

    // Helper para validar coordenadas
    auto in_bounds = [&](int r, int c){ return r >= 0 && r < instancia.rows && c >= 0 && c < instancia.cols; };
    
    // Leer obstáculos
    int o = readInst(in, "N_OBSTACLES");
    for (int i=0;i<o; ++i) {
        std::string line; expect((bool)std::getline(in, line), "faltan líneas de obstáculos");
        std::istringstream ss(line); int r, c; ss >> r >> c;
        expect(!ss.fail(), "línea mal formada para obstáculo");
        expect(in_bounds(r,c), "coordenadas de obstáculo fuera de la grilla");
        instancia.obstacle[r][c] = true;
    }

    int u = readInst(in, "N_URGENCIES");
    for (int i=0;i<u; ++i) {
        std::string line; expect((bool)std::getline(in, line), "faltan líneas de urgencias");
        std::istringstream ss(line); int r, c; double val; ss >> r >> c >> val;
        expect(!ss.fail(), "línea mal formada para urgencia");
        expect(in_bounds(r,c), "coordenadas de urgencia fuera de la grilla");
        expect(val >= 0.0, "valor de urgencia negativo");
        instancia.urgency[r][c] = val;
    }

    return instancia;
}