cmake_minimum_required(VERSION 3.28.0)

if(WIN32)
	set(PLATFORM "Windows")

	target_link_libraries(${tar} PRIVATE wsock32 ws2_32)
else()
	message(FATAL_ERROR "OS not supported!")
endif()

target_sources(
	${tar}
	PRIVATE
	"${PLATFORM}/Socket/Socket.cpp"
)