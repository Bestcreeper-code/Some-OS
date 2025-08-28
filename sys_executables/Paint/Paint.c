#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/FileSystem.h"
#include "../../src/headers/time.h"
#include "../../src/headers/random.h"
#include "../../src/headers/vga_modes.h"
#include "../../src/headers/mouse.h"
#include "Graphics/graphics.h"
#include "res.h"
#include "../../src/data/globals.h"
#include "../../src/data/textconsts.h"

#define MAX_WIDTH 320
#define MAX_HEIGHT 175
#define START_Y 25

char* canvas = NULL;
char* filepath = NULL;


void app_main(int argc, char** argv) {
    fs_set((FATFS*)FATFS_SYS_ADDR, 0);
    vga_set_mode(0x13);
    enable_mouse_display();
    clear_13h_screen(0x9); // dark blue
    reset_palette();
    
    char* file_data = NULL;
    int file_size = 0;
    FIL file;
    UINT br;

    // Try loading paint file if path provided
    if (argc > 1 && check_path_exists(argv[1], FT_FILE) == FR_OK
        && f_open(&file, argv[1], FA_READ) == FR_OK) {
        
        file_size = f_size(&file);
        file_data = malloc(file_size);
        if (file_data != NULL) {
            if (f_read(&file, file_data, file_size, &br) == FR_OK && br == file_size) {
                f_close(&file);
                filepath = argv[1];                
                goto done;
            }
            free(file_data);
            file_data = NULL;
        }
        f_close(&file);
    }

    // If file failed to load, create default paint header
newfile:
    file_size = sizeof(PaintFileHeader) + 320 * 175;
    file_data = create_default_paint_header();
    if (file_data == NULL) {
        return;
    }
done:

    PaintFileHeader* header = (PaintFileHeader*)file_data;
    // Validate magic and dimensions
    if (strcmp(header->magic, PAINT_FILE_MAGIC) != 0 || (header->height * header->width) == 0) {
        free(file_data);
        goto newfile;
    }

    // Setup palette colors from header->colors
    uint8_t* col = (uint8_t*)&header->colors;
    uint8_t* data = (uint8_t*)header + header->data_start;
    int palette_size = (data - col) / 3; // number of RGB colors

    for (int i = 0; i < palette_size; i++) {
        set_palette_color(i + 64, col[i * 3], col[i * 3 + 1], col[i * 3 + 2]);
    }

    // Allocate canvas buffer
    canvas = malloc(header->height * header->width);
    if (!canvas) {
        printf("not enough memory\n");
        sleep(2000);
        free(file_data);
        return;
    }

    // Copy pixel data from file_data to canvas (fixed pointer arithmetic)
    memcpy(canvas, (char*)header + header->data_start, header->height * header->width);



    uint8_t current_color = 0x0; // black

    Vector2 top_left = {(MAX_WIDTH - header->width) / 2, (MAX_HEIGHT - header->height) / 2 + START_Y};
    Vector2 bottom_right = {
        top_left.x + header->width,
        top_left.y + header->height
    };

    int brush_size = 1; 
    bool redraw_ui = true;    
    
redraw:
    for (int y = 0; y < header->height; y++) {
        for (int x = 0; x < header->width; x++) {
            put_pixel(top_left.x + x, top_left.y + y, canvas[y * header->width + x]);
        }
    }

    while (1) {
        if (redraw_ui) {
            Draw_Rect((Vector2){0,0},320,START_Y,0x9); // clear top bar
            char temp_str[10];
            sprintf(temp_str, "COL:%03d", current_color);
            draw_bitmap_string(temp_str, 0, 0, 4, 6, 0, NULL, true, 1);
            Draw_Rect((Vector2){0,6},7,7,current_color);

            sprintf(temp_str, "SIZ:%03d", brush_size);

            draw_bitmap_string(temp_str, 50, 0, 4, 6, 0, NULL, true, 1);
            redraw_ui = false;
            draw_bitmap_string("C: Color | X: Brush Size | P: New color | Ctrl+S save", 0, 15, 4, 6, 0x37, NULL, true, 1);

        }

        short mx, my;
        Get_Mouse_Pos(&mx, &my);

        if (Get_Mouse_Button(MOUSE_BUTTON_LEFT)) {
            for (int dy = 0; dy < brush_size; dy++) {
                for (int dx = 0; dx < brush_size; dx++) {
                    int draw_x = mx + dx - top_left.x - brush_size / 2;
                    int draw_y = (my - 1) + dy - top_left.y - brush_size / 2;

                    if (draw_x >= 0 && draw_x < header->width && draw_y >= 0 && draw_y < header->height) {
                        canvas[draw_y * header->width + draw_x] = current_color;
                        put_pixel(top_left.x + draw_x, top_left.y + draw_y, current_color);
                    }
                }
            }
        } else if (Get_Mouse_Button(MOUSE_BUTTON_RIGHT)) {
            for (int dy = 0; dy < brush_size; dy++) {
                for (int dx = 0; dx < brush_size; dx++) {
                    int draw_x = mx + dx - top_left.x - brush_size / 2;
                    int draw_y = (my - 1) + dy - top_left.y - brush_size / 2;

                    if (draw_x >= 0 && draw_x < header->width && draw_y >= 0 && draw_y < header->height) {
                        canvas[draw_y * header->width + draw_x] = 0x3F; // white
                        put_pixel(top_left.x + draw_x, top_left.y + draw_y, 0x3F);
                    }
                }
            }
        } else if (Get_Mouse_Button(MOUSE_BUTTON_MIDDLE)) {
            if (mx >= top_left.x && mx < bottom_right.x && my >= top_left.y && my < bottom_right.y) {
                int cx = mx - top_left.x;
                int cy = my - top_left.y;
                current_color = canvas[cy * header->width + cx];
            }
        }

        unsigned char key = getc_nb();
        switch (key)
        {
        case 'c':case 'C':{
            draw_bitmap_string("Enter color index (0-255):", 80, 0, 4, 6, 0, NULL, true, 1);
            char* input = String_Input_Popup(90, 80, 140);
            uint8_t color = atoi(input);
            free(input);
            if(color <= get_color_palette_size())current_color = color;
            else{
                draw_bitmap_string("Invalid color index!", 80, 12, 4, 6, 0x4, NULL, true, 1);
                sleep(1000);
            }
            redraw_ui = true;
            goto redraw;
            break;
        }

        case 'x':case 'X':{
            draw_bitmap_string("Enter Brush Size(max 200):", 80, 0, 4, 6, 0, NULL, true, 1);
            char* input = String_Input_Popup(90, 80, 140);
            uint8_t chose_size = atoi(input);
            free(input);
            if(chose_size <= 200)brush_size = chose_size;
            else{
                draw_bitmap_string("Invalid size!", 80, 12, 4, 6, 0x4, NULL, true, 1);
                sleep(1000);
            }
            redraw_ui = true;
            goto redraw;
            break;
        }

        case 'p':case 'P':
            uint8_t r,g,b;
            uint8_t* vars[3] = {&r,&g,&b};
            char vars_names[3] = {'r','g','b'};

            for(int i = 0;i<3;i++){
                Draw_Rect((Vector2){80, 0}, 160 , 6 , 0x09); 
                char msg[25];
                sprintf(msg,"Enter the wanted %c value",vars_names[i]);
                draw_bitmap_string(msg, 80, 0, 4, 6, 0, NULL, true, 1);
                char* input = String_Input_Popup(90, 80, 140);
                *vars[i] = atoi(input);
                free(input);
            }
            current_color = set_new13h_color(r,g,b);
            redraw_ui = true;
            goto redraw;
            break;

        case ControlCombo('s'):case ControlCombo('S'):
            // Save file
            goto save_to_file;
            break;
        default:
            break;
        }

        
    }

save_to_file:
    clear_13h_screen(0x9); // dark blue
    if (filepath == NULL) {
ask_f_name:
        draw_bitmap_string("Enter file name to save (e.g., file.paint):", 10, 10, 4, 6, 0, NULL, true, 1);
        char* input = String_Input_Popup(20, 10, 280);
        if(input == NULL || strlen(input) == 0){
            free(input);
            goto ask_f_name;
        }
        if (strcmp(&input[strlen(input) - 6], ".paint")) {
            char* new_input = malloc(strlen(input) + 7);
            sprintf(new_input, "%s.paint", input);
            free(input);
            input = new_input;
        }
        filepath = input;
    }
    // Update header colors from current palette
    PaintFileHeader* hdr = malloc(sizeof(PaintFileHeader) + palette_size * 3 + header->width * header->height);
    for (int i = 0; i < palette_size; i++) {
        RGBColor color = get_palette_color(64 + i);
        hdr->colors[i] = color;
    }
    memcpy(hdr, header, sizeof(PaintFileHeader));
    hdr->data_start = sizeof(PaintFileHeader) + palette_size * 3;

    memcpy((char*)hdr + hdr->data_start, canvas, header->width * header->height);
    // Write to file
    clear_13h_screen(0x9); // dark blue
    FIL save_file;
    if (f_open(&save_file, filepath, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        UINT bw;
        if (f_write(&save_file, hdr, sizeof(PaintFileHeader) + palette_size * 3 + header->width * header->height, &bw) == FR_OK && bw > 0) {
            f_close(&save_file);
            draw_bitmap_string("File saved successfully!", 80, 12, 4, 6, 0x2, NULL, true, 1);
            sleep(2000);
        } else {
            f_close(&save_file);
            draw_bitmap_string("Failed to write to file!", 80, 12, 4, 6, 0x4, NULL, true, 1);
            sleep(2000);
        }
    } else {
        draw_bitmap_string("Failed to open file for writing!", 80, 12, 4, 6, 0x4, NULL, true, 1);
        sleep(2000);
    }


    // Cleanup (never reached currently)
    free(canvas);
    free(file_data);
    reset_palette();
}
