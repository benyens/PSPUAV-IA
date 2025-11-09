# ===========================================
#  PSP-UAV  | Proyecto IA-PSP 2025
# ===========================================
.PHONY: all clean deepclean runs sweep vis showcase resultsclean

# ---------- COMPILADOR ----------
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
TARGET := PSP-UAV
SRCS := PSP-UAV.cpp instance.cpp
HDRS := instance.hpp sites.hpp pvp_ga.hpp pvp_fitness.hpp pvp_chromosome.hpp simulate.hpp utils.hpp dump_json.hpp

# ---------- PYTHON ----------
PYTHON := python
SWEEP := sweep.py
VIS := vis_snapshot.py
SHOWCASE := showcase.py

# ---------- DIRECTORIOS ----------
INSTDIR := instancias
RESDIR := resultados
SHOWDIR := $(RESDIR)/showcase
PDFDIR := $(RESDIR)/pdfs
SNAPDIR := $(RESDIR)/snapshots

# ---------- COMPILAR ----------
all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	@echo "Compilando $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)
	@echo "Binario listo: ./$(TARGET)"

# ---------- LIMPIEZAS ----------
clean:
	@echo "Limpiando binarios..."
	@rm -f $(TARGET)
	@echo "Limpieza básica completa."

resultsclean:
	@echo "Limpiando solo resultados experimentales..."
	@rm -rf $(RESDIR)
	@rm -f *.csv *.json *.png *.pdf
	@echo "Resultados experimentales eliminados."

deepclean: clean
	@echo "Limpieza total..."
	@rm -rf $(RESDIR)
	@rm -f *.csv *.json *.png *.pdf
	@echo "Todo limpio (binarios + resultados + gráficos)."

# ---------- RUNS RÁPIDOS ----------
run1a: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_01_a.txt 5 1000 50 --seed=7 --gen=20 --pop=100 --outdir=$(SHOWDIR)
run1b: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_01_b.txt 5 1000 50 --outdir=$(SHOWDIR)
run2a: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_02_a.txt 10 1000 50 --outdir=$(SHOWDIR)
run2b: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_02_b.txt 10 1000 50 --outdir=$(SHOWDIR)
run3a: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_03_a.txt 15 1000 50 --outdir=$(SHOWDIR)
run3b: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_03_b.txt 15 1000 50 --outdir=$(SHOWDIR)

# ---------- SHOWCASE (corre y visualiza la última instancia) ----------
showcase: 
	@echo "Limpiando rastros de algún showcase anterior..."
	@rm -rf $(SHOWDIR)
	@rm -f *.csv *.json *.png *.pdf
	make run1a
	@echo "Ejecutando showcase y generando visualización..."
	@latest=$$(ls -t $(SHOWDIR)/*.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay snapshots en $(SHOWDIR)/. Ejecuta make run1a o cualquier otra instancia."; \
	else \
	    echo "Usando snapshot: $$latest"; \
	    $(PYTHON) $(SHOWCASE) --animate --fps 60; \
		$(PYTHON)  $(VIS) "$$latest"; \
	fi

# ---------- BARRIDO Y VISUALIZACIÓN ----------
sweep: $(TARGET) $(SWEEP)
	@echo "Corriendo barrido completo de experimentos..."
	$(PYTHON) $(SWEEP)

vis: $(VIS)
	@echo "Visualizando snapshot más reciente..."
	@latest=$$(ls -t $(SNAPDIR)/*.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay JSONs en $(SNAPDIR)/. Ejecuta primero un run con --dump o make showcase."; \
	else \
	    echo "Mostrando $$latest"; \
	    $(PYTHON) $(VIS) "$$latest"; \
	fi
