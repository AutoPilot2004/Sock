cmake_minimum_required(VERSION 3.28.0)

set(tar Sock)

project(
	${tar}
	VERSION 0.1.0
	DESCRIPTION "A simple platform-agnostic socket library (currently Windows only)"
	LANGUAGES CXX
)

add_library(${tar} STATIC)
add_library(${tar}::${tar} ALIAS ${tar})

set_target_properties(
	${tar}
	PROPERTIES 
	CXX_STANDARD 23
)

target_include_directories(${tar} PUBLIC "src")

add_subdirectory("src/Sock")