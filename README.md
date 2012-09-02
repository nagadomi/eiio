# eiio

Image Input Library.

# Installation

(ubuntu)

    sudo apt-get install libjpeg-dev libpng-dev libgif-dev
    ./configure
    make
    sudo make install
	sudo ldconfig

# Usage

```C
#include <stdio.h>
#include <assert.h>
#include <eiio.h>

int main(void)
{
	int y;
    
	/* from file */
	eiio_image_t *image = eiio_read_file("./lena.jpg");
	/* from memory */
	/* eiio_image_t *image = eiio_read_blob(data_ptr, data_len); */
    
	if (image == NULL) {
		fprintf(stderr, "error!\n");
		return -1;
	}
	
	assert(image->color == EIIO_COLOR_RGB); // always 8bit RGB
        
	for (y = 0; y < image->height; ++y) {
		int x;
		for (x = 0; x < image->width; ++x) {
			unsigned char r = eiio_get_pixel(image, x, y, EIIO_CHANNEL_R);
			unsigned char g = eiio_get_pixel(image, x, y, EIIO_CHANNEL_G);
			unsigned char b = eiio_get_pixel(image, x, y, EIIO_CHANNEL_B);
			// ...
		}
	}
    	
	eiio_image_free(&image);
    	
	return 0;
}
```

compiling

    gcc eiio_example.c -o eiio_example -O2 -leiio
