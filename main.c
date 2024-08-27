#include <windows.h>
#include "alien.h"
#include "game.h"
#include "ship.h"

// some common name: hwnd "handle to window"
// uMsg "unsigned message"

#define TIMER_ID 1
#define UPDATE_INTERVAL 500  // Update every 500 milliseconds

Ship player;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "Space Invaders Window Class";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Space Invaders",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, GAME_WIDTH, GAME_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    InitAliens();  // Initialize aliens
    InitShip(&player); // Initialize ship


    ShowWindow(hwnd, nCmdShow);

    SetTimer(hwnd, TIMER_ID, UPDATE_INTERVAL, NULL);


    MSG msg = {0}; //create an MSG, set all values to 0
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg); //not sure
        DispatchMessage(&msg); //sends msg to WindowProc
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        KillTimer(hwnd, TIMER_ID);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));

            DrawGameBackground(hdc);


            for (int i = 0; i < MAX_ALIENS; i++) {
                DrawAlien(hdc, aliens[i].x, aliens[i].y);
            }
            DrawShip(hdc, &player);



            EndPaint(hwnd, &ps);
        }
        return 0;
    case WM_TIMER:
        if(wParam == TIMER_ID) {
            UpdateAliens();
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_KEYDOWN:
        switch (wParam) {
            case VK_LEFT:
                MoveShip(&player, -10);
                break;
            case VK_RIGHT:
                MoveShip(&player, 10);
                break;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


