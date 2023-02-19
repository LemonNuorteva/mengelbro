cmake_minimum_required(VERSION 3.20)

project(mengelbro)

#language setup
set(project_languages
                            C
                            CXX
                            CUDA
)
set(CMAKE_C_STANDARD        17)
set(CMAKE_CXX_STANDARD      20)

SET(ENV{LD_LIBRARY_PATH} "/usr/local/cuda/lib64:$ENV{LD_LIBRARY_PATH}")

set(CMAKE_VS_PLATFORM_TOOLSET_CUDA 11.8)
set(CMAKE_CUDA_FLAGS
    "-gencode arch=compute_50,code=[sm_50,compute_50]  -gencode arch=compute_52,code=[sm_52,compute_52]  -gencode arch=compute_61,code=[sm_61,compute_61]  -gencode arch=compute_70,code=[sm_70,compute_70] -gencode arch=compute_75,code=[sm_75,compute_75]"
)
link_directories("/usr/local/cuda/lib64")


set(libraries_to_link   "")

find_package(Threads)
list(APPEND libraries_to_link Threads::Threads)

find_package(OpenMP)
list(APPEND libraries_to_link OpenMP::OpenMP_CXX)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

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
	".cu"
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
