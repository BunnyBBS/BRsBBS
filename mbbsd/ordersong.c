#include "bbs.h"

#define lockreturn(unmode, state)  if(lockutmpmode(unmode, state)) return
#define lockreturn0(unmode, state) if(lockutmpmode(unmode, state)) return 0
#define lockbreak(unmode, state)   if(lockutmpmode(unmode, state)) break
#define SONGBOOK  "etc/SONGBOOK"
#define OSONGPATH "etc/SONGO"

//#define ORDERSONG_MIN_NUMPOST   (3)
//#define ORDERSONG_MAX_BADPOST   (1)
//#define ORDERSONG_MIN_NUMLOGINDAYS   (30)
#define ORDERSONG_PAYMONEY   (100)
#define ORDERSONG_HOUR   (24)

#define MAX_SONGS (MAX_ADBANNER-100) // (400) XXX MAX_SONGS should be fewer than MAX_ADBANNER.

static void sortsong(void);

static int
do_order_song(void)
{
    char            sender[IDLEN + 1], receiver[IDLEN + 1] = "", buf[200],
		    genbuf[200], filename[256], say[51];
    char            trans_buffer[PATHLEN];
    char            address[IDLEN+1];
    FILE           *fp, *fp1;
    fileheader_t    mail;
    int             nsongs;
    char save_title[STRLEN];
    const char *override_receiver = NULL;

#if defined(ORDERSONG_MAX_BADPOST) && defined(ASSESS)
    if (cuser.badpost > ORDERSONG_MAX_BADPOST) {
        vmsgf("為避免濫用，留言前請先消除退文記錄至 %d 篇以下",
                ORDERSONG_MAX_BADPOST);
        return 0;
    }
#endif
	
#ifdef ORDERSONG_MIN_NUMLOGINDAYS
    if (cuser.numlogindays < ORDERSONG_MIN_NUMLOGINDAYS) {
        vmsgf("為避免濫用，留言前要先有%s %d %s",
                STR_LOGINDAYS, ORDERSONG_MIN_NUMLOGINDAYS, STR_LOGINDAYS_QTY);
        return 0;
    }
#endif

#ifdef ORDERSONG_MIN_NUMPOST
    if (cuser.numposts < ORDERSONG_MIN_NUMPOST) {
        vmsgf("為避免濫用，留言前要先有%d篇發文", ORDERSONG_MIN_NUMPOST);
        return 0;
    }
#endif

    strlcpy(buf, Cdatedate(&now), sizeof(buf));
    lockreturn0(OSONG, LOCK_MULTI);
    pwcuReload();

    while (1) {
	char ans[4];
	clear();
	vs_hdr2(" " BBSMNAME2 "超市 ", " 點播動態");
	prints("親愛的 %s 歡迎來到心情點播留言系統\n\n", cuser.userid);
	outs(ANSI_COLOR(1) "注意心情點播內容請勿涉及謾罵 人身攻擊 猥褻"
	     "公然侮辱 誹謗\n"
	     "若有上述違規情形，站方將保留決定是否公開內容的權利\n"
         "且違規者將不受匿名保護(其 ID 可被公佈於公開看板)\n"
	     "如不同意請按 Q 離開。" ANSI_RESET "\n");
	getdata(6, 0,"開始留言(Y)  看點歌本(S)  離開[Q] ", ans, sizeof(ans), LCECHO);

	if (ans[0] == 'y'){
	    break;
	}else if (ans[0] == 's') {
	    a_menu("留言範本", SONGBOOK, 0, 0, NULL, NULL);
	    clear();
	}else if (ans[0] == 'q') {
	    unlockutmpmode();
	    return 0;
    }else{
	    unlockutmpmode();
	    return 0;
	}
	}

    getdata_str(7, 0, "留言者(可匿名): ", sender, sizeof(sender), DOECHO,
                cuser.userid);

#ifdef USE_ANGEL_SONG
    override_receiver = angel_order_song(receiver, sizeof(receiver));
#endif

    if (!*receiver)
        getdata(8, 0, "留言給(可匿名): ", receiver, sizeof(receiver), DOECHO);

    do {
	getdata(9, 0, "想要要對他/她說..:", say, sizeof(say), DOECHO);
	reduce_blank(say, say);
	if (!say[0]) {
	    bell();
	    mvouts(10, 0, "請重新輸入想說的內容");
	}
    } while (!say[0]);

    snprintf(save_title, sizeof(save_title), "%s:%s", sender, say);

    if (override_receiver) {
        *address = 0;
    } else do {
        getdata_str(11, 0, "寄到誰的信箱(站內真實ID)?",
                    address, sizeof(address), LCECHO, receiver);
        if (!*address)
            break;
        if (searchuser(address, address)) {
            if (is_rejected(address)) {
                vmsg("對方拒收");
                continue;
            } else {
                break;
            }
        }
        vmsg("請輸入站內 ID 或直接 ENTER");
    } while (1);

#ifdef ORDERSONG_PAYMONEY

#ifdef ORDERSONG_HOUR
	time4_t dtime = time(0);
	move(b_lines-2, 0);
	if(cuser.lastsong < ((int)(++dtime) - (3600 * (int)ORDERSONG_HOUR)))
		prints("上次點歌已超過%d小時，免支付點歌費用！", (int)ORDERSONG_HOUR);
	else{
#endif //ORDERSONG_HOUR
		prints("即將支付點播費用 %d %s", ORDERSONG_PAYMONEY, MONEYNAME);
		getdata(b_lines - 1, 0, "確定要支付了嗎？ (y/N)",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
		    unlockutmpmode();
		    return 0;
		}

		reload_money();
		if (cuser.money < (int)ORDERSONG_PAYMONEY){
			vmsg(MONEYNAME "不夠繳納點播費用...");
			unlockutmpmode();
			return 0;
		}else
			pay((int)ORDERSONG_PAYMONEY, "繳納點播費用。");
#ifdef ORDERSONG_HOUR
	}
#endif //ORDERSONG_HOUR

#endif //ORDERSONG_PAYMONEY

    vmsg("接著要選範本囉...");
    a_menu("留言範本", SONGBOOK, 0, 0, trans_buffer, NULL);
    if (!trans_buffer[0] || strstr(trans_buffer, "home") ||
	strstr(trans_buffer, "boards") || !(fp = fopen(trans_buffer, "r"))) {
	unlockutmpmode();
	return 0;
    }
#ifdef DEBUG
    vmsg(trans_buffer);
#endif
    strlcpy(filename, OSONGPATH, sizeof(filename));
    stampfile(filename, &mail);
    unlink(filename);

    if (!(fp1 = fopen(filename, "w"))) {
	fclose(fp);
	unlockutmpmode();
	return 0;
    }
    strlcpy(mail.owner, "[心情點播]", sizeof(mail.owner));
    snprintf(mail.title, sizeof(mail.title), "◇ %s 留言給 %s ", sender,
             receiver);

    while (fgets(buf, sizeof(buf), fp)) {
	char           *po;
	if (!strncmp(buf, "標題: ", 6)) {
	    clear();
	    move(10, 10);
	    outs(buf);
	    pressanykey();
	    fclose(fp);
	    fclose(fp1);
	    unlockutmpmode();
	    return 0;
	}
	while ((po = strstr(buf, "<~Src~>"))) {
	    const char *dot = "";
	    if (is_validuserid(sender) && strcmp(sender, cuser.userid) != 0)
		dot = ".";
	    po[0] = 0;
	    snprintf(genbuf, sizeof(genbuf), "%s%s%s%s", buf, sender, dot, po + 7);
	    strlcpy(buf, genbuf, sizeof(buf));
	}
	while ((po = strstr(buf, "<~Des~>"))) {
            const char *r = receiver;
#ifdef PLAY_ANGEL
            if (strstr(po, "小天使") && strstr(receiver, "小天使") &&
                override_receiver) {
                r = override_receiver;
            }
#endif
	    po[0] = 0;
	    snprintf(genbuf, sizeof(genbuf), "%s%s%s", buf, r, po + 7);
	    strlcpy(buf, genbuf, sizeof(buf));
	}
	while ((po = strstr(buf, "<~Say~>"))) {
	    po[0] = 0;
	    snprintf(genbuf, sizeof(genbuf), "%s%s%s", buf, say, po + 7);
	    strlcpy(buf, genbuf, sizeof(buf));
	}
	fputs(buf, fp1);
    }
    fclose(fp1);
    fclose(fp);

    log_filef("etc/osong.log",  LOG_CREAT,
              "id: %-12s ◇ %s 留言給 %s : \"%s\", 轉寄至 %s\n",
              cuser.userid, sender, receiver, say, address);

    LOG_IF(LOG_CONF_OSONG_VERBOSE,
           log_filef("log/osong_verbose.log",  LOG_CREAT,
                     "%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
                     Cdate(&now), cuser.userid, trans_buffer, address,
                     sender, receiver, say));

#ifdef USE_ANGEL_SONG
    if (override_receiver)
        angel_log_order_song(override_receiver);
#endif

    if (append_record(OSONGPATH "/" FN_DIR, &mail, sizeof(mail)) != -1) {
	pwcuSetLastSongTime(now);
	/* Jaky 超過 MAX_ADBANNER 首歌就開始砍 */
	// XXX 載入的順序會長得像是:
	// 3. ◆ <系統> 動態看板   SYSOP [01/23/08]
	// 4. ◆ <點歌> 動態看板   Ptt   [08/26/09]
	// 5. ◆ <廣告> 動態看板   SYSOP [08/22/09]
	// 6. ◆ <看板> 動態看板   SYSOP [04/16/09]
	// 由於點歌部份算是早載入的，不能直接用 MAX_ADBANNER 不然後面都沒得玩。
	nsongs = get_num_records(OSONGPATH "/" FN_DIR, sizeof(mail));
	if (nsongs > MAX_SONGS) {
	    // XXX race condition
	    delete_range(OSONGPATH "/" FN_DIR, 1, nsongs - MAX_SONGS);
	}
	snprintf(genbuf, sizeof(genbuf), "%s says \"%s\" to %s.",
		sender, say, receiver);
	log_usies("OSONG", genbuf);
    }
    snprintf(save_title, sizeof(save_title), "%s:%s", sender, say);
    hold_mail(filename, receiver, save_title);
    if (*address) {
	bsmtp(filename, save_title, address, NULL);
    }

    clear();
    outs(
	 "\n\n  恭喜您完成囉...\n"
	 "  一小時內動態看板會自動重新更新，\n"
	 "  大家就可以看到您的心情點播留言囉！\n\n"
	 "  有任何問題可以到 " BN_NOTE " 板的精華區找答案，\n"
	 "  也可在 " BN_NOTE " 板精華區看到自己的留言記錄。\n"
	 "  有任何寶貴的意見也歡迎到 " BN_NOTE " 看板提出，\n"
	 "  讓親切的板主為您服務。\n");
    pressanykey();
    sortsong();
    topsong();

    unlockutmpmode();
    return 1;
}

int
ordersong(void)
{
    do_order_song();
    return 0;
}

// topsong

#define QCAST     int (*)(const void *, const void *)

typedef struct songcmp_t {
    char            name[100];
    char            cname[100];
    int             count;
}               songcmp_t;


static int
count_cmp(songcmp_t * b, songcmp_t * a)
{
    return (a->count - b->count);
}

int
topsong(void)
{
    more(FN_TOPSONG, YEA);
    return 0;
}


static void
sortsong(void)
{
    FILE           *fo, *fp = fopen(BBSHOME "/" FN_USSONG, "r");
    songcmp_t       songs[MAX_SONGS + 1];
    int             n;
    char            buf[256], cbuf[256];
    int totalcount = 0;

    memset(songs, 0, sizeof(songs));
    if (!fp)
	return;
    if (!(fo = fopen(FN_TOPSONG, "w"))) {
	fclose(fp);
	return;
    }
    totalcount = 0;
    /* XXX: 除了前 MAX_SONGS 首, 剩下不會排序 */
    while (fgets(buf, 200, fp)) {
	chomp(buf);
	strip_blank(cbuf, buf);
	if (!cbuf[0] || !isprint2((int)cbuf[0]))
	    continue;

	for (n = 0; n < MAX_SONGS && songs[n].name[0]; n++)
	    if (!strcmp(songs[n].cname, cbuf))
		break;
	strlcpy(songs[n].name, buf, sizeof(songs[n].name));
	strlcpy(songs[n].cname, cbuf, sizeof(songs[n].cname));
	songs[n].count++;
	totalcount++;
    }
    qsort(songs, MAX_SONGS, sizeof(songcmp_t), (QCAST) count_cmp);
    fprintf(fo,
	    "    " ANSI_COLOR(36) "──" ANSI_COLOR(37) "名次" ANSI_COLOR(36) "──────" ANSI_COLOR(37)
	    "範本" ANSI_COLOR(36) "───────────" ANSI_COLOR(37) "次數" ANSI_COLOR(36)
	    "──" ANSI_COLOR(32) "共%d次" ANSI_COLOR(36) "──" ANSI_RESET "\n", totalcount);
    for (n = 0; n < 100 && songs[n].name[0]; n++) {
	fprintf(fo, "      %5d. %-38.38s %4d " ANSI_COLOR(32) "[%.2f]" ANSI_RESET "\n", n + 1,
		songs[n].name, songs[n].count,
		(float)songs[n].count / totalcount);
    }
    fclose(fp);
    fclose(fo);
}
