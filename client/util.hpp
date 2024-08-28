//
// Created by ldx on 24-8-23.
//
#pragma once
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <jsoncpp/json/json.h>
namespace cloud{
    namespace fs = boost::filesystem;
    class FileUtil{
    private:
        std::string _filename;
    public:
        FileUtil(const std::string  &filename):_filename(filename){}
        int64_t FileSize(){
            struct stat st;
            if(stat(_filename.c_str(), &st) < 0){
                std::cerr << "get file stat failed!" << std::endl;
                return -1;
            }
            return st.st_size;
        }
        bool GetContent(std::string *body){
            size_t fsize = FileSize();
            return GetPosLen(body,0,fsize);
        }
        bool GetPosLen(std::string *body, size_t pos, size_t len){
            size_t fsize = FileSize();
            if(pos + len > fsize){
                std::cout << "get file len is error " << std::endl;
                return false;
            }
            std::ifstream ifs;
            ifs.open(_filename,std::ios::binary);
            if(ifs.is_open() == false){
                std::cout << "read open file failed! \n";
                return false;
            }
            ifs.seekg(pos, std::ios::beg);
            body->resize(len);
            ifs.read(&(*body)[0],len);
            if(ifs.good() == false){ //检测文件流状态是否良好
                std::cout << "get file content failed\n";
                return false;
            }
            ifs.close();
            return true;
        }
        bool Remove(){
            if(Exists() == false){
                return true;
            }
            remove(_filename.c_str()); //删除文件
            return true;
        }
        bool Exists(){
            fs::path p(_filename);
            return fs::exists(p);
        }
        time_t LastMTime(){
            //最后的修改时间
            struct stat st;
            if(stat(_filename.c_str(), &st) < 0){
                std::cout << "get file stat failed\n";
                return -1;
            }
            return st.st_mtime;
        }
        time_t LastATime(){
            //最后的访问时间
            struct stat st;
            if(stat(_filename.c_str(), &st) < 0){
                std::cout << "get file stat failed\n";
                return -1;
            }
            return st.st_atime;
        }
        std::string FileName(){
            // /home/ldx/text.txt
            size_t pos = _filename.find_last_of("/"); // 找到最后一个/
            if(pos == std::string::npos){
                return _filename;
            }
            return _filename.substr(pos+1);
        }

        bool SetContent(const std::string &body){
            std::ofstream  ofs;
            ofs.open(_filename, std::ios::binary);
            if(ofs.is_open() == false){
                std::cout << "write open file failed!\n";
                return false;
            }
            ofs.write(&body[0], body.size());
            if(ofs.good() == false){
                std::cout <<"write file content failed！\n";
                return false;
            }
            ofs.close();
            return true;
        }
        /**
         * 获得目录中的所有文件名(不包含目录)
         * @param array
         * @return
         */
        bool ScanDirectory(std::vector<std::string> *array){
            for(auto &p : fs::directory_iterator(_filename)){
                if(fs::is_directory(p) == true){
                    continue;
                }
                array->push_back(fs::path(p).relative_path().string());
            }
            return true;
        }
    };
}