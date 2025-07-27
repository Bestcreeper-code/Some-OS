#include "headers/FileSystem.h"
#include "headers/string.h"
#include "headers/memory.h"

void print_dir (const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;


    res = f_opendir(&dir, path);                   /* Open the directory */
    if (res == FR_OK) {
        nfile = ndir = 0;
        for (;;) {
            res = f_readdir(&dir, &fno);           /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;         /* Error or end of dir */
            if (fno.fattrib & AM_DIR) {            /* It is a directory */
                printf("| %s/\n",fno.fname);
                ndir++;
            } else {                               /* It is a file */
                printf("| %s\n", fno.fname);
                nfile++;
            }
        }
        f_closedir(&dir);
        printf("%d dirs, %d files.\n", ndir, nfile);
    } else {
        printf("Failed to open \"%s\". (%u)\n", path, res);
    }

}



FRESULT check_path_exists(const char *path, FileType type) {
    FILINFO fno;
    
    
    FRESULT res = f_stat(path, &fno);
    if (res != FR_OK) {
        return res;  
    }

  
    if (type == E_FT_DIR && !(fno.fattrib & AM_DIR)) { //Want a Dir but is File
        return FR_NO_PATH; 
    }

    
    if (type == E_FT_FILE && (fno.fattrib & AM_DIR)) { //Want a File but is Dir
        return FR_NO_PATH; 
    }

    return FR_OK;  
}

//currdir is changed 
FRESULT change_Current_Dir(char** currdir, char* path) {
    int part_count;
    char** cut_dir = Split(*currdir, '/', 0, &part_count);
    if (!cut_dir) return FR_NO_FILE;

    // Absolute path to root
    if (Starts_With(path, "0:/")) {
        EndSplit(cut_dir, part_count);

        if (check_path_exists(path, E_FT_DIR) == FR_OK) {
            free(*currdir);
            *currdir = strdup(path);  // Duplicate absolute path
            return FR_OK;
        } else {
            return FR_NO_FILE;
        }
    }

    while (Starts_With(path, "../") && part_count > 0) {
        for(char i=0;i<3;i++){
            RemoveChar(path, 0); // Remove "../"
        }
        free(cut_dir[--part_count]); // Remove last part
    }


    if (Starts_With(path, "./")) {
        for(char i=0;i<2;i++){
            RemoveChar(path, 0); // Remove "./"
        }
    }

    // Reconstruct current path and append relative part
    char* basepath = Concat(cut_dir, part_count, '/');
    char* arr[2] = { basepath, path };
    char* finalpath = Concat(arr, 2, '/');

    EndSplit(cut_dir, part_count);
    free(basepath);

    // Check if finalpath is a valid directory
    if (check_path_exists(finalpath, E_FT_DIR) == FR_OK) {
        free(*currdir);
        *currdir = finalpath;
        return FR_OK;
    } else {
        free(finalpath);
        return FR_NO_FILE;
    }
}
