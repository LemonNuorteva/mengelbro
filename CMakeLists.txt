cmake_minimum_required(VERSION 3.20)

project(mengelbro)

#language setup
set(project_languages
                            C
                            CXX
                            ISPC
)
set(CMAKE_C_STANDARD        17)
set(CMAKE_CXX_STANDARD      20)

set(libraries_to_link   "")

find_package(Threads)
list(APPEND libraries_to_link Threads::Threads)

find_package(OpenMP)
list(APPEND libraries_to_link OpenMP::OpenMP_CXX)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
list(APPEND libraries_to_link Qt6::Widgets)

find_package(fmt)
list(APPEND libraries_to_link fmt::fmt)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavdevice
    libavformat
    libavcodec
    libavutil
    libavfilter
    libswresample
    libswscale
)
list(APPEND libraries_to_link PkgConfig::FFMPEG)

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
