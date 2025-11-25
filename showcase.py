# showcase.py
import argparse
import sys
from pathlib import Path
import json
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from matplotlib.animation import FuncAnimation, PillowWriter

# ----------------- utilidades -----------------
def find_latest_snap(dir_path: Path) -> Path | None:
    snaps = sorted(dir_path.glob("*_snap.json"), key=lambda p: p.stat().st_mtime, reverse=True)
    return snaps[0] if snaps else None

def load_snap(p: Path) -> dict:
    with open(p, "r", encoding="utf-8") as f:
        return json.load(f)

def base_axes(rows, cols, title):
    fig, ax = plt.subplots(figsize=(7,7))
    ax.set_aspect('equal')
    ax.set_xlim(-0.5, cols-0.5); ax.set_ylim(rows-0.5, -0.5)
    ax.set_xticks(range(cols)); ax.set_yticks(range(rows))
    ax.set_xlabel("c"); ax.set_ylabel("r"); ax.set_title(title)
    return fig, ax

def draw_obstacles(ax, obstacles):
    for (r,c) in obstacles:
        ax.add_patch(Rectangle((c-0.5, r-0.5), 1, 1, color='black', alpha=0.35, lw=0.6))

def fig_sites(d):
    fig, ax = base_axes(d["rows"], d["cols"], "Sitios de urgencia y obstáculos")
    draw_obstacles(ax, d.get("obstacles", []))
    sites = d.get("sites", [])
    if sites:
        w = [s["w"] for s in sites]; wmin, wmax = min(w), max(w)
        # evita división por cero si todos los w son iguales
        denom = (wmax - wmin) if (wmax - wmin) > 1e-9 else 1.0
        sizes = [30 + 170*(wi - wmin)/denom for wi in w]
        xs = [s["c"] for s in sites]; ys = [s["r"] for s in sites]
        ax.scatter(xs, ys, s=sizes, alpha=0.7, edgecolors='k', linewidths=0.3)
    fig.tight_layout(); return fig

def fig_kernel(d):
    fig, ax = base_axes(d["rows"], d["cols"], "Kernel evolutivo (GA)")
    draw_obstacles(ax, d.get("obstacles", []))
    k = d.get("kernel", [])
    if len(k) >= 2:
        xs = [c for r,c in k]; ys = [r for r,c in k]
        ax.plot(xs, ys, lw=1.4)
    fig.tight_layout(); return fig

def fig_routes(d):
    fig, ax = base_axes(d["rows"], d["cols"], "Rutas de drones (tick a tick)")
    draw_obstacles(ax, d.get("obstacles", []))
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    for di, route in enumerate(d.get("routes", [])):
        if len(route) >= 2:
            xs = [p[1] for p in route]; ys = [p[0] for p in route]
            ax.plot(xs, ys, lw=1.6, color=colors[di % len(colors)], label=f"d{di}")
    if d.get("routes"):
        ax.legend(loc="upper right", fontsize=8)
    fig.tight_layout(); return fig

def visit_heat(rows, cols, routes):
    H = np.zeros((rows, cols), dtype=np.float32)
    for route in routes:
        for (r,c) in route:
            if 0 <= r < rows and 0 <= c < cols:
                H[r,c] += 1
    return H

def fig_heat(d, norm=False):
    rows, cols = d["rows"], d["cols"]
    H = visit_heat(rows, cols, d.get("routes", []))
    ttl = "Heatmap de visitas"
    if norm:
        k = d.get("max_drones", len(d.get("routes", [])))
        T = max((len(r) for r in d.get("routes", [])), default=1)
        denom = max(1, k*T)
        H = H/denom
        ttl = "Heatmap normalizado (visitas/(k·T))"
    # NaN para obstáculo -> celda en blanco
    for (r,c) in d.get("obstacles", []):
        if 0 <= r < rows and 0 <= c < cols:
            H[r,c] = np.nan
    fig, ax = base_axes(rows, cols, ttl)
    im = ax.imshow(H, origin='upper', interpolation='nearest')
    fig.colorbar(im, ax=ax); fig.tight_layout(); return fig

def animate_ticks(d, fps: float, outname: str):
    routes = d.get("routes", [])
    T = max((len(r) for r in routes), default=0)
    if T == 0 or not routes:
        print("Snapshot sin 'routes', no se puede animar.")
        return None
    fig, ax = base_axes(d["rows"], d["cols"], "Animación de rutas (ticks)")
    draw_obstacles(ax, d.get("obstacles", []))
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    lines, heads = [], []
    for di, route in enumerate(routes):
        col = colors[di % len(colors)]
        (ln,) = ax.plot([], [], lw=1.8, color=col, alpha=0.95)
        hd = ax.scatter([], [], s=45, marker='s', color=col, zorder=5)
        lines.append(ln); heads.append(hd)

    def init():
        for ln in lines: ln.set_data([], [])
        for hd in heads: hd.set_offsets(np.empty((0, 2)))
        return [*lines, *heads]

    def update(t):
        for i, route in enumerate(routes):
            xs = [p[1] for p in route[:t+1]]
            ys = [p[0] for p in route[:t+1]]
            lines[i].set_data(xs, ys)
            if t < len(route):
                heads[i].set_offsets([[route[t][1], route[t][0]]])
            else:
                heads[i].set_offsets(np.empty((0, 2)))
        ax.set_title(f"Animación de rutas — tick {t+1}/{T}")
        return [*lines, *heads]

    ani = FuncAnimation(fig, update, frames=T, init_func=init, blit=True, interval=int(1000//fps))
    gifs_dir = Path("resultados/gifs")
    gifs_dir.mkdir(parents=True, exist_ok=True)
    outgif = gifs_dir / f"{Path(outname).name}.gif"
    ani.save(outgif, writer=PillowWriter(fps=fps), dpi=150)
    print("Gif guardado:", outgif)

    # Lo siguiente es para pasar el gif a sus frames, para poner la animación en el informe.
    # ----------------------------------------------------------
    #  OPCIÓN EXTRA: Exportar frames PNG de ALTA calidad
    # ----------------------------------------------------------
    frames_dir = Path("resultados/gifs/frames")
    frames_dir.mkdir(parents=True, exist_ok=True)

    for t in range(T):
        fig_f, ax_f = base_axes(d["rows"], d["cols"], f"Tick {t+1}/{T}")
        draw_obstacles(ax_f, d.get("obstacles", []))

        for di, route in enumerate(routes):
            xs = [p[1] for p in route[:t+1]]
            ys = [p[0] for p in route[:t+1]]
            color = colors[di % len(colors)]
            ax_f.plot(xs, ys, lw=2.0, color=color, alpha=0.95)
            if t < len(route):
                ax_f.scatter(route[t][1], route[t][0], s=55, marker='s', color=color, zorder=5)

        fig_f.savefig(
            frames_dir / f"pspuav_{Path(outname).stem}_frame_{t:03d}.png",
            dpi=250,
            bbox_inches="tight"
        )
        plt.close(fig_f)

    print("Frames PNG guardados en:", frames_dir)
    # ----------------------------------------------------------

    return outgif

# ----------------- main -----------------
def main():
    ap = argparse.ArgumentParser(description="Showcase PSP-UAV: detectar último snapshot y visualizar/animar")
    ap.add_argument("--dir", default="resultados/showcase", help="carpeta donde buscar *_snap.json")
    ap.add_argument("--snap", default="", help="ruta a un snapshot específico (sobrescribe --dir)")
    ap.add_argument("--animate", action="store_true", help="crear GIF de rutas por tick")
    ap.add_argument("--fps", type=float, default=4, help="FPS para el GIF")
    args = ap.parse_args()

    chosen: Path | None
    if args.snap:
        chosen = Path(args.snap)
        if not chosen.exists():
            print(f"No existe el snapshot: {chosen}")
            sys.exit(1)
    else:
        dpath = Path(args.dir)
        if not dpath.exists():
            print(f"Carpeta no existe: {dpath}")
            sys.exit(1)
        chosen = find_latest_snap(dpath)
        if not chosen:
            print(f"No se encontraron *_snap.json en {dpath}")
            sys.exit(1)

    print("Snapshot:", chosen)
    data = load_snap(chosen)

    # Mostrar figuras (se abren en una misma sesión interactiva)
    fig_sites(data); fig_kernel(data); fig_routes(data); fig_heat(data, norm=False); fig_heat(data, norm=True)
    if args.animate:
        outname = chosen.with_suffix('').name + "_live"
        animate_ticks(data, fps=args.fps, outname=outname)

    plt.show()

if __name__ == "__main__":
    main()
