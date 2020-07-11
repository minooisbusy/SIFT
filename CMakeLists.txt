cmake_minimum_required( VERSION 3.0 )

PROJECT( sift )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -march=native")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
if(COMPILER_SUPPORTS_CXX17)
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
     add_definitions(-DCOMPILERWITHC17)
     message(STATUS " Using flag -std=c++17.")
endif()
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(OpenCV REQUIRED)


include_directories(
${PROJECT_SOURCE_DIR}
"/usr/include/eigen3"
#"/usr/local/include/opencv4"
)
message(STATUS ${OpenCV})

add_executable(${PROJECT_NAME}
src/main.cc 
src/Frame.cc
src/dataset.cc
)
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
#TODO deactivate하고 돌려보기
#set(OpenCV_LIBS /usr/local/lib/opencv4/opencv_*)
target_link_libraries(${PROJECT_NAME}
${OpenCV_LIBS}
${Eigen_LIBS}
)
