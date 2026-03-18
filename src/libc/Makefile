IS_LIBC := no

SOURCES_C = $(subst ./,,$(shell find -type f -name "*.c"))
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/libk/%.o, $(SOURCES_C))
DIRECTORIES = $(subst .,,$(subst ./,/,$(subst $(BUILD_DIR),,$(shell find -type d))))

TARGET_CFLAGS += -ffreestanding -fstack-protector
TARGET_CINCLUDES += -Iinclude
TARGET_CDEFINES += -D__is_libk

.PHONY: all clean scaffold libk

all: scaffold libk

scaffold:
	@mkdir -p $(BUILD_DIR)/libk
	@mkdir -p $(addprefix $(BUILD_DIR)/libk,$(DIRECTORIES))
	@$(COMPILEFLAGS_SH) $(PRECOMPILER_OPTS) $(TARGET_CFLAGS) $(TARGET_CINCLUDES) $(TARGET_CDEFINES) $(TARGET_LDFLAGS) $(TARGET_LIBS)

libk: $(BUILD_DIR)/libk.a

$(BUILD_DIR)/libk.a: $(OBJECTS_C)
	@$(TARGET_AR) rcs $@ $^
	@echo Created libk.a
	@cp $(BUILD_DIR)/libk.a $(LIBDIR)

$(BUILD_DIR)/libk/%.o: %.c
	@echo Compiling $<...
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $< $(TARGET_CINCLUDES) $(TARGET_CDEFINES)

install:
	@cp -R --preserve=timestamps include/. $(INCLUDEDIR)/.

clean:
	@rm -f $(BUILD_DIR)/libk.a

-include $(OBJECTS_C:.o=.d)