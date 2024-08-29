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
    float x, y;
    int width, height;
    u32 color;
} Ship;

// Enemy structure
typedef struct {
    float x, y;
    int width, height;
    u32 color;
    int alive;
} Enemy;

// Bullet structure
typedef struct {
    float x, y;
    int width, height;
    u32 color;
    int active;
} Bullet;

#define MAX_ENEMIES 55
#define ENEMY_ROWS 5
#define ENEMY_COLS 11
#define MAX_BULLETS 10
#define TARGET_FPS 60
#define FRAME_TARGET_TIME (1000.0f / TARGET_FPS)

static int running = 1;
static Render_Buffer render_buffer;
static Ship player_ship;
static Enemy enemies[MAX_ENEMIES];
static Bullet bullets[MAX_BULLETS];
static int enemy_direction = 1; // 1 for right, -1 for left
static int frame_counter = 0;

static LARGE_INTEGER frequency;
static LARGE_INTEGER last_time;
static float delta_time = 0.0f;

// Key state variables
static int left_key_down = 0;
static int left_key_changed = 0;

static int right_key_down = 0;
static int right_key_changed = 0;

static int up_key_down = 0;
static int up_key_changed = 0;

static int clamp(int min, int val, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

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
    player_ship.x = (render_buffer.width - player_ship.width) / 2.0f;
    player_ship.y = render_buffer.height - player_ship.height - 10;
    player_ship.color = 0x00FF00; // green color
}

static void init_enemies() {
    int enemy_width = 40;
    int enemy_height = 30;
    float start_x = 50;
    float start_y = 50;
    float spacing_x = 60;
    float spacing_y = 50;

    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            int index = row * ENEMY_COLS + col;
            enemies[index].width = enemy_width;
            enemies[index].height = enemy_height;
            enemies[index].x = start_x + col * spacing_x;
            enemies[index].y = start_y + row * spacing_y;
            enemies[index].color = 0xFFFFFF; // White color
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
    draw_rect_in_pixels((int)player_ship.x, (int)player_ship.y, 
                        (int)player_ship.x + player_ship.width, 
                        (int)player_ship.y + player_ship.height, 
                        player_ship.color);
}

static void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            draw_rect_in_pixels((int)enemies[i].x, (int)enemies[i].y,
                                (int)enemies[i].x + enemies[i].width,
                                (int)enemies[i].y + enemies[i].height,
                                enemies[i].color);
        }
    }
}

static void draw_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            draw_rect_in_pixels((int)bullets[i].x, (int)bullets[i].y,
                                (int)bullets[i].x + bullets[i].width,
                                (int)bullets[i].y + bullets[i].height,
                                bullets[i].color);
        }
    }
}

static void move_ship(float dx) {
    player_ship.x += dx;
    player_ship.x = clamp(0, player_ship.x, render_buffer.width - player_ship.width);
}

static void move_enemies() {
    float move_amount = 10.f; // 50 per update
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
                enemies[i].y += 40; // Move down
                move_down = 1;
            } else {
                enemies[i].x += enemy_direction * move_amount;
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
            bullets[i].x = player_ship.x + (player_ship.width / 2.0f) - (bullets[i].width / 2.0f);
            bullets[i].y = player_ship.y - bullets[i].height;
            bullets[i].active = 1;
            break;
        }
    }
}

static void move_bullets(float dt) {
    float bullet_speed = 300.0f; // 300 pixels per second
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= bullet_speed * dt;
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

static void init_timer() {
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&last_time);
}

static float get_delta_time() {
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    float delta = (float)(current_time.QuadPart - last_time.QuadPart) / frequency.QuadPart;
    last_time = current_time;
    return min(0.1f, delta); // to prevent overlarge jumps
}

static void update_game(float dt) {
    float ship_speed = 1.0f; // 1 pixel per update
    static float shoot_cooldown = 0.0f;

    // Handle ship movement based on key states
    if (left_key_down) {
        move_ship(-ship_speed);
        left_key_changed = 0;
    }
    if (right_key_down) {
        move_ship(ship_speed);
        right_key_changed = 0;
    }
    if (up_key_changed) {
        if (shoot_cooldown <= 0.0f) {
            shoot_bullet();
            shoot_cooldown = 0.2f; // 200ms cooldown
        }
        up_key_changed = 0; // Reset the up key state after handling
    }

    // Update shoot cooldown
    if (shoot_cooldown > 0) {
        shoot_cooldown -= dt;
    }

    // Clear screen
    draw_rect_in_pixels(0, 0, render_buffer.width, render_buffer.height, 0x000000);
    
    // Draw ship
    draw_ship();

    // Draw enemies
    draw_enemies();

    // Draw bullets
    draw_bullets();

    // Move enemies
    static float enemy_move_timer = 0.0f;
    enemy_move_timer += dt;
    if (enemy_move_timer >= 0.5f) { // Move enemies every 50ms
        move_enemies(dt);
        enemy_move_timer = 0.0f;
    }

    // Move bullets
    move_bullets(dt);

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
    init_timer();

    while (running) {
        MSG message;
        while (PeekMessageA(&message, win32_window, 0, 0, PM_REMOVE)) {
            switch(message.message) {
                case WM_CLOSE:
                case WM_QUIT: {
                    running = 0;
                }
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    int32_t is_down  = ((message.lParam & (1 << 31)) == 0); //bit set if KEYUP
                    
                    switch (message.wParam) {
                        case VK_LEFT: {
                            left_key_changed = left_key_down != is_down;
                            left_key_down = is_down;
                            break;
                        }

                        case VK_RIGHT: {
                            right_key_changed = right_key_down != is_down;
                            right_key_down = is_down;
                            break;
                        }

                        case VK_UP: {
                            up_key_changed = up_key_down != is_down;
                            up_key_down = is_down;
                            break;
                        }
                    }
                    break;
                }
                default: {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

            }

        }

        float frame_time = get_delta_time();
        update_game(frame_time);

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