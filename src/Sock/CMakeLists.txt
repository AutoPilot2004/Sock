cmake_minimum_required(VERSION 3.28.0)

target_sources(
	${tar}
	PRIVATE
	"Parse.cpp"
)

add_subdirectory("Platform_Impl")