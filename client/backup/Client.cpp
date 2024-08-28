//
// Created by ldx on 24-8-28.
//
#include "client.hpp"

#define BACKUP_FILE "../backup.dat"
#define BACKUP_DIR "../backup"

int main(){
    cloud::Backup backup(BACKUP_DIR,BACKUP_FILE);
    backup.RunModule();
    return 0;
}