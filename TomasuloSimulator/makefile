#########################
# customise these
CFILES := CacheEmulator/EmulateCache.c CPU/CPU.c Global/Utils/PrintUtility.c TomasuloSimulator.c
PROG := TomasuloSimulator
CFLAGS := -Wall -Wextra -g
LDFLAGS :=
########################

# -MMD generates dependencies while compiling
CFLAGS += -MMD
CC := gcc

OBJFILES := $(CFILES:.c=.o)
DEPFILES := $(CFILES:.c=.d)

$(PROG) : $(OBJFILES)
		$(LINK.o) $(LDFLAGS) -o $@ $^

clean :
		rm -f $(PROG) $(OBJFILES) $(DEPFILES)

-include $(DEPFILES)