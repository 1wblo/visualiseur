# Makefile pour le visualiseur de structures de donnees.
# Necessite SFML 3 :
#   macOS   : brew install sfml
#   Linux   : sudo apt-get install libsfml-dev (verifier avec
#             pkg-config --modversion sfml-graphics que c'est bien >= 3)
#
# Trouver les bons chemins d'inclusion/liaison, dans l'ordre de
# preference :
#   1. pkg-config, s'il est installe et connait sfml-graphics.
#   2. `brew --prefix sfml` sur macOS (brew install sfml n'installe pas
#      forcement pkg-config, donc on ne peut pas compter dessus) : on
#      pointe directement vers le dossier d'installation Homebrew
#      (/opt/homebrew sur Apple Silicon, /usr/local sur Intel).
#   3. A defaut, les flags -l... "nus" (fonctionne si SFML est installe
#      dans un chemin deja connu du compilateur, ex: apt sur Linux).

OPTIONS = -O2 -Wall -std=c++17

PKGCONFIG := $(shell pkg-config --exists sfml-graphics 2>/dev/null && echo oui)
BREW_PREFIX := $(shell command -v brew >/dev/null 2>&1 && brew --prefix sfml 2>/dev/null)

ifeq ($(PKGCONFIG),oui)
    SFML_CFLAGS := $(shell pkg-config --cflags sfml-graphics sfml-window sfml-system)
    SFML_LIBS   := $(shell pkg-config --libs sfml-graphics sfml-window sfml-system)
else ifneq ($(BREW_PREFIX),)
    SFML_CFLAGS := -I$(BREW_PREFIX)/include
    SFML_LIBS   := -L$(BREW_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system
else
    SFML_CFLAGS :=
    SFML_LIBS   := -lsfml-graphics -lsfml-window -lsfml-system
endif

visualiseur: main.cpp arbreavl.h arbrerougenoir.h arbrebst.h dessinarbre.h analysegraphe.h graphe.h
	g++ $(OPTIONS) $(SFML_CFLAGS) -o visualiseur main.cpp $(SFML_LIBS)

test: test_arbres.cpp arbreavl.h arbrerougenoir.h arbrebst.h dessinarbre.h
	g++ $(OPTIONS) -o test_arbres test_arbres.cpp
	./test_arbres

clean:
	rm -f visualiseur
	rm -f test_arbres
	rm -f *.o
	rm -f *~
