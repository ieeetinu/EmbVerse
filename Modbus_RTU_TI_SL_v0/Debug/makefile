################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/tms470_4.9.5

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./uartstdio.obj" \
"./startup_ccs.obj" \
"./modbus_rtu.obj" \
"./main.obj" \
-l"libc.a" \
-l"E:\temp\Modbus_RTU_TI_SL\driverlib-cm4f\Debug\driverlib-cm4f.lib" \
"../Modbus_RTU_TI_SL_v0.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Modbus_RTU_TI_SL_v0.out

dependents:
	-cd E:/temp/Modbus_RTU_TI_SL/driverlib-cm4f/Debug && $(MAKE) clean all

# Tool invocations
Modbus_RTU_TI_SL_v0.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS) E:/temp/Modbus_RTU_TI_SL/driverlib-cm4f/Debug/driverlib-cm4f.lib
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv5/tools/compiler/tms470_4.9.5/bin/cl470" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -g --gcc --define=ccs="ccs" --define=PART_LM4F120H5QR --define=TARGET_IS_BLIZZARD_RA1 --diag_warning=225 --display_error_number -z --stack_size=256 -m"Modbus_RTU_TI_SL_v0.map" --heap_size=0 -i"C:/ti/ccsv5/tools/compiler/tms470_4.9.5/lib" -i"C:/ti/ccsv5/tools/compiler/tms470_4.9.5/include" --reread_libs --warn_sections --display_error_number --rom_model -o "Modbus_RTU_TI_SL_v0.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(TMS470_EXECUTABLE_OUTPUTS__QUOTED) "Modbus_RTU_TI_SL_v0.out"
	-$(RM) "main.pp" "modbus_rtu.pp" "startup_ccs.pp" "uartstdio.pp" 
	-$(RM) "main.obj" "modbus_rtu.obj" "startup_ccs.obj" "uartstdio.obj" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-"C:/ti/ccsv5/utils/tiobj2bin/tiobj2bin" "Modbus_RTU_TI_SL_v0.out" "Modbus_RTU_TI_SL_v0.bin" "C:/ti/ccsv5/tools/compiler/tms470_4.9.5/bin/ofd470" "C:/ti/ccsv5/tools/compiler/tms470_4.9.5/bin/hex470" "C:/ti/ccsv5/utils/tiobj2bin/mkhex4bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build
E:/temp/Modbus_RTU_TI_SL/driverlib-cm4f/Debug/driverlib-cm4f.lib:

-include ../makefile.targets
