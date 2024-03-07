cmake_minimum_required(VERSION 3.23)

project(modbus)

if(NOT TARGET ${PROJECT_NAME})

  message(STATUS "Running ./autogen.sh")
  execute_process(COMMAND ./autogen.sh 
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                RESULT_VARIABLE AUTOGEN_RESULT)

  if(NOT AUTOGEN_RESULT EQUAL "0")
    message(FATAL_ERROR "autogen.sh failed")
  endif()

  message(STATUS "Running ./configure")
  execute_process(COMMAND ./configure
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                RESULT_VARIABLE CONFIGURE_RESULT) #cd ${CMAKE_CURRENT_LIST_DIR} && ./autogen.sh && ./configure)

  if(NOT CONFIGURE_RESULT EQUAL "0")
    message(FATAL_ERROR "configure failed")
  endif()

  add_library(${PROJECT_NAME})

  target_sources(${PROJECT_NAME} PRIVATE FILE_SET configuration TYPE HEADERS FILES config.h)
  target_sources(${PROJECT_NAME} PUBLIC FILE_SET public_headers TYPE HEADERS FILES src/modbus.h src/modbus-rtu.h src/modbus-tcp.h src/modbus-version.h)
  target_sources(${PROJECT_NAME} PRIVATE src/modbus.c src/modbus-data.c src/modbus-rtu.c src/modbus-tcp.c)
endif()
