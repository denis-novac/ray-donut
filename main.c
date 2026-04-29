#include "raylib.h"
#include <stdint.h>
#include <string.h>

#define R(mul,shift,x,y) \
  _ = x; \
  x -= mul*y>>shift; \
  y += mul*_>>shift; \
  _ = 3145728-x*x-y*y>>11; \
  x = x*_>>10; \
  y = y*_>>10;

int8_t b[1760], z[1760];

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    const int fontSize = 15;

    InitWindow(screenWidth, screenHeight, "Raylib - Donut.c");
    SetTargetFPS(60);

    int sA = 1024, cA = 0, sB = 1024, cB = 0, _;

    while (!WindowShouldClose()) {
        // calc logic (identic with original)
        memset(b, 32, 1760);
        memset(z, 127, 1760);
        int sj = 0, cj = 1024;

        for (int j = 0; j < 90; j++) {
            int si = 0, ci = 1024;
            for (int i = 0; i < 324; i++) {
                int R1 = 1, R2 = 2048, K2 = 5120 * 1024;

                int x0 = R1 * cj + R2,
                    x1 = ci * x0 >> 10,
                    x2 = cA * sj >> 10,
                    x3 = si * x0 >> 10,
                    x4 = R1 * x2 - (sA * x3 >> 10),
                    x5 = sA * sj >> 10,
                    x6 = K2 + R1 * 1024 * x5 + cA * x3,
                    x7 = cj * si >> 10,
                    x = 40 + 30 * (cB * x1 - sB * x4) / x6,
                    y = 11 + 15 * (cB * x4 + sB * x1) / x6, // center
                    N = (-cA * x7 - cB * ((-sA * x7 >> 10) + x2) - ci * (cj * sB >> 10) >> 10) - x5 >> 7;

                int o = x + 80 * y;
                int8_t zz = (x6 - K2) >> 15;
                if (22 > y && y > 0 && x > 0 && 80 > x && zz < z[o]) {
                    z[o] = zz;
                    b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
                }
                R(5, 8, ci, si)
            }
            R(9, 7, cj, sj)
        }

        BeginDrawing();
            ClearBackground(BLACK);

            for (int y = 0; y < 22; y++) {
                for (int x = 0; x < 80; x++) {
                    char charToPrint = b[x + y * 80];
                    if (charToPrint != ' ') {
                        DrawText(TextFormat("%c", charToPrint), x * 10, y * 20, fontSize, GREEN);
                    }
                }
            }
            
            DrawFPS(10, 10);
        EndDrawing();

        R(5, 7, cA, sA);
        R(5, 8, cB, sB);
    }

    CloseWindow();
    return 0;
}
