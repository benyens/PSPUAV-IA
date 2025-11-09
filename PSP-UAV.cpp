#include "instance.hpp"
#include "sites.hpp"
#include "pvp_chromosome.hpp"
#include "pvp_fitness.hpp"
#include "pvp_ga.hpp"
#include "simulate.hpp"
#include "utils.hpp"
#include "dump_json.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <limits>
#include <filesystem>   // <-- NUEVO
namespace fs = std::filesystem;

// Programa principal para resolver el problema PSP-UAV usando un algoritmo genético basado en el paper.
int main(int argc, char** argv) {
    // Uso base: ./PSP-UAV <instancia.txt> <max_drones> <K> <T>
    // Opcionales:
    //   --seed=123    --gen=40     --pop=200     --Nmult=3
    //   --csv=salida.csv  --tag=run1  --dump=archivo.json  --outdir=resultados/showcase
    if (argc < 5) {
        std::cerr << "Uso: " << argv[0]
                  << " <instancia.txt> <max_drones> <K> <T> [opciones]\n"
                  << "Ejemplo: " << argv[0]
                  << " instancias/PSP-UAV_01_a.txt 5 1000 50 --gen=20 --pop=100 --outdir=resultados/showcase\n";
        return 1;
    }

    try {
        // --- CLI obligatorios
        const std::string path = argv[1];
        int max_drones = std::stoi(argv[2]);
        long long K    = std::stoll(argv[3]);
        int T          = std::stoi(argv[4]);

        // --- CLI opcionales
        unsigned seed = 12345;
        int generations_override = -1;   // -1 => usa mapping desde K
        int population_override  = -1;   // -1 => 100
        int Nmult = 3;                   // N = m + (m/Nmult); ej 3 => ~1.33*m | si pones 1 => 2*m
        std::string csv_path = "";       // vacío => no loggea
        std::string tag = "";            // etiqueta libre
        std::string dump_json = "";      // si no vacío, vuelca snapshot JSON a esa ruta
        std::string outdir    = "resultados/showcase"; // <-- NUEVO (carpeta por defecto)

        // Parseo opciones
        for (int i=5; i<argc; ++i) {
            std::string a = argv[i];
            auto take = [&](const std::string& key)->std::string{
                auto p = a.find(key);
                if (p==0) return a.substr(key.size());
                return "";
            };
            if (a.rfind("--seed=",0)==0)       { seed = (unsigned)std::stoul(take("--seed=")); }
            else if (a.rfind("--gen=",0)==0)   { generations_override = std::stoi(take("--gen=")); }
            else if (a.rfind("--pop=",0)==0)   { population_override  = std::stoi(take("--pop=")); }
            else if (a.rfind("--Nmult=",0)==0) { Nmult = std::max(1, std::stoi(take("--Nmult="))); }
            else if (a.rfind("--csv=",0)==0)   { csv_path = take("--csv="); }
            else if (a.rfind("--tag=",0)==0)   { tag = take("--tag="); }
            else if (a.rfind("--dump=",0)==0)  { dump_json = take("--dump="); }
            else if (a.rfind("--outdir=",0)==0){ outdir    = take("--outdir="); } // <-- NUEVO
        }

        Timer T_all; // tiempo total

        // =========================
        print_section("INPUT");
        print_kv("instancia", path);
        print_kv("max_drones", max_drones);
        print_kv("K", K);
        print_kv("T", T);
        if (!tag.empty())  print_kv("tag", tag);
        print_kv("seed", (long long)seed);

        // =========================
        print_section("LOAD INSTANCE");
        Timer T_load;
        Instance instancia = loadInstance(path);
        double t_load = T_load.elapsed();
        print_kv("rows", instancia.rows);
        print_kv("cols", instancia.cols);
        size_t nObs=0, nUrg=0;
        for (int r=0; r<instancia.rows; ++r)
            for (int c=0; c<instancia.cols; ++c) {
                if (instancia.obstacle[r][c]) ++nObs;
                if (instancia.urgency[r][c] > 0.0) ++nUrg;
            }
        print_kv("obstaculos", (long long)nObs);
        print_kv("celdas_urg>0", (long long)nUrg);
        print_kv("bases", (long long)instancia.bases.size());
        print_kv("t_load_s", t_load, 6);

        // =========================
        print_section("SITES + D (8-neigh BFS)");
        Timer T_sites;
        auto data = buildPVPData(instancia);
        double t_sites = T_sites.elapsed();
        int m = (int)data.sites.size();
        print_kv("m_sites", m);
        print_kv("t_sites_s", t_sites, 6);

        if (m == 0) {
            std::cout << "No hay sitios con urgencia. Termina.\n";
            return 0;
        }

        // =========================
        print_section("GA (paper)");
        // Largo del walk (N): m + m/Nmult (si Nmult=1 => 2*m, si Nmult=3 => ~1.33*m)
        int N = m + m / std::max(1, Nmult);
        // pesos
        std::vector<double> weights(m);
        for (int i=0; i<m; ++i) weights[i] = data.sites[i].w;

        GAParams params;
        params.population  = (population_override>0 ? population_override : 100);
        params.generations = (generations_override>0 ? generations_override
                            : std::max(1, (int)std::min<long long>(50, K/50))); // K=1000 => ~20
        params.pcross = 0.70;
        params.pmut   = 0.01;
        params.seed   = seed;

        print_kv("population", params.population);
        print_kv("generations", params.generations);
        print_kv("pcross", params.pcross, 3);
        print_kv("pmut", params.pmut, 3);
        print_kv("N", N);

        Timer T_ga;
        auto ga = run_ga_pvp(data.dist, weights, m, N, params);
        double t_ga = T_ga.elapsed();

        print_kv("best_cost", ga.bestCost, 6);
        print_kv("t_ga_s", t_ga, 6);
        std::cout << "  best_walk[0..min(30)]: ";
        for (size_t i=0; i<ga.bestWalk.size() && i<30; ++i) std::cout<<ga.bestWalk[i]<<" ";
        std::cout<<(ga.bestWalk.size()>30?"...":"")<<"\n";

        std::cout << "  hist_cost: ";
        for (size_t i=0;i<ga.history.size();++i){
            if (i) std::cout << ", ";
            std::cout << ga.history[i];
        }
        std::cout << "\n";

        // =========================
        print_section("SIMULATION (urgency accumulation)");
        Timer T_sim;
        auto kernel = kernel_from_walk(instancia, data, ga.bestWalk);
        auto rutas  = assign_drones_on_kernel(kernel, max_drones, T);
        double urg_total = simulate_urgency_accum(instancia, rutas, T);
        double t_sim = T_sim.elapsed();

        print_kv("urg_total", urg_total, 3);
        print_kv("kernel_len", (long long)kernel.size());
        print_kv("t_sim_s", t_sim, 6);

        // preview ruta dron 0
        if (!rutas.empty()) {
            std::cout << "  D0[0..min(20)]: ";
            for (int i=0; i<std::min(20, T); ++i) {
                auto [rr,cc]=rutas[0][i];
                std::cout<<"("<<rr<<","<<cc<<") ";
            }
            std::cout << "\n";
        }

        // =========================
        print_section("TOTAL TIME");
        double t_all = T_all.elapsed();
        print_kv("t_total_s", t_all, 6);

        // =========================
        // CSV logging (si se solicitó)
        if (!csv_path.empty()) {
            std::vector<std::string> header = {
                "tag","instancia","seed","pop","gen","N","m",
                "best_cost","urg_total","t_load_s","t_sites_s","t_ga_s","t_sim_s","t_total_s"
            };
            std::vector<std::string> row = {
                tag, path, s(seed), s(params.population), s(params.generations), s(N), s(m),
                s(ga.bestCost), s(urg_total), s(t_load), s(t_sites), s(t_ga), s(t_sim), s(t_all)
            };
            csv_append_row(csv_path, header, row);
            std::cout << "CSV -> " << csv_path << "\n";
        }

        // =========================
        // HOOK de snapshot JSON (para visualización / animación)
        // Si no se entregó --dump, generamos un nombre automático dentro de outdir
        if (dump_json.empty()) {
            try { fs::create_directories(outdir); } catch (...) { /* no fatal */ }
            fs::path inst_path(path);
            std::string base = inst_path.stem().string(); // ej: PSP-UAV_01_a
            // Nombre: <inst>_g<gen>_p<pop>_n<Nmult>_s<seed>_snap.json
            dump_json = (fs::path(outdir) /
                        (base + "_g" + s(params.generations) +
                                "_p" + s(params.population) +
                                "_n" + s(Nmult) +
                                "_s" + s(seed) + "_snap.json")).string();
        }

        dump_run_snapshot(dump_json, instancia, data, max_drones, T,
                          ga.bestWalk, kernel, rutas);
        std::cout << "Dump JSON -> " << dump_json << "\n";

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
}
