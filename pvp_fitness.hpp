#pragma once
#include <vector>
#include <limits>
#include <algorithm>

// walk: valores en {1..m}, tamaño m
inline double cost_max_weighted_revisit(const std::vector<int>& walk, const std::vector<std::vector<int>>& D, const std::vector<double>& w) {
    const int m = (int)w.size();
    const int N = (int)walk.size();
    const int INF = std::numeric_limits<int>::max() / 8;

    // Posiciones donde aparece cada sitio (1..m)
    std::vector<std::vector<int>> positions(m + 1);
    for (int t = 0; t < N; ++t) positions[walk[t]].push_back(t);

    auto travel_time = [&](int a, int b)->int{
        // Tiempo de viaje entre walk[a] y walk[b]
        // Sumando D entre posiciones consecutivas
        int t = a;
        long long tot = 0;
        do {
            int cur = walk[t];
            int nxt = walk[(t + 1) % N];
            int dist = D[cur - 1][nxt - 1];
            if (dist >= INF) return INF; // no conectado
            tot += dist;
            t = (t + 1) % N;
        } while (t != b);
        return (tot > INF) ? INF : (int)tot;
    };

    double worst = 0.0;
    for (int i = 1; i <= m; ++i) {
        if (positions[i].empty()) return 1e18;
        int maxGap = 0;
        for (size_t p = 0; p < positions[i].size(); ++p) {
            int a = positions[i][p];
            int b = positions[i][(p + 1) % positions[i].size()];
            int gap = travel_time(a, b);
            if (gap > maxGap) maxGap = gap;
        }
        worst = std::max(worst, w[i - 1] * (double)maxGap);
    }
    return worst; // valor a minimizar
}