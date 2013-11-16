# eiio

The eiio is a simple image input library for C.
This library includes support for PNG, JPEG and GIF.

# Installation

(Ubuntu)

    sudo apt-get install libjpeg-dev libpng-dev libgif-dev
    ./autogen.sh
    ./configure
    make
    sudo make install
    sudo ldconfig

# Usage

```C
#include <stdio.h>
#include <assert.h>
#include "eiio.h"

int main(void)
{
	int y;

	/* read from file */
	eiio_image_t *image = eiio_read_file("./lena.jpg");
	/* read from memory */
	/* eiio_image_t *image = eiio_read_blob(data_ptr, data_len); */
	
	if (image == NULL) {
		fprintf(stderr, "load error!\n");
		return -1;
	}
	
	assert(image->color == EIIO_COLOR_RGB); // always 8bit RGB
        
	for (y = 0; y < image->height; ++y) {
		int x;
		for (x = 0; x < image->width; ++x) {
			eiio_uint8_t r = eiio_get_pixel(image, x, y, EIIO_CHANNEL_R);
			eiio_uint8_t g = eiio_get_pixel(image, x, y, EIIO_CHANNEL_G);
			eiio_uint8_t b = eiio_get_pixel(image, x, y, EIIO_CHANNEL_B);
			// processing...
		}
	}
	eiio_image_free(&image);
    	
	return 0;
}
```

compiling

    gcc eiio_example.c -o eiio_example -O2 -leiio

or

    gcc eiio_example.c -o eiio_example -O2 `pkg-config --cflags --libs eiio`
