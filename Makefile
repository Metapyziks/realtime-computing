# Makefile for MCB2470 board
# Change the name of the exercise below as required.
# Your C file should have the same name (without .c at the end)
# NOTE: NO SPACES ALLOWED IN EXERCISE NAME
EXERCISE = ex3

# If using 32Mb external memory, define EXTMEM
# If need 32 bit LED port,comment out this line.
EXTMEM = 1


# Where to find include files and startup files
ROOTPATH = T:/ENGINEERING/Realtime/

# the lcd library
LCDLIB = $(ROOTPATH)/lcd/liblcd.a

# Flags for the C compiler
# We tell it the processor, to optimise for size and where to look for include files
# (and lots of helpful warnings)
INCLUDES = -I$(ROOTPATH)/include -I$(ROOTPATH)/lcd
CFLAGS   = -mcpu=arm7tdmi -Os -Wall -Wcast-align -Wcast-qual -Wimplicit \
	   -Wmissing-declarations -Wmissing-prototypes -Wnested-externs \
	   -Wpointer-arith -Wredundant-decls -Wshadow \
	   -Wstrict-prototypes $(INCLUDES)

	   
# Flags for the linker
# We tell it that we are going to specify the start up files, and the linker script to use.
# The linker script defines the addresses to use for flash and RAM memory
ifdef EXTMEM
LD_SCRIPT = $(ROOTPATH)/build_files/link_64k_512k_rom_extmem.ld
STARTFILES = $(ROOTPATH)/startup/startup.o $(ROOTPATH)/startup/cstartup_extmem.o $(ROOTPATH)/startup/sdram.o
else
LD_SCRIPT = $(ROOTPATH)/build_files/link_64k_512k_rom.ld
STARTFILES = $(ROOTPATH)/startup/startup.o $(ROOTPATH)/startup/cstartup.o
endif

LDFLAGS  = -nostartfiles -T $(LD_SCRIPT)


$(EXERCISE).hex: $(EXERCISE).elf
	arm-none-eabi-objcopy -O ihex $(EXERCISE).elf $(EXERCISE).hex
	arm-none-eabi-size $(EXERCISE).elf

$(EXERCISE).elf: $(EXERCISE).o
	arm-none-eabi-gcc $(LDFLAGS) $(STARTFILES) $(EXERCISE).o -lc -lm $(LCDLIB) -o $(EXERCISE).elf
	
$(EXERCISE).o: $(EXERCISE).c utils.h Makefile
	cs-rm -f $(EXERCISE).o $(EXERCISE).elf $(EXERCISE).hex $(EXERCISE).lst
ifdef EXTMEM
	arm-none-eabi-gcc -c $(CFLAGS) -D EXTMEM $(EXERCISE).c
else
	arm-none-eabi-gcc -c $(CFLAGS) $(EXERCISE).c
endif

	
.PHONY:	clean
clean:
	cs-rm -f $(EXERCISE).o $(EXERCISE).elf $(EXERCISE).hex $(EXERCISE).lst
	
#program: $(TARGET)
#	$(PROGRAM) $(TARGET)

