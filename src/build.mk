local_dir	:= src
local_src	:= $(wildcard $(addprefix $(local_dir)/, *.cpp))
local_objs	:= $(subst .cpp,.o, $(local_src))
common_objs	:= $(local_objs)

sources		+= $(local_src)

$(local_objs): $(local_src)
