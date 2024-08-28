//
// Created by ldx on 24-8-27.
/// 用于记录所有存放文件的备份信息

#ifndef TEST_DATA_HPP
#define TEST_DATA_HPP

#include <unordered_map>
#include <pthread.h>
#include "util.hpp"
#include "config.hpp"

namespace cloud {
    typedef struct BackupInfo { //struct 默认public
        bool pack_flag; //是否被压缩
        size_t fsize; //文件大小
        time_t mtime; //最后的修改时间
        time_t atime; //最后的访问时间
        std::string real_path; //文件实际存储路径
        std::string pack_path; //压缩文件存储路径
        std::string url;

        bool NewBackupInfo(const std::string &realpath) {
            FileUtil fu(realpath);
            if (fu.Exists() == false) {
                std::cerr << "new backupInfo: file not exists!\n";
                exit(1);
            }
            Config *config = Config::GetInstance();
            std::string packdir = config->GetPackDir();
            std::string packsuffix = config->GetPackFileSuffix();
            std::string download_prefix = config->GetDownloadPrefix();
            pack_flag = false;
            fsize = fu.FileSize();
            mtime = fu.LastMTime();
            atime = fu.LastATime();
            real_path = realpath;
            // ./backdir/a.txt  -->  ./packdir/a.txt.lz
            pack_path = packdir + fu.FileName() + packsuffix;
            // ./backdir/a.tx  -->  ./download/a.txt
            url = download_prefix + fu.FileName();
            return true;
        }
    } BackupInfo;

    class DataManager {
    private:
        std::string _backup_file;
        pthread_rwlock_t _rwlock;
        std::unordered_map<std::string, BackupInfo> _table; // [url,data]
    public:
        DataManager() {
            _backup_file = Config::GetInstance()->GetBackupFile();
            pthread_rwlock_init(&_rwlock, nullptr);//初始化读写锁（读的时候共享，写的时候独占）
            InitLoad();
        }
        ~DataManager() {
            pthread_rwlock_destroy(&_rwlock);
        }

    public:
        bool Insert(const BackupInfo &info) {
            pthread_rwlock_wrlock(&_rwlock);
            _table[info.url] = info;
            pthread_rwlock_unlock(&_rwlock);
            Storage();
            return true;
        }

        bool Update(const BackupInfo &info) {
            pthread_rwlock_wrlock(&_rwlock);
            _table[info.url] = info;
            pthread_rwlock_unlock(&_rwlock);
            Storage();
            return true;
        }

        bool GetByUrl(const std::string &url, BackupInfo *info) {
            pthread_rwlock_wrlock(&_rwlock);
            auto it = _table.find(url);
            if (it == _table.end()) {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            *info = it->second;
            pthread_rwlock_unlock(&_rwlock);
            return true;
        }

        bool GetByRealPath(const std::string &realPath, BackupInfo *info) {
            pthread_rwlock_wrlock(&_rwlock);
            for (const auto &e: _table) {
                if (e.second.real_path == realPath) {
                    *info = e.second;
                    pthread_rwlock_unlock(&_rwlock);
                    return true;
                }
            }
            pthread_rwlock_unlock(&_rwlock);
            return false;
        }

        bool GetAll(std::vector<BackupInfo> *info_v) {
            pthread_rwlock_wrlock(&_rwlock);
            for (const auto &e: _table) {
                info_v->push_back(e.second);
            }
            pthread_rwlock_unlock(&_rwlock);
            return true;
        }

        bool Storage() {
            // 获取所有数据
            std::vector<BackupInfo> info_v;
            GetAll(&info_v);
            // 添加到Json
            Json::Value root;
            for (int i = 0; i < info_v.size(); i++) {
                Json::Value item;
                item["pack_flag"] = info_v[i].pack_flag;
                item["fsize"] = (Json::Int64) info_v[i].fsize;
                item["atime"] = (Json::Int64) info_v[i].atime;
                item["mtime"] = (Json::Int64) info_v[i].mtime;
                item["real_path"] = info_v[i].real_path;
                item["pack_path"] = info_v[i].pack_path;
                item["url"] = info_v[i].url;
                root.append(item);
            }
            // 序列化
            std::string body;
            JsonUtil::Serialize(root, &body);
            // 写入到文件
            FileUtil fu(_backup_file);
            fu.SetContent(body);
            return true;
        }

        bool InitLoad() {
            // 读取数据
            FileUtil fu(_backup_file);
            if (fu.Exists() == false) {
                return false;
            }
            std::string body;
            fu.GetContent(&body);
            // 反序列化
            Json::Value root;
            JsonUtil::UnSerialize(body, &root);
            for (int i = 0; i < root.size(); i++) {
                BackupInfo info;
                info.pack_flag = root[i]["pack_flag"].asBool();
                info.fsize = root[i]["fsize"].asInt64();
                info.atime = root[i]["atime"].asInt64();
                info.mtime = root[i]["mtime"].asInt64();
                info.pack_path = root[i]["pack_path"].asString();
                info.real_path = root[i]["real_path"].asString();
                info.url = root[i]["url"].asString();
                Insert(info);
            }
            return true;
        }

        bool GetByKey(const std::string &key, std::string val){
            auto it = _table.find(key);

        }

    };
}


#endif //TEST_DATA_HPP
