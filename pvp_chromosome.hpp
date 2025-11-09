#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <stdexcept>

// Cromosoma = [swaps (m)] [added_nodes (N-m)] [insert_idx (N-m)]
//   swaps:        valores en {0..m} ; 0 = no-swap, x>0 = swap con (x-1)
//   added_nodes:  valores en {1..m}
//   insert_idx:   valores en {0..N-1}
// Especificación basada en Von Moll et al. (2018).  (walk factible) 
struct Chromosome { 
    int m = 0, N = 0; // m = número de sitios, N = número de nodos totales
    std::vector<int> genes; // genes.size() == m + 2*(N - m)
 
    int idx_swaps(int i) const {return i;} // [0..m-1]
    int idx_addnode(int k) const {return m + k;} // [0..N-m-1]
    int idx_insert(int k) const {return m + (N - m) + k;} // [0..N-m-1]
};

// Decodifica a un walk de largo N con símbolos en {1..m}
inline std::vector<int> decode(const Chromosome& c) {
    if (c.N < c.m) throw std::runtime_error("decode: N < m");
    std::vector<int> a(c.N, 0); // walk a[0..N-1]

    // Base walk inicial: [1..m]
    for (int i = 0; i < c.m; ++i) {a[i] = i + 1;}

    // 1) Aplicar swaps en orden inverso (mantiene factibilidad)
    for (int i = c.m - 1; i >= 0; --i) {
        int other = c.genes[c.idx_swaps(i)]; // 0 = no-swap, x>0 = swap con (x-1)
        if (other > 0) {
            int j = other - 1; // swap con posición j
            if (j<0 || j>=c.m) throw std::runtime_error("decode: swap índice fuera de rango");
            std::swap(a[i], a[j]);
        }
    }

    // 2) Insertar repeticiones
    for (int k =0; k < c.N - c.m; ++k) {
        int node = c.genes[c.idx_addnode(k)]; // valor en {1..m}
        int insert_idx = c.genes[c.idx_insert(k)]; // valor en {0..N-1}
        if (node < 1 || node > c.m) throw std::runtime_error("decode: added_node fuera de rango");
        if (insert_idx < 0 || insert_idx > (int)a.size()) throw std::runtime_error("decode: insert_idx fuera de rango");

        if (a[insert_idx] == 0) {
            a[insert_idx] = node;
        } else {
            // Mover elementos a la derecha desde insert_idx
            a.insert(a.begin() + insert_idx, node);
            auto it0 = std::find(a.begin(), a.end(), 0);
            if (it0==a.end()) a.pop_back(); else a.erase(it0);
        }
    }

    // relleno de seguridad (no debería quedar ninguno)
    for (int i = 0; i < c.N;++i) if (a[i]==0) a[i]=1;
    return a; 
}

// Inicialización aleatoria factible
inline Chromosome randomChromosome(std::mt19937& rng, int m, int N) {
    Chromosome c; c.m=m; c.N=N;
    c.genes.assign(m + 2 * (N - m), 0);
    std::uniform_int_distribution<int> pickSwap(0, m); // 0 = no-swap, x>0 = swap con (x-1)
    std::uniform_int_distribution<int> pickNode(1, m);
    std::uniform_int_distribution<int> pickInsert(0, N - 1);
    for (int i = 0; i < m; ++i) c.genes[c.idx_swaps(i)] = pickSwap(rng);    
    for (int k = 0; k < N - m; ++k) {
        c.genes[c.idx_addnode(k)] = pickNode(rng);
        c.genes[c.idx_insert(k)] = pickInsert(rng);
    }
    return c;
}




