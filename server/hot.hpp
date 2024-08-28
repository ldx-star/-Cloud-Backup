//
// Created by ldx on 24-8-27.
//
/// 热点管理模块

#ifndef TEST_HOT_HPP
#define TEST_HOT_HPP
#include "data.hpp"

extern cloud::DataManager *_data;

namespace cloud{
    class HotManager{
    private:
        std::string _back_dir;
        std::string _pack_dir;
        std::string _pack_suffix;
        int _hot_time;
    private:
        // 非热点文件返回真，热点文件返回假
        bool HotJudge(const std::string &filename){
            FileUtil fu(filename);
            time_t last_atime = fu.LastATime();
            time_t cur_time = time(nullptr);
            if(cur_time - last_atime > _hot_time){
                return true;
            }
            return false;
        }
    public:
        HotManager(){
            Config *config = Config::GetInstance();
            _pack_dir = config->GetPackDir();
            _back_dir = config->GetBackDir();
            _pack_suffix = config->GetPackFileSuffix();
            _hot_time = config->GetHotTime();
            FileUtil pack_file(_pack_dir);
            FileUtil back_file(_back_dir);
            pack_file.CreateDirectory();
            back_file.CreateDirectory();
        }
        void RunModule(){
            while (true){
                // 遍历备份目录，获取所有文件名
                FileUtil fu(_back_dir);
                std::vector<std::string> filenames;
                fu.ScanDirectory(&filenames);

                // 判断文件是否是热点文件
                for(const auto &filename : filenames){
                    if(HotJudge(filename) == false){
                        continue;//热点文件不需要特别处理
                    }
                    // 非热点文件需要压缩
                    BackupInfo info;
                    if(_data->GetByRealPath(filename,&info) == false){
                        // 存在一个文件，但是没有备份
                        info.NewBackupInfo(filename);
                    }
                    FileUtil no_hot_file(filename);
                    no_hot_file.Compress(info.pack_path);
                    std::cout <<"非热点文件：" << filename << " 压缩成功" << std::endl;

                    // 删除源文件
                    no_hot_file.Remove();
                    info.pack_flag = true;
                    _data->Update(info);
                }
                usleep(1000); //每隔1秒检测一次热点文件
            }
        }
    };
}

#endif //TEST_HOT_HPP
