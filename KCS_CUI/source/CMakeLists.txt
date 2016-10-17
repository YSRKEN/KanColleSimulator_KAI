cmake_minimum_required(VERSION 3.1)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
find_package(Threads REQUIRED)

## Set our project name
project(KCS_CUI)

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
add_executable(KCS_CUI ${kcs_kai_src})
target_link_libraries(KCS_CUI Threads::Threads)
