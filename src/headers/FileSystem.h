#include "../../FatFs/ff.h"
#include "io.h"

typedef enum {
    E_FT_FILE,
    E_FT_DIR,
    E_FT_BOTH
} FileType;

void print_dir (const char *path);
FRESULT check_path_exists(
    const char *path, //Path to the File/Dir
    FileType type //E_FT_FILE, E_FT_DIR or E_FT_BOTH
);

FRESULT change_Current_Dir(char** currdir,char* path);