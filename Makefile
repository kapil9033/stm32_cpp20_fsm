BUILD_DIR ?= build

.PHONY: clean

clean:
	cmake --build "$(BUILD_DIR)" --target clean
