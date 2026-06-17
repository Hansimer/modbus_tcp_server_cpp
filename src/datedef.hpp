
#pragma once

#include <string>
#include <chrono>   
#include <map>
#include <vector>

#define REGINDEX_SET_MODE_1 0x0001 //机械臂模式
#define REGINDEX_SWITCH_MODE_2 0x0002 //切换模式 从00 切换到FF



using namespace std;

namespace modbus_tcp_server_cpp
{
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

        
        struct joint_state_
        {
            float rad_pose_current;
            float rad_vel_current;
            int8_t mode;
            int8_t statues;
            int8_t error;
        };

        struct pose_
        {
            float x;
            float y;
            float z;
            float rx;
            float ry;
            float rz;
        };

        struct robot_info
        {
            joint_state_ joint_neck; //脖子
            joint_state_ joint_rotate; //旋转
            joint_state_ joint_lift; //提升
            joint_state_ joint_arm_left[8]; //机械臂左边8个关节
            joint_state_ joint_arm_right[8]; //机械臂右边8个关节
            pose_ pose_neck;
            pose_ pose_rotate;
            pose_ pose_lift;
            pose_ pose_arm_left;
            pose_ pose_arm_right;

        };

        //原始数据
        struct action_data_
        {
            float d1;
            float d2;
            float d3;
            float d4;
            float d5;
            float d6;
            float d7;
            float d8;
            float d9;
            float d10;
            float d11;
            float d12;
            float d13;
            float d14;
        };

        //运动类型单臂节点运动
        struct single_arm_joint_
        {
            float joint1_rad;
            float joint2_rad;
            float joint3_rad;
            float joint4_rad;
            float joint5_rad;
            float joint6_rad;
            float joint7_rad;
        };

        //运动类型双臂节点运动
        struct dual_arm_joint_
        {
            float armL_joint1_rad;
            float armL_joint2_rad;
            float armL_joint3_rad;
            float armL_joint4_rad;
            float armL_joint5_rad;
            float armL_joint6_rad;
            float armL_joint7_rad;
            float armR_joint1_rad;
            float armR_joint2_rad;
            float armR_joint3_rad;
            float armR_joint4_rad;
            float armR_joint5_rad;
            float armR_joint6_rad;
            float armR_joint7_rad;
            
        };

        //运动类型等待时间
        struct wait_delay_
        {
            float delay_s;
        };

        //运动类型单臂点对点运动
        struct single_arm_PizP2P_
        {
            float x;
            float y;
            float z;
            float rx;
            float ry;
            float rz;
        };
        //运动类型双臂点对点运动
        struct dual_arm_PizP2P_
        {
            float armL_x;
            float armL_y;
            float armL_z;
            float armL_rx;
            float armL_ry;
            float armL_rz;
            float armR_x;
            float armR_y;
            float armR_z;
            float armR_rx;
            float armR_ry;
            float armR_rz;
        };
        //运动类型单臂线运动
        struct single_arm_Pizline_
        {
            float x;
            float y;
            float z;
            float rx;
            float ry;
            float rz;
        };
        //运动类型双臂线运动
        struct dual_arm_Pizline_
        {
            float armL_x;
            float armL_y;
            float armL_z;
            float armL_rx;
            float armL_ry;
            float armL_rz;
            float armR_x;
            float armR_y;
            float armR_z;
            float armR_rx;
            float armR_ry;
            float armR_rz;
        };
        //运动类型单臂轴运动
        struct Axis_servo_pose_
        {
            float dis;   
        };


        struct action_info_
        {
            std::int16_t index;
            std::int16_t sub_index;
            std::uint8_t type;

            union Data
            {
                action_data_ action_data;
                single_arm_joint_ single_arm_joint;
                dual_arm_joint_ dual_arm_joint;
                wait_delay_ wait_delay;
                single_arm_PizP2P_ single_arm_PizP2P;
                dual_arm_PizP2P_ dual_arm_PizP2P;
                single_arm_Pizline_ single_arm_Pizline;
                dual_arm_Pizline_ dual_arm_Pizline;
                Axis_servo_pose_ Axis_servo_pose;
            } info_;
        };


        struct dev_params_info_
        {
            std::string mode;
            float speed_convert_factor;
        };

        struct array_actions_info_
        {
            std::vector<action_info_> actions;
        };

///////////////////////////////////////////////fb块相关
        struct fb_delay_
        {
            bool in_execut;//执行
            uint16_t in_time_set;//一轮询周期为基数

            bool out_busy;//正在执行
            bool out_err;//报错
            bool out_done;//完成


            bool temp_execut_last;
            uint16_t temp_time_cnt;//一轮询周期为基数

        };

        /// @brief 获取机器人模型
        struct fb_getbotmodel_
        {
            bool in_execut;//执行
            string str_checkmodel;//验证的运动模型

            bool out_busy;//正在执行
            bool out_err;//报错
            bool out_done;//完成


            bool temp_execut_last;           

        };

        /// @brief 获取机器人模型
        // struct fb_getbotmodel_
        // {
        //     bool in_execut;//执行
        //     string str_checkmodel;//验证的运动模型

        //     bool out_busy;//正在执行
        //     bool out_err;//报错
        //     bool out_done;//完成


        //     bool temp_execut_last;           

        // };



}
