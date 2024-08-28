//
// Created by ldx on 24-8-28.
//

#ifndef TEST_CLIENT_HPP
#define TEST_CLIENT_HPP
#include "httplib.h"
#include "data.hpp"
#include "util.hpp"
#include <unistd.h>

namespace cloud{
#define SERVER_ADDR "172.18.23.131"
#define SERVER_PORT 9090
    class Backup{
    private:
        std::string _back_dir;
        DataManager *_dataManager;
    public:
        Backup(const std::string  &back_dir, const std::string &back_file)
            : _back_dir(back_dir)
        {
            fs::create_directories(back_dir);
            std::ofstream file(back_file);
            file.close();
            _dataManager = new DataManager(back_file);
        }
        std::string GetFileIdentifier(const std::string &filename){
            // a.txt-fsize-mtime
            FileUtil fu(filename);
            std::stringstream ss;
            ss << fu.FileName() << "-" << fu.FileSize() << "-" << fu.LastMTime();
            return ss.str();
        }
        bool Upload(const std::string &filename){
            // 获取文件数据
            FileUtil fu(filename);
            std::string body;
            fu.GetContent(&body);

            // 搭建http客户端上传数据
            httplib::Client client(SERVER_ADDR, SERVER_PORT);
            httplib::MultipartFormData item;
            item.content = body;
            item.name = "file";
            item.content_type = "application/octet-stream";
            item.filename = fu.FileName();
            httplib::MultipartFormDataItems items;
            items.push_back(item);

            auto res = client.Post("/upload", items);
            if(!res || res->status != 200){
                return false;
            }
            return true;
        }

        bool IsNeedUpload(const std::string &filename){
            // 判断上传文件是否是新增文件，或者不是新增但是被修改过的文件
            std::string id;
            if(_dataManager->GetByKey(filename,id) != false){
                // 有历史记录
                std::string new_id = GetFileIdentifier(filename);
                if(new_id == id){
                    //文件没有被修改，不需要重新上传
                    return false;
                }
            }
            // 没有历史记录
            FileUtil fu(filename);
            if(time(nullptr) - fu.LastMTime() < 3){
                // 3秒钟前刚修改过，认为文件还在修改中
                return false;
            }
            return true;
        }

        bool RunModule(){
            while(true){
                // 遍历获取指定文件夹中的所有文件
                FileUtil fu(_back_dir);
                std::vector<std::string> files;
                fu.ScanDirectory(&files);
                // 判断文件是否需要上传
                for(const auto &file : files){
                    if(IsNeedUpload(file) == false){
                        continue;
                    }
                    if(Upload(file) == true){
                        _dataManager->Insert(file, GetFileIdentifier(file));// 增加文件备份信息
                        std::cout << file << "upload success!\n";
                    }
                }
                sleep(1);
                std::cout << "---------------------------loop end-----------------------------" << std::endl;
            }
        }
    };
}
#endif //TEST_CLIENT_HPP
