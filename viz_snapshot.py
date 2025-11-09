#!/usr/bin/env python3
import json, math
from pathlib import Path
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import numpy as np
def load_snapshot(path):
    with open(path, "r") as f:
        return json.load(f)

# ---------- Dibujo helpers ----------
def draw_grid(ax, rows, cols, obstacles):
    ax.set_aspect('equal')
    ax.set_xlim(-0.5, cols-0.5); ax.set_ylim(rows-0.5, -0.5)
    # celdas
    for r in range(rows):
        for c in range(cols):
            rect = Rectangle((c-0.5, r-0.5), 1, 1, fill=False, linewidth=0.2)
            ax.add_patch(rect)
    # obstáculos
    for r, c in obstacles:
        rect = Rectangle((c-0.5, r-0.5), 1, 1, color='black')
        ax.add_patch(rect)
    ax.set_xticks(range(cols)); ax.set_yticks(range(rows))
    ax.grid(False); ax.set_xlabel("c"); ax.set_ylabel("r")

def plot_sites(ax, sites):
    # tamaño ∝ w
    w = [s["w"] for s in sites]
    wmin, wmax = (min(w), max(w)) if w else (0,1)
    sizes = [40 + 160*(wi - wmin)/(wmax - wmin + 1e-9) for wi in w]
    xs = [s["c"] for s in sites]
    ys = [s["r"] for s in sites]
    ax.scatter(xs, ys, s=sizes, alpha=0.6, edgecolors='k', linewidths=0.3)
    # índice del sitio
    for s in sites:
        ax.text(s["c"], s["r"], str(s["idx"]), ha='center', va='center', fontsize=6, color='k')

def plot_kernel(ax, kernel):
    if len(kernel) < 2: return
    xs = [c for r,c in kernel]
    ys = [r for r,c in kernel]
    ax.plot(xs, ys, linewidth=1.0, alpha=0.7)

def plot_routes(ax, routes, max_drones_to_show=5):
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    D = min(len(routes), max_drones_to_show)
    for d in range(D):
        route = routes[d]
        xs = [c for r,c in route]
        ys = [r for r,c in route]
        ax.plot(xs, ys, linewidth=1.8, alpha=0.9, label=f"d{d}", color=colors[d % len(colors)])
        # marca inicio
        if route:
            ax.scatter([route[0][1]],[route[0][0]], s=40, marker='s', color=colors[d % len(colors)])

def plot_grid(ax, rows, cols):
    ax.set_aspect('equal')
    ax.set_xlim(-0.5, cols-0.5); ax.set_ylim(rows-0.5, -0.5)
    ax.set_xticks(range(cols)); ax.set_yticks(range(rows))
    ax.grid(False); ax.set_xlabel("c"); ax.set_ylabel("r")

def draw_obstacles(ax, obstacles):
    from matplotlib.patches import Rectangle
    for r, c in obstacles:
        ax.add_patch(Rectangle((c-0.5, r-0.5), 1, 1, color='black', alpha=0.9))

def draw_sites(ax, sites):
    if not sites: return
    w = [s["w"] for s in sites]
    wmin, wmax = (min(w), max(w)) if w else (0, 1)
    sizes = [30 + 170*(wi - wmin)/(wmax - wmin + 1e-9) for wi in w]
    xs = [s["c"] for s in sites]
    ys = [s["r"] for s in sites]
    ax.scatter(xs, ys, s=sizes, alpha=0.55, edgecolors='k', linewidths=0.3, label="sites (w)")
    # opcional: índices
    # for s in sites: ax.text(s["c"], s["r"], str(s["idx"]), ha='center', va='center', fontsize=6)

def draw_kernel(ax, kernel):
    if len(kernel) < 2: return
    xs = [c for r, c in kernel]
    ys = [r for r, c in kernel]
    ax.plot(xs, ys, linewidth=1.2, alpha=0.85, label="kernel")

def draw_routes(ax, routes, max_drones_to_show=5):
    import matplotlib.pyplot as plt
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    D = min(len(routes), max_drones_to_show)
    for d in range(D):
        route = routes[d]
        if not route: continue
        xs = [p[1] for p in route]
        ys = [p[0] for p in route]
        ax.plot(xs, ys, linewidth=1.8, alpha=0.9, label=f"d{d}", color=colors[d % len(colors)])
        ax.scatter([xs[0]], [ys[0]], s=36, marker='s', color=colors[d % len(colors)])

def compute_visit_heat(rows, cols, routes):
    """Matriz H[r,c] = # de visitas de cualquier dron a la celda (r,c)."""
    H = np.zeros((rows, cols), dtype=np.int32)
    for route in routes:
        for (r, c) in route:
            if 0 <= r < rows and 0 <= c < cols:
                H[r, c] += 1
    return H

def mask_obstacles(M, obstacles):
    """Devuelve una copia float con NaN en obstáculos para que se vean ‘vacíos’."""
    X = M.astype(float).copy()
    for (r, c) in obstacles:
        if 0 <= r < X.shape[0] and 0 <= c < X.shape[1]:
            X[r, c] = np.nan
    return X


# ---------- Main function ----------

def save_all_figures(snap_path):
    with open(snap_path, "r") as f:
        data = json.load(f)

    # Figura 1: sites
    fig, ax = plt.subplots()
    for (r,c,_) in data["urgencies"]:
        ax.scatter(c, r, c='orange', s=20)
    for (r,c) in data["obstacles"]:
        ax.scatter(c, r, c='black', s=10)
    ax.set_title("Sitios de urgencia y obstáculos")
    ax.invert_yaxis()
    basepath = Path(snap_path).with_suffix('')  # quita .json
    fig.savefig(basepath.parent / f"{basepath.name}_sites.png", dpi=180)
    plt.close(fig)

    # Figura 2: kernel
    fig, ax = plt.subplots()
    for (r,c) in data["kernel"]:
        ax.scatter(c, r, c='blue', s=10)
    ax.set_title("Kernel de cobertura")
    ax.invert_yaxis()
    fig.savefig(basepath.parent / f"{basepath.name}_kernel.png", dpi=180)
    plt.close(fig)

    # Figura 3: rutas
    fig, ax = plt.subplots()
    colors = ["red","green","blue","purple","orange","cyan"]
    for i, route in enumerate(data["routes"]):
        if not route: continue
        xs = [p[1] for p in route]
        ys = [p[0] for p in route]
        ax.plot(xs, ys, color=colors[i % len(colors)], lw=1)
    ax.set_title("Rutas de drones (tick a tick)")
    ax.invert_yaxis()
    fig.savefig(basepath.parent / f"{basepath.name}_routes.png", dpi=180)
    plt.close(fig)

    # Fig 4: GRID combinado (obstáculos + sitios + kernel + rutas)
    rows = data.get("rows", 0)
    cols = data.get("cols", 0)
    obstacles = data.get("obstacles", [])
    sites = data.get("sites", [])
    kernel = data.get("kernel", [])
    routes = data.get("routes", [])
    k = data.get("max_drones", len(routes))
    T = data.get("T", 0)

    fig, ax = plt.subplots(figsize=(8,8))
    plot_grid(ax, rows, cols)
    draw_obstacles(ax, obstacles)
    draw_sites(ax, sites)
    draw_kernel(ax, kernel)
    draw_routes(ax, routes, max_drones_to_show=min(6, len(routes)))
    ax.legend(loc='upper right', fontsize=8)
    ax.set_title(f"GRID combinado (k={k}, T={T})")
    fig.tight_layout()
    basepath = Path(snap_path).with_suffix('')
    fig.savefig(basepath.parent / f"{basepath.name}_grid.png", dpi=180)
    plt.close(fig)

    # Fig 5: Heatmap de visitas
    H = compute_visit_heat(rows, cols, routes)
    H_mask = mask_obstacles(H, obstacles)

    fig, ax = plt.subplots(figsize=(8, 8))
    im = ax.imshow(H_mask, origin='upper', interpolation='nearest')
    plt.colorbar(im, ax=ax, shrink=0.85, label="Visitas")
    ax.set_title(f"Heatmap visitas (k={k}, T={T})")
    ax.set_xlabel("c"); ax.set_ylabel("r")
    # grid fino para referencia
    ax.set_xticks(range(cols)); ax.set_yticks(range(rows))
    ax.set_xticklabels(range(cols)); ax.set_yticklabels(range(rows))
    ax.set_aspect('equal')
    basepath = Path(snap_path).with_suffix('')
    fig.tight_layout()
    fig.savefig(basepath.parent / f"{basepath.name}_heat.png", dpi=180)
    plt.close(fig)

    # (Opcional) versión normalizada: visitas / (k*T)  -> frecuencia [0,1]
    if k and T:
        Hn = H.astype(float) / float(k*T)
        Hn_mask = mask_obstacles(Hn, obstacles)
        fig, ax = plt.subplots(figsize=(8, 8))
        im = ax.imshow(Hn_mask, origin='upper', interpolation='nearest', vmin=0.0, vmax=1.0)
        plt.colorbar(im, ax=ax, shrink=0.85, label="Frecuencia (visitas/(k·T))")
        ax.set_title(f"Heatmap normalizado (k={k}, T={T})")
        ax.set_xlabel("c"); ax.set_ylabel("r")
        ax.set_xticks(range(cols)); ax.set_yticks(range(rows))
        ax.set_xticklabels(range(cols)); ax.set_yticklabels(range(rows))
        ax.set_aspect('equal')
        fig.tight_layout()
        fig.savefig(basepath.parent / f"{basepath.name}_heat_norm.png", dpi=180)
        plt.close(fig)

    print(f"Guardadas figuras en {basepath.parent}")

if __name__ == "__main__":
    import sys
    if len(sys.argv)<2:
        print("uso: python viz_snapshot.py salida_01a.json")
        sys.exit(1)
    save_all_figures(sys.argv[1])
    plt.close('all')

