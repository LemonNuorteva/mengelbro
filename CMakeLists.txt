cmake_minimum_required(VERSION 3.20)

project(mengelbro)

set(libraries_to_link   "")

find_package(Threads)
list(APPEND libraries_to_link Threads::Threads)

find_package(OpenMP)
list(APPEND libraries_to_link OpenMP::OpenMP_CXX)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
list(APPEND libraries_to_link Qt5::Widgets)

add_compile_options(-march=native)

#sources
set(source_dirs      "")
set(include_dirs     "")
set(sources          "")
set(source_file_types
    ".cpp"
    ".c"    
)

list(APPEND source_dirs ${CMAKE_CURRENT_SOURCE_DIR}/src)
list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/inc)

foreach(dir IN LISTS source_dirs)
    foreach(filetype IN LISTS source_file_types)
        set(tmp_sources "")
        file(GLOB tmp_sources ${dir}/*${filetype})
        list(APPEND sources ${tmp_sources})
    endforeach()    
endforeach()


add_executable(${PROJECT_NAME}
    ${sources}
)

#headers
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${include_dirs}
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${libraries_to_link})