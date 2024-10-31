# 添加可执行文件和源文件
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB PBSOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(chat_server ${SOURCES} ${PBSOURCES})

set_target_properties(chat_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                             ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_link_libraries(chat_server jsoncpp ${_REFLECTION} ${_GRPC_GRPCPP}
                      ${_PROTOBUF_LIBPROTOBUF} hiredis mysqlcppconn)
