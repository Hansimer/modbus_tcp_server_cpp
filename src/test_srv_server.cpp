#include <rclcpp/rclcpp.hpp>
#include <my_interfaces/srv/srv_move_axis.hpp>
#include "/home/nvidia/raybot_connection_ws/install/ethercat_control/include/ethercat_control/ethercat_control/srv/get_active_robot_model.hpp"
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

     service_client_ = this->create_client<ethercat_control::srv::GetActiveRobotModel>(
      "/raybot_dual_arms/get_active_model");
        call_service_timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),
      std::bind(&TestSrvServer::call_service_and_get_result, this)
    );
      // 2. 调用服务并获取返回值
    call_service_and_get_result();
    RCLCPP_INFO(this->get_logger(), "Test Service Server started, waiting for request...");
  }

private:
  rclcpp::Service<my_interfaces::srv::SrvMoveAxis>::SharedPtr service_;
  rclcpp::Client<ethercat_control::srv::GetActiveRobotModel>::SharedPtr service_client_;
  rclcpp::TimerBase::SharedPtr call_service_timer_; // 定时器
  //服务回调函数
 void call_service_and_get_result() {
    // 定时器只调用一次，调用后销毁
    call_service_timer_->cancel();

    // 等待服务端上线（最多等200秒）
    if (!service_client_->wait_for_service(std::chrono::seconds(200))) {
      RCLCPP_ERROR(this->get_logger(), "服务端未上线，无法调用");
      return;
    }

    // 创建空请求
    auto request = std::make_shared<ethercat_control::srv::GetActiveRobotModel::Request>();

    // 异步发送请求 + 回调处理结果（不阻塞）
    service_client_->async_send_request(
      request,
      [this](rclcpp::Client<ethercat_control::srv::GetActiveRobotModel>::SharedFuture future) {
        auto response = future.get();
        if (response->success) {
          RCLCPP_INFO(this->get_logger(), "服务调用成功！");
          RCLCPP_INFO(this->get_logger(), "robot_name: %s", response->robot_name.c_str());
          RCLCPP_INFO(this->get_logger(), "enabled_groups: %s", response->enabled_groups.c_str());
          // 在这里保存结果到成员变量，供其他地方使用
          std::string active_robot_name_ = response->robot_name;
          std::string active_groups_ = response->enabled_groups;
        } else {
          RCLCPP_ERROR(this->get_logger(), "服务调用失败：%s", response->message.c_str());
        }
      }
    );
  }
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