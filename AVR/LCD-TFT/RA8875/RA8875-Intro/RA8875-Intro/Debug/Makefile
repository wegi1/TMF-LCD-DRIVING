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
../RA8875-drv.c \
../RA8875-intro.c


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
RA8875-drv.o \
RA8875-intro.o

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
RA8875-drv.o \
RA8875-intro.o

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
RA8875-drv.d \
RA8875-intro.d

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
RA8875-drv.d \
RA8875-intro.d

OUTPUT_FILE_PATH +=RA8875-Intro.elf

OUTPUT_FILE_PATH_AS_ARGS +=RA8875-Intro.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler























./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=32000000UL  -I".." -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\include"  -Os -ffunction-sections -fdata-sections -fshort-enums -mrelax -g3 -Wall -mmcu=atxmega256a3bu -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\gcc\dev\atxmega256a3bu" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

Fonts/%.o: ../Fonts/%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=32000000UL  -I".." -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\include"  -Os -ffunction-sections -fdata-sections -fshort-enums -mrelax -g3 -Wall -mmcu=atxmega256a3bu -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\gcc\dev\atxmega256a3bu" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
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
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="RA8875-Intro.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mrelax -mmcu=atxmega256a3bu -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\XMEGAA_DFP\1.0.39\gcc\dev\atxmega256a3bu"  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "RA8875-Intro.elf" "RA8875-Intro.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "RA8875-Intro.elf" "RA8875-Intro.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "RA8875-Intro.elf" > "RA8875-Intro.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "RA8875-Intro.elf" "RA8875-Intro.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "RA8875-Intro.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "RA8875-Intro.elf" "RA8875-Intro.a" "RA8875-Intro.hex" "RA8875-Intro.lss" "RA8875-Intro.eep" "RA8875-Intro.map" "RA8875-Intro.srec" "RA8875-Intro.usersignatures"
	