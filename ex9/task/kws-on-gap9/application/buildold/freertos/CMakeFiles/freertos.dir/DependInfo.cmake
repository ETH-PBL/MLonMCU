
# Consider dependencies only in project.
set(CMAKE_DEPENDS_IN_PROJECT_ONLY OFF)

# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer_os/timer_asm.S" "/app/mlonmcu/MLonMCU/ex9/task/kws-on-gap9/application/build/freertos/CMakeFiles/freertos.dir/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer_os/timer_asm.S.obj"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/freertos_task.S" "/app/mlonmcu/MLonMCU/ex9/task/kws-on-gap9/application/build/freertos/CMakeFiles/freertos.dir/freertos_kernel/freertos_task.S.obj"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/portable/GCC/RI5CY-GAP9/port_asm.S" "/app/mlonmcu/MLonMCU/ex9/task/kws-on-gap9/application/build/freertos/CMakeFiles/freertos.dir/freertos_kernel/portable/GCC/RI5CY-GAP9/port_asm.S.obj"
  )
set(CMAKE_ASM_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_ASM
  "CONFIG_BOARD_GAP9EVK_V1_3"
  "CONFIG_BOARD_GAP9MOD_V1_0_B"
  "CONFIG_DEVICE_TREE"
  "CONFIG_DRIVER_APS256XXN"
  "CONFIG_DRIVER_APS256XXN_AS_DEFAULT"
  "CONFIG_DRIVER_CLUSTERTEAM"
  "CONFIG_DRIVER_CRC32"
  "CONFIG_DRIVER_DEFAULT_FLASH"
  "CONFIG_DRIVER_DEFAULT_RAM"
  "CONFIG_DRIVER_FLASH_API"
  "CONFIG_DRIVER_FLASH_PARTITION_V2=1"
  "CONFIG_DRIVER_MRAM"
  "CONFIG_DRIVER_MX25U51245G"
  "CONFIG_DRIVER_MX25U51245G_AS_DEFAULT"
  "CONFIG_DRIVER_POWERSUPPLYGPIO"
  "CONFIG_DRIVER_POWERSUPPLY_API"
  "CONFIG_DRIVER_RAM_API"
  "PI_LOG_DEFAULT_LEVEL=PI_LOG_NONE"
  "USE_FLASH_PARTITION_V2=1"
  "__BSP_CRC__"
  "__BSP_FLASH__"
  "__BSP_POWERSUPPLY__"
  "__BSP_RAM__"
  )

# The include file search paths:
set(CMAKE_ASM_TARGET_INCLUDE_PATH
  "/app/install/gap9-sdk/rtos/pmsis/api/timer"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer_os/include"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/portable/GCC/RI5CY-GAP9"
  "/app/install/gap9-sdk/gvsoc/gvsoc/include"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/libs/include"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/include"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/gap9/include"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/gap9/config"
  "/app/install/gap9-sdk/rtos/pmsis/tools/allocator/include"
  "/app/install/gap9-sdk/libs/baselibs/printf"
  "/app/install/gap9-sdk/rtos/pmsis/archi/chips/gap9/include"
  "/app/install/gap9-sdk/rtos/pmsis/archi/include"
  "/app/install/gap9-sdk/rtos/pmsis/archi/linker"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/gap9/pmsis/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/maestro/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/i2s/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/efuse/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/fll/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/cluster/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_core/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/fc_event/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/pad/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/gpio/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/ffc/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/perf/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/watchdog/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/i2c/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/i3c/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_datamove/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/spim/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_timeout/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_timestamp/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/dmacpy/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/pwm/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/rtc/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/xip/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_hyperbus/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_octospi/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_aes/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_aes_dual_core/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_cpi/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_csi2/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_uart/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_fifo/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/udma_sdio/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/quiddikey/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/include"
  "/app/install/gap9-sdk/rtos/pmsis/implem/rtos/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/boards/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/buffer"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/crc/crc32/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/crc/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/flash/api/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/flash/mram/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/flash/mx25u51245g/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/flash/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/powersupply/api/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/powersupply/powersupplygpio/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/powersupply/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/ram/api/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/ram/aps256xxn/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/ram/include"
  "/app/install/gap9-sdk/rtos/pmsis/bsp/include"
  "devicetree/internal/include"
  "devicetree/include"
  "devicetree/fpv2/include"
  "/app/install/gap9-sdk/rtos/pmsis/include"
  "/app/install/gap9-sdk/rtos/pmsis/tools/assert/include"
  "/app/install/gap9-sdk/rtos/pmsis/api/include/chips/gap9"
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/fll_pmu"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/pmsis/backend/include"
  "/app/install/gap9-sdk/rtos/pmsis/api/include"
  "/app/install/gap9-sdk/rtos/pmsis/api/include/pmsis/rtos"
  )

# The set of dependency files which are needed:
set(CMAKE_DEPENDS_DEPENDENCY_FILES
  "/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer_os/timer.c" "freertos/CMakeFiles/freertos.dir/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer_os/timer.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/app/install/gap9-sdk/rtos/pmsis/implem/chips/gap9/timer_os/timer.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/event.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/event.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/event.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/event_groups.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/event_groups.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/event_groups.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/list.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/list.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/list.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/portable/GCC/RI5CY-GAP9/port.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/portable/GCC/RI5CY-GAP9/port.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/portable/GCC/RI5CY-GAP9/port.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/queue.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/queue.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/queue.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/scheduler.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/scheduler.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/scheduler.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/semaphore-v2.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/semaphore-v2.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/semaphore-v2.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/stream_buffer.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/stream_buffer.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/stream_buffer.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/sync_obj-v3.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/sync_obj-v3.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/sync_obj-v3.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/task-v3.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/task-v3.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/task-v3.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/tasks.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/tasks.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/tasks.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/freertos_kernel/timers.c" "freertos/CMakeFiles/freertos.dir/freertos_kernel/timers.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/freertos_kernel/timers.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/gap9/config/FreeRTOS_util.c" "freertos/CMakeFiles/freertos.dir/vendors/gwt/gap9/config/FreeRTOS_util.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/vendors/gwt/gap9/config/FreeRTOS_util.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/gap9/src/device/freertos_it.c" "freertos/CMakeFiles/freertos.dir/vendors/gwt/gap9/src/device/freertos_it.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/vendors/gwt/gap9/src/device/freertos_it.c.obj.d"
  "/app/install/gap9-sdk/rtos/pmsis/os/freeRTOS/vendors/gwt/gap9/src/device/gap_it.c" "freertos/CMakeFiles/freertos.dir/vendors/gwt/gap9/src/device/gap_it.c.obj" "gcc" "freertos/CMakeFiles/freertos.dir/vendors/gwt/gap9/src/device/gap_it.c.obj.d"
  )

# Targets to which this target links which contain Fortran sources.
set(CMAKE_Fortran_TARGET_LINKED_INFO_FILES
  )

# Targets to which this target links which contain Fortran sources.
set(CMAKE_Fortran_TARGET_FORWARD_LINKED_INFO_FILES
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
