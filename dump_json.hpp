#pragma once
#include "instance.hpp"
#include "sites.hpp"
#include <string>
#include <fstream>
#include <iostream>

// Escribe la instancia y la solución en un archivo JSON para visualización o análisis posterior.
inline void dump_run_snapshot(const std::string& outpath,
                              const Instance& I,
                              const PVPData& data,
                              int max_drones,
                              int T,
                              const std::vector<int>& bestWalk,
                              const std::vector<std::pair<int,int>>& kernel,
                              const std::vector<std::vector<std::pair<int,int>>>& rutas)
{
    std::ofstream out(outpath);
    if(!out){ std::cerr<<"[dump] no pude abrir "<<outpath<<"\n"; return; }

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
            out<<"["<<r<<","<<c<<"]";
        }
    }
    out << "],\n";

    // Urgencias iniciales (>0)
    out << "  \"urgencies\": [";
    first = true;
    for(int r=0;r<I.rows;++r) for(int c=0;c<I.cols;++c){
        double u = I.urgency[r][c];
        if(u>0){
            if(!first) out<<",";
            first=false;
            out<<"["<<r<<","<<c<<","<<u<<"]";
        }
    }
    out << "],\n";

    // Bases
    out << "  \"bases\": [";
    for(size_t i=0;i<I.bases.size();++i){
        if(i) out<<",";
        out<<"{\"id\":"<<I.bases[i].id<<",\"r\":"<<I.bases[i].row<<",\"c\":"<<I.bases[i].col<<"}";
    }
    out << "],\n";

    // Sitios
    out << "  \"sites\": [";
    for(size_t i=0;i<data.sites.size();++i){
        if(i) out<<",";
        out<<"{\"idx\":"<<(int)(i+1)<<",\"r\":"<<data.sites[i].row<<",\"c\":"<<data.sites[i].col<<",\"w\":"<<data.sites[i].w<<"}";
    }
    out << "],\n";

    // bestWalk
    out << "  \"best_walk\": [";
    for(size_t i=0;i<bestWalk.size();++i){
        if(i) out<<",";
        out<<bestWalk[i];
    }
    out << "],\n";

    // kernel
    out << "  \"kernel\": [";
    for(size_t i=0;i<kernel.size();++i){
        if(i) out<<",";
        out<<"["<<kernel[i].first<<","<<kernel[i].second<<"]";
    }
    out << "],\n";

    // rutas
    out << "  \"routes\": [";
    for(size_t d=0; d<rutas.size(); ++d){
        if(d) out<<",";
        out << "[";
        for(size_t t=0;t<rutas[d].size();++t){
            if(t) out<<",";
            out<<"["<<rutas[d][t].first<<","<<rutas[d][t].second<<"]";
        }
        out << "]";
    }
    out << "]\n";

    out << "}\n";
}
