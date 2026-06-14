#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <tinyxml2.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "json.hpp"  // JSON解析（ROS2自带）
#include "rclcpp/rclcpp.hpp"
#include "../log.hpp"

using namespace tinyxml2;
using json = nlohmann::json;

namespace modbus_tcp_server_cpp
{
// ====================== PDO 配置结构体 ======================
    struct ActionEntry_
    {
        std::uint16_t index;
        std::uint8_t sub_index;
        std::uint8_t type;  
        //0-延时 d1：延时时间（s）
        // 1 提升伺服 d1为提升高度
        // 2 旋转伺服 d1为旋转角度（度）
        // 3 激发视觉
        // 11 左臂关节运动
        // 12 右臂关节运动
        // 13 双臂关节运动
        // 21 左臂ptp运动
        // 22 右臂ptp运动
        // 23 双臂ptp运动
        // 31 左臂line运动
        // 32 右臂line运动
        // 33 双臂line运动
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
    // 解析后的PDO列表
    // std::vector<ActionEntry_> pdo_entries;


    // ====================== 设备配置 ======================
    struct DeviceConfig_component
    {
        std::string device_name;
        std::string running_name;
        std::map<std::string, std::string> params; //keyval_map

        // 快速获取
        std::string getString(const std::string& key) {
            return params.count(key) ? params[key] : "";
        }

        int getUint16(const std::string& key) {
            uint32_t value= params.count(key) ? std::stoul(params[key]) : 0;
            return static_cast<uint16_t>(value) ;
        }

        int getUint32(const std::string& key) {
            uint32_t value= params.count(key) ? std::stoul(params[key]) : 0;
            return static_cast<uint32_t>(value) ;
        }

        int getUint8(const std::string& key) {
            uint32_t value= params.count(key) ? std::stoul(params[key]) : 0;
            return static_cast<uint8_t>(value) ;
        }


        double getDouble(const std::string& key) {
            return params.count(key) ? std::stod(params[key]) : 0.0;
        }   
    };

    // ====================== 设备配置 ======================
    struct DeviceConfig_action
    {
        std::map<std::string, std::string> params; //keyval_map        
        std::map<std::string, std::vector<ActionEntry_> > action_entries_map; //keyval_map
        // 快速获取
        std::string getString(const std::string& key) {
            return params.count(key) ? params[key] : "";
        }

        int getUint16(const std::string& key) {
            uint32_t value= params.count(key) ? std::stoul(params[key]) : 0;
            return static_cast<uint16_t>(value) ;
        }

        int getUint32(const std::string& key) {
            uint32_t value= params.count(key) ? std::stoul(params[key]) : 0;
            return static_cast<uint32_t>(value) ;
        }

        int getUint8(const std::string& key) {
            uint32_t value= params.count(key) ? std::stoul(params[key]) : 0;
            return static_cast<uint8_t>(value) ;
        }


        double getDouble(const std::string& key) {
            return params.count(key) ? std::stod(params[key]) : 0.0;
        }

        // ====================== 解析 JSON PDO ======================
        void parsePdoJson(std::string key)
    {
        std::string json_str = getString(key);
        if (json_str.empty())
        {
            LOG_WARN("key [%s] json string is empty", key.c_str());
            return;
        }

        try
        {
            nlohmann::json j = nlohmann::json::parse(json_str);
            for (auto& item : j)
            {
            ActionEntry_ entry{}; // 零初始化

            // 十进制解析，删除第三个参数 16
            entry.index     = static_cast<uint16_t>(std::stoul(item["index"].get<std::string>()));
            entry.sub_index = static_cast<uint8_t>(std::stoul(item["sub_index"].get<std::string>()));
            entry.type      = static_cast<uint8_t>(std::stoul(item["type"].get<std::string>()));

            // 批量解析 d1~d14，简化代码
            auto get_float = [&](const char* field) -> float {
                return static_cast<float>(std::stod(item[field].get<std::string>()));
            };
            entry.d1  = get_float("d1");
            entry.d2  = get_float("d2");
            entry.d3  = get_float("d3");
            entry.d4  = get_float("d4");
            entry.d5  = get_float("d5");
            entry.d6  = get_float("d6");
            entry.d7  = get_float("d7");
            entry.d8  = get_float("d8");
            entry.d9  = get_float("d9");
            entry.d10 = get_float("d10");
            entry.d11 = get_float("d11");
            entry.d12 = get_float("d12");
            entry.d13 = get_float("d13");
            entry.d14 = get_float("d14");

            // 存入 map
            action_entries_map[key].push_back(entry);
            LOG_INFO("Parse action ok, index=%u type=%u", entry.index, entry.type);
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("PDO JSON parse failed: %s", e.what());
    }
}
        
    };

    // ====================== 全局配置解析 ======================
    class ConfigParser
    {
    public:
        std::vector<DeviceConfig_component> devices_cfg;
        std::vector<DeviceConfig_action> actions;

    public:
        bool loadFile(const std::string& xml_path)
        {
            XMLDocument doc;
            if (doc.LoadFile(xml_path.c_str()) != XML_SUCCESS)
                return false;

            XMLElement* root = doc.FirstChildElement("root");
            if (!root) return false;

            XMLElement* item_root = root->FirstChildElement("item");
            if (!item_root) return false;



            XMLElement* components = item_root->FirstChildElement("components");
            if (!components) return false;

            // 遍历设备
            XMLElement* dev = components->FirstChildElement("item");
            while (dev)
            {
                DeviceConfig_component cfg;
                cfg.device_name = dev->Attribute("name");
                if (dev->Attribute("running_name"))
                    cfg.running_name = dev->Attribute("running_name");
                // 读取 keyval_map
                XMLElement* keyval = dev->FirstChildElement("keyval_map");
                if (keyval)
                {
                    XMLElement* param = keyval->FirstChildElement("item");
                    while (param)
                    {
                        const char* key = param->Attribute("key");
                        const char* val = param->Attribute("value");
                        if (key && val) cfg.params[key] = val;
                        param = param->NextSiblingElement("item");
                    }
                }

                devices_cfg.push_back(cfg);
                dev = dev->NextSiblingElement("item");
            }

            XMLElement* xmlaction = item_root->FirstChildElement("actions");
            if (!xmlaction) return false;
            // 遍历设备
            XMLElement* xmlactions = xmlaction->FirstChildElement("item");
            while (xmlactions)
            {
                DeviceConfig_action action;
                const char* key = xmlactions->Attribute("key");
                const char* val = xmlactions->Attribute("value");
                if (key && val) action.params[key] = val;                
                    action.parsePdoJson("init_action");
                    actions.push_back(action);
                xmlactions = xmlactions->NextSiblingElement("item");
            }
            return true;
        }


    };
}


#endif