#set(SRC_LIST call_userservice.cc ../user.pb.cc)
set(SRC_LIST call_friendservice.cc ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer mprpc protobuf)