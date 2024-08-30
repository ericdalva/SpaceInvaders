import re

def transform_alien_image():
    # Read the original file
    with open('alien.h', 'r') as file:
        content = file.read()

    # Find the array data
    pattern = r'static const uint32_t alien_image\[IMAGE_WIDTH_ALIEN \* IMAGE_HEIGHT_ALIEN\] = \{([\s\S]*?)\};'
    match = re.search(pattern, content)
    
    if not match:
        print("Array data not found in the file.")
        return

    array_data = match.group(1)

    # Split the data into individual values
    values = re.findall(r'0x[0-9A-Fa-f]+', array_data)

    # Transform the values
    transformed_values = ['0xFFFFFF' if value != '0x000000' else '0x000000' for value in values]

    # Reconstruct the array
    new_array_data = ',\n    '.join(transformed_values)
    new_array = f"static const uint32_t alien_image[IMAGE_WIDTH * IMAGE_HEIGHT] = {{\n    {new_array_data}\n}};"

    # Write the new array to a file
    with open('transformed_alien_image.h', 'w') as file:
        file.write(new_array)

    print("Transformation complete. New array written to 'transformed_alien_image.h'")

if __name__ == "__main__":
    transform_alien_image()