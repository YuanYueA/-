cmake_minimum_required(VERSION 2.8.3)
add_definitions("-w -std=c++11")

#debug
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g3 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")  

project(quant_trade)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
INCLUDE_DIRECTORIES(./include)

AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/src CPP_LIST)
LINK_DIRECTORIES(./lib)
#LINK_DIRECTORIES(./strategy)


#add executable file

ADD_EXECUTABLE(quant_trade ${CPP_LIST})
#add link library
TARGET_LINK_LIBRARIES(quant_trade thostmduserapi thosttraderapi handy)
#TARGET_LINK_LIBRARIES(IMEtrader ta_abstract ta_common ta_func ta_libc hiredis)
TARGET_LINK_LIBRARIES(quant_trade -lpthread -lm)
#TARGET_LINK_LIBRARIES(IMEtrader -lmysqlclient)
TARGET_LINK_LIBRARIES(quant_trade -ldl -rdynamic -lstdc++ -lrt)


set (EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" )
