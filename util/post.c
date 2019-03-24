#include "bbs.h"

static int
payArtMoney(int uid, int money, const char *item GCC_UNUSED, ...)
{
    va_list ap;
    char reason[STRLEN*3] ="";

    if (!money)
        return 0;

    if (item) {
        va_start(ap, item);
        vsnprintf(reason, sizeof(reason)-1, item, ap);
        va_end(ap);
    }

    int oldm, newm;
    const char *userid;
    time4_t     dtime = time(0);

    assert(money != 0);
    userid = getuserid(uid);
    assert(userid);
    assert(reason);

    if (!userid)
        return -1;

    oldm = moneyof(uid);
    newm = deumoney(uid, -money);

    {
        char buf[PATHLEN];
        sethomefile(buf, userid, FN_RECENTPAY);
        log_payment(buf, money, oldm, newm, reason, (int)(++dtime));
    }

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

    if (!is_validuserid(owner)){
        money = 0;
    }
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

    int uid;
    if ((uid = searchuser(owner, owner)) != 0) {
        payArtMoney(uid, -money, "%s 看板發文稿酬: %s", board, fhdr.title);
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
