#include "fb2png.h"
#include "log.h"

// for native asset manager
#include <sys/types.h>
#include <assert.h>


//fb test
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Let's take a picture of framebuffer*/
int fbfd = -1;
unsigned int *fbmmap;

char framebuffer_device[256] = "/dev/graphics/fb0";
char framebuffer_device_fb1[256] = "/dev/graphics/fb1";
char framebuffer_device_fb2[256] = "/dev/graphics/fb2";

struct fb_var_screeninfo scrinfo;
struct fb_fix_screeninfo fscrinfo;
#define DEFAULT_SAVE_PATH "/sdcard/fb0.png"
#define DEFAULT_SAVE_PATH_FB1 "/sdcard/fb1.png"
#define DEFAULT_SAVE_PATH_FB2 "/sdcard/fbdump_nexus4_fb2.png"

// #define DEFAULT_SAVE_PATH "/data/local/fbdump.png"
#define L(...) LOGV( __VA_ARGS__)

//E/AndroidRuntime(24842): java.lang.UnsatisfiedLinkError: Native method not found: org.onaips.vnc.ScreenshotManager.takeScreenShot:()V
//void org.onaips.vnc.ScreenshotManager.takeScreenShot()
/*void Java_org_onaips_vnc_ScreenshotManager_takeScreenShot(JNIEnv* env, jobject obj) {
	fbmmap = MAP_FAILED;




	if ((fbfd = open(framebuffer_device, O_RDWR)) == -1) {
		printf("Cannot open fb device %s\n", framebuffer_device);
		return -1;
	}


	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
		printf("ioctl error\n");
		return -1;
	}

	unsigned char *raw;
	raw = malloc(fscrinfo.line_length * scrinfo.yres);

	if (!raw) {
	  printf("*********** raw malloc error :( ");
	} else
	  printf("*********** raw malloc no error :)  ");

	free(raw);

	struct timeval  tv;
	gettimeofday(&tv, NULL);

	double time_in_mill =
	         (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond

	char *path;
	path = DEFAULT_SAVE_PATH;
	int ret = fb2png(path);
	if (!ret)
	  printf("***********Image saved to %s\n", path);

} */

int fb2() {
	printf("Entering screenshot-main\n");

	fbmmap = MAP_FAILED;
	// printf("Initializing fbmmap to MAP_FAILED\n");

	printf("Opening fb2");
	if ((fbfd = open(framebuffer_device_fb2, O_RDWR)) == -1) {
		printf("Cannot open fb device %s\n", framebuffer_device_fb2);
		return -1;
	}
	printf("Opened fb2");

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
		printf("ioctl error\n");
		return -1;
	}

	unsigned char *raw;
	raw = malloc(fscrinfo.line_length * scrinfo.yres);

	if (!raw) {
	  printf("*********** raw malloc error :( \n");
	} else
	  printf("*********** raw malloc no error :) \n ");

	free(raw);

	struct timeval  tv;
	gettimeofday(&tv, NULL);

	char *path;
	int ret;
	path = DEFAULT_SAVE_PATH_FB2;
	ret = fb2pngfb1(path);
	if (!ret) {
	  printf("***********Image saved to %s\n", path);
	  return 0;
	}
	return -1;
} 

int fb() {
	printf("Entering screenshot-main\n");

	fbmmap = MAP_FAILED;
	printf("Initializing fbmmap to MAP_FAILED\n");

	// printf("Opening fb and ");
	// if ((fbfd = open(framebuffer_device, O_RDWR)) == -1) {
	// 	printf("Cannot open fb device %s\n", framebuffer_device);
	// 	return -1;
	// }
	// printf("Opened fb\n");

	// if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
	// 	printf("ioctl error\n");
	// 	return -1;
	// }

	// unsigned char *raw;
	// raw = malloc(fscrinfo.line_length * scrinfo.yres);

	// if (!raw) {
	//   printf("*********** raw malloc error :( \n");
	// } else
	//   printf("*********** raw malloc no error :) \n ");

	// free(raw);

	struct timeval  tv;
	gettimeofday(&tv, NULL);


	char *path;
	path = DEFAULT_SAVE_PATH;
	int ret = fb2png(path);
	if (!ret) {
	  printf("***********Image saved to %s\n", path);
	  return 0;
	}else 
	  printf("Failed to save %s\n", path);
	return -1;

}

int fb1() {
	printf("Entering screenshot-main\n");

	fbmmap = MAP_FAILED;
	// printf("Initializing fbmmap to MAP_FAILED\n");

	printf("Opening fb1");
	if ((fbfd = open(framebuffer_device_fb1, O_RDWR)) == -1) {
		printf("Cannot open fb device %s\n", framebuffer_device_fb1);
		return -1;
	}
	printf("Opened fb1");

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
		printf("ioctl error\n");
		return -1;
	}

	unsigned char *raw;
	raw = malloc(fscrinfo.line_length * scrinfo.yres);

	if (!raw) {
	  printf("*********** raw malloc error :( \n");
	} else
	  printf("*********** raw malloc no error :) \n ");

	free(raw);

	struct timeval  tv;
	gettimeofday(&tv, NULL);

	char *path;
	int ret;

	path = DEFAULT_SAVE_PATH_FB1;

	ret = fb2pngfb1(path);
	if (!ret) {
	  printf("***********Image saved to %s\n", path);
	  return 0;
	} 
	return -1;
} 


int main(int argc, char **argv)
{
	fb();
	fb1();
	// fb2();

	return 0;
}


