
#pragma once

#include <string>
#include <chrono>   

#define REGINDEX_SET_MODE_1 0x0001 //机械臂模式
#define REGINDEX_SWITCH_MODE_2 0x0002 //切换模式 从00 切换到FF


struct joint_state_
{
    float rad_pose;
    float rad_vel;
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
    joint_state_ joint_neck;
    joint_state_ joint_arm_left[8];
    joint_state_ joint_arm_right[8];
    pose_ pose_current;

};
