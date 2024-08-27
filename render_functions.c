
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