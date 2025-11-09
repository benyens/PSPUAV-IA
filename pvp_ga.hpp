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
inline GAResult run_ga_pvp(const std::vector<std::vector<int>>& D, const std::vector<double>& w, int m, int N, GAParams P) {
    std::mt19937 rng(P.seed);

    auto eval = [&](const Chromosome& c)->std::pair<double, std::vector<int>>{
        auto walk = decode(c);
        double cost = cost_max_weighted_revisit(walk, D, w);
        return {cost, std::move(walk)};
    };

    // Poblacion aleatoria inicial
    std::vector<Chromosome> population(P.population);
    for (auto& c : population) c = randomChromosome(rng, m, N);

    // Evaluar + orden (truncation)
    std::vector<std::pair<double,Chromosome>> pool;
    pool.reserve(P.population);
    for (const auto& c : population) {
        auto [cost, _] = eval(c);
        pool.push_back({cost, c});
    }

    std::sort(pool.begin(), pool.end(), [](const auto& A, const auto& B){ return A.first < B.first; });

    GAResult result;
    result.bestChromosome = pool.front().second;
    result.bestWalk = eval(result.bestChromosome).second;
    result.bestCost = pool.front().first;

    for (int g = 0; g<P.generations; ++g) {
        // Nueva generación
        std::vector<Chromosome> next;
        next.reserve(P.population);

        // Elitismo: conservar el mejor
        next.push_back(pool.front().second);

        // Rellenar nueva población
        for (int i = 0; (int)next.size() < P.population; ++i) {
            Chromosome a = pool[i % P.population].second;
            Chromosome b = pool[(i + 1) % P.population].second;
            crossover(rng, a, b, P.pcross);
            mutate(rng, a, P.pmut);
            mutate(rng, b, P.pmut);
            next.push_back(std::move(a));
            if ((int)next.size()<P.population) next.push_back(std::move(b));
        }

        // Evaluar + ordenar
        pool.clear();
        for (const auto& c : next) {
            auto [cost, _] = eval(c);
            pool.push_back({cost, c});
        }
        std::sort(pool.begin(), pool.end(), [](auto& A, auto& B){ return A.first < B.first; });

        // Actualizar el mejor encontrado
        if (pool.front().first < result.bestCost) {
            result.bestCost = pool.front().first;
            result.bestChromosome = pool.front().second;
            result.bestWalk = eval(result.bestChromosome).second;
        }
        result.history.push_back(pool.front().first); // costo mejor de la generación
    }
    return result;
}
