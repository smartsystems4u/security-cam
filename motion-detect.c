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
#include "motion-detect.h"

//handle signals
void MessageHandler(int iArg)
{
  signal( SIGALRM, MessageHandler);
  secs += iArg;
  
  struct itimerval tv;
  tv.it_interval.tv_sec = 0;
  tv.it_interval.tv_usec = 0;
  tv.it_value.tv_sec = SEC_INTERVAL;
  tv.it_value.tv_usec = USEC_INTERVAL;
  
  setitimer(ITIMER_REAL, &tv, 0 );
  
  //Do we have motion?
  if( CheckGPIO() == 1 )
    Payload(); //do your thing
}

//destroy process
void Destructor(int iArg)
{
  printf( "Exiting now from the destructor!.\n");
  signal(SIGINT, Destructor);
  exit(0);
}

//snap pictures on webcam
struct filename_store Capture_Images( int recording_time, int recording_interval, char* destination_path )
{
  //Current design is tailored to my rig. I have two webcams so these are used to grab images.
  //Adjust code according to your rig.

  struct filename_store imagefiles;
  struct obstack mystack; //memorypool
  imagefiles.nr_of_filenames = 0;
  imagefiles.size_of_chunks = 255;
  obstack_init( &mystack );

  time_t start_time = time(NULL);
  while( difftime( time(NULL), start_time) < (double)recording_time )
  {
    char command[255];
    char command2[255];
    char filename[255];
    char filename2[255];
    time_t current_time_t = time(NULL);
    struct tm *current_time_tm = NULL;
    current_time_tm = localtime(&current_time_t);
    strftime( &filename, 255, "vid0-%Y-%m-%d-%H-%M-%S.jpg", current_time_tm);
    obstack_grow( &mystack, &filename, imagefiles.size_of_chunks );
    imagefiles.nr_of_filenames++;
    sprintf( &command, "fswebcam -d /dev/video0 -r 640x480 \"%s/%s\"", destination_path, filename );
    system( command );
    strftime( &filename2, 255, "vid1-%Y-%m-%d-%H-%M-%S.jpg", current_time_tm);
    obstack_grow( &mystack, &filename2, imagefiles.size_of_chunks );
    imagefiles.nr_of_filenames++;
    sprintf( &command2, "fswebcam -d /dev/video1 -r 1280x960 -p YUYV \"%s/%s\"", destination_path, filename2 );
    system( command2 );
    sleep(recording_interval);
  }
  //return the filenames
  imagefiles.pFilenames = obstack_finish( &mystack ); 
  return imagefiles; 
}

//mail snapshots to someone
void Mail( char* to, char* subject, char* body, struct filename_store* filenamestore )
{
  char message[8096]; //choosing a hard limit here for safety
  if( filenamestore != NULL )
  {
    //create string with list of filenames
    char names[5000];
    char singlename[255];
    int i = 0;
    int listindex = 0;
    for( i=0; i<filenamestore->nr_of_filenames; i++ )
    {
      memcpy( &singlename, filenamestore->pFilenames+(i*filenamestore->size_of_chunks), filenamestore->size_of_chunks );
      int length = strlen( &singlename );
      memcpy(&names[listindex], &singlename, length);
      listindex += length;
      names[listindex] = ' ';
      listindex++; 
    }
    names[listindex] = 0; //terminate the string

    //use mutt to mail
    sprintf( &message, "echo \"%s\" | mutt -s \"%s\" %s -a %s -- ", body, subject, to, &names );
  } else sprintf( &message, "echo \"%s\" | mutt -s \"%s\" %s", body, subject, to );
  
  char curdir[255];
  getcwd( &curdir, 255 );
  chdir( IMAGE_PATH );
  system( message );
  chdir( &curdir );
}

//setup GPIO
void SetupGPIO()
{
  system( "echo 7 > /sys/class/gpio/export" ); //export serial RT (GPIO 7, pin 8) as GPIO
  system( "echo in > /sys/class/gpio/gpio7/direction" ); //configure for reading
}

//Check PIR on GPIO for motion
int CheckGPIO()
{
  FILE* gpiofile = fopen( "/sys/class/gpio/gpio7/value", "r" );
  char value = fgetc( gpiofile );
  fclose( gpiofile );
  
  if( value == '1')
    return 1;
  else
    return 0; 
}

//if motion detected then fire payload
void Payload()
{
  //grab frames from webcams (for x sec, after y sec. delay)
  //changestate to spent
  //-- check for GPIO to fall
  //---- changestate to armed
  //notify of payload
  struct filename_store grabnames;
  grabnames.nr_of_filenames = 0;
  grabnames.size_of_chunks = 255;
  
  printf( "Motion detected!\n" );  
  
  printf( "Grabbing images from webcam(s) every %d seconds for a total of %d seconds, saving files to %s.\n", RECORDING_TIME, RECORDING_INTERVAL, IMAGE_PATH );
  grabnames = Capture_Images( RECORDING_TIME, RECORDING_INTERVAL, IMAGE_PATH );
  printf( "emailing files to: %s.\n", IMAGE_ADDRESS );
  Mail( IMAGE_ADDRESS, "Motion Detected", "New motion detected.", &grabnames );
  printf( "sending notification email to: %s.\n", NOTIFICATION_ADDRESS );
  Mail( NOTIFICATION_ADDRESS, "motion was detected!", "sensors just detected motion, check photobucket for images", NULL );  

  if( CheckGPIO() == 1 )
    printf("GPIO still high waiting for state to go to \"ARMED\". \n" );
    
  while( CheckGPIO() == 1 )
  {
    sleep(1);
    printf( "." );
  }
    
  printf( "System is now ARMED again.\n" );          
}


int detect_motion()
{
  //setup GPIO
  SetupGPIO();

  //setup timer
  struct itimerval tv;
  tv.it_interval.tv_sec = 0;
  tv.it_interval.tv_usec = 0;
  tv.it_value.tv_sec = SEC_INTERVAL;
  tv.it_value.tv_usec = USEC_INTERVAL;
  setitimer(ITIMER_REAL, &tv, 0);
  
  //register for alarm and exit signals
  signal(SIGALRM, MessageHandler);
  signal(SIGINT, Destructor );
  while(1)
  {
    sleep(400); //yield to other programs
  }

  printf( "Exiting now..\n" );
  return 0;
}
