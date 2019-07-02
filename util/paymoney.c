#include "bbs.h"

static int
payMoney(const char *username, int money, const char *reason)
{
    int oldm, newm, uid;
    time4_t dtime = time(0);
    time4_t now = (++dtime);

    uid = searchuser(username, NULL);
    if(uid == 0)
        return 0;

    oldm = moneyof(uid);
    newm = deumoney(uid, -money);

    char *buf[PATHLEN];
    snprintf(buf, PATHLEN, "/home/bbs/home/%c/%s/%s", username[0], username, FN_RECENTPAY);
    log_filef(buf,
              LOG_CREAT,
              "%s %s $%d ($%d => $%d) %s\n",
              Cdatelite(&now),
              money >= 0 ? "支出" : "收入",
              money >= 0 ? money : -money,
              oldm,
              newm,
              reason);

    return newm;
}

int main(int argc, char **argv)
{
    attach_SHM();
    if(argc != 4) {
    	printf("usage: %s <username> <money> <reason>\n", argv[0]);
    	return 0;
    }

    if(is_validuserid(argv[1])){
      payMoney(argv[1], atoi(argv[2]), argv[3]);
    }else{
        printf("No this user.\n");
    }
    return 0;
}
