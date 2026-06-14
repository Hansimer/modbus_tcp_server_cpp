
#pragma once

#include <string>
#include <chrono>   

#define REGINDEX_SET_MODE_1 0x0001 //机械臂模式
#define REGINDEX_SWITCH_MODE_2 0x0002 //切换模式 从00 切换到FF


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
