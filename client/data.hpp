//
// Created by ldx on 24-8-27.
/// 用于记录所有存放文件的备份信息

#ifndef TEST_DATA_HPP
#define TEST_DATA_HPP

#include <unordered_map>
#include <pthread.h>
#include "util.hpp"

namespace cloud {
    class DataManager {
    private:
        std::string _backup_file;
        std::unordered_map<std::string, std::string> _table; // [filename,id]
    public:
        DataManager(const std::string &backup_file)
            :_backup_file(backup_file)
        {
            InitLoad();
        }
        ~DataManager() {}

    public:
        bool Storage() {
            // 获取所有备份信息
            std::stringstream ss;
            for(auto data : _table){
                ss << data.first << " " << data.second << std::endl;
            }
            FileUtil fu(_backup_file);
            fu.SetContent(ss.str());
            return true;
        }
        void Split(const std::string &data, const std::string &sep, std::vector<std::string> &string_vec){
            size_t pos = 0, idx = 0;
            while(true){
                pos = data.find(sep,idx); // 从idx开始找
                if(pos == std::string::npos){
                    break;
                }
                if(pos == idx){
                    idx = pos + sep.size();
                    continue;
                }
                std::string sub = data.substr(idx,pos-idx);
                string_vec.push_back(sub);
                idx = pos + sep.size();
            }
            if(idx < data.size()){
                string_vec.push_back(data.substr(idx));
            }
        }
        bool InitLoad() {
            // 读取数据
            FileUtil fu(_backup_file);
            std::string body;
            fu.GetContent(&body);

            // 数据解析
            std::vector<std::string> string_vec;
            Split(body,"\n",string_vec);
            return true;
        }

        bool Insert(const std::string &key, const std::string &val) {
            _table[key] = val;
            Storage();
            return true;
        }

        bool Update(const std::string &key, const std::string &val) {
            _table[key] = val;
            Storage();
            return true;
        }

        bool GetByKey(const std::string &key, std::string &val){
            auto it = _table.find(key);
            if(it == _table.end()){
                return false;
            }
            val = it->second;
            return true;
        }

    };
}


#endif //TEST_DATA_HPP
