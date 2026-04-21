FLASH_FILES += hex/BNReluConvolution0_weights.hex
FLASH_FILES += hex/BNReluConvolution1_weights.hex
FLASH_FILES += hex/BNReluConvolution2_weights.hex
FLASH_FILES += hex/BNReluConvolution3_weights.hex
FLASH_FILES += hex/BNReluConvolution4_weights.hex
FLASH_FILES += hex/BNReluConvolution5_weights.hex
FLASH_FILES += hex/BNReluConvolution6_weights.hex
FLASH_FILES += hex/BNReluConvolution7_weights.hex
FLASH_FILES += hex/BNReluConvolution8_weights.hex
FLASH_FILES += hex/FullyConnected10_weights.hex
FLASH_FILES += hex/inputs.hex

READFS_FILES := $(FLASH_FILES)
APP_CFLAGS += -DFS_READ_FS
#PLPBRIDGE_FLAGS += -f