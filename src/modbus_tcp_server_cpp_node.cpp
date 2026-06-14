#include <rclcpp/rclcpp.hpp>
#include <my_interfaces/srv/srv_move_axis.hpp>
#include <modbus/modbus.h>
#include <thread>
#include <mutex>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include "log.hpp"
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "modbustcp/modbustcp_server.hpp"


using namespace std::chrono_literals;
using namespace std;

// 全局入口函数
int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<modbus_tcp_server_cpp::ModbusTcpServerCppNode>();
  node->init();
  node->start();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}