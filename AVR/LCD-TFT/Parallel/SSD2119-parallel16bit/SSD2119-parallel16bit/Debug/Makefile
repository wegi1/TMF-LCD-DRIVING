################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../Fonts \
../Clk


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../Browser48x48_ico.c \
../Fonts/terminal16x16.c \
../Fonts/terminal5x12.c \
../Fonts/terminal8x8b.c \
../Fonts/Times16AA332.c \
../Fonts/Times16AABW.c \
../Fonts/Times16AAtest.c \
../Fonts/Times16WA.c \
../MGlass_mono.c \
../SSD2119-drv.c \
../SSD-parallel16bit.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
Browser48x48_ico.o \
Fonts/terminal16x16.o \
Fonts/terminal5x12.o \
Fonts/terminal8x8b.o \
Fonts/Times16AA332.o \
Fonts/Times16AABW.o \
Fonts/Times16AAtest.o \
Fonts/Times16WA.o \
MGlass_mono.o \
SSD2119-drv.o \
SSD-parallel16bit.o

OBJS_AS_ARGS +=  \
Browser48x48_ico.o \
Fonts/terminal16x16.o \
Fonts/terminal5x12.o \
Fonts/terminal8x8b.o \
Fonts/Times16AA332.o \
Fonts/Times16AABW.o \
Fonts/Times16AAtest.o \
Fonts/Times16WA.o \
MGlass_mono.o \
SSD2119-drv.o \
SSD-parallel16bit.o

C_DEPS +=  \
Browser48x48_ico.d \
Fonts/terminal16x16.d \
Fonts/terminal5x12.d \
Fonts/terminal8x8b.d \
Fonts/Times16AA332.d \
Fonts/Times16AABW.d \
Fonts/Times16AAtest.d \
Fonts/Times16WA.d \
MGlass_mono.d \
SSD2119-drv.d \
SSD-parallel16bit.d

C_DEPS_AS_ARGS +=  \
Browser48x48_ico.d \
Fonts/terminal16x16.d \
Fonts/terminal5x12.d \
Fonts/terminal8x8b.d \
Fonts/Times16AA332.d \
Fonts/Times16AABW.d \
Fonts/Times16AAtest.d \
Fonts/Times16WA.d \
MGlass_mono.d \
SSD2119-drv.d \
SSD-parallel16bit.d

OUTPUT_FILE_PATH +=SSD2119-parallel16bit.elf

OUTPUT_FILE_PATH_AS_ARGS +=SSD2119-parallel16bit.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler























./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=32000000UL  -I".." -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atxmega256a3bu -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\gcc\dev\atxmega256a3bu" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

Fonts/%.o: ../Fonts/%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=32000000UL  -I".." -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atxmega256a3bu -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\gcc\dev\atxmega256a3bu" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="SSD2119-parallel16bit.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mrelax -mmcu=atxmega256a3bu -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\gcc\dev\atxmega256a3bu"  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "SSD2119-parallel16bit.elf" "SSD2119-parallel16bit.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "SSD2119-parallel16bit.elf" "SSD2119-parallel16bit.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "SSD2119-parallel16bit.elf" > "SSD2119-parallel16bit.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "SSD2119-parallel16bit.elf" "SSD2119-parallel16bit.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "SSD2119-parallel16bit.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "SSD2119-parallel16bit.elf" "SSD2119-parallel16bit.a" "SSD2119-parallel16bit.hex" "SSD2119-parallel16bit.lss" "SSD2119-parallel16bit.eep" "SSD2119-parallel16bit.map" "SSD2119-parallel16bit.srec" "SSD2119-parallel16bit.usersignatures"
	