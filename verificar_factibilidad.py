import json
import sys

if len(sys.argv) != 2:
    print("Uso: python3 verificar_factibilidad.py archivo_snap.json")
    exit(1)

with open(sys.argv[1], "r", encoding="utf-8") as f:
    S = json.load(f)

R = S["rows"]
C = S["cols"]
T = S["T"]
obstacles = set(tuple(o) for o in S.get("obstacles", []))
routes = S.get("routes", [])

ok = True

for d, route in enumerate(routes):
    # 1) largo de la ruta
    if len(route) != T:
        print(f"[WARN] Dron {d}: len(route)={len(route)} distinto de T={T}")
    for t in range(len(route) - 1):
        r1, c1 = route[t]
        r2, c2 = route[t+1]

        # 2) dentro de la grilla
        if not (0 <= r2 < R and 0 <= c2 < C):
            print(f"[ERR] Dron {d} sale de la grilla en t={t+1}: ({r2},{c2})")
            ok = False

        # 3) no pisa obstáculo
        if (r2, c2) in obstacles:
            print(f"[ERR] Dron {d} pisa obstáculo en t={t+1}: ({r2},{c2})")
            ok = False

        # 4) movimiento adyacente (8 vecinos)
        if abs(r2 - r1) > 1 or abs(c2 - c1) > 1:
            print(f"[ERR] Dron {d} hace movimiento no adyacente "
                  f"t={t}->{t+1}: ({r1},{c1}) -> ({r2},{c2})")
            ok = False

if ok:
    print("La solución es FACTIBLE según las restricciones básicas.")
else:
    print("La solución tiene violaciones (ver mensajes arriba).")
