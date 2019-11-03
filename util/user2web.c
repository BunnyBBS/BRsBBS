#define _UTIL_C_
#include "bbs.h"

#ifdef USE_BBS2WEB
/* 這個程式是將所有的使用者的簡單資料列出來，供網站串接同步使用者清單
 * 產生格式：
 * username, nickname, post_num, login_num, money, email
 */
int check(void *data, int n, userec_t *u) {
    (void)data;
    (void)n;

    if (!u->userid[0])
        return 0;

    printf("%s,%s,%d,%d,%d,%s\n",
           u->userid, u->nickname, u->numposts,
           u->numlogindays, u->money, u->email);
    return 0;
}

int main(void)
{
    chdir(BBSHOME);
    attach_SHM();

    if(passwd_init())
	exit(1);

    passwd_apply(NULL, check);
    return 0;
}
#else
int main(int argc, const char **argv)
{
    return 0;
}
#endif //USE_BBS2WEB
