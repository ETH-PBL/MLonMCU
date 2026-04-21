#!/bin/bash

if [ "$1" == "-h" ] ; then
    echo "PLATFORM: gvsoc, board"
    echo "APPL: 0 (record), 1 (read .wav)"
    echo "MFCC computation: 0 (online), 1 (precomputed)"
    exit 0
fi

export PLATFORM=$1
export APPL=$2
export MFCC=$3

# export PATH=/home/osboxes/mlonmcu_libs/linux-x64/bin:$PATH 
# export LD_LIBRARY_PATH=/home/osboxes/mlonmcu_libs/linux-x64/lib64/:$LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/osboxes/mlonmcu_libs/miniconda3/lib

export GAP_RISCV_GCC_TOOLCHAIN=/app/install/gcc/gap9/

source /app/install/gap9-sdk/sourceme.sh

export WAV_FILE=/app/mlonmcu/MLonMCU/ex9/task/kws-on-gap9/left_b528edb3_nohash_0.wav

cd application/

cmake -B build
cmake --build build --target menuconfig
cmake --build build --target run # --verbose
