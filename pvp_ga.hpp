#pragma once
#include "pvp_chromosome.hpp"
#include "pvp_fitness.hpp"
#include <random>
#include <algorithm>
#include <utility>

struct GAParams {
    int population = 100; // tamaño de la población según el paper
    int generations = 20; // número de generaciones según el paper (se puede mapear desde k)
    double pcross = 0.7; // probabilidad de cruce según el paper 
    double pmut = 0.01; // probabilidad de mutación según el paper
    unsigned seed = 12345; // semilla del RNG
};

struct GAResult {
    Chromosome bestChromosome; // mejor cromosoma encontrado
    double bestCost = 1e18; // mejor costo encontrado
    std::vector<int> bestWalk; // mejor camino encontrado
    std::vector<double> history; // historial de costos por generación (opcional)
};

// Genera un cromosoma aleatorio factible
inline void mutate(std::mt19937& rng, Chromosome& c, double pmut) {
    std::bernoulli_distribution flip(pmut);
    std::uniform_int_distribution<int> dist_swap(0, c.m);
    std::uniform_int_distribution<int> dist_addnode(1, c.m);
    std::uniform_int_distribution<int> dist_insert(0, c.N - 1);

    for (int i = 0; i < c.m; ++i) if (flip(rng)) c.genes[c.idx_swaps(i)] = dist_swap(rng);
    for (int k = 0; k < c.N - c.m; ++k) {
        if (flip(rng)) c.genes[c.idx_addnode(k)] = dist_addnode(rng);
        if (flip(rng)) c.genes[c.idx_insert(k)] = dist_insert(rng);
    }
}

// Cruza dos cromosomas con probabilidad pcross
inline void crossover(std::mt19937& rng, Chromosome& a, Chromosome& b, double pcross) {
    if (a.genes.size() != b.genes.size()) return;
    std::bernoulli_distribution dox(pcross);
    if(!dox(rng)) return;
    std::uniform_int_distribution<int> cut(1, (int)a.genes.size() - 1);
    int cpos = cut(rng);
    for (int i=cpos; i<(int)a.genes.size(); ++i) std::swap(a.genes[i], b.genes[i]);
}

// Ejecuta el algoritmo genético para el PVP
inline GAResult run_ga_pvp_urgency(
    const Instance& instancia,
    const PVPData& data,
    int max_drones,
    int T,
    int m,
    int N,
    GAParams P
) {
    std::mt19937 rng(P.seed);

    // Contexto para fitness
    PVPContext ctx{ &instancia, &data, max_drones, T };

    // Función de evaluación usando FO
    auto eval = [&](const Chromosome& c)->std::pair<double, std::vector<int>>{
        double cost = fitness_urgency(c, ctx);
        auto walk = decode_walk(c);
        return {cost, std::move(walk)};
    };

    // Población inicial
    std::vector<Chromosome> population(P.population);
    for (auto& c : population) c = randomChromosome(rng, m, N);

    // Evaluación inicial
    std::vector<std::pair<double, Chromosome>> pool;
    pool.reserve(P.population);
    for (const auto& c : population) {
        auto [cost, _] = eval(c);
        pool.push_back({cost, c});
    }
    std::sort(pool.begin(), pool.end(), [](auto& A, auto& B){ return A.first < B.first; });

    GAResult result;
    result.bestChromosome = pool.front().second;
    result.bestWalk = eval(result.bestChromosome).second;
    result.bestCost = pool.front().first;

    // Evolución
    for (int g = 0; g < P.generations; ++g) {
        std::vector<Chromosome> next;
        next.reserve(P.population);

        next.push_back(pool.front().second); // elitismo

        for (int i = 0; (int)next.size() < P.population; ++i) {
            Chromosome a = pool[i % P.population].second;
            Chromosome b = pool[(i + 1) % P.population].second;
            crossover(rng, a, b, P.pcross);
            mutate(rng, a, P.pmut);
            mutate(rng, b, P.pmut);
            next.push_back(std::move(a));
            if ((int)next.size() < P.population) next.push_back(std::move(b));
        }

        pool.clear();
        for (const auto& c : next) {
            auto [cost, _] = eval(c);
            pool.push_back({cost, c});
        }
        std::sort(pool.begin(), pool.end(), [](auto& A, auto& B){ return A.first < B.first; });

        if (pool.front().first < result.bestCost) {
            result.bestCost = pool.front().first;
            result.bestChromosome = pool.front().second;
            result.bestWalk = eval(result.bestChromosome).second;
        }

        result.history.push_back(pool.front().first);
    }

    return result;
}


