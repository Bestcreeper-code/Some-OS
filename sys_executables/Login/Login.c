#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/FileSystem.h"
#include "../../src/headers/time.h"
#include "../../src/headers/random.h"
#include "../../src/headers/vga_modes.h"
#include "Graphics/graphics.h"
#include "res.h"
#include "../../src/data/globals.h"
#include "../../src/data/textconsts.h"

#define MAX_ATTEMPTS 3
#define LOCKDOWN_TIME 5*60 // seconds


void app_main(int argc, char** argv) {
    fs_set((FATFS*)FATFS_SYS_ADDR, 0);
    vga_set_mode(0x13);
    clear_13h_screen(0x9); // dark blue

    draw_bitmap_string("INIT", 320, 200, 4, 6, 0x3F, NULL, true, false, 0);
    sleep(1000);
    FIL timefile;

    if(f_open(&timefile, "0:/SYSTEM_CORE/Security/locktime.tim", FA_READ) == FR_OK) {
        rtc_time_t rtc;
        f_read(&timefile, &rtc, sizeof(rtc_time_t), NULL);
        f_close(&timefile);
        uint32_t l_ts = rtc_to_unix_timestamp(&rtc);

        rtc_read_time(&rtc);
        uint32_t c_ts = rtc_to_unix_timestamp(&rtc);
        if (c_ts < l_ts) {
            goto locked_down;
        }
        
    }



    if (check_path_exists("0:/SYSTEM_CORE/Security/kys.dta", FT_FILE) != FR_OK) {
    new_username:
        clear_13h_screen(0x9);
        draw_bitmap_string("Create a new user:", 0, 0, 4, 6, 0x3F, NULL, true, false, 0);

        char* usrnm = NULL;
        while (usrnm == NULL || !*usrnm) {
            usrnm = String_Input_Popup(97, 136, 4 * 12, false); // 12 visible chars
        }

        clear_13h_screen(0x9);
        draw_bitmap_string(usrnm, 0, 0, 4, 6, 0x3F, NULL, true,  false, 0);
        draw_bitmap_string("Create a new password:", 0, 10, 4, 6, 0x3F, NULL, true,  false, 0);

        char* pwrd = NULL;
        while (pwrd == NULL || !*pwrd) {
            pwrd = String_Input_Popup(97, 136, 4 * 12, true);
            if (pwrd == NULL) goto new_username;
        }

        int usr_len = strlen(usrnm) + 1;
        int pwrd_len = strlen(pwrd) + 1;

        // Generate random key
        int key_size = rand() % 11 + 1;
        char* key = malloc(key_size);
        for (int i = 0; i < key_size; i++) {
            key[i] = (rand() % 255) + 1;
        }

        // Encrypt password (including '\0' at the end)
        char* encrypt_pword = xor_crypt(pwrd, pwrd_len, key, key_size);

        uint16_t fulldata_size = 3 + usr_len + pwrd_len + key_size;
        char* fulldata = malloc(fulldata_size);

        int pos = 0;
        fulldata[pos++] = (char)usr_len;
        fulldata[pos++] = (char)pwrd_len;
        fulldata[pos++] = (char)key_size;

        memcpy(&fulldata[pos], usrnm, usr_len);           // username + '\0'
        pos += usr_len;

        memcpy(&fulldata[pos], encrypt_pword, pwrd_len); 
        pos += pwrd_len;

        memcpy(&fulldata[pos], key, key_size);

        // Save to file
        FIL file;
        f_open(&file, "0:/SYSTEM_CORE/Security/kys.dta", FA_WRITE | FA_CREATE_ALWAYS);
        f_write(&file, fulldata, fulldata_size, NULL);
        f_close(&file);

        free(key);
        free(encrypt_pword);
        free(fulldata);
        free(usrnm);
        free(pwrd);
    }
login:
    // Read user data from file
    clear_13h_screen(0x9);
    FIL file;
    f_open(&file, "0:/SYSTEM_CORE/Security/kys.dta", FA_READ);

    UINT file_size = f_size(&file);
    char* buffer = malloc(file_size);
    f_read(&file, buffer, file_size, NULL);
    f_close(&file);

    // Extract username (null-terminated)
    uint8_t usr_len = buffer[0];
    uint8_t pwrd_len = buffer[1];
    uint8_t key_size = buffer[2];

    if (usr_len == 0 || pwrd_len == 0 || key_size == 0 || file_size != (3 + usr_len + pwrd_len + key_size)) {
        // Invalid data, restart setup/ make a new user
        free(buffer);
        goto new_username;
    }

    char* username = malloc(usr_len);
    memcpy(username, &buffer[3], usr_len);

    char* encrypted_pword = malloc(pwrd_len);
    memcpy(encrypted_pword, &buffer[3 + usr_len], pwrd_len);

    // Extract key starting after encrypted password
    char* key = malloc(key_size);
    memcpy(key, &buffer[3 + usr_len + pwrd_len], key_size);

    // Decrypt password
    char* pword = xor_crypt(encrypted_pword, pwrd_len, key, key_size);

    // Login attempts
    short attempts = 0;
    while (attempts < MAX_ATTEMPTS) {
        clear_13h_screen(0x9);
        char* usrnm = malloc(usr_len + 6);
        sprintf(usrnm, "Hello %s", username);
        draw_bitmap_string(username, 0, 0, 4, 6, 0x3F, NULL, true,  false, 0);
        draw_bitmap_string("Type your password:", 0, 10, 4, 6, 0x3F, NULL, true,  false, 0);

        char* entered_pw = NULL;
        while (entered_pw == NULL || !*entered_pw) {
            entered_pw = String_Input_Popup(97, 136, 4 * 12, true);
        }

        char* encrypted_entered_pw = xor_crypt(entered_pw, strlen(entered_pw) + 1, key, key_size);

        if (memcmp(encrypted_entered_pw, encrypted_pword, pwrd_len) == 0) {
            clear_13h_screen(0x9);
            draw_bitmap_string("Access granted!", 0, 0, 4, 6, 0x3F, NULL, true,  false, 0);
            sleep(1000);

            free(encrypted_entered_pw);
            free(entered_pw);
            free(pword);
            free(username);
            free(encrypted_pword);
            free(key);
            free(buffer);
            return;
        } else {
            attempts++;
            clear_13h_screen(0x9);
            draw_bitmap_string("Incorrect password!", 0, 0, 4, 6, 0x3F, NULL, true,  false, 0);
            sleep(1000);

            free(encrypted_entered_pw);
            free(entered_pw);
        }
    }
    
    // Max attempts reached
    free(pword);
    free(username);
    free(encrypted_pword);
    free(key);
    free(buffer);

    clear_13h_screen(0x34);
    draw_bitmap_string("Too many failed attempts!", 0, 0, 4, 6, 0x3F, NULL, true,  false, 0);

    rtc_time_t rtc;
    rtc_read_time(&rtc);

    rtc_add_seconds(&rtc, LOCKDOWN_TIME); 

    f_open(&timefile, "0:/SYSTEM_CORE/Security/locktime.tim", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&timefile, &rtc, sizeof(rtc_time_t), NULL);
    f_close(&timefile);
    
locked_down:
    rtc_time_t l_rtc;
    f_open(&timefile, "0:/SYSTEM_CORE/Security/locktime.tim", FA_READ);
    f_read(&timefile, &l_rtc, sizeof(rtc_time_t), NULL);
    f_close(&timefile);


    uint32_t ts = rtc_to_unix_timestamp(&l_rtc);
    while (true)
    {
        rtc_read_time(&rtc);
        if(rtc_to_unix_timestamp(&rtc) > ts ){
            goto login;
        };
        clear_13h_screen(0x34);
        draw_bitmap_string("Too many failed attempts!", 0, 0, 4, 6, 0x3F, NULL, true,  false, 0);
        draw_bitmap_string("System locked for", 0, 10, 4, 6, 0x3F, NULL, true,  false, 0);
        
        uint32_t now_ts = rtc_to_unix_timestamp(&rtc);
        uint32_t remaining = (ts > now_ts) ? (ts - now_ts) : 0;

        uint32_t hours = remaining / 3600;
        uint32_t minutes = (remaining % 3600) / 60;
        uint32_t seconds = remaining % 60;
        char str[43];
        sprintf(str, "%02d H | %02d Min | %02d Sec", hours, minutes, seconds);


        draw_bitmap_string(str, 0, 20, 4, 6, 0x3F, NULL, true, false, 0);
        sleep(1000);
    }
    
}
