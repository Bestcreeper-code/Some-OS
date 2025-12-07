#include "graphics.h"
#include "../res.h"
#include "../../../src/headers/video.h"
#include "../../../src/headers/string.h"
#include "../../../src/headers/math.h"
#include "../../../src/headers/multiboot_info.h"
#include "../../../src/config/config.h"



void Draw_Quad(const Vector2* verts, char color) {
    // verts is an array of 4 points: verts[0], verts[1], verts[2], verts[3]
    // Find min and max Y
    int minY = verts[0].y;
    int maxY = verts[0].y;
    for (int i = 1; i < 4; i++) {
        if (verts[i].y < minY) minY = verts[i].y;
        if (verts[i].y > maxY) maxY = verts[i].y;
    }

    // For each scanline
    for (int y = minY; y <= maxY; y++) {
        float intersections[4];  // Max 4 intersections possible (for convex quad)
        int count = 0;

        // Check edges for intersection with scanline y
        for (int i = 0; i < 4; i++) {
            Vector2 v1 = verts[i];
            Vector2 v2 = verts[(i+1) % 4];

            if ((y >= v1.y && y < v2.y) || (y >= v2.y && y < v1.y)) {
                // Edge crosses scanline y

                float t = (float)(y - v1.y) / (v2.y - v1.y);
                float x = v1.x + t * (v2.x - v1.x);
                intersections[count++] = x;
            }
        }

        if (count >= 2) {
            // Sort intersections
            if (intersections[0] > intersections[1]) {
                float temp = intersections[0];
                intersections[0] = intersections[1];
                intersections[1] = temp;
            }
            // Draw horizontal line between pairs
            memset((void*)&graph_mode_fb[y * 512 + (int)intersections[0]], color, (int)intersections[1] - (int)intersections[0]);
        }
    }
}

void Draw_Rect(Vector2 pos, int width, int height,char color){
#if (QEMU)
    int pitch = 512;
#else
    int pitch = Get_multiboot_info()->framebuffer_pitch;
#endif
    for (int i = 0; i < height; i++)
    {
        memset((void*)&graph_mode_fb[(int)((pos.y+i) * pitch + pos.x)], color, width);
    }
}

