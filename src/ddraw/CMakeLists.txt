add_library(dxlib_ddraw STATIC)
target_sources(dxlib_ddraw PRIVATE ddraw.cpp ddraw_clipper.cpp ddraw_surface.cpp)

target_include_directories(dxlib_ddraw
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Vulkan REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(dxlib_ddraw PRIVATE SDL3::SDL3 Vulkan::Vulkan)

target_compile_definitions(dxlib_ddraw PUBLIC DDRAW_STATIC)

add_library(dxlib::ddraw ALIAS dxlib_ddraw)
