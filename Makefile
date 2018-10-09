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
  -include $(dependencies)
endif

%.o: %.cpp
	$(CXX) -c $< -o $@ -MMD -MP -MF $(patsubst %.o,%.d,$@) $(CPPFLAGS) $(CXXFLAGS) $(WFLAGS)

# compile_commands.json can be generated with "make CXX=clang++"
.PHONY: compile_commands.json
compile_commands.json:
	@$(MAKE) -n -B | grep "^$(CXX) -c" | ./convert-to-compile-commands.sh > compile_commands.json
