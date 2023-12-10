# setup cmake version
cmake_minimum_required(VERSION 3.23.0)

project(mandelbrot_oneapi)

# set(MKL_THREADING tbb_thread)


# setup compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Wno-unknown-cuda-version")

set(libraries_to_link   "")

find_package(Threads)
list(APPEND libraries_to_link Threads::Threads)

# setup oneAPI
# find_package(MKL REQUIRED)
# list(APPEND libraries_to_link MKL::MKL)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
list(APPEND libraries_to_link Qt6::Widgets)

find_package(fmt)
list(APPEND libraries_to_link fmt::fmt)

find_package(IntelSYCL REQUIRED)

set_property(TARGET IntelSYCL::SYCL_CXX PROPERTY
	INTERFACE_COMPILE_OPTIONS -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend=nvptx64-nvidia-cuda --offload-arch=sm_61)

set_property(TARGET IntelSYCL::SYCL_CXX PROPERTY
	INTERFACE_LINK_OPTIONS -fsycl -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend=nvptx64-nvidia-cuda --offload-arch=sm_61)

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

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${include_dirs}
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${libraries_to_link})
add_sycl_to_target(TARGET ${PROJECT_NAME} SOURCES ${sources})

# setup source files
#file(GLOB_RECURSE SOURCES src/*.cpp)

# setup executable
#add_executable(mandelbrot_oneapi ${SOURCES} ${HDRS})

# include directories
#target_include_directories(mandelbrot_oneapi PRIVATE ${MKL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# link mkl
#target_link_libraries(mandelbrot_oneapi PRIVATE ${MKL_LIBRARIES})

# setup install
# install(TARGETS mandelbrot_oneapi DESTINATION bin)

# cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
