PORT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
NEWOS_ROOT ?= $(abspath $(PORT_DIR)/../..)
TINYGL_ROOT ?= $(PORT_DIR)/vendor/tinygl
OUT ?= build/glxgears-newos.elf

TINYGL_SRCS := \
	$(TINYGL_ROOT)/src/api.c \
	$(TINYGL_ROOT)/src/arrays.c \
	$(TINYGL_ROOT)/src/clear.c \
	$(TINYGL_ROOT)/src/clip.c \
	$(TINYGL_ROOT)/src/get.c \
	$(TINYGL_ROOT)/src/image_util.c \
	$(TINYGL_ROOT)/src/init.c \
	$(TINYGL_ROOT)/src/light.c \
	$(TINYGL_ROOT)/src/list.c \
	$(TINYGL_ROOT)/src/matrix.c \
	$(TINYGL_ROOT)/src/misc.c \
	$(TINYGL_ROOT)/src/select.c \
	$(TINYGL_ROOT)/src/specbuf.c \
	$(TINYGL_ROOT)/src/texture.c \
	$(TINYGL_ROOT)/src/vertex.c \
	$(TINYGL_ROOT)/src/zbuffer.c \
	$(TINYGL_ROOT)/src/zline.c \
	$(TINYGL_ROOT)/src/zmath.c \
	$(TINYGL_ROOT)/src/zpostprocess.c \
	$(TINYGL_ROOT)/src/zraster.c \
	$(TINYGL_ROOT)/src/ztext.c \
	$(TINYGL_ROOT)/src/ztriangle.c

.PHONY: all clean check-inputs
all: check-inputs $(OUT)

check-inputs:
	@test -x "$(NEWOS_ROOT)/toolchain/newos-gcc" || { echo "missing NEWOS_ROOT/toolchain/newos-gcc; build the NEWOS toolchain first" >&2; exit 1; }
	@test -f "$(TINYGL_ROOT)/include/TGL/gl.h" || { echo "TinyGL submodule is missing; initialize this repository's vendor/tinygl submodule" >&2; exit 1; }

$(OUT): src/glxgears_newos.c src/allocator.c $(TINYGL_SRCS)
	@mkdir -p $(dir $@)
	NEWOS_FPU=1 "$(NEWOS_ROOT)/toolchain/newos-gcc" \
		-DNO_DEBUG_OUTPUT -DTGL_FEATURE_RENDER_BITS=32 \
		-I"$(TINYGL_ROOT)/include" -I"$(TINYGL_ROOT)/src" \
		-I"$(NEWOS_ROOT)/user/lib" \
		-o "$@" src/glxgears_newos.c src/allocator.c $(TINYGL_SRCS) -lm

clean:
	rm -rf build
