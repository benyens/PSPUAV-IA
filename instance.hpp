#pragma once
#include <vector>
#include <string>
#include <stdexcept>

// Este archivo es solo una interfaz que crea la definición de instancia.

// Definición de la instancia del problema
struct Base{
    int id;
    int row;
    int col;
};

// Instancia de la grilla
struct Instance {
    int rows = 0, cols = 0;
    std::vector<std::vector<bool>> obstacle; // obstacle[row][col] == true si es celda restringida
    std::vector<std::vector<double>> urgency; // urgency[row][col] = urgencia inicial de la celda
    std::vector<Base> bases;
};

Instance loadInstance(const std::string& ruta_instancia);