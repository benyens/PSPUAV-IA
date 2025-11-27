# PSP-UAV – Optimización de Patrullaje mediante Algoritmo Genético

## 1. Descripción general

Este proyecto implementa una solución al **problema PSP-UAV (Patrolling Scheduling Problem for Unmanned Aerial Vehicles)**, cuyo objetivo es **maximizar la cobertura de celdas con distintos niveles de urgencia** utilizando un número limitado de drones en un entorno discretizado con obstáculos.

El enfoque implementado corresponde a un **algoritmo evolutivo (genético)** basado en la metodología descrita en el paper *"Multi-Mission Learning Workflow with Genetic Algorithms (MMLW-GA)"*.  
Este algoritmo busca mejorar rutas de patrullaje mediante procesos iterativos de selección, cruce y mutación de soluciones candidatas, balanceando exploración y explotación en el espacio de búsqueda.

---

## 2. Estructura del proyecto

```
PSP-UAV/
│
├── PSP-UAV.cpp
├── instance.cpp / .hpp
├── pvp_ga.hpp
├── pvp_chromosome.hpp
├── pvp_fitness.hpp
├── simulate.hpp
├── dump_json.hpp
├── sweep.py
├── showcase.py
├── vis_snapshot.py
├── instancias/
├── informe/
├── resultados/
│   ├── showcase/
│   ├── gifs/
│   ├── pdfs/
│   └── snapshots/
└── Makefile
```

---

## 3. Compilación

```bash
make
```

Genera:

```
./PSP-UAV
```

---

## 4. Ejecución básica

```bash
./PSP-UAV instancias/PSP-UAV_01_a.txt 5 1000 50
```

---

## 5. Ejemplos con Makefile

```bash
make run1a # Correr una instancia de ejemplo basada en la 01_a
make all # Compila todo
make sweep # Hace un barrido de todos los experimentos
make vis # Crea las visualizaciones los experimentos 
make showcase # Permite crear una simulación de una <run>, con SHOWCASE_RUN = <run> se elige alguna de las runs de ejemplo
```

---

## 6. Visualización de resultados

```bash
python3 showcase.py --animate --fps 60
```

---

## 7. Metodología del algoritmo genético

Resumen:

- inicialización
- evaluación de aptitud
- selección
- cruce
- mutación
- reemplazo
- simulación final

---

## 8. Referencia bibliográfica

Basado en:

> **Multi-Mission Learning Workflow with Genetic Algorithms (MMLW-GA)**,  
> Applied Intelligence Journal, 2023.

---

## 9. Observaciones

- modular y reproducible
- requiere `matplotlib`, `numpy`, `Pillow`
- animaciones no afectan resultados
- `--outdir` separa resultados
