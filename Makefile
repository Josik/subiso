CC          := gcc
UCW_CFLAGS  := $(shell pkg-config --cflags libucw)
UCW_LFLAGS  := $(shell pkg-config --libs libucw)
CFLAGS      := -std=gnu99 -c -MMD -MP $(UCW_CFLAGS) -Wno-implicit-function-declaration -O3
LFLAGS      := -std=gnu99 $(UCW_LFLAGS)
SOURCEDIR   := src
BUILDDIR    := build
C_FILES     := $(wildcard $(SOURCEDIR)/*.c)
OBJ_FILES   := $(addprefix $(BUILDDIR)/,$(notdir $(C_FILES:.c=.o)))
DEP_FILES   := $(addprefix $(BUILDDIR)/,$(notdir $(C_FILES:.c=.d)))
BIN_NAME    := grs

.PHONY: clean debug

$(BIN_NAME) : $(OBJ_FILES)
	$(CC) $(LFLAGS) $(OBJ_FILES) -o $(BIN_NAME)

$(BUILDDIR)/%.o : $(SOURCEDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

tests: CFLAGS += -DTESTING
tests: $(BIN_NAME)

debug: CFLAGS += -DLOCAL_DEBUG -g
debug: $(BIN_NAME)

clean:
	rm -f $(OBJ_FILES) $(DEP_FILES) $(BIN_NAME)

-include $(DEP_FILES)
