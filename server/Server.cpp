//
// Created by ldx on 24-8-28.
//
#include "hot.hpp"
#include "service.hpp"
#include <thread>
cloud::DataManager *_data;

void HotTest(){
    cloud::HotManager hot;
    hot.RunModule();
}
void ServiceTest(){
    cloud::Service srv;
    srv.RunModule();
}
int main(){
    _data = new cloud::DataManager();
    std::thread thread_hot_manager(HotTest);
    std::thread thread_service(ServiceTest);
    thread_hot_manager.join();
    thread_service.join();
    return 0;
}
