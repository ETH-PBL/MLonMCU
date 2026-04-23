cmake_minimum_required(VERSION 3.19.5)

# Application

set(TARGET_NAME         "application")
set(MAIN                "application.c")

# Cmake pre-inits

set(CONFIG_GAP_SDK_HOME $ENV{GAP_SDK_HOME})
include ($ENV{GAP_SDK_HOME}/utils/cmake/setup.cmake)
set(BOARD_NAME gap9_evk)
set(PMSIS_OS freertos)
set(platform $ENV{PLATFORM})

# kconfig options
set (io   host)
set (DEMO 1)

set (FLASH_TYPE DEFAULT)
set (RAM_TYPE   DEFAULT)
set (MODEL_L3_FLASH   AT_MEM_L3_DEFAULTFLASH)
set (EXEC_FROM_FLASH true)
set (MODEL_L3_RAM   AT_MEM_L3_DEFAULTRAM)

set (FREQ_CL 50)
set (FREQ_FC 50)
set (FREQ_SFU 50)
set (VOLTAGE 650)
set (CLUSTER_STACK_SIZE 8192)
set (CLUSTER_SLAVE_STACK_SIZE 4096)
set (TOTAL_STACK_SIZE $(shell expr $(CLUSTER_STACK_SIZE) \+ $(CLUSTER_SLAVE_STACK_SIZE) \* $(CLUSTER_NUM_CORES)))
set (MODEL_L1_MEMORY $(shell expr 120000 \- $(TOTAL_STACK_SIZE)))
set (MODEL_L2_MEMORY 1000000)
set (MODEL_L3_MEMORY 8000000)

set (WAV_FILE $ENV{WAV_FILE})
set (MFCC $ENV{MFCC})
set (APPL $ENV{APPL})


set(GAP9_SDK_DIR         $ENV{GAP_SDK_HOME})
set(BUILD_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(DSP_DIR              ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Librariesv2/)
set(DSP_LUT_DIR          ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Librariesv2/TransformFunctions/LUT_Tables/)
set(EMUL_DIR             ${GAP9_SDK_DIR}/tools/autotiler_v3/Emulation/)
set(AUTOTILER_DIR        ${GAP9_SDK_DIR}/tools/autotiler_v3/Autotiler/)
set(CNNGEN_DIR           ${GAP9_SDK_DIR}/tools/autotiler_v3/CNN_Generators/) # MFCC v1
set(AUTOTILER_MFCC_DIR   ${GAP9_SDK_DIR}/tools/autotiler_v3/Generators/MFCC/)
set(WAVIO_DIR            ${GAP9_SDK_DIR}/libs/gap_lib/include/)
set(DSP_GEN_DIR          ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/)
set(MFCC_SRCG            ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/DSP_Generators.c)
set(MFCC_SRCGv2          ${GAP9_SDK_DIR}/tools/autotiler_v3/DSP_Generators/DSP_Generatorsv2.c)
set(MFCC_MODEL_GEN       ${BUILD_DIR}/GenMFCC)
set(MFCC_HEAD            ${BUILD_DIR}/MFCC_params.h)
set(MFCC_PARAMS_JSON     ${CMAKE_CURRENT_SOURCE_DIR}/MfccConfig.json)
set(MFCC_SRC_CODE        ${BUILD_DIR}/MfccKernels.c)

# Application
add_custom_command (
    OUTPUT    ${MFCC_MODEL_GEN}
    COMMAND   python ${DSP_GEN_DIR}/DSP_LUTGen.py ${MFCC_PARAMS_JSON} --build_dir ${BUILD_DIR} --save_params_header ${MFCC_HEAD} --save_text
)

add_custom_command (
    OUTPUT    ${MFCC_SRC_CODE}
    COMMAND   ${MFCC_MODEL_GEN} -o ${BUILD_DIR} -c ${BUILD_DIR} ${MODEL_GEN_EXTRA_FLAGS}
    DEPENDS   ${MFCC_MODEL_GEN}
)

add_custom_target (
    MFCC_HEAD
    WORKING_DIRECTORY    ${BUILD_DIR}
    COMMAND              python ${DSP_GEN_DIR}/DSP_LUTGen.py ${MFCC_PARAMS_JSON} --build_dir ${BUILD_DIR} --save_params_header ${MFCC_HEAD} --save_text
    SOURCES              ${MFCC_PARAMS_JSON}
    BYPRODUCTS           ${MFCC_HEAD}
)


add_custom_target (
    MFCC_MODEL_GEN
    WORKING_DIRECTORY    ${BUILD_DIR}
    COMMAND              gcc -g -o ${MFCC_MODEL_GEN} -I${BUILD_DIR} -I${DSP_GEN_DIR} -I${TILER_INC} -I${CNNGEN_DIR} -I${TILER_EMU_INC} ${CMAKE_CURRENT_SOURCE_DIR}/MfccModel.c ${MFCC_SRCGv2}  ${TILER_LIB} ${TABLE_CFLAGS} ${COMPILE_MODEL_EXTRA_FLAGS} -DUSE_POWER=${USE_POWER} ${SDL_FLAGS}
    BYPRODUCTS           ${MFCC_MODEL_GEN}   
    DEPENDS              ${MFCC_HEAD}         
)


# add_custom_target (
#     graph
#     WORKING_DIRECTORY   ${BUILD_DIR}
#     COMMAND             SFU -i ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src -C
#     SOURCES             ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src
#     BYPRODUCTS          ${BUILD_DIR}/Graph_L2_Descr.c ${BUILD_DIR}/Graph_L2_Descr.h ${BUILD_DIR}/L2_Descr_Table.c ${BUILD_DIR}/L2_Descr_Table.h
# )

add_custom_target (
    graph
    WORKING_DIRECTORY   ${BUILD_DIR}
    COMMAND             SFU -i ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src -C
    SOURCES             ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src
    BYPRODUCTS          ${BUILD_DIR}/Graph_L2_Descr.c ${BUILD_DIR}/Graph_L2_Descr.h ${BUILD_DIR}/Graph.c ${BUILD_DIR}/Graph.h
)

# add_custom_command(
#     OUTPUT "${BUILD_DIR}/Graph_L2_Descr.c" 
#            "${BUILD_DIR}/L2_Descr_Table.c"
#            "${BUILD_DIR}/Graph_L2_Descr.h"
#     COMMAND SFU -i "${CMAKE_CURRENT_SOURCE_DIR}/Graph.src" -C
#     DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Graph.src"
#     WORKING_DIRECTORY "${BUILD_DIR}"
#     COMMENT "Running SFU to generate Graph descriptors..."
#     VERBATIM
# )

list (APPEND TARGET_SRCS 
                         ${DSP_DIR}TransformFunctions/FftLibraryf32.c
                         ${DSP_DIR}TransformFunctions/FftLibraryf16.c 
                         ${DSP_DIR}ComplexMathFunctions/CmplxFunctionsf32.c
                         ${DSP_DIR}ComplexMathFunctions/CmplxFunctionsf16.c
                         ${DSP_DIR}FastMathFunctions/PiecewiseMathf16.c
                         ${DSP_DIR}MatrixFunctions/MatVectDSP.c
                         ${DSP_DIR}WindowFunctions/PreProcessingf16.c
                         ${DSP_DIR}TransformFunctions/MfccLibraryf32.c
                         ${DSP_DIR}TransformFunctions/MfccLibraryf16.c
                         ${BUILD_DIR}/MfccKernels.c 
                         ${GAP9_SDK_DIR}/libs/gap_lib/wav_io/wavIO.c
                         ${MAIN}
                         dac.c
                         ${BUILD_DIR}/Graph_L2_Descr.c
                         ${BUILD_DIR}/Graph.c
                         ${GAP9_SDK_DIR}/rtos/sfu/SFU_RT.c
                         ${GAP9_SDK_DIR}/rtos/sfu/sfu_pmsis_runtime.c
                         localutil.c
)

# add_executable (${TARGET_NAME} ${TARGET_SRCS})

list (APPEND TARGET_INCS 
                         ${TILER_INC}
                         ${TILER_EMU_INC}
                         ${GAP_SDK_HOME}/libs/gap_lib/include
                         ${CMAKE_CURRENT_SOURCE_DIR}/generate/inc
                         ${GAP9_SDK_DIR}/rtos/pmsis/bsp/include/bsp
                         ${GAP9_SDK_DIR}/rtos/pmsis/bsp/ram/api/include
                         ${GAP9_SDK_DIR}/rtos/pmsis/bsp/flash/api/include
                         ${GAP9_SDK_DIR}/rtos/pmsis/bsp/flash/hyperflash/include
                         ${GAP9_SDK_DIR}/rtos/pmsis/bsp/include/bsp/fs
                         ${GAP9_SDK_DIR}/rtos/sfu/include
                         ${DSP_DIR}
                         ${DSP_DIR}/FastMathFunctions
                         ${AUTOTILER_DIR}/CNN_Generators # MFCC v1
                         ${DSP_LUT_DIR}
                         ${AUTOTILER_DIR}
                         ${AUTOTILER_MFCC_DIR}
                         ${WAVIO_DIR}
                         ${DSP_GEN_DIR}
                         .
)


# DORY
set (NUM_CORES 8)
set (GAP_SDK 1)
set (TARGET_CHIP_FAMILY_GAP9 1)

FILE (GLOB DORY_C_Sources ${CMAKE_CURRENT_SOURCE_DIR}/generate/src/*.c)
list (APPEND TARGET_SRCS ${DORY_C_Sources})

list(APPEND TARGET_CFLAGS   -O2
                            -s
                            -w
                            -mno-memcpy
                            -fno-tree-loop-distribute-patterns
                            -Wno-incompatible-pointer-types
)

list(APPEND TARGET_PREPROCESSOR # -DPERF
                                -DNUM_CORES=${NUM_CORES}
                                -DGAP_SDK=${GAP_SDK}
                                -DFLASH_TYPE=${FLASH_TYPE}
                                -DUSE_${FLASH_TYPE}
                                -DUSE_${RAM_TYPE}
                                -DSTACK_SIZE=${CLUSTER_STACK_SIZE}
                                -DSLAVE_STACK_SIZE=${CLUSTER_SLAVE_STACK_SIZE}
                                -DFREQ_FC=${FREQ_FC}
                                -DFREQ_CL=${FREQ_CL}
                                -DFREQ_SFU=${FREQ_SFU}
                                -DVOLTAGE=${VOLTAGE}
                                -DWAV_FILE=${WAV_FILE}
                                -DMFCC=${MFCC}
                                -DAPPL=${APPL}
                                # -DPRINTDEB
                                # -DPRINT_INOUT
                                # -DCHECKSUM
                                # -DVERBOSE
                                -DSILENT
                                -DCONFIG_GAP9_EVK
                                -DFS_READ_FS
                                -DCONFIG_FLASH_LAYOUT_CUSTOM
                                -DMODEL_L2_MEMORY=${MODEL_L2_MEMORY}
                                -DTARGET_CHIP_FAMILY_GAP9
                                -DFABRIC
                                -DCLUSTER
                                # -DDEBUG
                                # -DPROF_NET
                                -mhwloopalign
                                # -DSTATS
                                -DSINGLE_CORE_DMA
)

# FLASH
FILE(GLOB DORY_WEIGHTS ${CMAKE_CURRENT_SOURCE_DIR}/generate/hex/*.hex)
list (APPEND READFS_FILES ${DORY_WEIGHTS})
list (APPEND READFS_FILES ${WAV_FILE}                        
)

project (${TARGET_NAME} C ASM)

add_executable (${TARGET_NAME} ${TARGET_SRCS})


#===============================================================================
# SFU graph generation
#===============================================================================

sfu_add_graphs(${TARGET_NAME} SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Graph.src)

# gvsoc_control_script(${CMAKE_CURRENT_SOURCE_DIR}/gvcontrol.py)

target_include_directories(${TARGET_NAME} PUBLIC ${TARGET_INCS})

target_compile_options(${TARGET_NAME} PUBLIC   ${TARGET_CFLAGS}
                                               ${TARGET_PREPROCESSOR}

)
target_link_libraries(${TARGET_NAME} PRIVATE m)
target_link_libraries(${TARGET_NAME} PRIVATE ${TILER_LIB})
list(APPEND TABLE_CFLAGS -lm)


# CMake post initialization
setupos(${TARGET_NAME})






