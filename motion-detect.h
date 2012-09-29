// this is the code for the 
// motion detection project for the bifferboard.
//
// barebones, we need the following functionality
// 1. read value from gpio using SYSFS (/sys/class/gpio/gpioX/value) [DONE]
// 2. grab images from camera (shell command)[DONE]
// 3. check gpio every .5 sec (timer) [DONE]
// 4. mail images or tweet or whatever [DONE]
// 5. (optional) store images to local disk [DONE]
//
// Willem van Doesburg (c) 2011
//*************************************************************
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <obstack.h>
#include <stdlib.h>
#include <unistd.h>
//************************************************************
//
// Config section
//
//
//************************************************************

// update rate
#define SEC_INTERVAL 0 //seconds
#define USEC_INTERVAL 5000 // microseconds

// image dump path
#define IMAGE_PATH "/mnt/usb/dcim"

// email account for notification
#define NOTIFICATION_ADDRESS "your@email.here"

// email account for foto upload
#define IMAGE_ADDRESS "photobuckect@adress.here"

// amount of seconds to capture images (seconds)
#define RECORDING_TIME 4

// interval time between images (seconds)
#define RECORDING_INTERVAL 1
//************************************************************


//******* Obstack macros *************************************
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
//************************************************************

//signal handler
typedef void (*sighandler_t)(int);
static int secs = 0;

//filename storage struct
struct filename_store
{
  int nr_of_filenames;
  char* pFilenames;
  int size_of_chunks;
};
//************************

//handle signals
void MessageHandler(int iArg);

//destroy process
void Destructor(int iArg);

//snap pictures on  webcams
struct filename_store Capture_Images( int recording_time, int recording_interval, char* destination_path );

//mail pictures to someone
void Mail( char* to, char* subject, char* body, struct filename_store* filenamestore );

//setup PIR sensor on gpio
void SetupGPIO();

//poll PIR sensor on gpio
int CheckGPIO();

//when motion detected fire payload
void Payload();

//main loop
int detect_motion();
