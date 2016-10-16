cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 14) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "KCS_CUI" )

## Set our project name
project(${PROJECT_NAME})

set(kcs_kai_src 
	config.cpp
	exception.cpp
	fleet.cpp
	kammusu.cpp
	main.cpp
	mapdata.cpp
	other.cpp
	random.cpp
	result.cpp
	simulator.cpp
	weapon.cpp
)

## Define the executable
add_executable(${PROJECT_NAME} ${kcs_kai_src})
