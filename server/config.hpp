//
// Created by ldx on 24-8-26.
//

#ifndef TEST_CONFIG_HPP
#define TEST_CONFIG_HPP

#include <mutex>
#include "util.hpp"

namespace cloud {
#define CONFIG_FILE "../cloud.json"

    class Config {
    private:
        Config() { ReadConfigFile(); }

        static Config *_instance;
        static std::mutex _mutex;

    private:
        int _hot_time; //热点时间
        int _server_port; //监听端口
        std::string _server_ip; //监听地址
        std::string _download_prefix; //下载文件前缀
        std::string _packfile_suffix; //压缩文件后缀
        std::string _pack_dir; //上传文件存放路径
        std::string _back_dir; //压缩后，压缩包存放路径
        std::string _backup_file; //备份信息文件

    private:
        bool ReadConfigFile() {
            FileUtil fu(CONFIG_FILE);
            std::string body;
            if (fu.GetContent(&body) == false) {
                std::cerr << "load config file failed!\n";
                exit(1);
            }
            Json::Value root;
            if (JsonUtil::UnSerialize(body, &root) == false) {
                std::cerr << "parse config file failed!\n";
                exit(1);
            }
            _hot_time = root["hot_time"].asInt();
            _server_port = root["server_port"].asInt();
            _server_ip = root["server_ip"].asString();
            _download_prefix = root["download_prefix"].asString();
            _packfile_suffix = root["packfile_suffix"].asString();
            _back_dir = root["back_dir"].asString();
            _pack_dir = root["pack_dir"].asString();
            _backup_file = root["backup_file"].asString();
        }

    public:
        static Config* GetInstance(){
            if(_instance == nullptr){
                _mutex.lock();
                _instance = new Config();
                _mutex.unlock();
            }
            return _instance;
        }
        int GetHotTime(){
            return _hot_time;
        }
        int GetServerPort(){
            return _server_port;
        }
        std::string GetServerIp(){
            return _server_ip;
        }
        std::string GetDownloadPrefix() {
            return _download_prefix;
        }
        std::string GetPackFileSuffix() {
            return _packfile_suffix;
        }
        std::string GetPackDir() {
            return _pack_dir;
        }
        std::string GetBackDir() {
            return _back_dir;
        }
        std::string GetBackupFile() {
            return _backup_file;
        }
    };

    // 静态类内成员，需要在类外初始化
    Config *Config::_instance = nullptr;
    std::mutex Config::_mutex;
}

#endif //TEST_CONFIG_HPP
