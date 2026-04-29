find_package(SDL3 CONFIG REQUIRED)

add_executable(sdl3_ddraw_test
	sdl3_ddraw.c
)

target_include_directories(sdl3_ddraw_test
	PRIVATE
		${CMAKE_SOURCE_DIR}/src/ddraw/include
)

target_link_libraries(sdl3_ddraw_test
	PRIVATE
		dxlib::ddraw
		SDL3::SDL3
)

set_property(TARGET sdl3_ddraw_test PROPERTY LINKER_LANGUAGE CXX)

add_test(NAME sdl3_ddraw_test COMMAND sdl3_ddraw_test)
