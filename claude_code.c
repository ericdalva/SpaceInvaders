#include <windows.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t u32;
struct {
    int width, height;
    uint32_t *pixels;
} typedef Render_Buffer;

// Ship structure
typedef struct {
    int x, y;
    int width, height;
    u32 color;
} Ship;

// Enemy structure
typedef struct {
    int x, y;
    int width, height;
    u32 color;
    int alive;
} Enemy;

// Bullet structure
typedef struct {
    int x, y;
    int width, height;
    u32 color;
    int active;
} Bullet;

#define MAX_ENEMIES 55
#define ENEMY_ROWS 5
#define ENEMY_COLS 11
#define MAX_BULLETS 10

static int clamp(int min, int val, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static int running = 1;
static Render_Buffer render_buffer;
static Ship player_ship;
static Enemy enemies[MAX_ENEMIES];
static Bullet bullets[MAX_BULLETS];
static int enemy_direction = 1; // 1 for right, -1 for left
static int frame_counter = 0;

static void draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {
    x0 = clamp(0, x0, render_buffer.width);
    x1 = clamp(0, x1, render_buffer.width);
    y0 = clamp(0, y0, render_buffer.height);
    y1 = clamp(0, y1, render_buffer.height);

    u32 *row = render_buffer.pixels + x0 + render_buffer.width*y0;
    u32 *pixel = row;
    int stride = render_buffer.width;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            *pixel++ = color;
        }
        row += stride;
        pixel = row;
    }
}

static void init_ship() {
    player_ship.width = 50;
    player_ship.height = 30;
    player_ship.x = (render_buffer.width - player_ship.width) / 2;
    player_ship.y = render_buffer.height - player_ship.height - 10;
    player_ship.color = 0xFF0000; // Red color
}

static void init_enemies() {
    int enemy_width = 40;
    int enemy_height = 30;
    int start_x = 50;
    int start_y = 50;
    int spacing_x = 60;
    int spacing_y = 50;

    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            int index = row * ENEMY_COLS + col;
            enemies[index].width = enemy_width;
            enemies[index].height = enemy_height;
            enemies[index].x = start_x + col * spacing_x;
            enemies[index].y = start_y + row * spacing_y;
            enemies[index].color = 0x00FF00; // Green color
            enemies[index].alive = 1;
        }
    }
}

static void init_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].width = 5;
        bullets[i].height = 15;
        bullets[i].color = 0xFFFFFF; // White color
        bullets[i].active = 0;
    }
}

static void draw_ship() {
    draw_rect_in_pixels(player_ship.x, player_ship.y, 
                        player_ship.x + player_ship.width, 
                        player_ship.y + player_ship.height, 
                        player_ship.color);
}

static void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            draw_rect_in_pixels(enemies[i].x, enemies[i].y,
                                enemies[i].x + enemies[i].width,
                                enemies[i].y + enemies[i].height,
                                enemies[i].color);
        }
    }
}

static void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            draw_rect_in_pixels(bullets[i].x, bullets[i].y,
                                bullets[i].x + bullets[i].width,
                                bullets[i].y + bullets[i].height,
                                bullets[i].color);
        }
    }
}

static void move_ship(int dx) {
    player_ship.x += dx;
    player_ship.x = clamp(0, player_ship.x, render_buffer.width - player_ship.width);
}

static void move_enemies() {
    int move_down = 0;
    int edge_reached = 0;

    // Check if any enemy has reached the edge
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            if ((enemy_direction > 0 && enemies[i].x + enemies[i].width >= render_buffer.width - 10) ||
                (enemy_direction < 0 && enemies[i].x <= 10)) {
                edge_reached = 1;
                break;
            }
        }
    }

    // Move enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            if (edge_reached) {
                enemies[i].y += 20; // Move down
                move_down = 1;
            } else {
                enemies[i].x += enemy_direction * 2; // Move horizontally
            }
        }
    }

    // Change direction if edge was reached
    if (move_down) {
        enemy_direction *= -1;
    }
}

static void shoot_bullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player_ship.x + (player_ship.width / 2) - (bullets[i].width / 2);
            bullets[i].y = player_ship.y - bullets[i].height;
            bullets[i].active = 1;
            break;
        }
    }
}

static void move_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= 5; // Move upwards
            if (bullets[i].y + bullets[i].height < 0) {
                bullets[i].active = 0; // Deactivate bullet when it goes off-screen
            }
        }
    }
}

static void check_collisions() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].alive) {
                    // Check for collision
                    if (bullets[i].x < enemies[j].x + enemies[j].width &&
                        bullets[i].x + bullets[i].width > enemies[j].x &&
                        bullets[i].y < enemies[j].y + enemies[j].height &&
                        bullets[i].y + bullets[i].height > enemies[j].y) {
                        // Collision detected
                        enemies[j].alive = 0;
                        bullets[i].active = 0;
                        break;
                    }
                }
            }
        }
    }
}

static void update_game() {
    // Clear screen
    draw_rect_in_pixels(0, 0, render_buffer.width, render_buffer.height, 0x000000);
    
    // Draw ship
    draw_ship();

    // Draw enemies
    draw_enemies();

    // Draw bullets
    draw_bullets();

    // Move enemies every 30 frames (adjust this value to change speed)
    if (frame_counter % 30 == 0) {
        move_enemies();
    }

    // Move bullets
    move_bullets();

    // Check for collisions
    check_collisions();

    frame_counter++;
}

BITMAPINFO win32_bitmap_info;

#define WINDOW_FLAGS WS_VISIBLE|WS_OVERLAPPEDWINDOW

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{   
    WNDCLASS window_class = {0};

    window_class.lpfnWndProc   = WindowProc;
    window_class.lpszClassName = "Game_Window_class";
    window_class.style = CS_HREDRAW|CS_VREDRAW;

    RegisterClassA(&window_class);

    HWND win32_window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Space Invaders",
        WINDOW_FLAGS,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (win32_window == NULL) {
        return 0;
    }

    HDC handleDeviceContext = GetDC(win32_window);
    ShowWindow(win32_window, nCmdShow);

    init_ship();
    init_enemies();
    init_bullets();

    while (running) {
        MSG message;
        while (PeekMessageA(&message, win32_window, 0, 0, PM_REMOVE)) {
            switch (message.message) {
                case WM_KEYDOWN:
                    switch (message.wParam) {
                        case VK_LEFT:
                            move_ship(-5);
                            break;
                        case VK_RIGHT:
                            move_ship(5);
                            break;
                        case VK_UP:
                            shoot_bullet();
                            break;
                    }
                    break;
                default: {
                    TranslateMessage(&message); 
                    DispatchMessage(&message);
                }
            }
        } 

        update_game();

        StretchDIBits(handleDeviceContext, 0, 0, render_buffer.width, render_buffer.height, 
                      0, 0, render_buffer.width, render_buffer.height, render_buffer.pixels, 
                      &win32_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND win32_window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch(message) {
        case WM_CLOSE:
        case WM_DESTROY: {
            running = 0;
        } break;

        case WM_SIZE: {
            RECT rect;
            GetClientRect(win32_window, &rect);
            render_buffer.width = rect.right - rect.left;
            render_buffer.height = rect.bottom - rect.top;

            if (render_buffer.pixels) {
                VirtualFree(render_buffer.pixels, 0, MEM_RELEASE);
            }

            render_buffer.pixels = VirtualAlloc(0, sizeof(u32)*render_buffer.width*render_buffer.height,
                                                MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

            win32_bitmap_info.bmiHeader.biSize = sizeof(win32_bitmap_info.bmiHeader);
            win32_bitmap_info.bmiHeader.biWidth = render_buffer.width;
            win32_bitmap_info.bmiHeader.biHeight = -render_buffer.height;
            win32_bitmap_info.bmiHeader.biPlanes = 1;
            win32_bitmap_info.bmiHeader.biBitCount = 32;
            win32_bitmap_info.bmiHeader.biCompression = BI_RGB;

            init_ship();
            init_enemies();
            init_bullets();
        } break;
        default: {
            return DefWindowProc(win32_window, message, wParam, lParam);
        }
    }
    return result;
}