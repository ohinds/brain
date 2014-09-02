CC = g++

CFLAGS = -Wall \
         -Wno-reorder \
         -Wwrite-strings \
         -Wno-strict-aliasing \
         -Wno-unknown-pragmas \
         -std=c++11

CDEBUG = -g

COPTIM =

CINC = -I/usr/include/yaml-cpp

CLIB = \
       -lncurses \
       -lasound \
       -lsndfile \
       -lrtmidi \
       -lyaml-cpp \


OBJ_DIR = obj

.DELETE_ON_ERROR:

.SUFFIXES:
.SUFFIXES: .o .cpp

$(OBJ_DIR)/%.o: %.cpp %.h
	echo '[make: building $@]'
	$(CC) $(CFLAGS) $(CDEBUG) $(COPTIM) $(CINC) -o $@ -c $<

HDR_FILES = $(wildcard *.h)
ALL_SRC_FILES = $(wildcard ./*.cpp)
SRC_FILES = $(filter-out ./main_brain.cpp, $(ALL_SRC_FILES))
TMP_FILES = $(patsubst ./%,$(OBJ_DIR)/%,$(SRC_FILES))
OBJ_FILES = $(TMP_FILES:.cpp=.o)

default: prep $(OBJ_FILES) brain

prep:
	mkdir -p $(OBJ_DIR)

brain: $(OBJ_FILES) main_brain.cpp
	$(CC) $(CFLAGS) $(CDEBUG) $(COPTIM) $(CINC) -o $@ main_$@.cpp $(OBJ_FILES) $(CLIB)

clean:
	-rm $(OBJ_FILES)
	-rm brain
