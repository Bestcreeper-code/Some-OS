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
char* Get_Dir(char* path);
char* Get_Filename(char* path);