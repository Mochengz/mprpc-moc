#set(SRC_LIST userservice.cc ../user.pb.cc)
set(SRC_LIST friends_service.cc ../friend.pb.cc)

add_executable(provider ${SRC_LIST})

#动态链接库
target_link_libraries(provider mprpc protobuf)