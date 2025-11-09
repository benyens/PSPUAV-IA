#!/usr/bin/env python3
import os, sys, platform, subprocess, itertools, csv
from pathlib import Path

# ---------- CONFIG ----------
EXEC = "PSP-UAV.exe" if platform.system().lower().startswith("win") else "./PSP-UAV"

INSTANCES = [
    ("instancias/PSP-UAV_01_a.txt", 5, 1000, 50),
    ("instancias/PSP-UAV_01_b.txt", 5, 1000, 50),
    ("instancias/PSP-UAV_02_a.txt",10, 1000, 50),
    ("instancias/PSP-UAV_02_b.txt",10, 1000, 50),
    ("instancias/PSP-UAV_03_a.txt",15, 1000, 50),
    ("instancias/PSP-UAV_03_b.txt",15, 1000, 50),
]

# Barrido de parámetros (ajusta a gusto)
GENS   = [20, 40, 60]
POPS   = [100, 200]
NMULTS = [1, 3]       # 1 => N ~ 2m ; 3 => N ~ 1.33m
SEEDS  = [7, 42]

CSV_OUT = Path("resultados/exp_runs.csv")
PDF_DIR = Path("resultados/pdfs")
SNAP_DIR = Path("resultados/snapshots")
PDF_DIR.mkdir(parents=True, exist_ok=True)
CSV_OUT.parent.mkdir(parents=True, exist_ok=True)
SNAP_DIR.mkdir(parents=True, exist_ok=True)

# ---------- Helpers ----------
def run_cmd(cmd, cwd=None):
    print(">>", " ".join(str(x) for x in cmd))
    res = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    print(res.stdout)
    if res.returncode != 0:
        print(f"[WARN] proceso terminó con código {res.returncode}")
    return res.returncode

# ---------- Sweep: ejecuta binario y vuelca a CSV ----------
def sweep():
    # Cabecera CSV: el binario la crea si no existe, pero asegurar ruta
    if not CSV_OUT.exists():
        with open(CSV_OUT, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow([
                "tag","instancia","seed","pop","gen","N","m",
                "best_cost","urg_total","t_load_s","t_sites_s","t_ga_s","t_sim_s","t_total_s"
            ])

    for (inst, k, K, T), g, p, nm, seed in itertools.product(INSTANCES, GENS, POPS, NMULTS, SEEDS):
        tag = f"{Path(inst).stem}_g{g}_p{p}_n{nm}_s{seed}"
        cmd = [
            EXEC, inst, str(k), str(K), str(T),
            f"--gen={g}", f"--pop={p}", f"--Nmult={nm}",
            f"--seed={seed}", f"--csv={str(CSV_OUT)}", f"--tag={tag}", f"--dump={str(SNAP_DIR / (tag + '_snap.json'))}"
        ]
        rc = run_cmd(cmd)
        GENERAR_PNGS = True
        if GENERAR_PNGS:
            subprocess.run(["python", "vis_snapshot.py", str(SNAP_DIR / (tag + "_snap.json"))])


        # Si quieres que falle en el primer error:
        # if rc != 0: sys.exit(rc)

# ---------- Reportes PDF por instancia ----------
def make_reports():
    import pandas as pd
    import matplotlib.pyplot as plt
    from matplotlib.backends.backend_pdf import PdfPages

    df = pd.read_csv(CSV_OUT)
    # Normaliza columna instancia (por si viene con rutas)
    df["instancia_name"] = df["instancia"].apply(lambda s: Path(str(s)).name)

    for inst_name, gdf in df.groupby("instancia_name"):
        pdf_path = PDF_DIR / f"{inst_name.replace('.txt','')}.pdf"
        with PdfPages(pdf_path) as pdf:
            # Página 1: resumen tabla top por best_cost
            top = gdf.sort_values("best_cost").head(15)[
                ["tag","gen","pop","N","m","best_cost","urg_total","t_ga_s","t_sim_s","t_total_s"]
            ]
            fig = plt.figure(figsize=(11, 6))
            plt.axis("off")
            plt.title(f"Resumen {inst_name} (Top por best_cost)", fontsize=14)
            cell_text = top.values.tolist()
            cols = list(top.columns)
            table = plt.table(cellText=cell_text, colLabels=cols, loc="center")
            table.auto_set_font_size(False); table.set_fontsize(8)
            table.scale(1, 1.3)
            pdf.savefig(fig); plt.close(fig)

            # Página 2: best_cost vs generaciones (por pop)
            fig = plt.figure(figsize=(10,6))
            for pop, h in gdf.groupby("pop"):
                plt.plot(h["gen"], h["best_cost"], marker="o", linestyle="-", label=f"pop={pop}")
            plt.xlabel("Generaciones"); plt.ylabel("Best cost (max w_i * R(i))")
            plt.title(f"{inst_name} — best_cost vs generaciones")
            plt.legend(); plt.grid(True, alpha=0.3)
            pdf.savefig(fig); plt.close(fig)

            # Página 3: urg_total vs generaciones (por pop)
            fig = plt.figure(figsize=(10,6))
            for pop, h in gdf.groupby("pop"):
                plt.plot(h["gen"], h["urg_total"], marker="o", linestyle="-", label=f"pop={pop}")
            plt.xlabel("Generaciones"); plt.ylabel("Urgencia acumulada")
            plt.title(f"{inst_name} — urg_total vs generaciones")
            plt.legend(); plt.grid(True, alpha=0.3)
            pdf.savefig(fig); plt.close(fig)

            # Página 4: scatter best_cost vs urg_total (color por pop)
            fig = plt.figure(figsize=(10,6))
            for pop, h in gdf.groupby("pop"):
                plt.scatter(h["best_cost"], h["urg_total"], label=f"pop={pop}")
            plt.xlabel("Best cost"); plt.ylabel("Urgencia acumulada")
            plt.title(f"{inst_name} — tradeoff (best_cost vs urg_total)")
            plt.legend(); plt.grid(True, alpha=0.3)
            pdf.savefig(fig); plt.close(fig)

            # Página 5: tiempos (apilado simple)
            fig = plt.figure(figsize=(10,6))
            idx = range(len(gdf))
            plt.plot(gdf["t_ga_s"].values, label="GA (s)")
            plt.plot(gdf["t_sim_s"].values, label="SIM (s)")
            plt.plot(gdf["t_total_s"].values, label="TOTAL (s)")
            plt.title(f"{inst_name} — tiempos"); plt.xlabel("run #"); plt.ylabel("segundos")
            plt.legend(); plt.grid(True, alpha=0.3)
            pdf.savefig(fig); plt.close(fig)

        print(f"[OK] PDF -> {pdf_path}")

if __name__ == "__main__":
    # 1) Corre barrido
    sweep()
    # 2) Genera reportes PDF
    try:
        make_reports()
    except Exception as e:
        print("[WARN] No pude generar PDFs (matplotlib/pandas faltan?):", e)
# Fin sweep.py