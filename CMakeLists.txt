cmake_minimum_required(VERSION 3.28)
project(CG_TermProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(assimp)

# Add the executable
add_executable(main
    src/main.cpp
    src/model.cpp
    src/node2.cpp
    src/game/MainScene.cpp
)

# Link libraries
target_link_libraries(main PRIVATE
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    glfw
    glm::glm
    assimp
)

# Make sure the include directories are added
target_include_directories(main PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/")
