#include "bbs.h"

aidu_t fn2aidu(const char *fn)
{
  aidu_t aidu = 0;
  aidu_t type = 0;
  aidu_t v1 = 0;
  aidu_t v2 = 0;
  char fnbuf[FNLEN + 1];
  char *sp = NULL;

  if(fn == NULL)
    return 0;
  strncpy(fnbuf, fn, FNLEN);
  fnbuf[FNLEN] = '\0';
  sp = fnbuf;

  switch(*(sp ++))
  {
    case 'M':
      type = 0;
      break;
    case 'G':
      type = 1;
      break;
    default:
      return 0;
      break;
  }

  if(*(sp ++) != '.')
    return 0;
  v1 = strtoul(sp, &sp, 10);
  if(sp == NULL)
    return 0;
  if(*sp != '.' || *(sp + 1) != 'A')
    return 0;
  sp += 2;
  if(*(sp ++) == '.')
  {
    v2 = strtoul(sp, &sp, 16);
    if(sp == NULL)
      return 0;
  }
  aidu = ((type & 0xf) << 44) | ((v1 & 0xffffffff) << 12) | (v2 & 0xfff);

  return aidu;
}

char *aidu2aidc(char *buf, const aidu_t orig_aidu)
{
  const char aidu2aidc_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
  const int aidu2aidc_tablesize = sizeof(aidu2aidc_table) - 1;
  char *sp = buf + 8;
  aidu_t aidu = orig_aidu;
  aidu_t v;

  *(sp --) = '\0';
  while(sp >= buf)
  {
    /* FIXME: 能保證 aidu2aidc_tablesize 是 2 的冪次的話，
              這裡可以改用 bitwise operation 做 */
    v = aidu % aidu2aidc_tablesize;
    aidu = aidu / aidu2aidc_tablesize;
    *(sp --) = aidu2aidc_table[v];
  }
  return buf;
}

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
    const char *fn = fhdr.filename;

#ifdef USE_AID_URL
    char aidc[32] = "";
    aidu_t aidu = fn2aidu(fhdr.filename);

    aidu2aidc(aidc, aidu);
    fn = aidc;
#endif //USE_AID_URL

#ifndef URL_EXTENSION
#define URL_EXTENSION ""
#endif

#ifdef URL_WITH_BRDNAME
    log_filef(genbuf2, LOG_CREAT,
              "※ " URL_DISPLAYNAME ": " URL_PREFIX "/%s/%s%s\n", board, fn, URL_EXTENSION);
#else
    log_filef(genbuf2, LOG_CREAT,
              "※ " URL_DISPLAYNAME ": " URL_PREFIX "/%s%s\n", fn, URL_EXTENSION);
#endif //URL_WITH_BRDNAME

#endif //QUERY_ARTICLE_URL

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
