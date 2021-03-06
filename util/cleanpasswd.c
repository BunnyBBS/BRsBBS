#define _UTIL_C_
#include "bbs.h"

/* 當資料欄位有異動 例如用了舊的欄位 可用這個程式清除舊值 */
int
OpenCreate(const char *path, int flags)
{
    return open(path, flags | O_CREAT, DEFAULT_FILE_CREATE_PERM);
}
size_t strlcpy(dst, src, siz)
    char *dst;
    const char *src;
    size_t siz;
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
    do {
        if ((*d++ = *s++) == 0)
        break;
    } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
    if (siz != 0)
        *d = '\0';      /* NUL-terminate dst */
    while (*s++)
        ;
    }

    return(s - src - 1);    /* count does not include NUL */
}

int main(int argc, char *argv[])
{
    int i, fd, fdw;
    userec_t user;
    
    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    if ((fd = open(BBSHOME"/.PASSWDS", O_RDONLY)) < 0){
	perror("open .PASSWDS error");
	exit(-1);
    }	    

    if ((fdw = OpenCreate(BBSHOME"/.PASSWDS.new", O_WRONLY | O_TRUNC)) < 0){
	perror("open .PASSWDS.new error");
	exit(-1);
    }

    for(i = 0; i < MAX_USERS; i++){
	if (read(fd, &user, sizeof(user)) != sizeof(user))
	    break;
        strlcpy(&user.cellphone, "", sizeof(&user.cellphone));
    	write(fdw, &user, sizeof(user));
    }
    close(fd);
    close(fdw);

    if (i != MAX_USERS)
	fprintf(stderr, "ERROR\n");
    else{
	fprintf(stderr, "DONE\n");
	system("/bin/mv " BBSHOME "/.PASSWDS " BBSHOME "/.PASSWDS.bak");
	system("/bin/mv " BBSHOME "/.PASSWDS.new " BBSHOME "/.PASSWDS");
    }
    return 0;
}
