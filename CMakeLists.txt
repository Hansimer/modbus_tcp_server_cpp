cmake_minimum_required(VERSION 3.8)
project(modbus_tcp_server_cpp)
# 强制使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)
find_package(my_interfaces REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_library(MODBUS_LIB modbus)



# 编译可执行文件
add_executable(modbus_tcp_server_cpp_node 
src/modbus_tcp_server_cpp_node.cpp
src/modbustcp/modbustcp_server.cpp
)

target_link_libraries(modbus_tcp_server_cpp_node ${MODBUS_LIB})
ament_target_dependencies(
  modbus_tcp_server_cpp_node
  rclcpp
  std_srvs
  my_interfaces 
  ament_index_cpp
)

install(TARGETS
  modbus_tcp_server_cpp_node
  DESTINATION lib/${PROJECT_NAME}
)

# 2. 测试服务端节点（单独编译）
add_executable(test_srv_server 
  src/test_srv_server.cpp
)
ament_target_dependencies(
  test_srv_server
  rclcpp
  my_interfaces
)

target_compile_definitions(modbus_tcp_server_cpp_node 
PRIVATE PROJECT_NAME="${PROJECT_NAME}"
)

install(TARGETS
  test_srv_server
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY log
  DESTINATION log/${PROJECT_NAME}
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
