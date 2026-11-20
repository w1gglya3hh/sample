#!/bin/sh

set -eu

TARGET="$1"
SOURCE="src/$TARGET.cpp"

if [ ! -f "$SOURCE" ]; then
	echo "oops: no target '$TARGET'" >&2
	exit 1
fi

set -- -xc++ -std=c++23 -Wall -Wextra -Wpedantic -Ilib

case "$TARGET" in
	gl*)
		set -- $@ $(pkg-config --cflags --libs sdl3) -Wl,-framework,OpenGL
		;;
	vk*)
		set -- $@ $(pkg-config --cflags --libs sdl3) $(pkg-config --cflags vulkan)
		;;
	*)
		echo "oops: no rules to build target '$TARGET'" >&2
		exit 1
		;;
esac

exec ${CXX:-c++} $@ -o bin "$SOURCE"
