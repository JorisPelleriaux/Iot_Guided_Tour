# cmake_minimum_required(VERSION <specify CMake version here>)
project(Iot_Guided_Tour C)

set(CMAKE_C_STANDARD 11)

add_executable(Iot_Guided_Tour main.c XM1110_I2C.c XM1110_I2C.h)