cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(cpp-lox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME}
	"src/main.cpp" "src/Lox.cpp"  "src/Token.h"  "src/Scanner.h" "src/Scanner.cpp" "src/Expr.h" "src/AstPrinter.h" "src/AstPrinter.cpp")