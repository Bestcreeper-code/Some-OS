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
        buffer = (void*)0x200000; // Use a fixed address for simplicity
        if (buffer == NULL) {
            printf("Memory allocation failed.\n");
        } else {
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

                // If it returns (unlikely), print something
                printf("Returned from %s\n",file_path);

                // Wipe the memory region after execution
                memset(buffer, 0, fileSize);
                printf("Memory cleared.\n");
            } else {
                printf("File read error: %d\n", res);
            }
        }

        f_close(&file);
    } else {
        printf("Failed to open file: %d\n", res);
    }
}