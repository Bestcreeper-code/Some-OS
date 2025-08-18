#include "headers/FileSystem.h"
#include "headers/string.h"
#include "headers/memory.h"
#include "headers/time.h"
#include "headers/video.h"
#include "headers/vga_modes.h"
#include "headers/loader.h"
#include "data/textconsts.h"

#define EXEC_LOAD_ADRESS 0x200000


void print_dir(const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile = 0, ndir = 0;

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("Failed to open \"%s\". (%u)\n", path, res);
        return;
    }

    while (1) {
        memset(&fno,0,sizeof(fno));
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;

        // Skip volume labels
        if (fno.fattrib & AM_HID) continue;

        // Print with info about attr for debugging
        if (fno.fattrib & AM_DIR) {
            set_print_color(14);
            printf("%s/\n", fno.fname);
            ndir++;
        } else {
            set_print_color(15);
            printf("%s\n", fno.fname);
            nfile++;
        }
    }
    set_print_color(15);
    f_closedir(&dir);
    printf("%d dirs, %d files.\n", ndir, nfile);
}


char **read_dir(const char *path, int *amount) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile = 0;
    int capacity = 4;
    char **list = malloc(capacity * sizeof(char*));
    
    if (!list) {
        *amount = 0;
        return NULL;
    }

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        free(list);
        *amount = 0;
        return NULL;
    }

    while (1) {
        memset(&fno,0,sizeof(fno));
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) {
            break; // error or end of directory
        }

        if (nfile >= capacity) {
            capacity *= 2;
            char **tmp = realloc(list, capacity * sizeof(char*));
            if (!tmp) {
                // Clean up allocated memory on realloc failure
                for (int i = 0; i < nfile; i++) {
                    free(list[i]);
                }
                free(list);
                f_closedir(&dir);
                *amount = 0;
                return NULL;
            }
            list = tmp;
        }

        const char *name = fno.fname;
        int len = strlen(name);
        int extra = (fno.fattrib & AM_DIR) ? 1 : 0; // '/' if directory

        list[nfile] = malloc(len + extra + 1);
        if (!list[nfile]) {
            // Clean up on malloc failure
            for (int i = 0; i < nfile; i++) {
                free(list[i]);
            }
            free(list);
            f_closedir(&dir);
            *amount = 0;
            return NULL;
        }

        strcpy(list[nfile], name);
        if (extra) list[nfile][len] = '/';
        list[nfile][len + extra] = '\0';

        nfile++;
    }

    f_closedir(&dir);
    *amount = nfile;
    return list;
}



FRESULT check_path_exists(const char *path, FileType type) {
    if (type == E_FT_DIR) {
        DIR dir;
        FRESULT res = f_opendir(&dir, path);
        if (res != FR_OK) return FR_NO_PATH;
        f_closedir(&dir);
        return FR_OK;
    }

    if (type == E_FT_FILE) {
        FILINFO fno;
        FRESULT res = f_stat(path, &fno);
        if (res != FR_OK) return res;
        if (fno.fattrib & AM_DIR) return FR_NO_PATH;
        return FR_OK;
    }

    return FR_INVALID_OBJECT;  // Invalid FileType
}


//currdir is changed 
FRESULT change_Current_Dir(char** currdir, const char* _path) {

    if (!currdir || !_path || _path[0] =='/' || !*currdir) return FR_INVALID_PARAMETER;
    
    char* path = strdup(_path);
    if (!path) return FR_NOT_ENOUGH_CORE;

    int part_count;
    char** cut_dir = Split(*currdir, '/', 0, &part_count);
    if (!cut_dir) {
        free(path);
        return FR_NOT_ENOUGH_CORE;
    }

    // Absolute path to root
    if (Starts_With(path, "0:/")) {
        EndSplit(cut_dir, part_count);

        if (check_path_exists(path, E_FT_DIR) == FR_OK) {
            free(*currdir);
            *currdir = strdup(path);
            free(path);
            return (*currdir) ? FR_OK : FR_NOT_ENOUGH_CORE;
        } else {
            free(path);
            return FR_NO_FILE;
        }
    }

    // Handle "../" components
    while (Starts_With(path, "../") && part_count > 0) {
        // Remove "../" from path
        for(char i=0;i<3;i++){
            RemoveChar(path, 0); // Remove "../"
        }
        free(cut_dir[--part_count]);
    }

    // Handle "./"
    if (Starts_With(path, "./")) {
        for(char i=0;i<2;i++){
            RemoveChar(path, 0); // Remove "./"
        }
    }

    // Join base and relative path
    char* basepath = Concat(cut_dir, part_count, '/');
    EndSplit(cut_dir, part_count);

    if (!basepath) {
        free(path);
        return FR_NOT_ENOUGH_CORE;
    }


    
    char* arr[2] = { basepath, path };
    char* finalpath = Concat(arr, 2, '/');
    free(basepath);
    free(path);

    if (!finalpath) return FR_NOT_ENOUGH_CORE;

    // Final check
    if (check_path_exists(finalpath, E_FT_DIR) == FR_OK) {
        free(*currdir);
        *currdir = finalpath;
        return FR_OK;
    } else {
        free(finalpath);
        return FR_NO_FILE;
    }
}



int Load_bin_exe(const char* file_path){
    FIL file;              // File object
    FRESULT res;           // Result code
    UINT bytesRead;        // Number of bytes read
    void* buffer;
    UINT fileSize;


    // Open the binary file for reading
    res = f_open(&file,file_path, FA_READ);
    if (res == FR_OK) {
        fileSize = f_size(&file);  // Get file size

        // Allocate memory for file contents
        force_alloc(EXEC_LOAD_ADRESS, fileSize);
        buffer = (void*)EXEC_LOAD_ADRESS; // Use a fixed address for simplicity
        // Read the file contents into buffer
        res = f_read(&file, buffer, fileSize, &bytesRead);
        if (res == FR_OK && bytesRead == fileSize) {
            // File successfully read into buffer
            printf("File read successfully (%u bytes).\n", bytesRead);

            // Define a function pointer to the entry point
            int (*entry)(void) = (int (*)(void))buffer;

            printf("Jumping to %s...\n",file_path);

            // Call the loaded binary
            entry();

            vga_set_mode(0x03);
            
            ClearScreen();

            // If it returns (unlikely), print something
            printf("Returned from %s\n",file_path);

            // Wipe the memory region after execution
            memset(buffer, 0, fileSize);
            
            printf("Memory cleared.\n");
        } else {
            printf("File read error: %d\n", res);
        }

        f_close(&file);
    } else {
        printf("Failed to open file: %d\n", res);
    }
}

char* Get_Dir(char* path){
    if (!path) return NULL;
    int count;
    char** parts = Split(path, '/', 0, &count);
    char* res;
    if(count > 1)res = Concat(parts, count - 1, '/');
    else res = NULL;
    EndSplit(parts, count);

    return res;
}

char* Get_Filename(char* path){
    if (!path) return NULL;
    int count;
    char** parts = Split(path, '/', 0, &count);
    char* res = malloc(strlen(parts[count-1]) + 1);
    strcpy(res, parts[count-1]);
    EndSplit(parts, count);

    return res;
}