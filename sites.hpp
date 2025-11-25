#pragma once
#include "instance.hpp"
#include <vector>
#include <queue>
#include <limits>
#include <utility>

// Definición de sitios y datos para el PVP
struct Site {
    int row, col; // Posición en la grilla
    double w; // Peso del sitio (urgencia)
};

// Datos derivados para el PVP
struct PVPData { 
    std::vector<Site> sites; // |sites| = m
    std::vector<std::vector<int>> dist; // dist[i][j]: distancia entre site i y j
};

// Verifica si (row,col) está dentro de los límites de la instancia
static inline bool in_bounds(const Instance& instancia, int row, int col){
    return row >= 0 && row < instancia.rows && col >= 0 && col < instancia.cols;
}

// Construye datos del PVP a partir de la instancia
static inline const int DR[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
static inline const int DC[8] = {0, 1, 1, 1, 0, -1, -1, -1};    

// Construye datos del PVP a partir de la instancia
inline PVPData buildPVPData(const Instance& instancia) {
    PVPData data;

    for (int r = 0; r < instancia.rows; ++r) {
        for (int c = 0; c < instancia.cols; ++c) {
            if (!instancia.obstacle[r][c] && instancia.urgency[r][c] > 0.0) {
                data.sites.push_back(Site{r, c, instancia.urgency[r][c]});
            }
        }
    }
    
    // Calcular distancias entre sitios usando BFS 8-conexo
    const int m = (int)data.sites.size();
    data.dist.assign(m, std::vector<int>(m, 0));
    const int INF = std::numeric_limits<int>::max() / 4;

    for (int s = 0; s < m; ++s) {
        std::vector<std::vector<int>> dist(instancia.rows, std::vector<int>(instancia.cols, INF));
        std::queue<std::pair<int, int>> q;
        int sr = data.sites[s].row, sc = data.sites[s].col;
        dist[sr][sc] = 0;
        q.push({sr, sc});

        while (!q.empty()) {
            auto [r, c] = q.front(); q.pop();
            for (int dir = 0; dir < 8; ++dir) {
                int nr = r + DR[dir], nc = c + DC[dir];
                if (!in_bounds(instancia, nr, nc) || instancia.obstacle[nr][nc]) continue;
                if (dist[nr][nc] > dist[r][c] + 1) {
                    dist[nr][nc] = dist[r][c] + 1;
                    q.push({nr, nc});
                }
            }
        }

        for (int t = 0; t < m; ++t) {
            int tr = data.sites[t].row, tc = data.sites[t].col;
            data.dist[s][t] = dist[tr][tc]; // Puede ser INF si no hay camino
        }
    }
    return data;
}