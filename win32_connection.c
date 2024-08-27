#include <windows.h>
#include<stdint.h>

typedef uint32_t u32;
struct {
    int width, height;
    uint32_t *pixels;
} typedef Render_Buffer;

static int
clamp(int min, int val, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static int running = 1;
static Render_Buffer render_buffer;

static void
draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {
    
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
static void update_game() {
    draw_rect_in_pixels(0, 0, 1000, 1000, 0x00ff00);
}


BITMAPINFO win32_bitmap_info; //this is a bunch of junk I did not take the time to read

#define WINDOW_FLAGS WS_VISIBLE|WS_OVERLAPPEDWINDOW

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{   
    WNDCLASS window_class = {0};

    window_class.lpfnWndProc   = WindowProc;
    window_class.lpszClassName = "Game_Window_class";
    window_class.style = CS_HREDRAW|CS_VREDRAW;

    RegisterClassA(&window_class);
    // Create the window.

    HWND win32_window = CreateWindowExA(
        0,                              // Optional window styles.
        window_class.lpszClassName,                     // Window class
        "Space Invaders",               // Window text
        WINDOW_FLAGS,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (win32_window == NULL) //break if failed to make window
    {
        return 0;
    }

    HDC handleDeviceContext = GetDC(win32_window);
    ShowWindow(win32_window, nCmdShow);

    // Run the main loop.
    while (running) {
        //check messages from OS, use PeakMessage not GetMessage to keep running if no message
        MSG message;
        while (PeekMessageA(&message, win32_window, 0, 0, PM_REMOVE)) { //returns 0 if no messages

            switch (message.message) {
                default: {
                    TranslateMessage(&message); 
                    DispatchMessage(&message);
                }
            }
        } 
        
        //update the game
        update_game();
        
        //paint to the screen
        StretchDIBits(handleDeviceContext, 0, 0, render_buffer.width, render_buffer.height, 0, 0, render_buffer.width, render_buffer.height, render_buffer.pixels, &win32_bitmap_info, DIB_RGB_COLORS, SRCCOPY);


    }

    return 0;
}
//handle messages
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
                
                //fill the bitmapinfo
                win32_bitmap_info.bmiHeader.biSize = sizeof(win32_bitmap_info.bmiHeader);
                win32_bitmap_info.bmiHeader.biWidth = render_buffer.width;
                win32_bitmap_info.bmiHeader.biHeight = render_buffer.height;
                win32_bitmap_info.bmiHeader.biPlanes = 1;
                win32_bitmap_info.bmiHeader.biBitCount = 32;
                win32_bitmap_info.bmiHeader.biCompression = BI_RGB;
            } break;
            default: {
                return DefWindowProc(win32_window, message, wParam, lParam);
            }
        }
    return result;
}