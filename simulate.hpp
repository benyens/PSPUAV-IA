#pragma once
#include "instance.hpp"
#include "sites.hpp"            // para PVPData::sites (r,c,w)
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <algorithm>

// 8 vecinos
// Desplazamientos relativos
static inline const int SDR[8] = {-1,-1, 0, 1, 1, 1, 0,-1};
static inline const int SDC[8] = { 0, 1,  1, 1, 0,-1,-1,-1};

// Verifica si (r,c) está dentro de los límites de la instancia
static inline bool sbounds(const Instance& I, int r, int c){
    return 0<=r && r<I.rows && 0<=c && c<I.cols;
}


// BFS 8-conexo evitando obstáculos. Retorna camino (incluye start y end).
// Si no hay camino, retorna {start} (nos quedamos en sitio para no romper simulación).
inline std::vector<std::pair<int,int>>
shortest_path8(const Instance& I, int r0, int c0, int r1, int c1){
    if (r0==r1 && c0==c1) return {{r0,c0}};
    const int INF = std::numeric_limits<int>::max()/4;
    std::vector<std::vector<int>> dist(I.rows, std::vector<int>(I.cols, INF));
    std::vector<std::vector<std::pair<int,int>>> parent(I.rows, std::vector<std::pair<int,int>>(I.cols, {-1,-1}));
    std::queue<std::pair<int,int>> q;
    dist[r0][c0]=0; q.push({r0,c0});
    while(!q.empty()){
        auto [r,c]=q.front(); q.pop();
        for(int d=0; d<8; ++d){
            int nr=r+SDR[d], nc=c+SDC[d];
            if(!sbounds(I,nr,nc) || I.obstacle[nr][nc]) continue;
            if(dist[nr][nc] > dist[r][c]+1){
                dist[nr][nc]=dist[r][c]+1;
                parent[nr][nc]={r,c};
                q.push({nr,nc});
            }
        }
    }
    if(dist[r1][c1]>=INF){ // inalcanzable: nos quedamos
        return {{r0,c0}};
    }
    // reconstrucción
    std::vector<std::pair<int,int>> rev;
    for(int r=r1, c=c1; r!=-1; ){
        rev.push_back({r,c});
        auto p=parent[r][c];
        r=p.first; c=p.second;
    }
    std::reverse(rev.begin(), rev.end());
    return rev;
}

// Construye un kernel de posiciones en la grilla concatenando paths entre sitios del walk (cíclico)
inline std::vector<std::pair<int,int>>
kernel_from_walk(const Instance& I, const PVPData& data, const std::vector<int>& walk){
    std::vector<std::pair<int,int>> kernel;
    if(walk.empty()) return kernel;
    const int N = (int)walk.size();
    for(int t=0; t<N; ++t){
        int a = walk[t]-1;                // sites en 1..m -> 0..m-1
        int b = walk[(t+1)%N]-1;
        auto sa = data.sites[a]; auto sb = data.sites[b];
        auto seg = shortest_path8(I, sa.row, sa.col, sb.row, sb.col);
        if(!kernel.empty() && !seg.empty()) {
            // evita duplicar el primer nodo del segmento
            seg.erase(seg.begin());
        }
        kernel.insert(kernel.end(), seg.begin(), seg.end());
    }
    // por seguridad: si quedó vacío (caso extremo), usa al menos el primer sitio
    if(kernel.empty()){
        auto s = data.sites[walk.front()-1];
        kernel.push_back({s.row,s.col});
    }
    return kernel;
}

// Asigna rutas a k drones: se desfasan en el kernel para cubrir mejor.
// Devuelve rutas[d][t] = (r,c) para t in [0..T-1]
inline std::vector<std::vector<std::pair<int,int>>>
assign_drones_on_kernel(const std::vector<std::pair<int,int>>& kernel, int k, int T){
    std::vector<std::vector<std::pair<int,int>>> routes(k, std::vector<std::pair<int,int>>(T));
    if(kernel.empty()) return routes;
    int L = (int)kernel.size();
    if(k<=0) k=1;
    for(int d=0; d<k; ++d){
        // desfase uniforme
        int start = (long long)d * L / std::max(1,k);
        for(int t=0; t<T; ++t){
            routes[d][t] = kernel[(start + t) % L];
        }
    }
    return routes;
}

// Simulación de urgencia acumulada por T ticks:
// - cada tick: +1 a toda celda no-obstáculo
// - si un dron visita (r,c) ese tick: urg[r][c] = 0
inline double simulate_urgency_accum(const Instance& I,
                                     const std::vector<std::vector<std::pair<int,int>>>& routes,
                                     int T)
{
    // copia de urgencias iniciales
    std::vector<std::vector<double>> urg = I.urgency;
    double total = 0.0;
    int K = (int)routes.size();

    for(int t=0; t<T; ++t){
        // sube urgencia
        for(int r=0;r<I.rows;++r)
            for(int c=0;c<I.cols;++c)
                if(!I.obstacle[r][c]) urg[r][c] += 1.0;
        // visitas
        for(int d=0; d<K; ++d){
            auto [r,c] = routes[d][t];
            if (sbounds(I,r,c) && !I.obstacle[r][c]) urg[r][c] = 0.0;
        }
        // acumula
        for(int r=0;r<I.rows;++r)
            for(int c=0;c<I.cols;++c)
                if(!I.obstacle[r][c]) total += urg[r][c];
    }
    return total;
}