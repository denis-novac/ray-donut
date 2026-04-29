#include "raylib.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

// The legendary bitwise rotation macro remains untouched for max performance
#define R(mul,shift,x,y) \
  _ = x; \
  x -= mul*y>>shift; \
  y += mul*_>>shift; \
  _ = 3145728-x*x-y*y>>11; \
  x = x*_>>10; \
  y = y*_>>10;

int8_t b[1760], z[1760];
const char* densityChars = ".,-~:;=!*#$@";

// --- UI HELPERS ---

bool DrawCyberButton(Rectangle bounds, const char* text, Color baseColor, bool isActive) {
    bool clicked = false;
    Vector2 mousePoint = GetMousePosition();
    Color drawColor = isActive ? baseColor : Fade(baseColor, 0.3f);
    
    if (CheckCollisionPointRec(mousePoint, bounds)) {
        drawColor = ColorBrightness(baseColor, 0.5f);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) clicked = true;
    }
    
    DrawRectangleRec(bounds, Fade(drawColor, 0.4f));
    DrawRectangleLinesEx(bounds, 2.0f, drawColor);
    DrawText(text, (int)bounds.x + 15, (int)bounds.y + 10, 10, RAYWHITE);
    
    return clicked;
}

void DrawTelemetryBar(int x, int y, const char* label, float value, float maxVal, Color color) {
    DrawText(label, x, y, 10, LIGHTGRAY);
    float fillPct = fminf(fabsf(value) / maxVal, 1.0f);
    Rectangle bg = { (float)x + 70, (float)y, 100, 10 };
    Rectangle fg = { (float)x + 70 + (value < 0 ? 50 - (fillPct * 50) : 50), (float)y, fillPct * 50, 10 };
    
    DrawRectangleRec(bg, Fade(DARKGRAY, 0.5f));
    DrawRectangleRec(fg, color);
    DrawRectangleLinesEx(bg, 1.0f, GRAY);
    DrawText("L", x + 60, y, 10, DARKGRAY); // Left indicator
    DrawText("R", x + 175, y, 10, DARKGRAY); // Right indicator
}

int GetDensityIndex(char c) {
    char *ptr = strchr(densityChars, c);
    return ptr ? (int)(ptr - densityChars) : 0;
}

// --- MAIN ENGINES ---

typedef enum { MODE_MATRIX, MODE_HEATMAP, MODE_CYBERPUNK } VisualMode;

int main() {
    const int screenWidth = 1100;
    const int screenHeight = 500;
    InitWindow(screenWidth, screenHeight, "Holographic Donut Telemetry OS");
    SetTargetFPS(60);

    // Physics variables
    float A = 0.0f, B = 0.0f;
    float vA = 0.0f, vB = 0.0f;    
    float accel = 0.003f;          
    float friction = 0.96f;        
    float zoom = 1.0f;

    bool flightAssist = true;
    bool autoCruise = false;
    VisualMode currentMode = MODE_HEATMAP;
    
    int _; // Macro variable

    while (!WindowShouldClose()) {
        // --- INPUT & PHYSICS ---
        
        // Mouse Wheel Zoom
        zoom += GetMouseWheelMove() * 0.1f;
        if (zoom < 0.3f) zoom = 0.3f;
        if (zoom > 3.0f) zoom = 3.0f;

        if (autoCruise) {
            vA = vA * 0.9f + 0.05f * 0.1f;
            vB = vB * 0.9f + 0.03f * 0.1f;
        } else {
            // WASD / Arrows for RCS Thrusters
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    vA += accel;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  vA -= accel;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  vB -= accel;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) vB += accel;

            // Flight Assist (Inertia Dampening)
            if (flightAssist) {
                vA *= friction;
                vB *= friction;
            }
        }

        A += vA;
        B += vB;

        // Bridge real-world float physics to the integer rendering matrix
        int cA = (int)(cosf(A) * 1024);
        int sA = (int)(sinf(A) * 1024);
        int cB = (int)(cosf(B) * 1024);
        int sB = (int)(sinf(B) * 1024);
        
        // Adjust scales based on zoom
        int scaleX = (int)(30.0f * zoom);
        int scaleY = (int)(15.0f * zoom);

        // --- 3D RENDERING CORE ---
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
                    x = 40 + scaleX * (cB * x1 - sB * x4) / x6,
                    y = 11 + scaleY * (cB * x4 + sB * x1) / x6,
                    N = (-cA * x7 - cB * ((-sA * x7 >> 10) + x2) - ci * (cj * sB >> 10) >> 10) - x5 >> 7;
                int o = x + 80 * y;
                int8_t zz = (x6 - K2) >> 15;
                if (22 > y && y > 0 && x > 0 && 80 > x && zz < z[o]) {
                    z[o] = zz;
                    b[o] = densityChars[N > 0 ? N : 0];
                }
                R(5, 8, ci, si)
            }
            R(9, 7, cj, sj)
        }

        // --- DRAWING LOOP ---
        BeginDrawing();
            ClearBackground((Color){ 10, 15, 20, 255 }); // Deep space blue/black
            
            // Draw Hologram
            for (int y = 0; y < 22; y++) {
                for (int x = 0; x < 80; x++) {
                    char charToPrint = b[x + y * 80];
                    if (charToPrint != ' ') {
                        Color renderColor = GREEN;
                        
                        if (currentMode == MODE_HEATMAP) {
                            // Map density 0-11 to a color gradient (Blue -> Red)
                            int den = GetDensityIndex(charToPrint);
                            renderColor = ColorFromHSV(240.0f - (den * 20.0f), 0.8f, 0.9f);
                        } 
                        else if (currentMode == MODE_CYBERPUNK) {
                            // Psychedelic time/space mapping
                            renderColor = ColorFromHSV(GetTime() * 100.0f + x * 4.0f + y * 8.0f, 0.8f, 0.9f);
                        }

                        DrawText(TextFormat("%c", charToPrint), x * 10 + 20, y * 20 + 40, 15, renderColor);
                    }
                }
            }
            
            // --- UI DASHBOARD ---
            int uiX = 850;
            DrawLine(uiX - 30, 0, uiX - 30, screenHeight, Fade(SKYBLUE, 0.3f));
            
            DrawText("SYSTEM TELEMETRY", uiX, 30, 10, SKYBLUE);
            DrawText("RCS Control: WASD / Arrows", uiX, 50, 10, GRAY);
            DrawText("Scanner Zoom: Mouse Wheel", uiX, 65, 10, GRAY);

            // Velocity Gauges
            DrawTelemetryBar(uiX, 100, "PITCH (X)", vA, 0.15f, ORANGE);
            DrawTelemetryBar(uiX, 120, "YAW (Y)", vB, 0.15f, SKYBLUE);

            // Flight Assist Toggle
            Rectangle btnAssist = { (float)uiX, 160, 180, 30 };
            if (DrawCyberButton(btnAssist, flightAssist ? "[ON] FLIGHT ASSIST" : "[OFF] FLIGHT ASSIST", 
                                flightAssist ? GREEN : RED, flightAssist)) {
                flightAssist = !flightAssist;
            }

            // Auto-Cruise Toggle
            Rectangle btnAuto = { (float)uiX, 200, 180, 30 };
            if (DrawCyberButton(btnAuto, autoCruise ? "[ON] AUTO-CRUISE" : "[OFF] AUTO-CRUISE", 
                                PURPLE, autoCruise)) {
                autoCruise = !autoCruise;
                if(autoCruise) flightAssist = false; // Disable FA when cruising
            }

            // Visual Modes
            DrawText("VISUAL MODES", uiX, 260, 10, SKYBLUE);
            if (DrawCyberButton((Rectangle){ (float)uiX, 280, 180, 30 }, "1. MATRIX", LIME, currentMode == MODE_MATRIX)) 
                currentMode = MODE_MATRIX;
            if (DrawCyberButton((Rectangle){ (float)uiX, 320, 180, 30 }, "2. HEATMAP", ORANGE, currentMode == MODE_HEATMAP)) 
                currentMode = MODE_HEATMAP;
            if (DrawCyberButton((Rectangle){ (float)uiX, 360, 180, 30 }, "3. CYBERPUNK", MAGENTA, currentMode == MODE_CYBERPUNK)) 
                currentMode = MODE_CYBERPUNK;

            // Stop Thrusters (Emergency Brake)
            if (DrawCyberButton((Rectangle){ (float)uiX, 420, 180, 30 }, "EMERGENCY BRAKE", RED, false)) {
                autoCruise = false;
                vA = 0.0f; vB = 0.0f;
            }

            DrawFPS(20, 10);
            DrawText(TextFormat("ZOOM: %.1fx", zoom), 20, 30, 10, GRAY);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
