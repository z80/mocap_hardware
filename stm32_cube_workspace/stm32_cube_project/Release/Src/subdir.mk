################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/bmi085_io.c \
../Src/bno055.c \
../Src/bno055_io.c \
../Src/magdwick_imu.c \
../Src/task_bmi085.c \
../Src/task_imu.c \
../Src/task_led.c \
../Src/task_uart_output.c 

OBJS += \
./Src/bmi085_io.o \
./Src/bno055.o \
./Src/bno055_io.o \
./Src/magdwick_imu.o \
./Src/task_bmi085.o \
./Src/task_imu.o \
./Src/task_led.o \
./Src/task_uart_output.o 

C_DEPS += \
./Src/bmi085_io.d \
./Src/bno055.d \
./Src/bno055_io.d \
./Src/magdwick_imu.d \
./Src/task_bmi085.d \
./Src/task_imu.d \
./Src/task_led.d \
./Src/task_uart_output.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Src -I"D:/projects/mocap_project/stm32_cube_workspace/stm32_cube_project/bmi085" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/bmi085_io.cyclo ./Src/bmi085_io.d ./Src/bmi085_io.o ./Src/bmi085_io.su ./Src/bno055.cyclo ./Src/bno055.d ./Src/bno055.o ./Src/bno055.su ./Src/bno055_io.cyclo ./Src/bno055_io.d ./Src/bno055_io.o ./Src/bno055_io.su ./Src/magdwick_imu.cyclo ./Src/magdwick_imu.d ./Src/magdwick_imu.o ./Src/magdwick_imu.su ./Src/task_bmi085.cyclo ./Src/task_bmi085.d ./Src/task_bmi085.o ./Src/task_bmi085.su ./Src/task_imu.cyclo ./Src/task_imu.d ./Src/task_imu.o ./Src/task_imu.su ./Src/task_led.cyclo ./Src/task_led.d ./Src/task_led.o ./Src/task_led.su ./Src/task_uart_output.cyclo ./Src/task_uart_output.d ./Src/task_uart_output.o ./Src/task_uart_output.su

.PHONY: clean-Src

