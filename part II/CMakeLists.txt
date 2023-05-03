cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(c-lox C)

add_executable(
	${PROJECT_NAME}
	"src/main.c"
)