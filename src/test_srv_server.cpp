#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <my_interfaces/srv/srv_move_axis.hpp>
#include <ethercat_control/srv/get_active_robot_model.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <string>
#include <sstream>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <memory>
#include <functional>
#include <robot_controller/action/arm_motion.hpp>
#include <my_interfaces/action/act_motor_cmd.hpp>

using namespace std::chrono_literals;
namespace modbus_tcp_server_cpp
{
  using ArmMotion = robot_controller::action::ArmMotion;
  using ArmGoalHandle = rclcpp_action::ClientGoalHandle<ArmMotion>;

  using MotorCmd = my_interfaces::action::ActMotorCmd ;
  using GoalHandleMotor = rclcpp_action::ClientGoalHandle<MotorCmd>;
  #define STR_ACTION_LIFTMOTOR "/motor/lift_motioncmd"

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

    // 创建订阅器：话题名、队列深度、回调函数
        sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "/joint_states",
            10,  // QoS队列深度，默认10足够
            std::bind(&TestSrvServer::joint_cb, this, std::placeholders::_1)
        );
        RCLCPP_INFO(this->get_logger(), "已订阅 /joint_states");

        // 订阅 /joint_state/lift_servo 话题
        lift_servo_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "/joint_state/lift_servo",
            10,
            std::bind(&TestSrvServer::lift_servo_cb, this, std::placeholders::_1)
        );
        RCLCPP_INFO(this->get_logger(), "已订阅 /joint_state/lift_servo");

     service_client_ = this->create_client<ethercat_control::srv::GetActiveRobotModel>(
      "/raybot_dual_arms/get_active_model");

    call_service_timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),
      std::bind(&TestSrvServer::call_service_and_get_result, this)
    );

   
      // 2. 调用服务并获取返回值
    // call_service_and_get_result();
    RCLCPP_INFO(this->get_logger(), "Test Service Server started, waiting for request...");
  }

private:
  rclcpp::Service<my_interfaces::srv::SrvMoveAxis>::SharedPtr service_;
  rclcpp::Client<ethercat_control::srv::GetActiveRobotModel>::SharedPtr service_client_;
  rclcpp::TimerBase::SharedPtr call_service_timer_; // 定时器
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr lift_servo_sub_;
  rclcpp_action::Client<ArmMotion>::SharedPtr arm_action_client_;

  rclcpp_action::Client<MotorCmd>::SharedPtr motor_client_;

  bool motion_finished_ = false;
  bool motion_success_ = false;
  std::string active_robot_name_;
  std::string active_groups_;
  //服务回调函数
 void call_service_and_get_result() {
    // 定时器只调用一次，调用后销毁
    call_service_timer_->cancel();
   RCLCPP_INFO(this->get_logger(), "action1 client test");
    // 等待服务端上线（最多等200秒）
    if (!service_client_->wait_for_service(std::chrono::seconds(3))) {
      RCLCPP_ERROR(this->get_logger(), "服务端未上线，无法调用");
     // return;
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
          active_robot_name_ = response->robot_name;
          active_groups_ = response->enabled_groups;
        } else {
          RCLCPP_ERROR(this->get_logger(), "服务调用失败：%s", response->message.c_str());
        }
      }
    );

     arm_action_client_ = rclcpp_action::create_client<ArmMotion>(
    shared_from_this(),
    "/arm_motion_controller/arm_motion"
    );

     if (!arm_action_client_->wait_for_action_server(std::chrono::seconds(2))) {
        RCLCPP_ERROR(this->get_logger(), "ACTION未上线,无法调用");
     // return;
    }

   std::vector<std::string> joint_name1 = {
            "right_arm_1_joint",
            "right_arm_2_joint",
            "right_arm_3_joint",
            "right_arm_4_joint",
            "right_arm_5_joint",
            "right_arm_6_joint",
            "right_arm_7_joint",
            "left_arm_1_joint",
            "left_arm_2_joint",
            "left_arm_3_joint",
            "left_arm_4_joint",
            "left_arm_5_joint",
            "left_arm_6_joint",
            "left_arm_7_joint"
        };

        std::vector<double> pos1 = {
            0.54978830175813809,
            0.028953887371341476,
            1.5500396492588189,
            0.00033555829734998401,
            0.009203884727313847,
            -0.13005280867292951,
            -0.095298556447395461,
            -0.47222639817067036,
            -0.12693691019753681,
            -1.5114025081639495,
            0.091223919979574228,
            -0.1303883669702795,
            -0.12060923944750854,
            0.013038836697027951
        };

        std::vector<std::string> joint_name2 = {
            "right_arm_1_joint",
            "right_arm_2_joint",
            "right_arm_3_joint",
            "right_arm_4_joint",
            "right_arm_5_joint",
            "right_arm_6_joint",
            "right_arm_7_joint",
            "left_arm_1_joint",
            "left_arm_2_joint",
            "left_arm_3_joint",
            "left_arm_4_joint",
            "left_arm_5_joint",
            "left_arm_6_joint",
            "left_arm_7_joint"
        };

        std::vector<double> pos2 = {
            -0.25238777650680938,
            0.02914563496982718,
            1.5499437754595762,
            0.00043143209659283658,
            0.0092997585265566993,
            -0.12995693487368667,
            -0.09486712435080262,
            0.25967418524926622,
            -0.12722453159526537,
            -1.511690129561678,
            0.091271856879195645,
            -0.13048424076952236,
            -0.12051336564826569,
            0.012942962897785097
        };

        std::string group_name ="dual_arms";
        bool ex =true;
        double timout = 10.0;
        send_arm_joint_motion(group_name, joint_name1, pos1, ex, timout);

        ////motor lift
        motor_client_ = rclcpp_action::create_client<MotorCmd>(shared_from_this(), STR_ACTION_LIFTMOTOR);
        double target_pos=1000; double vel=1.0; double acc=0.5; double timeout=10.0;
        send_motor_pos_cmd(target_pos,vel,acc,timeout);

        

  }

  // servo 下发点位运动指令
    bool send_motor_pos_cmd(double target_pos, double vel=1.0, double acc=0.5, double timeout=10.0)
    {
      RCLCPP_INFO(this->get_logger(), "action12 client test");
        // 等待服务端上线
        if (!motor_client_->wait_for_action_server(std::chrono::seconds(5))) {
            RCLCPP_ERROR(get_logger(), "伺服Action服务未启动");
            return false;
        }

        // 填充运动目标
        MotorCmd::Goal goal;
        goal.mode = "pos";
        goal.target_pos = target_pos;
        goal.target_vel = vel;
        goal.acc = acc;
        goal.execute = true;
        goal.timeout = timeout;

        // 配置回调：反馈、结果
        rclcpp_action::Client<MotorCmd>::SendGoalOptions opt;
        opt.feedback_callback = std::bind(&TestSrvServer::feedback_cb, this, std::placeholders::_1, std::placeholders::_2);
        opt.result_callback = std::bind(&TestSrvServer::result_cb, this, std::placeholders::_1);

        // 异步发送目标
        auto future = motor_client_->async_send_goal(goal, opt);
        return true;
    }


// 实时运动反馈回调
    void feedback_cb(std::shared_ptr<GoalHandleMotor>, const std::shared_ptr<const MotorCmd::Feedback> fb)
    {
        std::cout << "[伺服反馈] 位置:" << std::fixed << std::setprecision(3)
                  << fb->current_pos << " 进度:" << fb->progress << std::endl;
    }

    // 运动结束结果回调
    void result_cb(const GoalHandleMotor::WrappedResult & res)
    {
        switch (res.code)
        {
        case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(get_logger(), "运动成功，终点位置:%.3f", res.result->final_pos);
            break;
        case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_WARN(get_logger(), "运动被取消");
            break;
        case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(get_logger(), "运动异常中止: %s", res.result->msg.c_str());
            break;
        default:
            RCLCPP_ERROR(get_logger(), "未知运动状态");
        }
    }



/////////////////////////////////arm action

  bool send_arm_joint_motion(
    const std::string & group,
    const std::vector<std::string> & j_names,
    std::vector<double> & j_pos,
    bool execute,
    double timeout = 5.0
)
{
  // 填充Goal
    ArmMotion::Goal goal;
    goal.group = group;
    goal.goal_type = "joints";
    goal.execute = execute;
    goal.joint_names = j_names;
    goal.joint_positions = j_pos;

    // 生成唯一command_id
    auto ts = std::time(nullptr);
    std::stringstream cmd_ss;
    cmd_ss << "motion_test_" << ts;
    goal.command_id = cmd_ss.str();

    // 重置状态标记
    motion_finished_ = false;
    motion_success_ = false;

    // 1. 构造发送配置，绑定反馈、结果回调
    rclcpp_action::Client<ArmMotion>::SendGoalOptions send_opt;
    send_opt.feedback_callback = std::bind(
        &TestSrvServer::feedback_callback,
        this,
        std::placeholders::_1,
        std::placeholders::_2
    );
    // 绑定运动完成结果回调（核心：异步不阻塞）
    send_opt.result_callback = std::bind(
        &TestSrvServer::motion_result_callback,
        this,
        std::placeholders::_1
    );

    // 异步发送goal，不阻塞当前函数
    arm_action_client_->async_send_goal(goal, send_opt);
    RCLCPP_INFO(get_logger(), "已发送运动目标，等待action服务端执行...");

    // 仅返回发送成功，实际结果由异步回调处理
    return true;
}

// 新增运动结果回调函数（异步接收结果，不占用当前线程）
void motion_result_callback(const ArmGoalHandle::WrappedResult & wrapped_result)
{
    motion_finished_ = true;
    if (wrapped_result.code == rclcpp_action::ResultCode::SUCCEEDED)
    {
        RCLCPP_INFO(get_logger(), "运动执行成功");
        motion_success_ = true;
    }
    else
    {
        RCLCPP_ERROR(get_logger(), "运动执行失败，错误码：%d", static_cast<int>(wrapped_result.code));
        motion_success_ = false;
    }
}

void feedback_callback(
    std::shared_ptr<ArmGoalHandle>,
    const std::shared_ptr<const ArmMotion::Feedback> feedback
)
{
    std::cout << "[运动反馈] 阶段:" << feedback->stage
              << " 进度:" << std::fixed << std::setprecision(3)
              << feedback->progress << std::endl;
}


  void service_callback(
    const std::shared_ptr<my_interfaces::srv::SrvMoveAxis::Request> request,
    std::shared_ptr<my_interfaces::srv::SrvMoveAxis::Response> response)
  {
    // 打印收到的请求
    RCLCPP_INFO(
      this->get_logger(),
      "Recv request -> id: %u, name: %s",
      static_cast<unsigned int>(request->id),
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

    RCLCPP_INFO(this->get_logger(), "Send response -> statues: %u", static_cast<unsigned int>(response->statues));
  }

  void joint_cb(const sensor_msgs::msg::JointState::SharedPtr msg)
    {
        size_t joint_cnt = std::min(msg->name.size(), msg->position.size());
       // RCLCPP_INFO(this->get_logger(), "==== 收到关节状态，共 %zu 个关节 ====", joint_cnt);

        // // 遍历打印每个关节名称+角度
        // for (size_t i = 0; i < joint_cnt; i++)
        // {
        //     std::cout << std::fixed << std::setprecision(6)
        //               << msg->name[i] << " : " << msg->position[i] << " rad\n";
        // }
    }

  // /joint_state/lift_servo 订阅回调
  void lift_servo_cb(const sensor_msgs::msg::JointState::SharedPtr msg)
    {
        size_t joint_cnt = std::min(msg->name.size(), msg->position.size());
        RCLCPP_INFO(this->get_logger(), "==== 收到 lift_servo 关节状态，共 %zu 个关节 ====", joint_cnt);

        for (size_t i = 0; i < joint_cnt; i++)
        {
            RCLCPP_INFO(this->get_logger(), "  %s: %.6f rad",
                msg->name[i].c_str(), msg->position[i]);
        }
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