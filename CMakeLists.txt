cmake_minimum_required(VERSION 3.8)
project(modbus_tcp_server_cpp)
# 强制使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#若未指定构建类型，默认设为 Debug
if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug)
endif()
#针对Debug模式强制添加调试选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#C语言
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -ggdb")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -ggdb")
#C++语言set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -00 -ggdb")#可选:-ggdb 生成更详细的gdb 调试信息，对addr2line 也兼容endif()
endif()
# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)   # 14
endif()



# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(my_interfaces REQUIRED)
find_package(ethercat_control REQUIRED)
find_package(robot_controller REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_library(MODBUS_LIB modbus)

# 1. 指定头文件路径
include_directories(/home/nvidia/raybot_connection_ws/install/ethercat_control/include/
                    /home/nvidia/raybot_connection_ws/install/my_interfaces/include/
                    /usr/include/modbus
                    /home/nvidia/raybot_connection_ws/install/robot_controller/include/
                    )

# 2. 指定库路径和链接库
link_directories(/home/nvidia/raybot_connection_ws/install/ethercat_control/lib
                /home/nvidia/raybot_connection_ws/install/robot_controller/lib
)
# target_link_libraries(test_srv_server ethercat_control)

# 编译可执行文件
add_executable(modbus_tcp_server_cpp_node 
src/modbus_tcp_server_cpp_node.cpp
src/modbustcp/modbustcp_server.cpp
src/ctrl_raybot/ctrl_raybot.cpp
)

target_link_libraries(modbus_tcp_server_cpp_node 
${MODBUS_LIB}
tinyxml2
)

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
  rclcpp_action
  my_interfaces
  sensor_msgs
  ethercat_control
  robot_controller
)

target_compile_definitions(modbus_tcp_server_cpp_node 
PRIVATE PROJECT_NAME="${PROJECT_NAME}"
)


# 4. 安装头文件到系统目录（关键步骤）
install(
  DIRECTORY include/
  DESTINATION include
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

install(DIRECTORY launch
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
