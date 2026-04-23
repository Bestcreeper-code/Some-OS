#include "FileSystem.h"
#include "drivers.h"
#include "string.h"
#include "memory.h"
#include "time.h"
#include "video.h"

// #include "loader.h"
#include "Logger.h"
#include "mouse.h"
#include "paging.h"

// #include "data/textconsts.h"



PARTITION VolToPart[16] = {
    {0, 0}, 
    {0, 1}, 
    {0, 2}, 
    {0, 3}, 
};//only HD rn

// void Set_vol_to_part_index(int index, PARTITION part) {
//     if (index < 0 || index >= (int)(sizeof(VolToPart)/sizeof(VolToPart[0]))) return;
//     VolToPart[index] = part;
// }

// void print_dir(const char *path)
// {
//     FRESULT res;
//     DIR dir;
//     FILINFO fno;
//     int nfile = 0, ndir = 0;

//     res = f_opendir(&dir, path);
//     if (res != FR_OK) {
//         printf("Failed to open \"%s\". (%u)\n", path, res);
//         return;
//     }

//     while (1) {
//         memset(&fno,0,sizeof(fno));
//         res = f_readdir(&dir, &fno);
//         if (res != FR_OK || fno.fname[0] == 0) break;

//         // Skip volume labels
//         if (fno.fattrib & AM_HID) continue;

//         // Print with info about attr for debugging
//         if (fno.fattrib & AM_DIR) {
//             set_print_color(14);
//             printf("%s/\n", fno.fname);
//             ndir++;
//         } else {
//             set_print_color(15);
//             printf("%s\n", fno.fname);
//             nfile++;
//         }
//     }
//     set_print_color(15);
//     f_closedir(&dir);
//     printf("%d dirs, %d files.\n", ndir, nfile);
// }


// char **read_dir(const char *path, int *amount) {
//     FRESULT res;
//     DIR dir;
//     FILINFO fno;
//     int nfile = 0;
//     int capacity = 4;
//     char **list = kmalloc(capacity * sizeof(char*));
    
//     if (!list) {
//         *amount = 0;
//         return NULL;
//     }

//     res = f_opendir(&dir, path);
//     if (res != FR_OK) {
//         kfree(list);
//         *amount = 0;
//         return NULL;
//     }

//     while (1) {
//         memset(&fno,0,sizeof(fno));
//         res = f_readdir(&dir, &fno);
//         if (res != FR_OK || fno.fname[0] == 0) {
//             break; // error or end of directory
//         }

//         if (nfile >= capacity) {
//             capacity *= 2;
//             char **tmp = realloc(list, capacity * sizeof(char*));
//             if (!tmp) {
//                 // Clean up allocated memory on realloc failure
//                 for (int i = 0; i < nfile; i++) {
//                     kfree(list[i]);
//                 }
//                 kfree(list);
//                 f_closedir(&dir);
//                 *amount = 0;
//                 return NULL;
//             }
//             list = tmp;
//         }

//         const char *name = fno.fname;
//         int len = strlen(name);
//         int extra = (fno.fattrib & AM_DIR) ? 1 : 0; // '/' if directory

//         list[nfile] = kmalloc(len + extra + 1);
//         if (!list[nfile]) {
//             // Clean up on kmalloc failure
//             for (int i = 0; i < nfile; i++) {
//                 kfree(list[i]);
//             }
//             kfree(list);
//             f_closedir(&dir);
//             *amount = 0;
//             return NULL;
//         }

//         strcpy(list[nfile], name);
//         if (extra) list[nfile][len] = '/';
//         list[nfile][len + extra] = '\0';

//         nfile++;
//     }

//     f_closedir(&dir);
//     *amount = nfile;
//     return list;
// }



// FRESULT check_path_exists(const char *path, FileType type) {
//     if (type == FT_DIR) {
//         DIR dir;
//         FRESULT res = f_opendir(&dir, path);
//         if (res != FR_OK) return FR_NO_PATH;
//         f_closedir(&dir);
//         return FR_OK;
//     }

//     if (type == FT_FILE) {
//         FILINFO fno;
//         FRESULT res = f_stat(path, &fno);
//         if (res != FR_OK) return res;
//         if (fno.fattrib & AM_DIR) return FR_NO_PATH;
//         return FR_OK;
//     }

//     if (type == FT_BOTH) {
//         // Check directory first
//         DIR dir;
//         FRESULT res_dir = f_opendir(&dir, path);
//         if (res_dir == FR_OK) {
//             f_closedir(&dir);
//             return FR_OK;
//         }

//         // Check file
//         FILINFO fno;
//         FRESULT res_file = f_stat(path, &fno);
//         if (res_file == FR_OK && !(fno.fattrib & AM_DIR)) {
//             return FR_OK;
//         }

//         return FR_NO_PATH;
//     }

//     return FR_INVALID_OBJECT;  // Invalid FileType
// }


// //currdir is changed 
// FRESULT change_Current_Dir(char** currdir, const char* _path) {

//     if (!currdir || !_path || _path[0] =='/' || !*currdir) return FR_INVALID_PARAMETER;
    
//     char* path = strdup(_path);
//     if (!path) return FR_NOT_ENOUGH_CORE;

//     int part_count;
//     char** cut_dir = Split(*currdir, '/', 0, &part_count);
//     if (!cut_dir) {
//         kfree(path);
//         return FR_NOT_ENOUGH_CORE;
//     }

//     // Absolute path to root
//     if (Starts_With(path, "0:/")) {
//         EndSplit(cut_dir, part_count);

//         if (check_path_exists(path, FT_DIR) == FR_OK) {
//             kfree(*currdir);
//             *currdir = strdup(path);
//             kfree(path);
//             return (*currdir) ? FR_OK : FR_NOT_ENOUGH_CORE;
//         } else {
//             kfree(path);
//             return FR_NO_FILE;
//         }
//     }

//     // Handle "../" components
//     while (Starts_With(path, "../") && part_count > 0) {
//         // Remove "../" from path
//         for(char i=0;i<3;i++){
//             RemoveChar(path, 0); // Remove "../"
//         }
//         kfree(cut_dir[--part_count]);
//     }

//     // Handle "./"
//     if (Starts_With(path, "./")) {
//         for(char i=0;i<2;i++){
//             RemoveChar(path, 0); // Remove "./"
//         }
//     }

//     // Join base and relative path
//     char* basepath = Concat(cut_dir, part_count, '/');
//     EndSplit(cut_dir, part_count);

//     if (!basepath) {
//         kfree(path);
//         return FR_NOT_ENOUGH_CORE;
//     }


    
//     char* arr[2] = { basepath, path };
//     char* finalpath = Concat(arr, 2, '/');
//     kfree(basepath);
//     kfree(path);

//     if (!finalpath) return FR_NOT_ENOUGH_CORE;

//     // Final check
//     if (check_path_exists(finalpath, FT_DIR) == FR_OK) {
//         kfree(*currdir);
//         *currdir = finalpath;
//         return FR_OK;
//     } else {
//         kfree(finalpath);
//         return FR_NO_FILE;
//     }
// }


// char* Get_Dir(const char* path){
//     if (!path) return NULL;
//     int count;
//     char** parts = Split(path, '/', 0, &count);
//     char* res;
//     if(count > 1)res = Concat(parts, count - 1, '/');
//     else res = NULL;
//     EndSplit(parts, count);

//     return res;
// }

// char* Get_Filename(const char* path){
//     if (!path) return NULL;
//     int count;
//     char** parts = Split(path, '/', 0, &count);
//     char* res = kmalloc(strlen(parts[count-1]) + 1);
//     strcpy(res, parts[count-1]);
//     EndSplit(parts, count);

//     return res;
// }

// char* Get_Ext(const char* path){
//     short len = strlen(path);
//     for (size_t i = len; i > 0; i--)
//     {
//         if(path[i] == '.'){
//             return (char*)&path[i];
//         }
//     }
//     return NULL;
// }

// int FS_Mount_Main_Partition(FATFS* fat_filesys){

//     FIL file;
//     char label[12];
//     DWORD vsn;
    
//     for (int i = 0; i < 4; i++) {
//         char vol[4];
        
//         snprintf(vol, sizeof(vol), "%d:", i);  // "0:", "1:", ...

//         if (f_mount(fat_filesys, vol, 1) == FR_OK) {
            
//             // dump_pd();
//             if (f_getlabel(vol, label, &vsn) == FR_OK ) {

//                 if (strcmp(label, OS_PARTITION_LABEL ) == 0) {
//                     Sys_log("Found OS partition at %s", vol);
//                     VolToPart[0].pd = 0;
//                     VolToPart[0].pt = i;
//                     VolToPart[i].pd = 0;
//                     VolToPart[i].pt = 0;

//                     return 0; // Success
//                 }
//             }
            

//             f_mount(NULL, vol, 0);  // Unmount if not the right one
//         }
//     }
    

// #if (DEV_BUILD == 1)
//     return f_mount(fat_filesys, "0:", 1);// try to mount 0: if not found(aka a .img formatted with fat)
    
// #endif
    
//     Sys_log("wtf\n");

//     return -1; // Not found
// }

// int mount_notthatrealroot(){

//     int mount_counter = 0;
// mounting:
//     Sys_log("trying to mount filesystem...\n");
//     int res = FS_Mount_Main_Partition(FatFsSys);

//     if (res != 0) {
//         Sys_Error("Failed to mount filesystem. Error code: %d\n Trying to mount again", res);
//         mount_counter++;
//         if(mount_counter < 3)goto mounting;
//         Sys_Error("No OS root partition found\n");
//         goto end_mounting;
//     } else {
//         Sys_Success("Filesystem mounted successfully.\n"); 
//         // get_string();
//     }
// end_mounting:
// }
// REGISTER_DRIVER_FS(notthatrealroot, mount_notthatrealroot);