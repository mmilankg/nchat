# Makefile based on the Mecklenberg book (Mecklenberg, Managing Projects with GNU Make, 3E, 2005)

programs	:=
sources		:=

objects		= $(subst .cpp,.o, $(sources))
dependencies	= $(subst .cpp,.d, $(sources))

include_dirs	:= include
CPPFLAGS	+= $(addprefix -I, $(include_dirs))
CXXFLAGS	+= -O0 -g3 -std=c++11
WFLAGS		+= -Wall -pedantic

vpath %.h $(include_dirs)

CXX	:= g++
MV	:= mv -f
RM	:= rm -f
SED	:= sed

all:

include	src/build.mk
include src/server/build.mk
include src/client/build.mk

.PHONY: all
all: $(programs)

.PHONY: clean
clean:
	$(RM) $(objects) $(programs) $(dependencies)

ifneq "$(MAKECMDGOALS)" "clean"
  include $(dependencies)
endif

%.o: %.cpp
	$(CXX) -o $@ -c $(CPPFLAGS) $(CXXFLAGS) $(WFLAGS) $<

%.d: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -M $< | \
	  $(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	$(MV) $@.tmp $@
