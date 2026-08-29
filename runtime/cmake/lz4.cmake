include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/lz4)
file(GLOB LZ4_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/libs/lz4/*.c")
target_sources(${PROJECT_NAME} PRIVATE ${LZ4_SOURCES})
