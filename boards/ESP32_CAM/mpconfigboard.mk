
SDKCONFIG += boards/sdkconfig.base
SDKCONFIG += boards/sdkconfig.240mhz
SDKCONFIG += boards/$(BOARD_DIR)/sdkconfig.esp32cam

FROZEN_MANIFEST ?= $(BOARD_DIR)/manifest.py

PART_SRC = $(BOARD_DIR)/partitions.csv