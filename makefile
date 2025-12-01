# ===========================================
#  PSP-UAV  | Proyecto IA-PSP 2025
# ===========================================
.PHONY: all clean deepclean runs sweep vis showcase resultsclean dirs verify verify_showcase verify_file rundefensa_all

# ---------- COMPILADOR ----------
CXX := g++

# DEBUG=1 -> compila con -g y sin optimizar
DEBUG ?= 0
ifeq ($(DEBUG),1)
  CXXFLAGS := -std=c++17 -O0 -g -Wall -Wextra
else
  CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
endif

TARGET := PSP-UAV
SRCS := PSP-UAV.cpp instance.cpp
HDRS := instance.hpp sites.hpp pvp_ga.hpp pvp_fitness.hpp pvp_chromosome.hpp simulate.hpp utils.hpp dump_json.hpp

# ---------- PYTHON ----------
PYTHON ?= python3
SWEEP := sweep.py
VIS := vis_snapshot.py
SHOWCASE := showcase.py
VERIFIER := verificar_factibilidad.py

# ---------- DIRECTORIOS ----------
INSTDIR := instancias
RESDIR := resultados
SHOWDIR := $(RESDIR)/showcase
PDFDIR := $(RESDIR)/pdfs
SNAPDIR := $(RESDIR)/snapshots

# ---------- COMPILAR ----------
all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	@echo "Compilando $(TARGET) (DEBUG=$(DEBUG))..."
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)
	@echo "Binario listo: ./$(TARGET)"

# ---------- CREAR DIRECTORIOS DE RESULTADOS ----------
dirs:
	@mkdir -p $(RESDIR) $(SHOWDIR) $(PDFDIR) $(SNAPDIR)

# ---------- LIMPIEZAS ----------
clean:
	@echo "Limpiando binarios..."
	@rm -f $(TARGET)
	@echo "Limpieza básica completa."

resultsclean:
	@echo "Limpiando solo resultados experimentales... (menos los .pdf)"
	@rm -rf $(RESDIR)
	@rm -f *.csv *.json *.png 
	@echo "Resultados experimentales eliminados."

deepclean: clean
	@echo "Limpieza total..."
	@rm -rf $(RESDIR)
	@rm -f *.csv *.json *.png
	@echo "Todo limpio (binarios + resultados + gráficos (menos los .pdf))."

# ---------- RUNS RÁPIDOS ----------
run1a: $(TARGET) dirs
	./$(TARGET) $(INSTDIR)/PSP-UAV_01_a.txt 30 1000 50 --seed=7 --gen=1000 --pop=10 --outdir=$(SHOWDIR)

run1b: $(TARGET) dirs
	./$(TARGET) $(INSTDIR)/PSP-UAV_01_b.txt 5 1000 50 --seed=7 --gen=1000 --pop=10 --outdir=$(SHOWDIR)

run2a: $(TARGET) dirs
	./$(TARGET) $(INSTDIR)/PSP-UAV_02_a.txt 10 1000 50 --seed=7 --gen=1000 --pop=10 --outdir=$(SHOWDIR)

run2b: $(TARGET) dirs
	./$(TARGET) $(INSTDIR)/PSP-UAV_02_b.txt 10 1000 50 --seed=7 --gen=1000 --pop=10 --outdir=$(SHOWDIR)

run3a: $(TARGET) dirs
	./$(TARGET) $(INSTDIR)/PSP-UAV_03_a.txt 15 1000 50 --seed=7 --gen=1000 --pop=10 --outdir=$(SHOWDIR)
run3b: $(TARGET) dirs
	./$(TARGET) $(INSTDIR)/PSP-UAV_03_b.txt 15 1000 50 --seed=7 --gen=1000 --pop=10 --outdir=$(SHOWDIR)

# Ejecutar todas las instancias rápidas
runs: run1a run1b run2a run2b run3a run3b

# ---------- SHOWCASE (corre y visualiza la última instancia) ----------
# Run por defecto para el showcase
SHOWCASE_RUN ?= run1a

showcase: $(TARGET) dirs
	@echo "Limpiando rastros de algún showcase anterior..."
	@rm -rf $(SHOWDIR)
	@rm -f *.csv *.json *.png 
	@mkdir -p $(SHOWDIR)
	@echo "Ejecutando $(SHOWCASE_RUN) para el showcase..."
	$(MAKE) $(SHOWCASE_RUN)
	@echo "Ejecutando showcase y generando visualización..."
	@latest=$$(ls -t $(SHOWDIR)/*.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay snapshots en $(SHOWDIR)/. Ejecuta algún run primero."; \
	else \
	    echo "Usando snapshot: $$latest"; \
	    $(PYTHON) $(SHOWCASE) --animate --fps 60; \
	    $(PYTHON) $(VIS) "$$latest"; \
	fi

# ---------- BARRIDO Y VISUALIZACIÓN ----------
sweep: $(TARGET) dirs $(SWEEP)
	@echo "Corriendo barrido completo de experimentos..."
	$(PYTHON) $(SWEEP)

vis: dirs $(VIS)
	@echo "Visualizando snapshot más reciente..."
	@latest=$$(ls -t $(SNAPDIR)/*.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay JSONs en $(SNAPDIR)/. Ejecuta primero un run con --dump o make sweep."; \
	else \
	    echo "Mostrando $$latest"; \
	    $(PYTHON) $(VIS) "$$latest"; \
	fi

# ---------- VERIFICAR FACTIBILIDAD ----------
# Verifica el snapshot más reciente en resultados/snapshots
verify: $(FAC)
	@latest=$$(ls -t $(SNAPDIR)/*_snap.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay snapshots en $(SNAPDIR). Ejecuta primero make sweep o algún run con --dump."; \
	    exit 1; \
	else \
	    echo "Verificando factibilidad de: $$latest"; \
	    $(PYTHON) $(FAC) "$$latest"; \
	fi

# Verifica el snapshot más reciente del showcase
verify_showcase: $(FAC)
	@latest=$$(ls -t $(SHOWDIR)/*_snap.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay snapshots en $(SHOWDIR). Ejecuta primero make showcase."; \
	    exit 1; \
	else \
	    echo "Verificando factibilidad de (showcase): $$latest"; \
	    $(PYTHON) $(FAC) "$$latest"; \
	fi

# ---------- VERIFICAR FACTIBILIDAD SOBRE JSON ESPECÍFICO ----------
# Uso:
#   make verify_file FILE=ruta/al/snapshot.json
verify_file:
	@if [ -z "$(FILE)" ]; then \
	    echo "Uso: make verify_file FILE=archivo.json"; \
	    exit 1; \
	fi
	@echo "Verificando factibilidad de: $(FILE)"
	$(PYTHON) $(FAC) "$(FILE)"

# ---------- DEFENSA / PRUEBAS MASIVAS ----------
DEFENSA_DIR := $(RESDIR)/defensa

# TODAS las instancias de la planilla
DEF_INSTANCES := PSP-UAV_01_a.txt PSP-UAV_01_b.txt \
                PSP-UAV_02_a.txt PSP-UAV_02_b.txt \
                PSP-UAV_03_a.txt PSP-UAV_03_b.txt 

# Valores que aparecen en la planilla
DEF_DRONES := 1 3 5 8        # columna "Drones"
DEF_T      := 100 150        # columna "T"

# Interpretamos "Iteraciones" como número de generaciones
DEF_GEN := 100              # puedes agregar más: 100 150 200 ...
DEF_POP := 50               # tamaños de población que quieres probar
DEF_NRUNS := 1              # corridas por configuración (seeds 1..NRUNS)
SEED := 6769				# seed base (seeds serán 1..NRUNS)

# Corre TODAS las combinaciones de:
#   instancia ∈ DEF_INSTANCES
#   drones   ∈ DEF_DRONES
#   T        ∈ DEF_T
#   gen      ∈ DEF_GEN
#   pop      ∈ DEF_POP
# y para cada config hace DEF_NRUNS corridas con seeds 1..NRUNS
rundefensa_all: $(TARGET) dirs
	@mkdir -p $(DEFENSA_DIR)
	@echo "=== INICIO DEFENSA MASIVA ==="
	@for inst in $(DEF_INSTANCES); do \
		instpath="$(INSTDIR)/$$inst"; \
		instbase=$${inst%.txt}; \
		echo "Procesando $$instbase ..."; \
		for d in $(DEF_DRONES); do \
			for tt in $(DEF_T); do \
				for g in $(DEF_GEN); do \
					for p in $(DEF_POP); do \
						for run in $$(seq 1 $(DEF_NRUNS)); do \
							outjson="$(DEFENSA_DIR)/$${instbase}_d$${d}_T$${tt}_g$${g}_p$${p}_run$${run}.json"; \
							echo ">>> GENERANDO: $$outjson"; \
							./$(TARGET) $$instpath $$d 1000 $$tt \
								--seed=$$run --gen=$$g --pop=$$p --dump="$$outjson"; \
						done; \
					done; \
				done; \
			done; \
		done; \
	done
	@echo "=== FIN ==="
