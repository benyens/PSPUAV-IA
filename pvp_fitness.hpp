#pragma once
#include <vector>
#include <cmath>
#include "instance.hpp"
#include "sites.hpp"
#include "simulate.hpp"
#include "pvp_chromosome.hpp"   // para Chromosome / decode

// Contexto para evaluar fitness según la FO del profesor
struct PVPContext {
    const Instance* inst;
    const PVPData*  data;
    int max_drones;
    int T;
};

// Reutilizamos tu decode original
inline std::vector<int> decode_walk(const Chromosome& c) {
    return decode(c);
}

// Fitness correcto: urgencia acumulada + penalizaciones (todo a MINIMIZAR)
inline double fitness_urgency(
    const Chromosome& c,
    const PVPContext& ctx
){
    const Instance& inst = *ctx.inst; // si prefieres, puedes usar *ctx.inst directamente
    const int R = inst.rows;
    const int C = inst.cols;

    // 1) Decodificar walk
    auto walk = decode_walk(c);
    if (walk.empty()) return 1e15;

    // 2) Construir kernel desde walk
    auto kernel = kernel_from_walk(inst, *ctx.data, walk);
    if (kernel.empty()) return 1e15;

    // 3) Asignar rutas a drones
    auto rutas = assign_drones_on_kernel(kernel, ctx.max_drones, ctx.T);
    if (rutas.empty()) return 1e15;

    // 4) FO principal: urgencia acumulada
    double urg_total = simulate_urgency_accum(inst, rutas, ctx.T);

    // 5) Penalizaciones
    const double BIG = 1000.0;
    double penalties = 0.0;

    // Matriz de cobertura (para penalizar sitios nunca visitados)
    std::vector<std::vector<bool>> visited(R, std::vector<bool>(C, false));

    int idle_drones = 0;
    int out_of_bounds_steps = 0;
    int obstacle_steps = 0;
    int non_adjacent_moves = 0;

    for (const auto& route : rutas) {
        if (route.empty()) {
            idle_drones++;
            continue;
        }
        // marca visitas y chequea cosas
        std::pair<int,int> prev = route.front();
        bool all_same = true;

        for (size_t t = 0; t < route.size(); ++t) {
            int r = route[t].first;
            int c = route[t].second;

            // fuera de grilla
            if (r < 0 || r >= R || c < 0 || c >= C) {
                out_of_bounds_steps++;
                continue; // no usamos más info de esta celda
            }

            // obstáculo
            if (inst.obstacle[r][c]) {
                obstacle_steps++;
            }

            visited[r][c] = true;

            if (t > 0) {
                int dr = std::abs(r - prev.first);
                int dc = std::abs(c - prev.second);
                // Permito 8-vecino; si no, penalizo
                if (std::max(dr, dc) > 1) non_adjacent_moves++;
                if (r != prev.first || c != prev.second) all_same = false;
            }
            prev = route[t];
        }

        if (all_same) idle_drones++; // dron que nunca se mueve "en serio"
    }

    // P1: salirse de la grilla
    if (out_of_bounds_steps > 0) {
        penalties += BIG * 10.0 * out_of_bounds_steps;
    }

    // P2: pasar por obstáculos
    if (obstacle_steps > 0) {
        penalties += BIG * 5.0 * obstacle_steps;
    }

    // P3: movimientos no adyacentes
    if (non_adjacent_moves > 0) {
        penalties += BIG * 2.0 * non_adjacent_moves;
    }

    // P4: sitios urgentes nunca visitados
    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            double w = inst.urgency[r][c];
            if (w > 0.0 && !visited[r][c]) {
                penalties += BIG * w; // ponderado por urgencia
            }
        }
    }

    // P5: drones ociosos
    if (idle_drones > 0) {
        penalties += BIG * 50.0 * idle_drones;
    }

    return urg_total + penalties;
}
