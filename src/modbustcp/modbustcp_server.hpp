#pragma once    


#include <rclcpp/rclcpp.hpp>
#include <my_interfaces/srv/srv_move_axis.hpp>
#include <modbus/modbus.h>
#include <thread>
#include <mutex>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include "../log.hpp"
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std::chrono_literals;
using namespace std;

namespace modbus_tcp_server_cpp
{
// 全局配置 & Modbus 资源
constexpr int MODBUS_TCP_PORT     = 5020;
constexpr int MAX_CLIENT_NUM      = 5;
constexpr int REGISTER_COUNT      = 20;
constexpr int POLL_INTERVAL_MS    = 50;
constexpr int RECONNECT_DELAY_MS  = 1000;



// ===================== 位姿结构体定义 =====================
struct PoseData
{
    int ID;
  int type;
  float d1;
  float d2;
  float d3;
  float d4;
  float d5;
  float d6;
  float d7;
  float d8;
};

class ModbusTcpServerCppNode : public rclcpp::Node
{
    public:
    explicit ModbusTcpServerCppNode();
        ~ModbusTcpServerCppNode();


    private:
    rclcpp::Client<my_interfaces::srv::SrvMoveAxis>::SharedPtr service_client_;
    rclcpp::TimerBase::SharedPtr edge_timer_;
    std::thread modbus_server_thread_;
    // 存储所有位姿数据
    std::vector<PoseData> pose_list_;
    std::string net_ip_;
    int net_port_;
    int net_max_clients_;
    int net_task_interval_ms_;

    modbus_mapping_t* g_modbus_map = nullptr;
    std::mutex        g_reg_mutex;
    uint16_t          g_prev_reg0 = 0;


    // ===================== 1. 从 ROS YAML 参数加载位姿 =====================

        void load_pose_from_params();
        void load_network_params();
        void wait_service_ready();


        // 调用服务（匹配 SrvMoveAxis 字段）
        void invoke_service();

        void detect_rising_edge();
        // 单个客户端处理
        void handle_single_client(modbus_t* ctx, int client_fd);


        // Modbus TCP 监听主循环
        void server_loop();
        void write_float_to_modbus_registers(modbus_mapping_t* mb_map, int base_addr, float value);
        float read_float_from_modbus_registers(modbus_mapping_t* mb_map, int base_addr);
    
    };
} // namespace modbus_tcp_server_cpp


