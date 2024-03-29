
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <syslog.h>
#include "config.h"
#include "osdep.h"

#define MAX_REMOTE_IP_LEN 32

static void get_remote_ip(int len, char *remote_ip)
{
    char frombuf[100];
    // note, SSH_CLIENT is deprecated since 2002
    char *ssh_client = getenv("SSH_CONNECTION");

    if (ssh_client) {
	// SSH_CONNECTION format: "client-ip client-port server-ip server-port"
	sscanf(ssh_client, "%s", frombuf);
    } else {
	strcpy(frombuf, "127.0.0.1");
    }

    strlcpy(remote_ip, frombuf, len);
}

/*
   show ban file
   if filename exist, print it out, sleep 10 second, and return 0;
   otherwise, return -1.
 */
static int showbanfile(const char *filename)
{
    FILE *fp;
    char buf[256];

    fp = fopen(filename, "r");
    if (fp)
    {
	while (fgets(buf, sizeof(buf), fp))
	    fputs(buf, stdout);
	printf("\n============================="
	       "=============================\n");
	fclose(fp);
	sleep(10);
    }
    return fp ? 0 : -1;
}

static int checkload()
{
    if (cpuload(NULL) <= MAX_CPULOAD)
    return 0;

    fputs("系統過載, 請稍後再來\n", stdout);
    sleep(10);

    return 1;
}

int main(int argc, const char **argv)
{
    int uid;
    int is_utf8 = 0;
    char remote_ip[MAX_REMOTE_IP_LEN + 1];
    (void)argc;

    if (strstr(argv[0], "utf8")) 
        is_utf8 = 1;

    openlog("bbsrf", LOG_PID | LOG_PERROR, LOG_USER);
    chdir(BBSHOME);
    uid = getuid();

    if (uid != BBSUID) {
	syslog(LOG_ERR, "UID DOES NOT MATCH");
	return -1;
    }
    if (!getpwuid(uid)) {
	syslog(LOG_ERR, "YOU DONT EXIST");
	return -1;
    }


    if (!showbanfile(BAN_FILE)) {
	return 1;
    }

    if (checkload()) {
    return 1;
    }

    get_remote_ip(sizeof(remote_ip), remote_ip);

    if (is_utf8)
        execl(BBSPROG, "mbbsd", "-D", "-e", "utf8", "-h", remote_ip, NULL);
    else
        execl(BBSPROG, "mbbsd", "-D", "-h", remote_ip, NULL);

    syslog(LOG_ERR, "execl(): %m");
    sleep(3); // prevent flooding

    return -1;
}
