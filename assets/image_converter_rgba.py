from PIL import Image
import sys

def image_to_c_array(image_path, output_path):
    img = Image.open(image_path).convert("RGBA")
    width, height = img.size
    
    with open(output_path, 'w') as f:
        f.write(f"#define IMAGE_WIDTH {width}\n")
        f.write(f"#define IMAGE_HEIGHT {height}\n\n")
        f.write("static const uint32_t background_data[IMAGE_WIDTH * IMAGE_HEIGHT] = {\n")
        
        pixels = img.load()
        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                hex_color = f"0x{r:02X}{g:02X}{b:02X}{a:02X}"
                f.write(f"    {hex_color},")
            f.write("\n")
        
        f.write("};\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py input_image.png output_array.c")
        sys.exit(1)
    
    image_to_c_array(sys.argv[1], sys.argv[2])
    print("Conversion complete!")