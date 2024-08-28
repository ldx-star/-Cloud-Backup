//
// Created by ldx on 24-8-28.
//

#ifndef TEST_SERVICE_HPP
#define TEST_SERVICE_HPP
#include "httplib.h"
#include "data.hpp"

extern cloud::DataManager *_data;

namespace cloud{
    class Service{
    private:
        int _server_port;
        std::string _server_ip;
        std::string _download_prefix;
        httplib::Server _server;

    private:
        static void Upload(const httplib::Request &req, httplib::Response &rsp){
            auto ret = req.has_file("file"); // 判断有没有“file”文件
            if(ret == false){
                rsp.status = 400; // 设置http状态码
                return;
            }
            const auto& file = req.get_file_value("file");
            std::string back_dir = Config::GetInstance()->GetBackDir();
            std::string realpath = back_dir + FileUtil(file.filename).FileName();
            FileUtil fu(realpath);
            fu.SetContent(file.content);// 将数据写入文件中
            BackupInfo info;
            info.NewBackupInfo(realpath); // 组织备份的文件信息
            _data->Insert(info);
            return;
        }
        static std::string TimetoStr(time_t t){
            std::string ret = std::ctime(&t);
            return ret;
        }
        static void ListShow(const httplib::Request &req, httplib::Response &rsp){
            // 获取所有的文件备份信息
            std::vector<BackupInfo> infos;
            _data->GetAll(&infos);
            // 根据备份信息组织html文件
            std::stringstream ss;
            ss << "<htmls><head><title>Download</title></head>";
            ss << "<body><h1>Download</h1><table>";
            for (auto &a : infos){
                ss << "<tr>";
                std::string filename = FileUtil(a.real_path).FileName();
                ss << "<td><a href='" << a.url << "'>" << filename << "</a></td>";
                ss << "<td align='right'>" << TimetoStr(a.mtime) << "</td>";
                ss << "<td align='right'>" << a.fsize / 1024 << "k</td>";
                ss << "</tr>";
            }
            ss << "</table></body></htmls>";
            rsp.body = ss.str();
            rsp.set_header("Content-Type", "text/html");
            rsp.status = 200;
            return;
        }
        static std::string GetETag(const BackupInfo &info){
            // etg: filename-fsize-mtime
            FileUtil fu(info.real_path);
            std::string etag = fu.FileName();
            etag += "-";
            etag += std::to_string(info.fsize);
            etag += "-";
            etag += std::to_string(info.mtime);
            return etag;
        }
        static void Download(const httplib::Request &req, httplib::Response &rsp){
            // 获取客户端请求的资源路径，根据资源路径获取备份文件信息
            BackupInfo info;
            _data->GetByUrl(req.path, &info);
            // 判断文件是否被压缩，如果被压缩需要先解压
            if(info.pack_flag ==  true){
                FileUtil fu(info.pack_path);
                fu.Decompress(info.real_path); // 将文件解压到备份文件目录下
                // 删除压缩包，修改备份信息
                fu.Remove();
                info.pack_flag = false;
                _data->Update(info);
            }

            bool reTrans = false;
            std::string old_etag;
            if(req.has_header("If-Range")){
                old_etag = req.get_header_value("If-Range");
                if(old_etag == GetETag(info)){
                    reTrans = true;
                }
            }

            //读取文件放入res.body中
            FileUtil fu(info.real_path);
            if(reTrans == false){
                fu.GetContent(&rsp.body);
                // 摄者头部响应字段
                rsp.set_header("Accept-Ranges", "bytes");
                rsp.set_header("ETag", GetETag(info));
                rsp.set_header("Content-Type", "application/octet-stream");
                rsp.status = 200;
            }else{
                // httplib内部实现了断点续传请求处理
                fu.GetContent(&rsp.body);
                rsp.set_header("Accept-Ranges", "bytes");
                rsp.set_header("ETag", GetETag(info));
                rsp.set_header("Content-Type", "application/octet-stream");
                rsp.status = 206; //区间请求响应是206
            }
        }
    public:
        Service(){
            Config *config = Config::GetInstance();
            _server_port = config->GetServerPort();
            _server_ip = config->GetServerIp();
            _download_prefix = config->GetDownloadPrefix();
        }
        bool RunModule(){
            _server.Post("/upload",Upload);// 服务端接收到upload请求，调用Upload
            _server.Get("/listshow", ListShow);
            _server.Get("/", ListShow);
            std::string download_url = _download_prefix + "(.*)";
            _server.Get(download_url, Download);
            _server.listen(_server_ip.c_str(),_server_port);
            return true;

        }

    };
}




#endif //TEST_SERVICE_HPP
