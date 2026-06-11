#include <rclcpp/rclcpp.hpp>
#include <my_interfaces/srv/srv_move_axis.hpp>
#include <string>

using namespace std::chrono_literals;
namespace modbus_tcp_server_cpp
{
class TestSrvServer : public rclcpp::Node
{
public:
  TestSrvServer()
  : Node("test_srv_server_node")
  {
    // 创建服务端，名称和客户端保持一致
    service_ = this->create_service<my_interfaces::srv::SrvMoveAxis>(
      "/my_interfaces/move_axis",
      std::bind(&TestSrvServer::service_callback, this, std::placeholders::_1, std::placeholders::_2)
    );

    RCLCPP_INFO(this->get_logger(), "Test Service Server started, waiting for request...");
  }

private:
  rclcpp::Service<my_interfaces::srv::SrvMoveAxis>::SharedPtr service_;

  // 服务回调函数
  void service_callback(
    const std::shared_ptr<my_interfaces::srv::SrvMoveAxis::Request> request,
    std::shared_ptr<my_interfaces::srv::SrvMoveAxis::Response> response)
  {
    // 打印收到的请求
    RCLCPP_INFO(
      this->get_logger(),
      "Recv request -> id: %u, name: %s",
      request->id,
      request->name.c_str()
    );

    // 填充响应字段
    // 模拟业务：id=1 返回100，其他返回 200
    if (request->id == 1)
    {
      response->statues = 100;
    }
    else
    {
      response->statues = 200;
    }

    RCLCPP_INFO(this->get_logger(), "Send response -> statues: %u", response->statues);
  }
};

} // namespace modbus_tcp_server_cpp

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<modbus_tcp_server_cpp::TestSrvServer>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}