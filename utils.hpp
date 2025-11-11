#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <iomanip>

// El propósito de este archivo es tener todas las funciones de utilidad
// como pueden ser los prints para los diferentes tipos y un pequeño helper
// para la creación y formato de los .csv
// Timer simple
struct Timer {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point t0;
    Timer() { reset(); }
    void reset() { t0 = clock::now(); }
    double elapsed() const { 
        using namespace std::chrono;
        return duration<double>(clock::now() - t0).count(); 
    }
};

// Helpers para impresión formateada
inline void print_section(const std::string& title) {
    std::cout << "\n==== " << title << " ====\n";
}

// key-value printing
inline void print_kv(const std::string& k, const std::string& v) {
    std::cout << "  " << k << ": " << v << "\n";
}

// overloads para otros tipos
inline void print_kv(const std::string& k, long long v) {
    std::cout << "  " << k << ": " << v << "\n";
}

// overloads para otros tipos
inline void print_kv(const std::string& k, int v) {
    std::cout << "  " << k << ": " << v << "\n";
}

// overloads para otros tipos
inline void print_kv(const std::string& k, double v, int prec=6) {
    std::cout << "  " << k << ": " << std::fixed << std::setprecision(prec) << v << "\n";
    std::cout.unsetf(std::ios::floatfield);
}

// Pequeño helper para CSV: crea si no existe y añade cabecera
inline void csv_append_row(const std::string& path,
                           const std::vector<std::string>& header,
                           const std::vector<std::string>& row)
{
    namespace fs = std::filesystem;
    bool write_header = !fs::exists(path) || fs::file_size(path)==0;
    std::ofstream out(path, std::ios::app);
    if (!out) { std::cerr << "WARN: no pude abrir CSV: " << path << "\n"; return; }
    if (write_header) {
        for (size_t i=0;i<header.size();++i){
            if (i) out<<",";
            out<<header[i];
        }
        out<<"\n";
    }
    for (size_t i=0;i<row.size();++i){
        if (i) out<<",";
        out<<row[i];
    }
    out<<"\n";
}

// stringify helpers
template<typename T>
inline std::string s(T v){ return std::to_string(v); }
inline std::string s(const std::string& v){ return v; }
