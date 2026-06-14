import launch
import launch_ros
from launch import LaunchDescription
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_share = get_package_share_directory('modbus_tcp_server_cpp')
    params_file = os.path.join(pkg_share, 'config', 'pose_config.yaml')
    return launch.LaunchDescription([
        launch_ros.actions.Node(
            package='modbus_tcp_server_cpp',
            executable='modbus_tcp_server_cpp_node',
            name='modbus_tcp_server_cpp_node',
            output='both',
            parameters=[params_file]
        )
    ])