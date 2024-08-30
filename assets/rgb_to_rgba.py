import re
import sys

def convert_rgb_to_rgba(input_file, output_file):
    with open(input_file, 'r') as f:
        content = f.read()

    # Convert RGB values to RGBA
    def rgb_to_rgba(match):
        hex_value = match.group(1)
        if hex_value == '0x000000':
            return '0x00000000'  # Fully transparent
        else:
            return f'{hex_value}FF'  # Fully opaque

    # Use regex to find and replace all 0xRRGGBB values
    new_content = re.sub(r'(0x[0-9A-Fa-f]{6})', rgb_to_rgba, content)

    # Update the array type
    new_content = new_content.replace('uint32_t', 'uint32_t')

    # Add a comment explaining the new format
    new_content = new_content.replace(
        'static const uint32_t',
        '// RGBA format: 0xRRGGBBAA (AA: 00 for transparent, FF for opaque)\n'
        'static const uint32_t'
    )

    with open(output_file, 'w') as f:
        f.write(new_content)

    print(f"Conversion complete. Output written to {output_file}")

if __name__ == "__main__":
    convert_rgb_to_rgba(sys.argv[1], sys.argv[2])