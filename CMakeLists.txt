cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(cpp-lox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
	${PROJECT_NAME}
	"src/main.cpp"
	"src/Lox.cpp"
	"src/Token.h"
	"src/Scanner.h"
	"src/Scanner.cpp"
	"src/Expr.h"
	"src/Parser.cpp"
	"src/Interpreter.h"
	"src/Interpreter.cpp"
	"src/Stmt.h"
	"src/Environment.cpp"
	"src/LoxCallable.h"
	"src/Clock.cpp"
	"src/LoxFunction.cpp"
	"src/LoxFunction.h"
	"src/LoxAnonFunction.h"
	"src/LoxAnonFunction.cpp" 
	"src/AnonFunction.cpp"
	"src/LoxFunction.h"
	"src/Resolver.cpp"
	"src/LoxClass.cpp"
	"src/LoxInstance.cpp"
)