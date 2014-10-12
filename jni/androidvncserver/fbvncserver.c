/*
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * This project is an adaptation of the original fbvncserver for the iPAQ
 * and Zaurus.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <sys/sysmacros.h>             /* For makedev() */

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>

#include <assert.h>
#include <errno.h>

/* libvncserver */
#include "rfb/rfb.h"
#include "rfb/keysym.h"

/*****************************************************************************/

/* Android does not use /dev/fb0. */
#define FB_DEVICE "/dev/graphics/fb0"
static char KBD_DEVICE[256] = "/dev/input/event3";
static char TOUCH_DEVICE[256] = "/dev/input/event1";
static struct fb_var_screeninfo scrinfo;
static int fbfd = -1;
static int kbdfd = -1;
static int touchfd = -1;
static unsigned short int *fbmmap = MAP_FAILED;
static unsigned short int *vncbuf;
static unsigned short int *fbbuf;

/* Android already has 5900 bound natively. */
#define VNC_PORT 5901
static rfbScreenInfoPtr vncscr;

static int xmin, xmax;
static int ymin, ymax;

/* No idea, just copied from fbvncserver as part of the frame differerencing
 * algorithm.  I will probably be later rewriting all of this. */
static struct varblock_t
{
	int min_i;
	int min_j;
	int max_i;
	int max_j;
	int r_offset;
	int g_offset;
	int b_offset;
	int rfb_xres;
	int rfb_maxy;
} varblock;

/*****************************************************************************/

static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl);
static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl);

/*****************************************************************************/

static void init_fb(void)
{
	size_t pixels;
	size_t bytespp;

	if ((fbfd = open(FB_DEVICE, O_RDONLY)) == -1)
	{
		printf("cannot open fb device %s\n", FB_DEVICE);
		exit(EXIT_FAILURE);
	}

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &scrinfo) != 0)
	{
		printf("ioctl error\n");
		exit(EXIT_FAILURE);
	}

	pixels = scrinfo.xres * scrinfo.yres;
	bytespp = scrinfo.bits_per_pixel / 8;

	fprintf(stderr, "xres=%d, yres=%d, xresv=%d, yresv=%d, xoffs=%d, yoffs=%d, bpp=%d\n", 
	  (int)scrinfo.xres, (int)scrinfo.yres,
	  (int)scrinfo.xres_virtual, (int)scrinfo.yres_virtual,
	  (int)scrinfo.xoffset, (int)scrinfo.yoffset,
	  (int)scrinfo.bits_per_pixel);

	fbmmap = mmap(NULL, pixels * bytespp, PROT_READ, MAP_SHARED, fbfd, 0);

	if (fbmmap == MAP_FAILED)
	{
		printf("mmap failed\n");
		exit(EXIT_FAILURE);
	}
}

static void cleanup_fb(void)
{
	if(fbfd != -1)
	{
		close(fbfd);
	}
}

static void init_kbd()
{
	if((kbdfd = open(KBD_DEVICE, O_RDWR)) == -1)
	{
		printf("cannot open kbd device %s\n", KBD_DEVICE);
		exit(EXIT_FAILURE);
	}
}

static void cleanup_kbd()
{
	if(kbdfd != -1)
	{
		close(kbdfd);
	}
}

static void init_touch()
{
    struct input_absinfo info;
        if((touchfd = open(TOUCH_DEVICE, O_RDWR)) == -1)
        {
                printf("cannot open touch device %s\n", TOUCH_DEVICE);
                exit(EXIT_FAILURE);
        }
    // Get the Range of X and Y
    if(ioctl(touchfd, EVIOCGABS(ABS_X), &info)) {
        printf("cannot get ABS_X info, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    xmin = info.minimum;
    xmax = info.maximum;
    if(ioctl(touchfd, EVIOCGABS(ABS_Y), &info)) {
        printf("cannot get ABS_Y, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    ymin = info.minimum;
    ymax = info.maximum;

}

static void cleanup_touch()
{
	if(touchfd != -1)
	{
		close(touchfd);
	}
}

/*****************************************************************************/

static void init_fb_server(int argc, char **argv)
{
	printf("Initializing server...\n");

	/* Allocate the VNC server buffer to be managed (not manipulated) by 
	 * libvncserver. */
	vncbuf = calloc(scrinfo.xres * scrinfo.yres, scrinfo.bits_per_pixel / 2);
	assert(vncbuf != NULL);

	/* Allocate the comparison buffer for detecting drawing updates from frame
	 * to frame. */
	fbbuf = calloc(scrinfo.xres * scrinfo.yres, scrinfo.bits_per_pixel / 2);
	assert(fbbuf != NULL);

	printf("*************** scrinfo.bits_per_pixel: %d \n", scrinfo.bits_per_pixel);
	/* TODO: This assumes scrinfo.bits_per_pixel is 16. */
	/* A pixel is one dot on the screen. The number of bytes in a pixel will depend 
	on the number of samples in that pixel and the number of bits in each sample.
	 A sample represents one of the primary colors in a color model. 
	 The RGB color model uses red, green, and blue samples respectively. 
	 Suppose you wanted to use 16-bit RGB color: 
	 You would have three samples per pixel (one for each primary color), 
	 five bits per sample (the quotient of 16 RGB bits divided by three samples),
	 and two bytes per pixel (the smallest multiple of eight bits in which the 16-bit pixel will fit). 
	 If you wanted 32-bit RGB color, you would have three samples per pixel again, 
	 eight bits per sample (since that's how 32-bit color is defined), 
	 and four bytes per pixel (the smallest multiple of eight bits in which the 32-bit pixel will fit.*/

	vncscr = rfbGetScreen(&argc, argv, scrinfo.xres, scrinfo.yres,
						 8, 4, 4);
	assert(vncscr != NULL);
	/*
		int bitsPerSample (bits per channel) ,int samplesPerPixel, int bytesPerPixel	 

	 1 bit B/W      1 bit   per pixel 1 sample  per pixel  1 bit  per sample
	 8 bit Gray     1 byte  per pixel 1 sample  per pixel  8 bits per sample
	16 bit Gray     2 bytes per pixel 1 sample  per pixel 16 bits per sample
	24 bit RGB      3 bytes per pixel 3 samples per pixel  8 bits per sample
	48 bit RGB      6 bytes per pixel 3 samples per pixel 16 bits per sample
	64 bit RGBI     8 bytes per pixel 4 samples per pixel 16 bits per sample
	16 bit Infrared 2 bytes per pixel 1 sample  per pixel 16 bits per sample
	from http://stuff.mit.edu/afs/sipb/project/scanner/bin/html/vuesc13.htm
	*/

	vncscr->desktopName = "Android";
	vncscr->frameBuffer = (char *)vncbuf;
	vncscr->alwaysShared = TRUE;
	vncscr->httpDir = NULL;
	vncscr->port = VNC_PORT;

	vncscr->kbdAddEvent = keyevent;
	vncscr->ptrAddEvent = ptrevent;
	vncscr->serverFormat.trueColour = TRUE; 
	vncscr->depth = vncscr->serverFormat.depth = 24;
	//vncscr->serverFormat.bitsPerPixel = scrinfo.bitsPerPixel;
/*
uint8_t 	bitsPerPixel
uint8_t 	depth
uint8_t 	bigEndian
uint8_t 	trueColour
uint16_t 	redMax
uint16_t 	greenMax
uint16_t 	blueMax
uint8_t 	redShift
uint8_t 	greenShift
uint8_t 	blueShift
uint8_t 	pad1
uint16_t 	pad2

*/
	printf("******vncscr->depth: %d\n",vncscr->serverFormat.depth);
	printf("******vncscr->bitsPerPixel: %d\n",vncscr->serverFormat.bitsPerPixel);
	printf("******vncscr->redMax: %d\n",vncscr->serverFormat.redMax);
	printf("******vncscr->greenMax: %d\n",vncscr->serverFormat.greenMax);
	printf("******vncscr->blueMax: %d\n",vncscr->serverFormat.blueMax);
	printf("******vncscr->redShift: %d\n",vncscr->serverFormat.redShift);
	printf("******vncscr->greenShift: %d\n",vncscr->serverFormat.greenShift);
	printf("******vncscr->blueShift: %d\n",vncscr->serverFormat.blueShift);


	rfbInitServer(vncscr);

	/* Mark as dirty since we haven't sent any updates at all yet. */
	rfbMarkRectAsModified(vncscr, 0, 0, scrinfo.xres, scrinfo.yres);
	printf("scrinfo.red.offset: %d\nscrinfo.green.offset: %d\nscrinfo.blue.offset:%d\n",
			scrinfo.red.offset,scrinfo.green.offset,scrinfo.blue.offset);
	printf("scrinfo.red.length: %d\nscrinfo.green.length: %d\nscrinfo.blue.length: %d\n",
	 scrinfo.red.length, scrinfo.green.length, scrinfo.blue.length);

	/* No idea. */
	varblock.r_offset = scrinfo.red.offset + scrinfo.red.length - 5;
	varblock.g_offset = scrinfo.green.offset + scrinfo.green.length - 5;
	varblock.b_offset = scrinfo.blue.offset + scrinfo.blue.length - 5;
	varblock.rfb_xres = scrinfo.yres;
	varblock.rfb_maxy = scrinfo.xres - 1;

	printf("varblock.r_offset: %d\nvarblock.g_offset: %d\nvarblock.b_offset:%d\nvarblock.rfb_xres: %d\nvarblock.rfb_maxy: %d\n", varblock.r_offset,varblock.g_offset, varblock.b_offset, varblock.rfb_xres, varblock.rfb_maxy);
}

// #define PIXEL_FB_TO_RFB(p,r,g,b) (((p>>r)<<16)&0x00ffffff)|((((p>>g))<<8)&0x00ffffff)|(((p>>b)&0x00ffffff))

// #define PIXEL_FB_TO_RFB(p,r,g,b) (((p>>r)<<16)&0x00ffffff)|((((p>>g))<<8)&0x00ffffff)|(((p>>b)&0x00ffffff))
// -> this one is worse :( 
#define PIXEL_FB_TO_RFB(p,r,g,b) (((p>>r)<<16)&0x1f001f)|((((p>>g)<<8)&0x1f001f)<<5)|(((p>>b)&0x1f001f)<<10)

static void update_screen(void)
{
	unsigned int *f, *c, *r;
	int x, y;

	varblock.min_i = varblock.min_j = 9999;
	varblock.max_i = varblock.max_j = -1;

	f = (unsigned int *)fbmmap;        /* -> framebuffer         */
	c = (unsigned int *)fbbuf;         /* -> compare framebuffer */
	r = (unsigned int *)vncbuf;        /* -> remote framebuffer  */

	for (y = 0; y < scrinfo.yres; y++)
	{
		/* faster ? increase compression level -_- */
		for (x = 0; x < scrinfo.xres; x ++)
		{
			unsigned int pixel = *f;

			if (pixel != *c)
			{
				*c = pixel;

				/* XXX: Undo the checkered pattern to test the efficiency
				 * gain using hextile encoding. */
				if (pixel == 0x18e320e4 || pixel == 0x20e418e3)
					pixel = 0x18e318e3;
				
				*r = PIXEL_FB_TO_RFB(pixel,
				  varblock.r_offset, varblock.g_offset, 
				  varblock.b_offset);

				if (x < varblock.min_i)
					varblock.min_i = x;
				else
				{
					if (x > varblock.max_i)
						varblock.max_i = x;

					if (y > varblock.max_j)
						varblock.max_j = y;
					else if (y < varblock.min_j)
						varblock.min_j = y;
				}
			}

			f++, c++;
			r++;
		}
	}

	if (varblock.min_i < 9999)
	{
		if (varblock.max_i < 0)
			varblock.max_i = varblock.min_i;

		if (varblock.max_j < 0)
			varblock.max_j = varblock.min_j;
//printf("Pixel: %d, R: %d, G: %d, B: %d", *f,
				  // varblock.r_offset, varblock.g_offset, 
				  // varblock.b_offset);

		/*fprintf(stderr, "Dirty page: %dx%d+%d+%d...\n",
		  (varblock.max_i+2) - varblock.min_i, (varblock.max_j+1) - varblock.min_j,
		  varblock.min_i, varblock.min_j);*/

		rfbMarkRectAsModified(vncscr, varblock.min_i, varblock.min_j,
		  varblock.max_i + 2, varblock.max_j + 1);

		rfbProcessEvents(vncscr, 2000);
	}
}


/*****************************************************************************/
void injectKeyEvent(uint16_t code, uint16_t value)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time,0);
    ev.type = EV_KEY;
    ev.code = code;
    ev.value = value;
    if(write(kbdfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    printf("injectKey (%d, %d)\n", code , value);    
}

static int keysym2scancode(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
    int scancode = 0;

    int code = (int)key;
    if (code>='0' && code<='9') {
        scancode = (code & 0xF) - 1;
        if (scancode<0) scancode += 10;
        scancode += KEY_1;
    } else if (code>=0xFF50 && code<=0xFF58) {
        static const uint16_t map[] =
             {  KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN,
                KEY_SOFT1, KEY_SOFT2, KEY_END, 0 };
        scancode = map[code & 0xF];
    } else if (code>=0xFFE1 && code<=0xFFEE) {
        static const uint16_t map[] =
             {  KEY_LEFTSHIFT, KEY_LEFTSHIFT,
                KEY_COMPOSE, KEY_COMPOSE,
                KEY_LEFTSHIFT, KEY_LEFTSHIFT,
                0,0,
                KEY_LEFTALT, KEY_RIGHTALT,
                0, 0, 0, 0 };
        scancode = map[code & 0xF];
    } else if ((code>='A' && code<='Z') || (code>='a' && code<='z')) {
        static const uint16_t map[] = {
                KEY_A, KEY_B, KEY_C, KEY_D, KEY_E,
                KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
                KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
                KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
                KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z };
        scancode = map[(code & 0x5F) - 'A'];
    } else {
        switch (code) {
            case 0x0003:    scancode = KEY_CENTER;      break;
            case 0x0020:    scancode = KEY_SPACE;       break;
            case 0x0023:    scancode = KEY_SHARP;       break;
            case 0x0033:    scancode = KEY_SHARP;       break;
            case 0x002C:    scancode = KEY_COMMA;       break;
            case 0x003C:    scancode = KEY_COMMA;       break;
            case 0x002E:    scancode = KEY_DOT;         break;
            case 0x003E:    scancode = KEY_DOT;         break;
            case 0x002F:    scancode = KEY_SLASH;       break;
            case 0x003F:    scancode = KEY_SLASH;       break;
            case 0x0032:    scancode = KEY_EMAIL;       break;
            case 0x0040:    scancode = KEY_EMAIL;       break;
            case 0xFF08:    scancode = KEY_BACKSPACE;   break;
            case 0xFF1B:    scancode = KEY_BACK;        break;
            case 0xFF09:    scancode = KEY_TAB;         break;
            case 0xFF0D:    scancode = KEY_ENTER;       break;
            case 0x002A:    scancode = KEY_STAR;        break;
            case 0xFFBE:    scancode = KEY_F1;        break; // F1
            case 0xFFBF:    scancode = KEY_F2;         break; // F2
            case 0xFFC0:    scancode = KEY_F3;        break; // F3
            case 0xFFC5:    scancode = KEY_F4;       break; // F8
            case 0xFFC8:    rfbShutdownServer(cl->screen,TRUE);       break; // F11            
        }
    }

    return scancode;
}

static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
	int scancode;

	printf("Got keysym: %04x (down=%d)\n", (unsigned int)key, (int)down);

	if ((scancode = keysym2scancode(down, key, cl)))
	{
		injectKeyEvent(scancode, down);
	}
}

void injectTouchEvent(int down, int x, int y)
{
    struct input_event ev;
    
    // Calculate the final x and y
    x = xmin + (x * (xmax - xmin)) / (scrinfo.xres);
    y = ymin + (y * (ymax - ymin)) / (scrinfo.yres);
    
    memset(&ev, 0, sizeof(ev));

    // Then send a BTN_TOUCH
    gettimeofday(&ev.time,0);
    ev.type = EV_KEY;
    ev.code = BTN_TOUCH;
    ev.value = down;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    // Then send the X
    gettimeofday(&ev.time,0);
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    // Then send the Y
    gettimeofday(&ev.time,0);
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    // Finally send the SYN
    gettimeofday(&ev.time,0);
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(touchfd, &ev, sizeof(ev)) < 0)
    {
        printf("write event failed, %s\n", strerror(errno));
    }

    printf("injectTouchEvent (x=%d, y=%d, down=%d)\n", x , y, down);    
}

static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl)
{
	/* Indicates either pointer movement or a pointer button press or release. The pointer is
now at (x-position, y-position), and the current state of buttons 1 to 8 are represented
by bits 0 to 7 of button-mask respectively, 0 meaning up, 1 meaning down (pressed).
On a conventional mouse, buttons 1, 2 and 3 correspond to the left, middle and right
buttons on the mouse. On a wheel mouse, each step of the wheel upwards is represented
by a press and release of button 4, and each step downwards is represented by
a press and release of button 5. 
  From: http://www.vislab.usyd.edu.au/blogs/index.php/2009/05/22/an-headerless-indexed-protocol-for-input-1?blog=61 */
	
	//printf("Got ptrevent: %04x (x=%d, y=%d)\n", buttonMask, x, y);
	if(buttonMask & 1) {
		// Simulate left mouse event as touch event
		injectTouchEvent(1, x, y);
		injectTouchEvent(0, x, y);
	} 
}
/*****************************************************************************/

void print_usage(char **argv)
{
	printf("%s [-k device] [-t device] [-h]\n"
		"-k device: keyboard device node, default is /dev/input/event3\n"
		"-t device: touch device node, default is /dev/input/event1\n"
		"-h : print this help\n");
}

int main(int argc, char **argv)
{
	printf("WELCOME TO GEORGIA TECH :) \n");
	if(argc > 1)
	{
		int i=1;
		while(i < argc)
		{
			if(*argv[i] == '-')
			{
				switch(*(argv[i] + 1))
				{
					case 'h':
						print_usage(argv);
						exit(0);
						break;
					case 'k':
						i++;
						strcpy(KBD_DEVICE, argv[i]);
						break;
					case 't':
						i++;
						strcpy(TOUCH_DEVICE, argv[i]);
						break;
				}
			}
			i++;
		}
	}

	printf("Initializing framebuffer device " FB_DEVICE "...\n");
	init_fb();
	printf("Initializing keyboard device %s ...\n", KBD_DEVICE);
	init_kbd();
	printf("Initializing touch device %s ...\n", TOUCH_DEVICE);
	init_touch();

	printf("Initializing VNC server:\n");
	printf("	width:  %d\n", (int)scrinfo.xres);
	printf("	height: %d\n", (int)scrinfo.yres);
	printf("	bpp:    %d\n", (int)scrinfo.bits_per_pixel);
	printf("	port:   %d\n", (int)VNC_PORT);
	init_fb_server(argc, argv);

	/* Implement our own event loop to detect 
	changes in the framebuffer. */
	while (1)
	{
		while (vncscr->clientHead == NULL)
			rfbProcessEvents(vncscr, 2000);

		rfbProcessEvents(vncscr, 2000);
		update_screen();
	}

	printf("Cleaning up...\n");
	cleanup_fb();
	cleanup_kdb();
	cleanup_touch();
}
