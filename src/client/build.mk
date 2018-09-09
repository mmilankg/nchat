local_dir	:= src/client
local_prog	:= $(local_dir)/ncClient
local_src	:= $(wildcard $(addprefix $(local_dir)/, *.cpp))
local_objs	:= $(subst .cpp,.o, $(local_src))

programs	+= $(local_prog)
sources		+= $(local_src)

LDFLAGS		:= -lrt -lpthread -lform -lmenu -lpanel -lncurses -lncurses++

$(local_prog): $(common_objs) $(local_objs)
	$(CXX) -o $@ $(LDFLAGS) $^
