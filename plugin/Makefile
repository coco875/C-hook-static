HOST_GCC=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES= plugin1.c
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti -O2 -Wl,--export-all-symbols

plugin.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@
