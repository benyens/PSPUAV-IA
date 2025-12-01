#pragma once
#include "instance.hpp"
#include "sites.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>

// Escribe la instancia y la solución en un archivo JSON para visualización o análisis posterior.
inline void dump_run_snapshot(const std::string& outpath,
                              const Instance& I,
                              const PVPData& data,
                              int max_drones,
                              int T,
                              const std::vector<int>& bestWalk,
                              const std::vector<std::pair<int,int>>& kernel,
                              const std::vector<std::vector<std::pair<int,int>>>& rutas, double total_time_s)
{
    std::ofstream out(outpath);
    if(!out){
        std::cerr << "[dump] no pude abrir " << outpath << "\n";
        return;
    }

    // ======================================================
    // 1) CHEQUEO DE FACTIBILIDAD
    // ======================================================
    bool feasible = true;
    std::string reason = "OK";

    auto in_grid = [&](int r, int c){
        return (r >= 0 && r < I.rows && c >= 0 && c < I.cols);
    };

    auto is_adjacent = [&](int r1, int c1, int r2, int c2){
        int dr = std::abs(r1 - r2);
        int dc = std::abs(c1 - c2);
        // permite quedarse quieto o moverse a cualquiera de los 8 vecinos
        return (dr <= 1 && dc <= 1);
    };

    for (size_t d = 0; d < rutas.size() && feasible; ++d) {
        const auto& ruta = rutas[d];

        // Largo de la ruta = T
        if ((int)ruta.size() != T) {
            feasible = false;
            reason = "Dron " + std::to_string(d) + ": longitud de ruta != T";
            break;
        }

        for (int t = 0; t < T && feasible; ++t) {
            int r = ruta[t].first;
            int c = ruta[t].second;

            // Dentro del grid
            if (!in_grid(r,c)) {
                feasible = false;
                reason = "Dron " + std::to_string(d) +
                         ": sale del grid en t=" + std::to_string(t);
                break;
            }

            // No pisa obstáculo
            if (I.obstacle[r][c]) {
                feasible = false;
                reason = "Dron " + std::to_string(d) +
                         ": pisa obstaculo en t=" + std::to_string(t);
                break;
            }

            // Movimiento adyacente (o quedarse quieto)
            if (t > 0) {
                int r_prev = ruta[t-1].first;
                int c_prev = ruta[t-1].second;
                if (!is_adjacent(r_prev, c_prev, r, c)) {
                    feasible = false;
                    reason = "Dron " + std::to_string(d) +
                             ": movimiento no adyacente en t=" +
                             std::to_string(t);
                    break;
                }
            }
        }
    }

    // ======================================================
    // 2) ESCRITURA DEL JSON
    // ======================================================

    out << "{\n";
    out << "  \"rows\": " << I.rows << ",\n";
    out << "  \"cols\": " << I.cols << ",\n";
    out << "  \"max_drones\": " << max_drones << ",\n";
    out << "  \"T\": " << T << ",\n";

    // Obstáculos
    out << "  \"obstacles\": [";
    bool first = true;
    for(int r=0;r<I.rows;++r) for(int c=0;c<I.cols;++c){
        if(I.obstacle[r][c]) {
            if(!first) out<<",";
            first=false;
            out << "[" << r << "," << c << "]";
        }
    }
    out << "],\n";

    // Urgencias iniciales (>0)
    out << "  \"urgencies\": [";
    first = true;
    for(int r=0;r<I.rows;++r) for(int c=0;c<I.cols;++c){
        double u = I.urgency[r][c];
        if(u > 0){
            if(!first) out<<",";
            first=false;
            out << "[" << r << "," << c << "," << u << "]";
        }
    }
    out << "],\n";

    // Bases
    out << "  \"bases\": [";
    for(size_t i=0;i<I.bases.size();++i){
        if(i) out<<",";
        out << "{\"id\":" << I.bases[i].id
            << ",\"r\":"   << I.bases[i].row
            << ",\"c\":"   << I.bases[i].col << "}";
    }
    out << "],\n";

    // Sitios
    out << "  \"sites\": [";
    for(size_t i=0;i<data.sites.size();++i){
        if(i) out<<",";
        out << "{\"idx\":" << (int)(i+1)
            << ",\"r\":"   << data.sites[i].row
            << ",\"c\":"   << data.sites[i].col
            << ",\"w\":"   << data.sites[i].w << "}";
    }
    out << "],\n";

    // bestWalk
    out << "  \"best_walk\": [";
    for(size_t i=0;i<bestWalk.size();++i){
        if(i) out<<",";
        out << bestWalk[i];
    }
    out << "],\n";

    // kernel
    out << "  \"kernel\": [";
    for(size_t i=0;i<kernel.size();++i){
        if(i) out<<",";
        out << "[" << kernel[i].first << "," << kernel[i].second << "]";
    }
    out << "],\n";

    // rutas
    out << "  \"routes\": [";
    for(size_t d=0; d<rutas.size(); ++d){
        if(d) out<<",";
        out << "[";
        for(size_t t=0;t<rutas[d].size();++t){
            if(t) out<<",";
            out << "[" << rutas[d][t].first << "," << rutas[d][t].second << "]";
        }
        out << "]";
    }
    out << "],\n";

    // Mejor Urgencia total (se puede calcular afuera)
    double urg_total = simulate_urgency_accum(I, rutas, T);
    out << "  \"best_urgency_total\": " << urg_total << ",\n";
    out << "  \"best_urgency_avg_per_tick\": " << (urg_total / T) << ",\n";

    // Tiempo total de ejecución (en segundos)
    out << "  \"total_time\": " << total_time_s << ", \n";

    // Campos de factibilidad
    out << "  \"feasible\": " << (feasible ? "true" : "false") << ",\n";
    out << "  \"feasible_reason\": \"" << reason << "\"\n";

    out << "}\n";
}
