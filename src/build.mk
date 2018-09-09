local_dir	:= src
local_src	:= $(addprefix $(local_dir)/, socket.cpp)
local_objs	:= $(subst .cpp,.o, $(local_src))
common_objs	:= $(local_objs)

sources		+= $(local_src)

$(local_objs): $(local_src)
