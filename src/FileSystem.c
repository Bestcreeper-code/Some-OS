#include "headers/FileSystem.h"
#include "headers/string.h"
#include "headers/memory.h"
#include "headers/time.h"
#include "headers/video.h"
#include "headers/vga_modes.h"
// #include "headers/loader.h"
#include "headers/Logger.h"
#include "headers/mouse.h"
#include "headers/paging.h"

#include "data/textconsts.h"

#define EXEC_LOAD_ADRESS 0x200000

PARTITION VolToPart[16] = {
    {0, 0}, 
    {0, 1}, 
    {0, 2}, 
    {0, 3}, 
};//only HD rn

void Set_vol_to_part_index(int index, PARTITION part) {
    if (index < 0 || index >= (int)(sizeof(VolToPart)/sizeof(VolToPart[0]))) return;
    VolToPart[index] = part;
}

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
    if (type == FT_DIR) {
        DIR dir;
        FRESULT res = f_opendir(&dir, path);
        if (res != FR_OK) return FR_NO_PATH;
        f_closedir(&dir);
        return FR_OK;
    }

    if (type == FT_FILE) {
        FILINFO fno;
        FRESULT res = f_stat(path, &fno);
        if (res != FR_OK) return res;
        if (fno.fattrib & AM_DIR) return FR_NO_PATH;
        return FR_OK;
    }

    if (type == FT_BOTH) {
        // Check directory first
        DIR dir;
        FRESULT res_dir = f_opendir(&dir, path);
        if (res_dir == FR_OK) {
            f_closedir(&dir);
            return FR_OK;
        }

        // Check file
        FILINFO fno;
        FRESULT res_file = f_stat(path, &fno);
        if (res_file == FR_OK && !(fno.fattrib & AM_DIR)) {
            return FR_OK;
        }

        return FR_NO_PATH;
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

        if (check_path_exists(path, FT_DIR) == FR_OK) {
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
    if (check_path_exists(finalpath, FT_DIR) == FR_OK) {
        free(*currdir);
        *currdir = finalpath;
        return FR_OK;
    } else {
        free(finalpath);
        return FR_NO_FILE;
    }
}



int Load_bin_exe(const char* file_path,int argc, char** argv){
    FIL file;              // File object
    FRESULT res;           // Result code
    UINT bytesRead;        // Number of bytes read
    void* buffer;
    UINT fileSize;

    memset((void*)0x200000,0,0x200000);
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
            Sys_log("File %s read successfully \n",file_path);
            
            // Define a function pointer to the entry point
            int (*entry)(int, char**) = (int (*)(int, char**))buffer;


            // Call the loaded binary
            entry(argc, argv);

            vga_set_mode(0x03);
            disable_mouse_display();
            ClearScreen();

            // Wipe the memory region after execution
            memset(buffer, 0, fileSize);
        
        } else {
            Sys_log("File read error(%s): %d\n", file_path, res);
        }

        f_close(&file);
    } else {
        Sys_log("Failed to open file %s : %d\n", file_path, res);
    }
}

char* Get_Dir(const char* path){
    if (!path) return NULL;
    int count;
    char** parts = Split(path, '/', 0, &count);
    char* res;
    if(count > 1)res = Concat(parts, count - 1, '/');
    else res = NULL;
    EndSplit(parts, count);

    return res;
}

char* Get_Filename(const char* path){
    if (!path) return NULL;
    int count;
    char** parts = Split(path, '/', 0, &count);
    char* res = malloc(strlen(parts[count-1]) + 1);
    strcpy(res, parts[count-1]);
    EndSplit(parts, count);

    return res;
}

char* Get_Ext(const char* path){
    short len = strlen(path);
    for (size_t i = len; i > 0; i--)
    {
        if(path[i] == '.'){
            return (char*)&path[i];
        }
    }
    return NULL;
}

int FS_Mount_Main_Partition(FATFS* fat_filesys){

    FIL file;
    char label[12];
    DWORD vsn;
    
    for (int i = 0; i < 4; i++) {
        char vol[4];
        
        snprintf(vol, sizeof(vol), "%d:", i);  // "0:", "1:", ...

        if (f_mount(fat_filesys, vol, 1) == FR_OK) {
            
            // dump_pd();
            if (f_getlabel(vol, label, &vsn) == FR_OK ) {

                if (strcmp(label, OS_PARTITION_LABEL ) == 0) {
                    Sys_log("Found OS partition at %s", vol);
                    VolToPart[0].pd = 0;
                    VolToPart[0].pt = i;
                    VolToPart[i].pd = 0;
                    VolToPart[i].pt = 0;

                    return 0; // Success
                }
            }
            

            f_mount(NULL, vol, 0);  // Unmount if not the right one
        }
    }
    

#if (DEV_BUILD == 1)
    f_mount(fat_filesys, "0:", 1);// try to mount 0: if not found(aka a .img formatted with fat)
    return 0;
#endif
    
    Sys_log("wtf\n");

    return -1; // Not found
}