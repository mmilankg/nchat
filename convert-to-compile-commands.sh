#!/bin/bash

# This is a helper script used by the Makefile for generating a
# compile_commands.json file
#
# Read a series of compile lines like...:
#
#    clang++ -c src/terminol/xcb/terminolc.cxx -o obj/terminol/xcb/terminolc.o -DVERSION=\"20171123.d40fd5b\" -iquote src -DDEBUG=1 -fpic -fno-rtti -std=c++17 -pthread -Werror -stdlib=libc++ -g -O0 -Wpedantic -Wextra -Wall -Wundef -Wshadow -Wredundant-decls -Wsign-compare -Wmissing-declarations -Wold-style-cast -Wmissing-field-initializers -Wno-format-zero-length -Wno-unused-function -Woverloaded-virtual -Wsign-promo -Wctor-dtor-privacy -Wnon-virtual-dtor -Wextra-semi -Wcomma -Wno-unused-parameter  -MMD -MP -MF obj/terminol/xcb/terminolc.dep
#
# ...and outputs a standard compile_commands.json:
#
#    [
#    { "directory": "/home/bagnose/build/personal/terminol/clang-debug",
#      "command":   "clang++ -c src/terminol/xcb/terminolc.cxx -o obj/terminol/xcb/terminolc.o -DVERSION=\\\"20171123.d40fd5b\\\" -iquote src -DDEBUG=1 -fpic -fno-rtti -std=c++17 -pthread -Werror -stdlib=libc++ -g -O0 -Wpedantic -Wextra -Wall -Wundef -Wshadow -Wredundant-decls -Wsign-compare -Wmissing-declarations -Wold-style-cast -Wmissing-field-initializers -Wno-format-zero-length -Wno-unused-function -Woverloaded-virtual -Wsign-promo -Wctor-dtor-privacy -Wnon-virtual-dtor -Wextra-semi -Wcomma -Wno-unused-parameter  -MMD -MP -MF obj/terminol/xcb/terminolc.dep",
#      "file":      "src/terminol/xcb/terminolc.cxx" },
#    ...
#    ]

FIRST=true

echo "["

while IFS='' read -r LINE || [[ -n "$LINE" ]]; do
    if [ $FIRST = true ]; then
        FIRST=false
    else
        echo ","
    fi
    echo    "{ \"directory\": \"$PWD\","
    echo    "  \"command\":   \"$(echo "$LINE" | sed -e 's|\\"|\\\\\\"|g')\","
    echo -n "  \"file\":      \"$(echo "$LINE" | awk '{print $3}')\" }"
done

echo ""
echo "]"
