#include "bbs.h"

#ifdef USE_BBS2WEB

int web_sync_board(int bid, const boardheader_t *board, char *type)
{
	int ret, code = 0;
	char uri[400] = "",buf[200];

	if(type != "NEW" && type != "SYNC")
		return 2;

	/* 板標在這裡同步會出錯，暫時不設計在這裡同步 */
	snprintf(uri, sizeof(uri), "/%s?type=%s&bid=%d&gid=%d&is_board=%d&name=%s&mod=%s&hide=%d&no_post=%d&friend_post=%d&no_reply=%d&no_money=%d&no_push=%d&ip_rec=%d&align=%d"
#ifdef BETA
			 "&beta=true"
#endif
			 , WEB_SYNCBRD_URI, type, bid, board->gid, ((board->brdattr & BRD_GROUPBOARD) ? 0 : 1),
			 board->brdname, board->BM,
			((board->brdattr & BRD_HIDE) ? 1 : 0), ((board->brdattr & BRD_NOPOST) ? 1 : 0),
			((board->brdattr & BRD_RESTRICTEDPOST) ? 1 : 0), ((board->brdattr & BRD_NOREPLY) ? 1 : 0), 
			((board->brdattr & BRD_NOCREDIT) ? 1 : 0), ((board->brdattr & BRD_NORECOMMEND) ? 1 : 0),
			((board->brdattr & BRD_IPLOGRECMD) ? 1 : 0), ((board->brdattr & BRD_ALIGNEDCMT) ? 1 : 0));

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", WEB_API_KEY);
	ret = thttp_get(&t, WEB_API_SERVER, uri, WEB_API_SERVER, buf);
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(ret)
		return 1;

    if(code == 200)
		return 0;
	else
		return code;

}

int
web_user_register(void)
{
	clear();
	vs_hdr2(" 網站服務 ", " 註冊帳號");

    char passbuf[PASSLEN], buf2[8];
    move(3, 0);
    prints("設定帳號：%s\n", cuser.userid);
	outs("請注意，本功\能僅限您不曾開通網站帳號時使用。\n");
	outs("如果您曾經開通網站帳號（含108年7月前舊註冊程序），請不要再使用本功\能。\n");
	outs("如果您忘記網站密碼，可使用重設密碼功\能將網站密碼重新設定為BBS登入密碼。\n");
	outs("以下操作需要先確認您的身份。\n");
	getdata(9, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	snprintf(buf2, sizeof(buf2), "%s", passbuf);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("密碼錯誤！");
		return 0;
	}

	if(!strcmp(cuser.email, "x")){
		vmsg("您沒有設定電子信箱，請先去設定再來。");
		return 0;
	}

	outs("\n請稍後，註冊中…\n");

	int ret, code = 0;
	char uri[320] = "",buf[200];
	snprintf(uri, sizeof(uri), "/%s?username=%s&passwd=%s&nickname=%s&email=%s&reg_ip=%s"
#ifdef BETA
			 "&beta=true"
#endif
			 , WEB_USERREG_URI, cuser.userid, buf2,
			 cuser.nickname, cuser.email, fromhost
			);

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", WEB_API_KEY);
	ret = thttp_get(&t, WEB_API_SERVER, uri, WEB_API_SERVER, buf);
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(ret){
		vmsg("系統錯誤，請稍後再試。(Error code: WUR-R-001)");
		return 0;
	}

    if(code == 200){
		move(1,0); clrtobot();
		mvouts(10, 0, "註冊成\功\！快到https://www.bunnybbs.tw/login登入吧！\n（帳號及密碼與BBS相同）");
		pressanykey();
		return 0;
    }else{
		vmsgf("系統錯誤，請稍後再試。(Error code: WUR-R-%03d)",code);
		return 0;
	}

	return 0;
}

int
web_user_resetpass(void)
{
	clear();
	vs_hdr2(" 網站服務 ", " 重設密碼");

    char passbuf[PASSLEN], buf2[8];
    move(3, 0);
    prints("設定帳號：%s\n", cuser.userid);
	outs("使用此功\能會將重設你的網站帳號密碼。\n");
	outs("以下操作需要先確認您的身份，請輸入BBS的密碼。\n");
	getdata(6, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	snprintf(buf2, sizeof(buf2), "%s", passbuf);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("密碼錯誤！");
		return 0;
	}

	outs("\n請稍後…\n");

	int ret, code = 0;
	char uri[320] = "",buf[200];
	snprintf(uri, sizeof(uri), "/%s?username=%s&passwd=%s"
#ifdef BETA
			 "&beta=true"
#endif
			 , WEB_RESETPASS_URI, cuser.userid, buf2);

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", WEB_API_KEY);
	ret = thttp_get(&t, WEB_API_SERVER, uri, WEB_API_SERVER, buf);
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(ret){
		vmsg("系統錯誤，請稍後再試。(Error code: WUP-R-001)");
		return 0;
	}

    if(code == 200){
		move(1,0); clrtobot();
		mvouts(10, 0, "重設成\功\！你的網站密碼已經重設與BBS相同。");
		pressanykey();
		return 0;
    }else{
		vmsgf("系統錯誤，請稍後再試。(Error code: WUP-R-%03d)",code);
		return 0;
	}

	return 0;
}

int
web_user_lock(void)
{
	clear();
	vs_hdr2(" 網站服務 ", " 鎖定帳號");

    char passbuf[PASSLEN], buf2[8], genbuf[3];
    move(4, 0);
    prints("設定帳號：%s\n", cuser.userid);
	outs("以下操作需要先確認您的身份。\n");
	getdata(6, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	snprintf(buf2, sizeof(buf2), "%s", passbuf);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("密碼錯誤！");
		return 0;
	}

	mvouts(9, 0, "鎖定後你的帳號將不能再存取網站服務，解鎖需洽工程業務處。");
	getdata(10, 0, "確定要鎖定網站帳號了嗎？ (y/N)",genbuf, 3, LCECHO);
	if (genbuf[0] != 'y') {
		vmsg("取消操作。");
		return 0;
	}

	outs("\n請稍後…\n");

	int ret, code = 0;
	char uri[320] = "",buf[200];
	snprintf(uri, sizeof(uri), "/%s?username=%s"
#ifdef BETA
			 "&beta=true"
#endif
			 , WEB_USERLOCK_URI, cuser.userid);

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", WEB_API_KEY);
	ret = thttp_get(&t, WEB_API_SERVER, uri, WEB_API_SERVER, buf);
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(ret){
		vmsg("系統錯誤，請稍後再試。(Error code: WUL-L-001)");
		return 0;
	}

    if(code == 200){
		move(1,0); clrtobot();
		mvouts(10, 0, "已鎖定你的網站帳號，如有需解鎖請洽工程業務處系統工程處。");
		pressanykey();
		return 0;
    }else{
		vmsgf("系統錯誤，請稍後再試。(Error code: WUL-L-%03d)",code);
		return 0;
	}

	return 0;
}

#endif //USE_BBS2WEB