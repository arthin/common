FREERTOS=/home/arthin/work/arm/freertos/code/trunk/FreeRTOS
mkdir -p FreeRTOS/include
cp $FREERTOS/Source/include/* FreeRTOS/include
cp $FREERTOS/Source/* FreeRTOS
mkdir -p  FreeRTOS/portable/GCC/ARM_CM4F
cp $FREERTOS/Source/portable/GCC/ARM_CM4F/* FreeRTOS/portable/GCC/ARM_CM4F
mkdir -p  FreeRTOS/portable/MemMang
cp $FREERTOS/Source/portable/MemMang/* FreeRTOS/portable/MemMang

#/Source/portable/GCC/ARM_CM4F
