include_directories(${PROJECT_ROOT_DIR}/include)
add_executable(IE main.cpp)
target_link_libraries(IE DragonLoader)