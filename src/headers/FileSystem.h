#include "../../FatFs/ff.h"
#include "../../FatFs/FatFs++.h"
#include "io.h"

typedef enum {
    FT_FILE,
    FT_DIR,
    FT_BOTH
} FileType;

void print_dir (const char *path);
char** read_dir(const char *path, int *amount);

FRESULT check_path_exists(
    const char *path, //Path to the File/Dir
    FileType type //FT_FILE, FT_DIR or FT_BOTH
);

int Load_bin_exe(const char* file_path,int argc, char** argv);

FRESULT change_Current_Dir(char** currdir,const char* path);

char* Get_Dir(const char* path);
char* Get_Filename(const char* path);
char* Get_Ext(const char* path);

void Set_vol_to_part_index(int index, PARTITION part);


int FS_Mount_Main_Partition(FATFS* fat_filesys);