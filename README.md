# ray-donut 🍩

A graphical port of Andy Sloane's famous "Donut.c", powered by the **Raylib** library.

## 📖 Overview
This project takes the iconic 3D spinning torus (donut) originally written to run in a terminal and brings it into a dedicated graphical window. 

While the original code relied on `printf` and terminal escape sequences, this version uses **Raylib** to render the ASCII characters as a grid of text, maintaining the original mathematical logic—including the fixed-point arithmetic and the custom lighting engine.

## ✨ Features
- **Zero Floats:** Retains the original "obfuscated" integer-only math.
- **Hardware Accelerated:** Renders via OpenGL (through Raylib) instead of the system console.
- **60 FPS Smoothness:** Uses the GPU's vertical sync for fluid rotation.
- **Classic Shading:** Uses the original character ramp `.,-~:;=!*#$@` to simulate lighting.

## 🛠️ Build Instructions

### Prerequisites
You must have [Raylib](https://www.raylib.com/) installed on your system.

### Compilation (Linux/macOS)
```bash
gcc main.c -o donut -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
