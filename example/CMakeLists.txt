add_executable(testTCPServer
  testTCPServer.cpp
)

target_link_libraries(testTCPServer
  MsgPkz::MsgPkz
)

add_executable(testTCPClient
  testTCPClient.cpp
)

target_link_libraries(testTCPClient
  MsgPkz::MsgPkz
)

add_executable(testSerialServer
  testSerialServer.cpp
)

target_link_libraries(testSerialServer
  MsgPkz::MsgPkz
)