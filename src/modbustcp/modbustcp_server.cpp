#include "modbustcp_server.hpp"


using namespace std::chrono_literals;
using namespace std;
namespace modbus_tcp_server_cpp
{
     ModbusTcpServerCppNode::ModbusTcpServerCppNode(): Node("modbus_tcp_server_cpp_node")
  {
    LOG_INFO("==== Modbus TCP Server C++ Node Started ====");

    // 1. 从 ROS 参数加载多组位姿
    load_pose_from_params();
    load_network_params();
    // 初始化 Modbus 寄存器
    if (!g_modbus_map)
    {
      g_modbus_map = modbus_mapping_new(0, 0, REGISTER_COUNT, 0);
      if (!g_modbus_map)
      {
        LOG_INFO("modbus_mapping_new failed");        
        return;
      }
    }

    g_modbus_map->tab_registers[1] = pose_list_.at(1).type; // 直接把第一组位姿的 type 写入寄存器1，方便调试查看
    write_float_to_modbus_registers(g_modbus_map, 10, pose_list_.at(1).d1); // 把第一组位姿的 d1 写入寄存器10-11
    float d1_val = read_float_from_modbus_registers(g_modbus_map, 10); // 从寄存器10-11读取 d1 值
    LOG_INFO("d1_val: %.2f", d1_val);

    

    // 启动 Modbus 监听线程
    modbus_server_thread_ = std::thread(&ModbusTcpServerCppNode::server_loop, this);

    // 上升沿检测定时器
    edge_timer_ = this->create_wall_timer(
      std::chrono::milliseconds(net_task_interval_ms_),
      std::bind(&ModbusTcpServerCppNode::detect_rising_edge, this)
    );

    // 创建服务客户端
    service_client_ = this->create_client<my_interfaces::srv::SrvMoveAxis>(
      "/my_interfaces/move_axis");
    wait_service_ready();
  }

  

  ModbusTcpServerCppNode::~ModbusTcpServerCppNode()
  {
    if (modbus_server_thread_.joinable())
    {
      modbus_server_thread_.join();
    }
    if (g_modbus_map)
    {
      modbus_mapping_free(g_modbus_map);
      g_modbus_map = nullptr;
    }
  }


   void ModbusTcpServerCppNode::load_pose_from_params()
{
  pose_list_.clear();
  LOG_INFO("Loading pose parameters...");

  // === 调试：列出所有参数 ===
  auto param_list = this->list_parameters({}, 1000);
  for (int i = 0; ; ++i)
  {
    std::string prefix = "pose_list." + std::to_string(i);

    // 先尝试声明再获取
    this->declare_parameter<int>(prefix + ".type", 0);
    this->declare_parameter<double>(prefix + ".d1", 0.0);
    this->declare_parameter<double>(prefix + ".d2", 0.0);
    this->declare_parameter<double>(prefix + ".d3", 0.0);
    this->declare_parameter<double>(prefix + ".d4", 0.0);
    this->declare_parameter<double>(prefix + ".d5", 0.0);
    this->declare_parameter<double>(prefix + ".d6", 0.0);
    this->declare_parameter<double>(prefix + ".d7", 0.0);
    this->declare_parameter<double>(prefix + ".d8", 0.0);

    // 读取值
    int type_val = this->get_parameter(prefix + ".type").as_int();

    // === 调试：打印值 ===
    LOG_INFO("pose_list.%d.type = %d", i, type_val);

    // 如果 type 为默认值 0，说明没读到 YAML 数据
    if (type_val == 0 && i >= 10)
    {
      LOG_INFO("No more poses found at index %d", i);
      break;
    }

    // 手动设置退出条件：最多读 100 组，且 type 为 0 时退出
    if (i >= 100)
    {
      break;
    }

    if (type_val == 0 && i > 0)
    {
      // 检查是否还有更多参数
      bool found = false;
      for (const auto& name : param_list.names)
      {
        std::string next_prefix = "pose_list." + std::to_string(i + 1);
        if (name.find(next_prefix) != std::string::npos)
        {
          found = true;
          break;
        }
      }
      if (!found) break;
    }

    PoseData pose{};
    pose.type = type_val;
    pose.d1 = static_cast<float>(this->get_parameter(prefix + ".d1").as_double());
    pose.d2 = static_cast<float>(this->get_parameter(prefix + ".d2").as_double());
    pose.d3 = static_cast<float>(this->get_parameter(prefix + ".d3").as_double());
    pose.d4 = static_cast<float>(this->get_parameter(prefix + ".d4").as_double());
    pose.d5 = static_cast<float>(this->get_parameter(prefix + ".d5").as_double());
    pose.d6 = static_cast<float>(this->get_parameter(prefix + ".d6").as_double());
    pose.d7 = static_cast<float>(this->get_parameter(prefix + ".d7").as_double());
    pose.d8 = static_cast<float>(this->get_parameter(prefix + ".d8").as_double());

    pose_list_.push_back(pose);

    LOG_INFO("Loaded pose[%d]: type=%d, d1=%.2f, d2=%.2f, d3=%.2f",
      i, pose.type, pose.d1, pose.d2, pose.d3);
  }

  LOG_INFO("Total loaded poses: %zu", pose_list_.size());
}

void ModbusTcpServerCppNode::load_network_params()
{
  LOG_INFO("Loading network parameters...");

  // 声明参数（YAML 中的值会自动覆盖默认值）
  this->declare_parameter<std::string>("network.ip", "192.168.20.230");
  this->declare_parameter<int>("network.port", 5020);
  this->declare_parameter<int>("network.max_clients", 5);
  this->declare_parameter<int>("network.task_interval", 100);

  // 读取参数
  net_ip_            = this->get_parameter("network.ip").as_string();
  net_port_          = static_cast<int>(this->get_parameter("network.port").as_int());
  net_max_clients_   = static_cast<int>(this->get_parameter("network.max_clients").as_int());
  net_task_interval_ms_ = static_cast<int>(this->get_parameter("network.task_interval").as_int());

  LOG_INFO("Network config: ip=%s, port=%d, max_clients=%d, task_interval=%dms",
    net_ip_.c_str(), net_port_, net_max_clients_, net_task_interval_ms_);
}



  // 等待服务端上线
void ModbusTcpServerCppNode::wait_service_ready()
{
  while (!service_client_->wait_for_service(1s))
  {
    if (!rclcpp::ok())
    {
      LOG_INFO("Node shut down while waiting for service!");
      return;
    }
    LOG_WARN("Waiting for service: /my_interfaces/move_axis...");
  }
  LOG_INFO("Successfully connected to service!");
}

  // 调用服务（匹配 SrvMoveAxis 字段）
  void ModbusTcpServerCppNode::invoke_service()
  {
    auto request = std::make_shared<my_interfaces::srv::SrvMoveAxis::Request>();
    // 填充请求参数，可根据业务修改
    request->id = 1;
    request->name = "test_axis";

    auto future = service_client_->async_send_request(request);
    auto ret_code = rclcpp::spin_until_future_complete(
      this->get_node_base_interface(), future, 1s);

    if (ret_code == rclcpp::FutureReturnCode::SUCCESS)
    {
      auto response = future.get();
      RCLCPP_INFO(
        this->get_logger(),
        "Service call OK | Response statues: %u",
        response->statues
      );
    }
    else
    {
      LOG_ERROR("Service call timeout or failed!");
    }
  }

  // 检测寄存器0 Bit0 上升沿 0->1
  void ModbusTcpServerCppNode::detect_rising_edge()
  {
    std::lock_guard<std::mutex> lock(g_reg_mutex);
    if (!g_modbus_map) return;
    

    uint16_t current_reg0 = g_modbus_map->tab_registers[0];
    uint8_t current_bit0  = static_cast<uint8_t>((current_reg0 >> 0) & 0x01);
    uint8_t last_bit0     = static_cast<uint8_t>((g_prev_reg0 >> 0) & 0x01);

    if (last_bit0 == 0 && current_bit0 == 1)
    {
      LOG_INFO("Rising Edge detected on reg0 bit0, trigger service!");
      invoke_service();
    }

    g_prev_reg0 = current_reg0;
  }

  // 单个客户端处理
  void ModbusTcpServerCppNode::handle_single_client(modbus_t* ctx, int client_fd)
  {
    uint8_t recv_buffer[MODBUS_TCP_MAX_ADU_LENGTH];
    bool client_online = true;

    // ========== 新增：获取客户端 IP + 端口 ==========
    struct sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN] = {0};
    uint16_t client_port = 0;

    // 获取对端 socket 地址
    if (getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len) == 0)
    {
        // 二进制IP转字符串
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        // 网络字节序转主机字节序端口
        client_port = ntohs(client_addr.sin_port);
        LOG_INFO("New client connected, fd: %d | IP: %s | Port: %d",
                 client_fd, client_ip, client_port);
    }
    else
    {
        LOG_WARN("New client connected, fd: %d | Get client IP failed, errno: %d",
                 client_fd, errno);
        LOG_INFO("New client connected, fd: %d", client_fd);
    }
    

    while (client_online)
    {
      try
      {
        int recv_len = modbus_receive(ctx, recv_buffer);
        if (recv_len <= 0)
        {
          LOG_WARN(
            "Client fd:%d disconnected | errno: %d | info: %s",
            client_fd, errno, strerror(errno));
          client_online = false;
          break;
        }

        std::lock_guard<std::mutex> lock(g_reg_mutex);
        modbus_reply(ctx, recv_buffer, recv_len, g_modbus_map);
      }
      catch (...)
      {
        LOG_ERROR("Client fd:%d communication exception", client_fd);
        client_online = false;
        break;
      }
    }

    close(client_fd);
    LOG_INFO("Client fd:%d has been closed", client_fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
  }

  // Modbus TCP 监听主循环
  void ModbusTcpServerCppNode::server_loop()
  {
    modbus_t* ctx = modbus_new_tcp(net_ip_.c_str(), net_port_);
    if (!ctx)
    {
      LOG_ERROR("Create modbus tcp context failed!");

      return;
    }
    modbus_set_debug(ctx, false);

    int server_fd = modbus_tcp_listen(ctx, MAX_CLIENT_NUM);
    if (server_fd == -1)
    {
      LOG_ERROR("Listen port %d failed!", MODBUS_TCP_PORT);
      modbus_free(ctx);
      return;
    }

    LOG_INFO("Modbus TCP Listen: %s:%d | Max Client: %d",
      net_ip_.c_str(), net_port_, net_max_clients_);

    while (true)
    {
      int client_fd = modbus_tcp_accept(ctx, &server_fd);
      if (client_fd == -1)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
        continue;
      }
      handle_single_client(ctx, client_fd);
    }

    close(server_fd);
    modbus_free(ctx);
  }


  float ModbusTcpServerCppNode::read_float_from_modbus_registers(modbus_mapping_t* mb_map, int base_addr)
{
    uint16_t reg_high = mb_map->tab_registers[base_addr];
    uint16_t reg_low  = mb_map->tab_registers[base_addr + 1];

    union {
        float f;
        uint8_t bytes[4];
    } u;

    // 大端模式还原
    u.bytes[1] = (reg_high >> 8) & 0xFF;
    u.bytes[0] = reg_high & 0xFF;
    u.bytes[3] = (reg_low >> 8) & 0xFF;
    u.bytes[2] = reg_low & 0xFF;

    return u.f;
}


// 把 float 写入 Modbus 寄存器（大端字序，标准Modbus格式：高字在前，高字节在前）
void ModbusTcpServerCppNode::write_float_to_modbus_registers(modbus_mapping_t* mb_map, int base_addr, float value)
{
    union {
        float f;
        uint8_t bytes[4];
    } u;
    u.f = value;

    // 大端模式：
    // 寄存器1（base_addr）：高16位（u.bytes[0], u.bytes[1]）
    // 寄存器2（base_addr+1）：低16位（u.bytes[2], u.bytes[3]）
    uint16_t reg_high = (static_cast<uint16_t>(u.bytes[1]) << 8) | u.bytes[0];
    uint16_t reg_low  = (static_cast<uint16_t>(u.bytes[3]) << 8) | u.bytes[2];

    mb_map->tab_registers[base_addr ]     = reg_high;
    mb_map->tab_registers[base_addr +1] = reg_low;
}



}