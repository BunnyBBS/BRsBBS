#define PWCU_IMPL
#include "bbs.h"

#ifdef CHESSCOUNTRY
static const char * const chess_photo_name[4] = {
    "photo_fivechess",
    "photo_cchess",
    "photo_connect6",
    "photo_go",
};

static const char * const chess_type[4] = {
    "五子棋",
    "象棋",
    "六子旗",
    "圍棋",
};
#endif

void
ban_usermail(const userec_t *u, const char *reason) {
    assert(u);
    if (!(u->userlevel & PERM_LOGINOK))
        return;
    if (!strchr(u->email, '@'))
        return;
    if (!reason || !*reason)
        reason = "(站長忘了打)";
    log_filef("etc/banemail", LOG_CREAT,
              "# %s: %s (by %s)\nA%s\n",
              u->userid, reason, cuser.userid, u->email);
}

int
kill_user(int num, const char *userid)
{
    userec_t u;
    char src[256], dst[256];

    if(!userid || num<=0 ) return -1;
    sethomepath(src, userid);
    friend_delete_all(userid, FRIEND_ALOHA);
    if (dashd(src)) {
	snprintf(src, sizeof(src),
			 "/bin/tar zcvf backup/user_%s.tgz home/%c/%s >/dev/null 2>&1;"
			 "/bin/rm -fr home/%c/%s >/dev/null 2>&1"
			 , userid, userid[0], userid, userid[0], userid);
	system(src);
    }

    memset(&u, 0, sizeof(userec_t));
    log_usies("KILL", getuserid(num));
    setuserid(num, "");
    passwd_sync_update(num, &u);
    return 0;
}

int
u_loginview(void)
{
    int             i, in;
    unsigned int    pbits = cuser.loginview;

    do {
        vs_hdr("設定進站畫面");
        move(4, 0);
        for (i = 0; i < NUMVIEWFILE && loginview_file[i][0]; i++) {
            // ignore those without file name
            if (!*loginview_file[i][0])
                continue;
            prints("    %c. %-20s %-15s \n", 'A' + i,
                   loginview_file[i][1], ((pbits >> i) & 1 ? "ˇ" : "Ｘ"));
        }

        in = i; // max i
        i = vmsgf("請按 [A-%c] 切換設定，按 [ENTER] 結束：", 'A'+in-1);
        if (i == '\r')
            break;

        // process i
        i = tolower(i) - 'a';
        if (i >= in || i < 0 || !*loginview_file[i][0]) {
            bell();
            continue;
        }
        pbits ^= (1 << i);
    } while (1);

    if (pbits != cuser.loginview) {
	pwcuSetLoginView(pbits);
    }
    return 0;
}

/* TODO(piaip) 把這自動化？ */
int u_cancelbadpost(void)
{
   int day, prev = cuser.badpost;
   char ans[3];
   int pass_verify = 1;

   // early check.
   if(cuser.badpost == 0) {
       vmsg("你並沒有退文.");
       return 0;
   }

   // early check for race condition
   // XXX 由於帳號API已同步化 (pwcuAPI*) 所以這個 check 可有可無
   if(search_ulistn(usernum,2)) {
       vmsg("請登出其他視窗, 否則不受理.");
       return 0;
   }

   // XXX reload account here? (also simply optional)
   pwcuReload();
   prev = cuser.badpost; // since we reloaded, update cache again.
   if (prev <= 0) return 0;

   // early check for time (must do again later)
   day = (now - cuser.timeremovebadpost ) / DAY_SECONDS;
   if (day < BADPOST_CLEAR_DURATION) {
       vmsgf("離下次可以申請解除尚有 %d 天。", BADPOST_CLEAR_DURATION - day);
       return 0;
   }

   // 某些 user 會一直失敗，原因不明；由 vmsg 改為 getdata.
   clear();
   // 無聊的 disclaimer...
   mvprints(1, 0, "預計退文將由 %d 篇變為 %d 篇，確定嗎[y/N]? ", prev, prev-1);
   do {
       if (vgets(ans, sizeof(ans), VGET_LOWERCASE | VGET_ASCII_ONLY) < 1 ||
           ans[0] != 'y') { pass_verify = 0; break; }
       mvprints(3, 0, "我願意遵守站方規定,組規,以及板規[y/N]? ");
       if (vgets(ans, sizeof(ans), VGET_LOWERCASE | VGET_ASCII_ONLY) < 1 ||
           ans[0] != 'y') { pass_verify = 0; break; }
       mvprints(5, 0, "我願意尊重與不歧視族群,不鬧板,尊重各板主權力[y/N]?");
       if (vgets(ans, sizeof(ans), VGET_LOWERCASE | VGET_ASCII_ONLY) < 1 ||
           ans[0] != 'y') { pass_verify = 0; break; }
       mvprints(7, 0, "我願意謹慎發表有意義言論,不謾罵攻擊,不跨板廣告[y/N]?");
       if (vgets(ans, sizeof(ans), VGET_LOWERCASE | VGET_ASCII_ONLY) < 1 ||
           ans[0] != 'y') { pass_verify = 0; break; }
   } while (0);

   if(!pass_verify)
   {
       vmsg("回答有誤，刪除失敗。請仔細看清站規與系統訊息後再來申請刪除.");
       return 0;
   }

   if (pwcuCancelBadpost() != 0) {
       vmsg("刪除失敗，請洽站務人員。");
       return 0;
   }

   log_filef("log/cancelbadpost.log", LOG_CREAT,
	   "%s %s 刪除一篇退文 (%d -> %d 篇)\n",
	   Cdate(&now), cuser.userid, prev, cuser.badpost);

   vmsgf("恭喜您已成功\刪除一篇退文 (由 %d 變為 %d 篇)",
	   prev, cuser.badpost);
   return 0;
}

/* 站務人員查詢使用者資料警訊系統 */
int
user_display_advanced_auth(void)
{
	if(!HasUserPerm(PERM_SYSOP)){
		vmsg("權限不足");
		return 0;
	}

	clear();
	vs_hdr2(" 民政事務局 ", " 授權認證設定");

	char userid[IDLEN+1];
	move(1,0);
	usercomplete("請輸入要設定的帳號 ", userid);
	if (!is_validuserid(userid)){
		vmsg("查無ID");
		return 0;
	}

	FILE *fp;
	char buf[200], code[9];
	sethomefile(buf, userid, "query_data.auth");
	move(4, 0);
	prints("設定帳號： %s\n\n", userid);
	if(fp = fopen(buf, "r")){
		outs( " " ANSI_COLOR(1;31) "注意: " ANSI_COLOR(1;37) "這個帳號已經有被設定一組授權碼尚未被使用，不可以設定第二組。" ANSI_RESET "\n\n");
		fgets(code, sizeof(code), fp);
		fclose(fp);
		outs(" 授權認證碼：" ANSI_COLOR(1)); outs(code); outs(ANSI_RESET "\n");
		outs(" 共計8碼，會出現英文字母I、L、O，不會出現數字0、1。\n");
		pressanykey();
	}else{
		if(!(fp = fopen(buf, "w"))){
			vmsg("系統錯誤，請稍後再試。(Error code: UDA-G-001)");
			return 0;
		}
		const char * const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
	    for (int i = 0; i < 8; i++)
		code[i] = chars[random() % strlen(chars)];
	    code[8] = '\0';
		fprintf(fp,"%s", code);
		fclose(fp);
		outs(" 授權認證碼：" ANSI_COLOR(1)); outs(code); outs(ANSI_RESET "\n");
		outs(" 共計8碼，會出現英文字母I、L、O，不會出現數字0、1。");
		pressanykey();
	}
	return 0;
}

int
user_display_advanced(const userec_t * u, int adminmode)
{
	if(!adminmode){
		vmsg("權限錯誤");
		return 0;
	}
	if(!HasUserPerm(PERM_SYSOP|PERM_ACCOUNTS|PERM_BBSADM)){
		vmsg("權限不足");
		return 0;
	}
	if(strcmp(u->userid, cuser.userid) == 0){
		vmsg("你不認識你自己的資料嗎？");
		return 0;
	}

	clear();
	vs_hdr2(" 民政事務局 ", " 使用者進階查詢");
    prints("\n即將查詢 %s 的資料\n", u->userid);

	if(!HasUserPerm(PERM_SYSOP)){
		char code[9], code_input[9], buf[200];
    	FILE *fp;
    	sethomefile(buf, cuser.userid, "query_data.auth");
		if (!(fp = fopen(buf, "r"))){
			vmsg("總站長沒有設定授權碼，請先洽詢總站長。");
			return 0;
		}
		fgets(code, sizeof(code), fp);
		fclose(fp);
		getdata(5, 0, "請輸入您的授權碼：", code_input, sizeof(code_input), DOECHO);

		if (!strcmp(code, code_input)){
			unlink(buf);
		}else{
			vmsg("認證失敗。");
			return 0;
		}
	}

	move(5, 0); clrtobot();
	outs(ANSI_COLOR(1;31) "注意: "ANSI_COLOR(37)"您將使用站長權限查詢使用者資料，若無合理理由請勿任意進行操作。" ANSI_RESET);
    char genbuf2[200];
	getdata(b_lines - 3, 0, "您確定要繼續操作？[y/N] ", genbuf2, 3, LCECHO);
	if(genbuf2[0] != 'y'){
		vmsg("下次請您看清楚再來。");
		return 0;
	}else{
		int inform_user = 1;
		userec_t        atuser;
		if(HasUserPerm(PERM_SYSOP)){
			getdata(b_lines - 2, 0, "通知被查詢的國民？[Y/n] ", genbuf2, 3, LCECHO);
			if(genbuf2[0] == 'n'){
				move(7, 0); clrtobot();
				outs(ANSI_COLOR(1;31) "注意："ANSI_COLOR(37)"如不通知國民，您需要一位具公務權限的國民證明您的操作，"ANSI_RESET"\n");
				outs("      "ANSI_COLOR(1)"您完成查詢後系統會通知該名公務人員，且您的操作仍然會被紀錄於國家安全局。"ANSI_RESET"\n");
				outs(ANSI_COLOR(1;33) "提醒："ANSI_COLOR(37)"請您遵照現有作業程序規定進行操作，避免違反大兔個資保護政策。"ANSI_RESET"\n\n");
				char  *witness[IDLEN+1]; int uid;
				usercomplete("請輸入協助證明之公務人員：", witness);
				if (!(uid = searchuser(witness, NULL))){
					vmsg("查無這個帳號！");
					return 0;
				}
			    passwd_sync_query(uid, &atuser);
                if (!(atuser.userlevel & PERM_ADMIN)) {
					vmsg("這個帳號不具有公務身份！");
					return 0;
                }
	            inform_user = 0;
			}
		}
		FILE           *fp;
		char            reason[100];
		char bmStr[IDLEN * 3 + 10];
		char * bmArr;
		int i;
		struct tm      ptime;
		char           *myweek = "日一二三四五六";
		localtime4_r(&now, &ptime);
		i = ptime.tm_wday << 1;

		clear();
		vs_hdr2(" 民政事務局 ", " 使用者進階查詢");
        mvouts(2 ,0, "查詢使用者資料，請輸入正當理由。未輸入理由將視同誤按不予查詢。");
		getdata(3, 0, "理由: ", reason, 40, DOECHO);

        if(reason[0] == NULL){
            vmsg("未輸入理由將視同誤按，下次請您看清楚再來。");
			return 0;
        }else{
            unlink("etc/queryData.mail");
			fp = fopen("etc/queryData.mail", "w+");
    		fprintf(fp,"\n國家安全局通知\n站務人員 %s 查詢了 %s 的個人資料，\n時間是%03d/%02d/%02d (%c%c) %02d:%02d:%02d，\n理由是 %s，\n如果您認為該站務人員的行為不當請立即至%s提報。\n若無其他異況可直接略過本通知。", cuser.userid, u->userid, ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday, myweek[i], myweek[i + 1],ptime.tm_hour, ptime.tm_min, ptime.tm_sec, reason, BN_SYSOP);
    		if(inform_user == 0)
    			fprintf(fp,"\n\n[此次操作未直接通知國民]\n本信已同時知會以下人員：%s", atuser.userid);
    		fclose(fp);
    		if(inform_user == 0)
    			mail_id(atuser.userid, "[通知] 有站務人員查詢 他人 個人資料", "etc/queryData.mail", "[國家安全局]");
    		else
    			mail_id(u->userid, "[通知] 有站務人員查詢 您的 個人資料", "etc/queryData.mail", "[國家安全局]");
    		post_file(BN_SECURITY, "[通知] 有站務人員查詢個人敏感資料", "etc/queryData.mail", "[國家安全局]");
            outs("正在查詢使用者資料…");
        }
	}

	clear();
    vs_hdr2(" 民政事務局 ", " 使用者進階查詢");
    outs("\n");
    prints("\t代號暱稱: %s (%s)\n\n", u->userid, u->nickname);
	prints("\t真實姓名: %s", u->realname);
#if FOREIGN_REG_DAY > 0
    prints(" %s%s",
	   u->uflag & UF_FOREIGN ? "(外籍: " : "",
	   u->uflag & UF_FOREIGN ?
		(u->uflag & UF_LIVERIGHT) ? "永久居留)" : "未取得居留權)"
		: "");
#elif defined(FOREIGN_REG)
	prints(" %s", u->uflag & UF_FOREIGN ? "(外籍)" : "");
#endif
	outs("\n"); // end of realname
		prints("\t職業學歷: %s\n", u->career);
		prints("\t居住地址: %s\n", u->address);
		prints("\t電子信箱: %s\n", u->email);
		prints("\t手機號碼: %s\n", u->cellphone);

    pressanykey();
	return 0;
}

void
user_display(const userec_t * u, int adminmode)
{
    int             diff = 0, tax;
    char            genbuf[200];

    // Many fields are available (and have sync issue) in user->query,
    // so let's minimize fields here.

    clear();
	vs_hdr2(" 民政事務局 ", " 使用者資料查詢");

    prints("\t代號暱稱: %s (%s)\n\n", u->userid, u->nickname);

	if(strcmp(u->userid, cuser.userid) == 0){
		prints("\t真實姓名: %s", u->realname);
#if FOREIGN_REG_DAY > 0
	    prints(" %s%s",
		   u->uflag & UF_FOREIGN ? "(外籍: " : "",
		   u->uflag & UF_FOREIGN ?
			(u->uflag & UF_LIVERIGHT) ? "永久居留)" : "未取得居留權)"
			: "");
#elif defined(FOREIGN_REG)
		prints(" %s", u->uflag & UF_FOREIGN ? "(外籍)" : "");
#endif
		outs("\n"); // end of realname
	    prints("\t職業學歷: %s\n", u->career);
	    prints("\t居住地址: %s\n", u->address);
	    prints("\t電子信箱: %s\n", u->email);
		prints("\t手機號碼: %s\n", u->cellphone);
    }

    prints("\t是否成年: %s滿18歲\n", u->over_18 ? "已" : "未");
	prints("\t%6s幣: %d " MONEYNAME "\n", BBSMNAME, u->money);
    prints("\t文章數量: %d 篇", u->numposts);
#ifdef ASSESS
    prints(" (退文: %u篇)", (unsigned int)u->badpost);
#endif // ASSESS
	prints("\n");
    sethomedir(genbuf, u->userid);
    prints("\t私人信箱: %d 封  (購買信箱: %d 封)\n",
	   get_num_records(genbuf, sizeof(fileheader_t)),
	   u->exmailbox);

    if (adminmode && HasUserPerm(PERM_SYSOP|PERM_ACCOUNTS)) {
		outc('\n');
		strcpy(genbuf, "bTCPRp#@XWBA#VSM0123456789ABCDEF");
		for (diff = 0; diff < 32; diff++)
		    if (!(u->userlevel & (1 << diff)))
			genbuf[diff] = '-';
		prints("\t帳號權限: %s\n", genbuf);
		prints("\t認證資料: %s\n", u->justify);
		prints("\t註冊狀況: %s\n", (u->userlevel & PERM_LOGINOK) ? "已註冊" : "未註冊");
	}

    if (adminmode && HasUserPerm(PERM_SYSOP|PERM_BOARD)) {
		int             i, count=0;
		boardheader_t  *bhdr;
		outs("\t擔任板主: ");
		for (i = num_boards(), bhdr = bcache; i > 0; i--, bhdr++) {
		    if ( is_uBM(bhdr->BM, u->userid)) {
				count++;
		    }
		}
		prints("%d 個看板\n",count);
#ifdef USE_BOARDTAX
		outs("\t納稅狀況: ");
		tax = getBoardTax(u->userid);
		if(tax == 0)
			outs("不須繳納\n");
		else{
			char *paid = isBrdTaxPaid(u->userid);
			if(paid == 0)
				outs("本月尚未繳納\n");
			else
				prints("%s",paid);
		}
		outc('\n');
#endif
	}

    prints("\t註冊日期: %s （已滿 %d 天）\n",
	    Cdate(&u->firstlogin), (int)((now - u->firstlogin)/DAY_SECONDS));
    prints("\t" STR_LOGINDAYS ": %d" STR_LOGINDAYS_QTY "\n",u->numlogindays);
    if (adminmode) {
        prints("\t最後上線: %s（%d 天沒來了）/ %s\n",
               Cdate(&u->lastlogin), (int)((now - u->lastlogin)/DAY_SECONDS), u->lasthost);
    } else {
	diff = (now - login_start_time) / 60;
	prints("\t停留期間: %d 小時 %2d 分\n",
	       diff / 60, diff % 60);
    }

    if (!adminmode)
    {
	outs((u->userlevel & PERM_LOGINOK) ?
		"\n您的註冊程序已經完成，歡迎加入本站" :
		"\n如果要提昇權限，請參考本站公佈欄辦理註冊");
    } else {
	// XXX list user pref here
	int i;
	static const char *uflag_desc[] = {
	    "拒收外信",
	    "新板加最愛",
	    "外藉",
	    "居留權",
	};
	static uint32_t uflag_mask[] = {
	    UF_REJ_OUTTAMAIL,
	    UF_FAV_ADDNEW,
	    UF_FOREIGN,
	    UF_LIVERIGHT,
	};
	char buf[PATHLEN];

	sethomefile(buf, u->userid, FN_FORWARD);
	if (dashs(buf) > 0)
	    outs("[自動轉寄]");

	for (i = 0; (size_t)i < sizeof(uflag_mask)/sizeof(uflag_mask[0]); i++)
	{
	    if (!(u->uflag & uflag_mask[i]))
		continue;
	    prints("[%s]", uflag_desc[i]);
	}
	prints("\n");
    }
}

int
list_user_board(void){
    char userid[IDLEN+1];
	int             i, j=0;
	boardheader_t  *bhdr;

	vs_hdr2(" 土地管理局 ", " 查詢使用者擔任的板主");
	usercomplete("請輸入要設定的ID ", userid);
	if (!is_validuserid(userid)){
		vmsg("查無ID");
		return 0;
	}
	move(1,0);clrtobot();
	prints("查詢ID  : %s\n",userid);
	outs("擔任板主: ");
	for (i = num_boards(), bhdr = bcache; i > 0; i--, bhdr++) {
	    if ( is_uBM(bhdr->BM, userid)) {
			outs(bhdr->brdname);
			outc(' ');
			j++;
	    }
	}
	outc('\n');
	if(j < 1){
		clear();
		vmsg("所查的ID沒有擔任板主");
		return 0;
	}else{
		mvouts(b_lines-1,0,"列出完成");
		pressanykey();
	}
}

void
mail_violatelaw(const char *crime, const char *police, const char *reason, const char *result)
{
    char            genbuf[200];
    fileheader_t    fhdr;
    FILE           *fp;

    sendalert(crime,  ALERT_PWD_PERM);

    sethomepath(genbuf, crime);
    stampfile(genbuf, &fhdr);
    if (!(fp = fopen(genbuf, "w")))
	return;
    fprintf(fp, "作者: [" BBSMNAME "警察局]\n"
	    "標題: [報告] 違法報告\n"
	    "時間: %s\n", ctime4(&now));
    fprintf(fp,
	    ANSI_COLOR(1;32) "%s" ANSI_RESET "判決：\n     " ANSI_COLOR(1;32) "%s" ANSI_RESET
	    "因" ANSI_COLOR(1;35) "%s" ANSI_RESET "行為，\n"
	    "違反本站站規，處以" ANSI_COLOR(1;35) "%s" ANSI_RESET "，特此通知\n\n"
	    "請到 " BN_LAW " 查詢相關法規資訊，並從主選單進入:\n"
	    "(P)lay【娛樂與休閒】=>(P)ay【" BBSMNAME2 "量販店 】=> (1)ViolateLaw 繳罰單\n"
	    "以繳交罰單。\n",
	    police, crime, reason, result);
    fclose(fp);
    strcpy(fhdr.title, "[報告] 違法判決報告");
    strcpy(fhdr.owner, "[" BBSMNAME "警察局]");
    sethomedir(genbuf, crime);
    append_record(genbuf, &fhdr, sizeof(fhdr));
}

void
kick_all(const char *user)
{
   userinfo_t *ui;
   int num = searchuser(user, NULL), i=1;
   while((ui = (userinfo_t *) search_ulistn(num, i)) != NULL)
       {
         if(ui == currutmp) i++;
         if ((ui->pid <= 0 || kill(ui->pid, SIGHUP) == -1))
                         purge_utmp(ui);
         log_usies("KICK ALL", user);
       }
}

void
violate_law(userec_t * u, int unum)
{
    char            ans[4], ans2[4];
    char            reason[128];
    move(1, 0);
    clrtobot();
    move(2, 0);
    outs("(1)Cross-post (2)亂發廣告信 (3)亂發連鎖信\n");
    outs("(4)騷擾站上使用者 (8)其他以罰單處置行為\n(9)砍 id 行為\n");
    getdata(5, 0, "(0)結束", ans, 3, DOECHO);
    switch (ans[0]) {
    case '1':
	strcpy(reason, "Cross-post");
	break;
    case '2':
	strcpy(reason, "亂發廣告信");
	break;
    case '3':
	strcpy(reason, "亂發連鎖信");
	break;
    case '4':
	while (!getdata(7, 0, "請輸入被檢舉理由以示負責：", reason, 50, DOECHO));
	strcat(reason, "[騷擾站上使用者]");
	break;
    case '8':
    case '9':
	while (!getdata(6, 0, "請輸入理由以示負責：", reason, 50, DOECHO));
	break;
    default:
	return;
    }
    getdata(7, 0, msg_sure_ny, ans2, 3, LCECHO);
    if (*ans2 != 'y')
	return;
    if (ans[0] == '9') {
	if (HasUserPerm(PERM_POLICE) && u->numlogindays > 60)
	{
	    vmsg("使用者" STR_LOGINDAYS "超過 60，無法砍除。");
	    return;
	}

        kick_all(u->userid);
	delete_allpost(u->userid);
        ban_usermail(u, reason);
        kill_user(unum, u->userid);
	post_violatelaw(u->userid, cuser.userid, reason, "砍除 ID");
    } else {
        kick_all(u->userid);
	u->userlevel |= PERM_VIOLATELAW;
	u->timeviolatelaw = now;
	u->vl_count++;
	passwd_sync_update(unum, u);
        if (*ans == '1' || *ans == '2')
            delete_allpost(u->userid);
	post_violatelaw(u->userid, cuser.userid, reason, "罰單處份");
	mail_violatelaw(u->userid, "站務警察", reason, "罰單處份");
    }
    pressanykey();
}

void Customize(void)
{
    char    done = 0;
    int     dirty = 0;
    int     key;

    const int col_opt = 54;

    /* cuser.uflag settings */
    static const unsigned int masks1[] = {
	UF_ADBANNER,
	UF_ADBANNER_USONG,
	UF_REJ_OUTTAMAIL,
	UF_DEFBACKUP,
    UF_SECURE_LOGIN,
	UF_FAV_ADDNEW,
	UF_FAV_NOHILIGHT,
	UF_NO_MODMARK,
	UF_COLORED_MODMARK,
#ifdef DBCSAWARE
	UF_DBCS_AWARE,
	UF_DBCS_DROP_REPEAT,
	UF_DBCS_NOINTRESC,
#endif
        UF_CURSOR_ASCII,
#ifdef USE_PFTERM
        UF_MENU_LIGHTBAR,
#endif
#ifdef PLAY_ANGEL
        UF_NEW_ANGEL_PAGER,
#endif
	0,
    };

    static const char* desc1[] = {
	"ADBANNER   顯示動態看板",
	"ADBANNER   顯示使用者心情點播(需開啟動態看板)",
	"MAIL       拒收站外信",
	"BACKUP     預設備份信件與其它記錄", //"與聊天記錄",
    "LOGIN      只允許\使用安全連線(ex, ssh)登入",
	"MYFAV      新板自動進我的最愛 (停用功\能)",
	"MYFAV      單色顯示我的最愛",
	"MODMARK    隱藏文章修改符號(推文/修文) (~)",
	"MODMARK    使用色彩代替修改符號 (+)",
#ifdef DBCSAWARE
	"DBCS       自動偵測雙位元字集(如全型中文)",
	"DBCS       忽略連線程式為雙位元字集送出的重複按鍵",
	"DBCS       禁止在雙位元中使用色碼(去除一字雙色)",
#endif
        "CURSOR     使用新式簡化游標",
#ifdef USE_PFTERM
        "CURSOR     (實驗性)啟用光棒選單系統",
#endif
#ifdef PLAY_ANGEL
        "ANGEL      (小天使)啟用新的神諭呼叫器設定界面",
#endif
	0,
    };

    while ( !done ) {
	int i = 0, ia = 0; /* general uflags */
	int iax = 0; /* extended flags */

	clear();
	showtitle("個人化設定", "個人化設定");
	move(2, 0);
	outs("您目前的個人化設定: \n");
	prints(ANSI_COLOR(32)"   %-11s%-*s%s" ANSI_RESET "\n",
		"分類", col_opt-11,
		"描述", "設定值");
	move(4, 0);

	/* print uflag options */
	for (i = 0; masks1[i]; i++, ia++)
	{
#ifdef PLAY_ANGEL
            // XXX dirty hack: ANGEL must be in end of list.
            if (strstr(desc1[i], "ANGEL ") == desc1[i] &&
                !HasUserPerm(PERM_ANGEL)) {
                ia--;
                continue;
            }
#endif
	    clrtoeol();
	    prints( ANSI_COLOR(1;36) "%c" ANSI_RESET
		    ". %-*s%s\n",
		    'a' + ia,
		    col_opt,
		    desc1[i],
		    HasUserFlag(masks1[i]) ?
		    ANSI_COLOR(1;36) "是" ANSI_RESET : "否");
	}
	/* extended stuff */
	{
	    static const char *wm[PAGER_UI_TYPES+1] =
		{"一般", "進階", "未來", ""};

	    prints("%c. %-*s%s\n",
		    '1' + iax++,
		    col_opt,
		    "PAGER      水球模式",
		    wm[cuser.pager_ui_type % PAGER_UI_TYPES]);
#ifdef PLAY_ANGEL
            // TODO move this to Ctrl-U Ctrl-P.
	    if (HasUserPerm(PERM_ANGEL))
	    {
		static const char *msgs[ANGELPAUSE_MODES] = {
		    "開放 (接受所有小主人發問)",
		    "停收 (只接受已回應過的小主人的問題)",
		    "關閉 (停止接受所有小主人的問題)",
		};
		prints("%c. %s%s\n",
			'1' + iax++,
			"ANGEL      小天使神諭呼叫器: ",
			msgs[currutmp->angelpause % ANGELPAUSE_MODES]);
	    }
#endif // PLAY_ANGEL
	}

	/* input */
	key = vmsgf("請按 [a-%c,1-%c] 切換設定，其它任意鍵結束: ",
		'a' + ia-1, '1' + iax -1);

	if (key >= 'a' && key < 'a' + ia)
	{
	    /* normal pref */
	    key -= 'a';

            if (masks1[key] == UF_SECURE_LOGIN && !mbbsd_is_secure_connection()) {
                vmsg("您必須使用安全連線才能修改此設定");
                continue;
            }
#ifdef USE_NOTILOGIN
            if (masks1[key] == UF_NOTIFY_LOGIN) {
                vmsg("請至「主選單→個人設定區→密碼與安全」設定。");
                continue;
            }
#endif
#ifdef USE_2FALOGIN
            if (masks1[key] == UF_TWOFA_LOGIN) {
                vmsg("請至「主選單→個人設定區→密碼與安全」設定。");
                continue;
            }
#endif

        	//大兔：打算拔掉這功能。所以不再開放設定這個選項。
            if (masks1[key] == UF_FAV_ADDNEW) {
                vmsg("此設定已停用。");
                continue;
            }

	    dirty = 1;
	    pwcuToggleUserFlag(masks1[key]);
	    continue;
	}

	if (key < '1' || key >= '1' + iax)
	{
	    done = 1; continue;
	}
	/* extended keys */
	key -= '1';

	switch(key)
	{
	    case 0:
		{
		    pwcuSetPagerUIType((cuser.pager_ui_type +1) % PAGER_UI_TYPES_USER);
		    vmsg("修改水球模式後請正常離線再重新上線");
		    dirty = 1;
		}
		continue;
	}
#ifdef PLAY_ANGEL
	if( HasUserPerm(PERM_ANGEL) ){
	    if (key == iax-1)
	    {
		angel_toggle_pause();
		// dirty = 1; // pure utmp change does not need pw dirty
		continue;
	    }
	}
#endif //PLAY_ANGEL
    }

    grayout(1, b_lines-2, GRAYOUT_DARK);
    move(b_lines-1, 0); clrtoeol();

    if(dirty)
    {
	outs("設定已儲存。\n");
    } else {
	outs("結束設定。\n");
    }

    redrawwin(); // in case we changed output pref (like DBCS)
    vmsg("設定完成");
}

static void set_chess(const char *name, int y,
                      uint16_t *p_win, uint16_t *p_lose, uint16_t *p_tie) {
    char buf[STRLEN];
    char prompt[STRLEN];
    char *p;
    char *strtok_pos;
    snprintf(buf, sizeof(buf), "%d/%d/%d", *p_win, *p_lose, *p_tie);
    snprintf(prompt, sizeof(prompt), "%s 戰績 勝/敗/和:", name);
    if (!getdata_str(y, 0, prompt, buf, 5 * 3 + 3, DOECHO, buf))
        return;
    p = strtok_r(buf, "/\r\n", &strtok_pos);
    if (!p) return;
    *p_win = atoi(p);
    p = strtok_r(NULL, "/\r\n", &strtok_pos);
    if (!p) return;
    *p_lose = atoi(p);
    p = strtok_r(NULL, "/\r\n", &strtok_pos);
    if (!p) return;
    *p_tie = atoi(p);
}

int
userPass_change(userec_t x, int adminmode, int unum)
{
    int i = 0, fail = 0;
    char buf[STRLEN], genbuf[200];
	
	clear();
	vs_hdr2(" " BBSNAME " ", " 修改密碼");
	
	move(2,0);
	prints("正在修改%s的密碼...\n\n",x.userid);
	if (!adminmode) {
		outs("以下操作需要先確認您的身份。\n");
		getdata(5, 0, MSG_PASSWD, buf, PASS_INPUT_LEN + 1, PASSECHO);
		buf[8] = '\0';
		if (!(checkpasswd(x.passwd, buf))){
			vmsg("密碼錯誤！");
			return 0;
		}
	}
	move(4,0);clrtobot();
    if (!getdata(4, 0, "請設定新密碼：", buf, PASS_INPUT_LEN + 1,PASSECHO)) {
	    vmsg("密碼設定取消, 繼續使用舊密碼");
		return 0;
	}
	strlcpy(genbuf, buf, PASSLEN);

	move(6, 0);
	outs("請注意設定密碼只有前八個字元有效，超過的將自動忽略。");

	getdata(5, 0, "請檢查新密碼：", buf, PASS_INPUT_LEN + 1, PASSECHO);
	if (strncmp(buf, genbuf, PASSLEN)) {
	    vmsg("新密碼確認失敗, 無法設定新密碼");
		return 0;
	}
	buf[8] = '\0';
	strlcpy(x.passwd, genpasswd(buf), sizeof(x.passwd));
	
	// for admin mode, do verify after.
	if (adminmode)
	{
            FILE *fp;
	    char  witness[3][IDLEN+1], title[100];
	    int uid;
	    for (i = 0; i < 3; i++) {
		if (!getdata(7 + i, 0, "請輸入協助證明之使用者：",
			     witness[i], sizeof(witness[i]), DOECHO)) {
		    outs("\n不輸入則無法更改\n");
			return 0;
		} else if (!(uid = searchuser(witness[i], NULL))) {
		    outs("\n查無此使用者\n");
		    return 0;
		} else {
		    userec_t        atuser;
		    passwd_sync_query(uid, &atuser);
                    if (!(atuser.userlevel & PERM_LOGINOK)) {
                        outs("\n使用者未通過認證，請重新輸入。\n");
                        i--;
                    } else if (atuser.numlogindays < 6*30) {
			outs("\n" STR_LOGINDAYS "未超過 180，請重新輸入\n");
			i--;
		    }
		    // Adjust upper or lower case
                    strlcpy(witness[i], atuser.userid, sizeof(witness[i]));
		}
	    }

	    if (i < 3 || vans(msg_sure_ny) != 'y')
			return 0;

	    sprintf(title, "%s 的密碼重設通知 (by %s)",x.userid, cuser.userid);
            unlink("etc/updatepwd.log");
	    if(! (fp = fopen("etc/updatepwd.log", "w")))
	    {
		move(b_lines-1, 0); clrtobot();
		outs("系統錯誤: 無法建立通知檔，請至 " BN_BUGREPORT " 報告。");
		return 0;
	    }

	    fprintf(fp, "%s 要求密碼重設:\n"
		    "見證人為 %s, %s, %s",
		    x.userid, witness[0], witness[1], witness[2] );
	    fclose(fp);

	    post_file(BN_SECURITY, title, "etc/updatepwd.log", "[系統安全局]");
	    mail_id(x.userid, title, "etc/updatepwd.log", cuser.userid);
	    for(i=0; i<3; i++)
	    {
		mail_id(witness[i], title, "etc/updatepwd.log", cuser.userid);
	    }
	}
	
    passwd_sync_update(unum, &x);

    if (!adminmode){
		vmsg("已修改密碼，請重新登入。");
		kick_all(x.userid);
	}else{
		kick_all(x.userid);
		vmsg("已修改密碼。");
	}
	
	return 0;
}

static const char *
sms_verify_send(char *user, char *authcode, char *cellphone)
{
	int ret, code = 0;
	char uri[320] = "",buf[200];

	snprintf(uri, sizeof(uri), "/%s?user=%s&cellphone=%s&code=%s"
#ifdef BETA
			 "&beta=true"
#endif
			 , SMS_VERIFY_URI, user, cellphone , authcode
			);

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", IBUNNY_API_KEY);
	ret = thttp_get(&t, IBUNNY_SERVER, uri, IBUNNY_SERVER, buf);
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(code == 200)
		return NULL;

	if(code == 400)
		return "帳號細節有誤，無法發送驗證碼。(SMS-V400)";
	if(code == 401)
		return "API串接出錯，請聯繫工程業務處協助。(SMS-V401)";
	if(code == 410)
		return "帳號限額已滿，無法發送驗證碼。(SMS-V410)";
	if(code == 500)
		return "伺服器出錯，請聯繫工程業務處協助。(SMS-V500)";
	if(ret)
		return "系統錯誤，請聯繫工程業務處協助。(SMS-V001)";
}
int cellphone_verify(char *user, char *cellphone)
{
	mvouts(6,0,"即將驗證您的手機號碼…");clrtobot();pressanykey();
	char code[7], code_input[7];
	const char *msg = NULL;
	const char * const chars = "1234567890";
    for (int i = 0; i < 6; i++)
		code[i] = chars[random() % strlen(chars)];
    code[6] = '\0';
	msg = sms_verify_send(user, code, cellphone);
	if (msg != NULL){
		move(10,0);
		prints("錯誤：%s",msg);
		return -1;
	}
	for (int i = 3; i > 0; i--) {
		if (i < 3) {
			char buf2[80];
			move(10, 0);
			snprintf(buf2, sizeof(buf2), ANSI_COLOR(1;31) "驗證碼錯誤，您還有 %d 次機會。" ANSI_RESET, i);
			outs(buf2);
		}
		code_input[0] = '\0';
		getdata(9, 0, "請輸入驗證碼：", code_input,
			sizeof(code_input), DOECHO);

		size_t length = strlen(code_input);
		if(length == 6){
			if (!strcmp(code, code_input)){
				return 0;
			}
		}
    }
    return -1;
}

void
uinfo_query(const char *orig_uid, int adminmode, int unum)
{
    userec_t        x;
    int    i = 0, fail;
    int             ans;
    char            buf[STRLEN];
    char            genbuf[200];
    char	    pre_confirmed = 0;
    int y = 0;
    int perm_changed;
    int mail_changed;
    int money_changed;
    int tokill = 0;
    int changefrom = 0;
    int xuid;

    fail = 0;
    mail_changed = money_changed = perm_changed = 0;

    // verify unum
    xuid = getuser(orig_uid, &x);
    if (xuid == 0)
    {
	vmsgf("找不到使用者 %s。", orig_uid);
	return;
    }
    if (xuid != unum)
    {
	move(b_lines-1, 0); clrtobot();
	prints(ANSI_COLOR(1;31) "錯誤資訊: unum=%d (lookup xuid=%d)"
		ANSI_RESET "\n", unum, xuid);
	vmsg("系統錯誤: 使用者資料號碼 (unum) 不合。請至 " BN_BUGREPORT "報告。");
	return;
    }
    if (strcmp(orig_uid, x.userid) != 0)
    {
	move(b_lines-1, 0); clrtobot();
	prints(ANSI_COLOR(1;31) "錯誤資訊: userid=%s (lookup userid=%s)"
		ANSI_RESET "\n", orig_uid, x.userid);
	vmsg("系統錯誤: 使用者 ID 記錄不不合。請至 " BN_BUGREPORT "報告。");
	return;
    }

    if(adminmode)
    	mvouts(b_lines-2, 0, "(1)修改資料 (2)設定密碼 (3)設定權限 (5)修改帳號 (7)罰單 (8)退文\n(M)修改信箱 (P)修改手機號碼 (A)進階查詢 (6)寵物 (4)砍除帳號 [0]結束");
    else
    	mvouts(b_lines-1, 0, "(1)修改資料 (2)設定密碼 (C)個人化設定 (M)修改信箱 (P)修改手機號碼 [0]結束");
    ans = vans("請輸入操作 ");

    if (ans > '2' && ans != 'c' && ans != 'm' && ans != 'p' && !adminmode)
	ans = '0';

    if (ans == '1' || ans == '3' || ans == 'm' || ans == 'p') {
	clear();
	y = 1;
	move(y++, 0);
	outs(msg_uid);
	outs(x.userid);
    }

    if (adminmode && ((ans >= '1' && ans <= '8') || ans == 'm') &&
	search_ulist(unum))
    {
	if (vans("使用者目前正在線上，修改資料會先踢下線。確定要繼續嗎？ (y/N): ")
		!= 'y')
		return;
	if (unum == usernum &&
	    vans("您正試圖修改自己的帳號；這可能會造成帳號損毀，確定要繼續嗎？ (y/N): ")
	    != 'y')
		return;
    }
    switch (ans) {
    case 'c':
	// Customize can only apply to self.
	if (!adminmode)
	    Customize();
	return;

    case 'a':
	if (adminmode)
	    user_display_advanced(&x, 1);
	return;

    case 'm':
	while (1) {
		if(!strcmp(x.userid, cuser.userid))
	    	getdata_str(y, 0,"電子信箱：", buf, sizeof(x.email), DOECHO, x.email);
		else
	    	getdata_str(y, 0,"電子信箱：", buf, sizeof(x.email), DOECHO, "");

	    strip_blank(buf, buf);

	    // fast break
	    if (!buf[0] || strcasecmp(buf, "x") == 0)
		break;

	    if (!check_regmail(buf))
		continue;

	    // XXX 這裡也要 emaildb_check
#ifdef USE_EMAILDB
	    {
		int email_count = emaildb_check_email(buf);

		if (email_count < 0)
		    vmsg("暫時不允許\ email 認證, 請稍後再試");
		else if (email_count >= EMAILDB_LIMIT)
		    vmsg("指定的 E-Mail 已註冊過多帳號, 請使用其他 E-Mail");
		else
		    break; // valid mail
		// invalid mail
		continue;
	    }
#endif
	    // valid mail.
	    break;
	}
	y++;

	// admins may want to use special names
	if (buf[0] &&
		strcmp(buf, x.email) &&
		(strchr(buf, '@') || adminmode)) {

	    // TODO 這裡也要 emaildb_check
#ifdef USE_EMAILDB
	    if (emaildb_update_email(x.userid, buf) < 0) {
		vmsg("暫時不允許\ email 認證, 請稍後再試");
		break;
	    }
#endif
	    strlcpy(x.email, buf, sizeof(x.email));
	    mail_changed = 1;
	}
	break;

    case 'p':
	if(!strcmp(x.userid, cuser.userid))
    	getdata_str(y, 0,"手機號碼：", buf, sizeof(x.cellphone), DOECHO, x.cellphone);
	else
    	getdata_str(y, 0,"手機號碼：", buf, sizeof(x.cellphone), DOECHO, "");

    if(!strcmp(x.cellphone, buf))
    	return;

	int msg = 0;
	if (!adminmode){
		msg = cellphone_verify(x.userid, buf);
	}

	if(msg != 0){
		vmsg("未修改。");
		return;
	}else
		strlcpy(x.cellphone, buf, sizeof(x.cellphone));
	break;

    case '1':
	move(0, 0);
	outs("請逐項修改。");

	getdata_buf(y++, 0, " 暱 稱  ：", x.nickname,
		    sizeof(x.nickname), DOECHO);
	if (adminmode) {
	    getdata_buf(y++, 0, "真實姓名：",
			x.realname, sizeof(x.realname), DOECHO);
	    getdata_buf(y++, 0, "居住地址：",
			x.address, sizeof(x.address), DOECHO);
	    getdata_buf(y++, 0, "學歷職業：", x.career,
		    sizeof(x.career), DOECHO);
	}

        /*do {
            snprintf(buf, sizeof(buf), x.over_18 ? "y" : "n");
            mvouts(y, 0, "本站部份看板可能有限制級內容只適合成年人士閱\讀。");
            getdata_buf(y+1, 0,"您是否年滿十八歲並同意觀看此類看板"
                        "(若否請輸入n)[y/n]: ", buf, 3, LCECHO);
        } while (buf[0] != 'y' && buf[0] != 'n');
        x.over_18 = buf[0] == 'y' ? 1 : 0;
        mvprints(y, 0, "是否年滿十八歲: %s\n\n", x.over_18 ? "是" : "否");
        y++;*/

#ifdef PLAY_ANGEL
	if (adminmode) {
	    const char* prompt;
	    userec_t the_angel;
	    if (x.myangel[0] == 0 || x.myangel[0] == '-' ||
		    (getuser(x.myangel, &the_angel) &&
		     the_angel.userlevel & PERM_ANGEL))
		prompt = "小 天 使：";
	    else
		prompt = "小天使（此帳號已無小天使資格）：";
	    while (1) {
	        userec_t xuser;
		getdata_str(y, 0, prompt, buf, IDLEN + 1, DOECHO,
			x.myangel);

		if (buf[0] == 0 || strcmp(buf, "-") == 0) {
		    strlcpy(x.myangel, buf, IDLEN + 1);
                    break;
                }

                if (strcmp(x.myangel, buf) == 0)
                    break;

                if (getuser(buf, &xuser) && (xuser.userlevel & PERM_ANGEL)) {
		    strlcpy(x.myangel, xuser.userid, IDLEN + 1);
                    x.timesetangel = now;
                    log_filef(BBSHOME "/log/changeangel.log",LOG_CREAT,
                              "%s 站長 %s 修改 %s 的小天使為 %s\n",
                              Cdatelite(&now), cuser.userid, x.userid, x.myangel);
		    break;
		}

		prompt = "小 天 使：";
	    }
            ++y;
	}
#endif

	if (adminmode) {
	    int tmp;
	    if (HasUserPerm(PERM_BBSADM)) {
		snprintf(genbuf, sizeof(genbuf), "%d", x.money);
		if (getdata_str(y++, 0, BBSMNAME "幣：", buf, 10, DOECHO, genbuf))
		    if ((tmp = atol(buf)) != 0) {
			if (tmp != x.money) {
			    money_changed = 1;
			    changefrom = x.money;
			    x.money = tmp;
			}
		    }
	    }
	    snprintf(genbuf, sizeof(genbuf), "%d", x.exmailbox);
	    if (getdata_str(y++, 0, "購買信箱：", buf, 6,
			    DOECHO, genbuf))
		if ((tmp = atoi(buf)) != 0)
		    x.exmailbox = (int)tmp;

	    getdata_buf(y++, 0, "認證資料：", x.justify,
			sizeof(x.justify), DOECHO);
	    getdata_buf(y++, 0, "最近光臨機器：",
			x.lasthost, sizeof(x.lasthost), DOECHO);

	    while (1) {
		struct tm t = {0};
		time4_t clk = x.lastlogin;
		localtime4_r(&clk, &t);
		snprintf(genbuf, sizeof(genbuf), "%04i/%02i/%02i %02i:%02i:%02i",
			t.tm_year + 1900, t.tm_mon+1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec);
		if (getdata_str(y, 0, "最近上線時間：", buf, 20, DOECHO, genbuf) != 0) {
		    int y, m, d, hh, mm, ss;
		    if (ParseDateTime(buf, &y, &m, &d, &hh, &mm, &ss))
			continue;
		    t.tm_year = y-1900;
		    t.tm_mon  = m-1;
		    t.tm_mday = d;
		    t.tm_hour = hh;
		    t.tm_min  = mm;
		    t.tm_sec  = ss;
		    clk = mktime(&t);
		    if (!clk)
			continue;
		    x.lastlogin= clk;
		}
		y++;
		break;
	    }

	    do {
		int max_days = (x.lastlogin - x.firstlogin) / DAY_SECONDS;
		snprintf(genbuf, sizeof(genbuf), "%d", x.numlogindays);
		if (getdata_str(y++, 0, STR_LOGINDAYS "：", buf, 10, DOECHO, genbuf))
		    if ((tmp = atoi(buf)) >= 0)
			x.numlogindays = tmp;
		if ((int)x.numlogindays > max_days)
		{
		    x.numlogindays = max_days;
		    vmsgf("根據此使用者最後上線時間，最大值為 %d.", max_days);
		    move(--y, 0); clrtobot();
		    continue;
		}
		break;
	    } while (1);

	    snprintf(genbuf, sizeof(genbuf), "%d", x.numposts);
	    if (getdata_str(y++, 0, "文章數目：", buf, 10, DOECHO, genbuf))
		if ((tmp = atoi(buf)) >= 0)
		    x.numposts = tmp;
	    move(y-1, 0); clrtobot();
	    prints("文章數目： %d (退: %d, 修改退文數請選項8)\n",
		    x.numposts, x.badpost);

	    snprintf(genbuf, sizeof(genbuf), "%d", x.vl_count);
	    if (getdata_str(y++, 0, "違法記錄：", buf, 10, DOECHO, genbuf))
		if ((tmp = atoi(buf)) >= 0)
		    x.vl_count = tmp;

#ifdef FOREIGN_REG
	    if (getdata_str(y++, 0, "住在 1)台灣 2)其他：", buf, 2, DOECHO, x.uflag & UF_FOREIGN ? "2" : "1"))
		if ((tmp = atoi(buf)) > 0){
		    if (tmp == 2){
			x.uflag |= UF_FOREIGN;
		    }
		    else
			x.uflag &= ~UF_FOREIGN;
		}
	    if (x.uflag & UF_FOREIGN)
		if (getdata_str(y++, 0, "永久居留權 1)是 2)否：", buf, 2, DOECHO, x.uflag & UF_LIVERIGHT ? "1" : "2")){
		    if ((tmp = atoi(buf)) > 0){
			if (tmp == 1){
			    x.uflag |= UF_LIVERIGHT;
			    x.userlevel |= (PERM_LOGINOK | PERM_POST);
			}
			else{
			    x.uflag &= ~UF_LIVERIGHT;
			    x.userlevel &= ~(PERM_LOGINOK | PERM_POST);
			}
		    }
		}
#endif
	}

        if (!adminmode) {
            mvouts(b_lines-1, 0,
                   "其它資料若需修改請洽 " BN_ID_PROBLEM " 看板\n");
        }
	break;

    case '2':
		userPass_change(x,adminmode,unum);
	break;

    case '3':
	{
	    unsigned int tmp = setperms(x.userlevel, str_permid);
	    if (tmp == x.userlevel)
		fail++;
	    else {
		perm_changed = 1;
		changefrom = x.userlevel;
		x.userlevel = tmp;
	    }
	}
	break;

    case '4':
	tokill = 1;
	{
	    char reason[STRLEN];
	    char title[STRLEN], msg[1024];
	    while (!getdata(b_lines-3, 0, "請輸入理由以示負責：", reason, 50, DOECHO));
	    if (vans(msg_sure_ny) != 'y')
	    {
		fail++;
		break;
	    }
	    pre_confirmed = 1;
	    snprintf(title, sizeof(title),
		    "刪除ID: %s (站長: %s)", x.userid, cuser.userid);
	    snprintf(msg, sizeof(msg),
		    "帳號 %s 由站長 %s 執行刪除，理由:\n %s\n\n"
		    "真實姓名:%s\n住址:%s\n認證資料:%s\nEmail:%s\n",
		    x.userid, cuser.userid, reason,
		    x.realname, x.address, x.justify, x.email);
	    post_msg(BN_SECURITY, title, msg, "[系統安全局]");
	}
	break;

    case '5':
        mvouts(b_lines - 8, 0, "\n"
           "已知很多使用者搞不清狀況改完 ID 大小寫會哭哭無法修改以前文章，\n"
           "且有不少管理/維護的問題，"
           ANSI_COLOR(1;31)
           "所以請停止讓使用者自行申請改大小寫的服務。\n" ANSI_RESET
           "除非是站務需求(如解決近似ID) 不然請勿使用此功\能改大小寫\n");
        clrtobot();

	if (getdata_str(b_lines - 3, 0, "新的使用者代號：", genbuf, IDLEN + 1,
			DOECHO, x.userid)) {
	    if (!is_validuserid(genbuf)) {
		outs("錯誤! 輸入的 ID 不合規定\n");
		fail++;
	    } else if (searchuser(genbuf, NULL)) {
		outs("錯誤! 已經有同樣 ID 的使用者\n");
		fail++;
#if !defined(NO_CHECK_AMBIGUOUS_USERID) && defined(USE_REGCHECKD)
	    } else if ( regcheck_ambiguous_userid_exist(genbuf) > 0 &&
			vans("此代號過於近似它人帳號，"
                             "確定使用者沒有要幹壞事嗎? [y/N] ") != 'y')
	    {
		    fail++;
#endif
	    } else
		strlcpy(x.userid, genbuf, sizeof(x.userid));
	}
	break;

    case '6':
	chicken_toggle_death(x.userid);
	break;

    case '7':
	violate_law(&x, unum);
	return;

    case '8':
#ifdef ASSESS
        reassign_badpost(x.userid);
#else
        vmsg("本站目前不支援退文設定。");
#endif
        return;

    default:
	return;
    }

    if (fail) {
	pressanykey();
	return;
    }

    if (!pre_confirmed)
    {
	if (vans(msg_sure_ny) != 'y')
	    return;
    }

    // now confirmed. do everything directly.

    // perm_changed is by sysop only.
    if (perm_changed) {
	sendalert(x.userid,  ALERT_PWD_PERM); // force to reload perm
	post_change_perm(changefrom, x.userlevel, cuser.userid, x.userid);
#ifdef PLAY_ANGEL
        // TODO notify Angelbeats
	if (x.userlevel & ~changefrom & PERM_ANGEL) {
            angel_register_new(x.userid);
            mail_id(x.userid, "翅膀長出來了！", "etc/angel_notify",
                    "[天使公會]");
        }
#endif
    }

    if (strcmp(orig_uid, x.userid)) {
	char            src[STRLEN], dst[STRLEN];
	kick_all(orig_uid);
	sethomepath(src, orig_uid);
	sethomepath(dst, x.userid);
	Rename(src, dst);
	setuserid(unum, x.userid);

	// log change for security reasons.
	char title[STRLEN];
	snprintf(title, sizeof(title), "變更ID: %s -> %s (站長: %s)",
		 orig_uid, x.userid, cuser.userid);
	post_msg(BN_SECURITY, title, title, "[系統安全局]");
    }
    /*if (mail_changed && !adminmode) {
	// wait registration.
	x.userlevel &= ~(PERM_LOGINOK | PERM_POST);
    }*/

    if (tokill) {
	kick_all(x.userid);
	delete_allpost(x.userid);
	kill_user(unum, x.userid);
	return;
    } else
	log_usies("SetUser", x.userid);

    if (money_changed) {
	char title[TTLEN+1];
	char msg[512];
	char reason[50];
	clrtobot();
	clear();
	while (!getdata(5, 0, "請輸入理由以示負責：",
		    reason, sizeof(reason), DOECHO));

	snprintf(msg, sizeof(msg),
		"   站長" ANSI_COLOR(1;32) "%s" ANSI_RESET "把" ANSI_COLOR(1;32) "%s" ANSI_RESET "的錢"
		"從" ANSI_COLOR(1;35) "%d" ANSI_RESET "改成" ANSI_COLOR(1;35) "%d" ANSI_RESET "\n"
		"   " ANSI_COLOR(1;37) "站長%s修改錢理由是：%s" ANSI_RESET,
		cuser.userid, x.userid, changefrom, x.money,
		cuser.userid, reason);
	snprintf(title, sizeof(title),
		"[安全報告] 站長%s修改%s金錢", cuser.userid,
		x.userid);
	post_msg(BN_SECURITY, title, msg, "[系統安全局]");
	setumoney(unum, x.money);
    }

    passwd_sync_update(unum, &x);

    if (adminmode)
	kick_all(x.userid);
}

int
u_info(void)
{
    move(2, 0);
    reload_money();
    user_display(cuser_ref, 0);
    uinfo_query (cuser.userid, 0, usernum);
    pwcuReload();
    strlcpy(currutmp->nickname, cuser.nickname, sizeof(currutmp->nickname));
    return 0;
}

void
showplans_userec(userec_t *user)
{
    char genbuf[ANSILINELEN];

    if(user->userlevel & PERM_VIOLATELAW)
    {
        int can_save = ((user->userlevel & PERM_LOGINOK) &&
                        (user->userlevel & PERM_BASIC)) ? 1 : 0;

        prints(" " ANSI_COLOR(1;31) "此人違規 %s" ANSI_RESET,
               can_save ? "尚未繳交罰單" : "");

        if (user->vl_count)
            prints(" (已累計 %d 次)", user->vl_count);
	return;
    }

    sethomefile(genbuf, user->userid, fn_plans);
    if (!show_file(genbuf, 7, MAX_QUERYLINES, SHOWFILE_ALLOW_COLOR))
        prints("《個人名片》%s 目前沒有名片\n", user->userid);
}

void
showplans(const char *uid)
{
    userec_t user;
    if(getuser(uid, &user))
       showplans_userec(&user);
}
/*
 * return value: how many items displayed */
int
showsignature(char *fname, int *j, SigInfo *si)
{
    FILE           *fp;
    char            buf[ANSILINELEN];
    int             i, lines = t_lines;
    char            ch;

    clear();
    move(2, 0);
    lines -= 3;

    setuserfile(fname, "sig.0");
    *j = strlen(fname) - 1;
    si->total = 0;
    si->max = 0;

    for (ch = '1'; ch <= '9'; ch++) {
	fname[*j] = ch;
	if ((fp = fopen(fname, "r"))) {
	    si->total ++;
	    si->max = ch - '1';
	    if(lines > 0 && si->max >= si->show_start)
	    {
		int y = vgety() + 1;
		prints(ANSI_COLOR(36) "【 簽名檔.%c 】" ANSI_RESET "\n", ch);
		lines--;
		if(lines > MAX_SIGLINES/2)
		    si->show_max = si->max;
		for (i = 0; lines > 0 && i < MAX_SIGLINES &&
			fgets(buf, sizeof(buf), fp) != NULL; i++)
		{
		    chomp(buf);
		    mvouts(y++, 0, buf);
		    lines--;
		}
		move(y, 0);
	    }
	    fclose(fp);
	}
    }
    if(lines > 0)
	si->show_max = si->max;
    return si->max;
}

int
u_editsig(void)
{
    int             aborted;
    char            ans[4];
    int             j, browsing = 0;
    char            genbuf[PATHLEN];
    SigInfo	    si;

    memset(&si, 0, sizeof(si));

browse_sigs:

    showsignature(genbuf, &j, &si);
    getdata(0, 0, (browsing || (si.max > si.show_max)) ?
	    "簽名檔 (E)編輯 (D)刪除 (N)翻頁 (Q)取消？[Q] ":
	    "簽名檔 (E)編輯 (D)刪除 (Q)取消？[Q] ",
	    ans, sizeof(ans), LCECHO);

    if(ans[0] == 'n')
    {
	si.show_start = si.show_max + 1;
	if(si.show_start > si.max)
	    si.show_start = 0;
	browsing = 1;
	goto browse_sigs;
    }

    aborted = 0;
    if (ans[0] == 'd')
	aborted = 1;
    else if (ans[0] == 'e')
	aborted = 2;

    if (aborted) {
	if (!getdata(1, 0, "請選擇簽名檔(1-9)？[1] ", ans, sizeof(ans), DOECHO))
	    ans[0] = '1';
	if (ans[0] >= '1' && ans[0] <= '9') {
	    genbuf[j] = ans[0];
	    if (aborted == 1) {
		unlink(genbuf);
		outs(msg_del_ok);
	    } else {
		setutmpmode(EDITSIG);
		aborted = veditfile(genbuf);
		if (aborted != EDIT_ABORTED)
		    outs("簽名檔更新完畢");
	    }
	}
	pressanykey();
    }
    return 0;
}

int
u_editplan(void)
{
    char            genbuf[200];

    getdata(b_lines - 1, 0, "名片 (D)刪除 (E)編輯 [Q]取消？[Q] ",
	    genbuf, 3, LCECHO);

    if (genbuf[0] == 'e') {
	int             aborted;

	setutmpmode(EDITPLAN);
	setuserfile(genbuf, fn_plans);
	aborted = veditfile(genbuf);
	if (aborted != EDIT_ABORTED)
	    outs("名片更新完畢");
	pressanykey();
	return 0;
    } else if (genbuf[0] == 'd') {
	setuserfile(genbuf, fn_plans);
	unlink(genbuf);
	outmsg("名片刪除完畢");
    }
    return 0;
}

/* 列出所有註冊使用者 */
struct ListAllUsetCtx {
    int usercounter;
    int totalusers;
    unsigned short u_list_special;
    int y;
};

static int
u_list_CB(void *data, int num, userec_t * uentp)
{
    char            permstr[8];
    register int    level;
    struct ListAllUsetCtx *ctx = (struct ListAllUsetCtx*) data;
    (void)num;

    if (uentp == NULL) {
	move(2, 0);
	clrtoeol();
	prints(ANSI_REVERSE "  使用者代號   %-25s   上站  文章  %s  "
	       "多久沒來     " ANSI_RESET "\n",
	       "綽號暱稱",
	       HasUserPerm(PERM_SEEULEVELS) ? "等級" : "");
	ctx->y = 3;
	return 0;
    }
    if (bad_user_id(uentp->userid))
	return 0;

    if ((uentp->userlevel & ~(ctx->u_list_special)) == 0)
	return 0;

    if (ctx->y == b_lines) {
	int ch;
	prints(ANSI_COLOR(34;46) "  已顯示 %d/%d 人(%d%%)  " ANSI_COLOR(31;47) "  "
	       "(Space)" ANSI_COLOR(30) " 看下一頁  " ANSI_COLOR(31) "(Q)" ANSI_COLOR(30) " 離開  " ANSI_RESET,
	       ctx->usercounter, ctx->totalusers, ctx->usercounter * 100 / ctx->totalusers);
	ch = vkey();
	if (ch == 'q' || ch == 'Q')
	    return -1;
	ctx->y = 3;
    }
    if (ctx->y == 3) {
	move(3, 0);
	clrtobot();
    }
    level = uentp->userlevel;
    strlcpy(permstr, "----", 8);
    if (level & PERM_SYSOP)
	permstr[0] = 'S';
    else if (level & PERM_ACCOUNTS)
	permstr[0] = 'A';
    else if (level & PERM_SYSOPHIDE)
	permstr[0] = 'p';

    if (level & (PERM_BOARD))
	permstr[1] = 'B';
    else if (level & (PERM_BM))
	permstr[1] = 'b';

    if (level & (PERM_XEMPT))
	permstr[2] = 'X';
    else if (level & (PERM_LOGINOK))
	permstr[2] = 'R';

    if (level & (PERM_CLOAK | PERM_SEECLOAK))
	permstr[3] = 'C';

    int ptr = (int)((now - uentp->lastlogin)/DAY_SECONDS);
    prints("%-14s %-27.27s%5d %5d  %s  %d天沒來了\n",
	   uentp->userid,
	   uentp->nickname,
	   uentp->numlogindays, uentp->numposts,
	   HasUserPerm(PERM_SEEULEVELS) ? permstr : "", ptr);
    ctx->usercounter++;
    ctx->y++;
    return 0;
}

int
u_list(void)
{
    char            genbuf[3];
    struct ListAllUsetCtx data, *ctx = &data;

    setutmpmode(LAUSERS);
    ctx->u_list_special = ctx->usercounter = 0;
    ctx->totalusers = SHM->number;
    if (HasUserPerm(PERM_SEEULEVELS)) {
	getdata(b_lines - 1, 0, "觀看 (1)特殊等級 [2]全部？",
		genbuf, 3, DOECHO);
	if (genbuf[0] == '1')
	    ctx->u_list_special = PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_LOGINOK | PERM_BM;
    }
    u_list_CB(ctx, 0, NULL);
    passwd_apply(ctx, u_list_CB);
    move(b_lines, 0);
    clrtoeol();
    prints(ANSI_COLOR(34;46) "  已顯示 %d/%d 的使用者(系統容量無上限)  "
	   ANSI_COLOR(31;47) "  (請按任意鍵繼續)  " ANSI_RESET, ctx->usercounter, ctx->totalusers);
    vkey();
    return 0;
}

/* vim:sw=4
 */
