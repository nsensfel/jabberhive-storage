################################################################################
PTHREADS_LIB_FLAGS = -pthread
MATH_LIB_FLAGS = -lm
################################################################################
CFLAGS += -O3
CFLAGS += -D_POSIX_SOURCE
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += $(PTHREADS_LIB_FLAGS)
CFLAGS += $(MATH_LIB_FLAGS)
################################################################################
EXECUTABLE = jh-lowercase
SRC_DIR = ./src
################################################################################
export
################################################################################

build: $(EXECUTABLE)

$(EXECUTABLE): $(SRC_DIR)/export.a
	$(CC) $(CFLAGS) -o $@ $<

$(SRC_DIR)/export.a:
	$(MAKE) -C $(SRC_DIR) export.a

clean:
	$(MAKE) -C $(SRC_DIR) clean
	rm -f $(EXECUTABLE)
