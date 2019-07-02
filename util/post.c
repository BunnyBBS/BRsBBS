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

void keeplog(FILE *fin, char *board, char *title, char *owner, int money, char *ip) {
    fileheader_t fhdr;
    char genbuf[256], genbuf2[256], buf[512];
    time4_t     dtime = time(0);
    FILE *fout;
    int bid;
    
    sprintf(genbuf, BBSHOME "/boards/%c/%s", board[0], board);
    stampfile(genbuf, &fhdr);

    if(!(fout = fopen(genbuf, "w"))) {
	perror(genbuf);
	return;
    }
    
    while(fgets(buf, 512, fin))
	fputs(buf, fout);
    
    fclose(fin);
    fclose(fout);

#ifdef MAX_POST_MONEY
    if (money > MAX_POST_MONEY)
    money = MAX_POST_MONEY;
#endif

    strncpy(fhdr.title, title, sizeof(fhdr.title) - 1);
    fhdr.title[sizeof(fhdr.title) - 1] = '\0';
    fhdr.multi.money = money;
    strcpy(fhdr.owner, owner);
    sprintf(genbuf, BBSHOME "/boards/%c/%s/.DIR", board[0], board);
    append_record(genbuf, &fhdr, sizeof(fhdr));
    /* XXX: bid of cache.c's getbnum starts from 1 */
    if((bid = getbnum(board)) > 0)
        touchbtotal(bid);

    sprintf(genbuf2, BBSHOME "/boards/%c/%s/%s", board[0], board, fhdr.filename);
    log_filef(genbuf2, LOG_CREAT,
              "\n--\n※ 發信站: " BBSNAME "(" MYHOSTNAME "), 來自: %s\n", ip);
#ifdef QUERY_ARTICLE_URL
    log_filef(genbuf2, LOG_CREAT,
              "※ " URL_DISPLAYNAME ": " URL_PREFIX "/%s\n", fhdr.filename);
#endif

    if (is_validuserid(owner)){
        char buf2[200];
        snprintf(buf2, sizeof(buf2), "%s 看板發文稿酬: %s", board, fhdr.title);
        payMoney(owner, -money, buf2);
    }

    LOG_IF(LOG_CONF_POST,
           log_filef("log/post", LOG_CREAT,
                     "%d %s boards/%c/%s/%s %d\n",
                     (int)(++dtime), owner, board[0], board,
                     fhdr.filename, money));
    printf("%s", fhdr.filename);
}

int main(int argc, char **argv)
{
    FILE *fp, *fp2;

    attach_SHM();
    resolve_boards();
    if(argc != 7) {
	printf("usage: %s <board name> <title> <owner> <file> <money> <ip>\n", argv[0]);
	return 0;
    }
    
    if(strcmp(argv[4], "-") == 0)
	fp = stdin;
    else {
	fp = fopen(argv[4], "r");
	if(!fp) {
	    perror(argv[4]);
	    return 1;
	}
    }

    char *title, buf[200] = "";
    if(fp2 = fopen(argv[2], "r+")){
        fgets(buf, sizeof(buf), fp2);
        fclose(fp2);
        title = buf;
    }else{
        perror(argv[2]);
        return 1;
    }

    int money = atoi(argv[5]);
    if(money < 0)
        money = 0;

    keeplog(fp, argv[1], title, argv[3], money, argv[6]);
    return 0;
}
