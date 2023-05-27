cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(c-lox C)

add_executable(
	${PROJECT_NAME}
	"src/main.c"
	"src/common.h"
	"src/memory.h"
	"src/memory.c"
	"src/chunk.h"
	"src/chunk.c"
	"src/debug.h"
	"src/debug.c"
	"src/value.h"
	"src/value.c"
	"src/vm.h"
	"src/vm.c"
	"src/compiler.h"
	"src/compiler.c"
	"src/scanner.h"
	"src/scanner.c"
)