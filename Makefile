CC = gcc

PNAME = niv

SRC  = $(shell find src -name "*.c")
OBJ  = $(SRC:.c=.o)
BIN = build

EXEC = $(BIN)/$(PNAME)
INCFLAGS  = -Isrc/

CCFLAGS += $(INCFLAGS)
CCFLAGS += -O0
CCFLAGS += -Wall
CCFLAGS += -pedantic

LDFLAGS  = $(INCFLAGS)
LDFLAGS += -lSDL2
LDFLAGS += -lSDL2_image

INSTALL_PATH = /usr/local/bin

all: build

run: build
	$(BIN)/$(PNAME) $*

build: $(OBJ)
	$(CC) $(CCFLAGS) -ggdb -o $(BIN)/$(PNAME) $(filter %.o,$^) $(LDFLAGS)

clean:
	rm $(BIN)/* $(OBJ)

install: build
	cp $(BIN)/$(PNAME) $(INSTALL_PATH)

%.o: %.c
	$(CC) -ggdb -o $@ -c $< $(CCFLAGS)

