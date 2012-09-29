#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "../motion-detect-project/motion-detect.h"

int main(void)
{
	pid_t daemon_pid, daemon_sid;
	
	//fork off the main process
	daemon_pid = fork();
	if( daemon_pid < 0 )
		exit(-1);

	if( daemon_pid > 0 )
		exit(0);

	//Change the file mode mask
	umask(0);

	//We won't do any logging because of the limited space available on the BB

	//Create session ID
	daemon_sid = setsid();
	if( daemon_sid < 0 )
		exit(-1);

	//chdir to root
	if( (chdir("/")) < 0 )
		exit(-1);
	
	//close std file descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	//do any initialization

	//main loop
	detect_motion();

	exit(0);

}
