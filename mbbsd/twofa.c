#include "bbs.h"

#ifdef USE_2FALOGIN

bool isFileExist();

static void twoFA_GenCode(char *buf, size_t len)
{
	const char * const chars = "1234567890";

    for (int i = 0; i < len; i++)
	buf[i] = chars[random() % strlen(chars)];
    buf[len] = '\0';
}

static void twoFA_GenRevCode(char *buf, size_t len)
{
	const char * const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

    for (int i = 0; i < len; i++)
	buf[i] = chars[random() % strlen(chars)];
    buf[len] = '\0';
}

static const char *
twoFA_Send(char *user, char *authcode)
{
	int ret, code = 0;
	char uri[320] = "",buf[200];

	snprintf(uri, sizeof(uri), "/%s?user=%s"
#ifdef BETA
			 "&beta=true&code=%s"
#endif
			 , IBUNNY_2FA_URI, user
#ifdef BETA
			 , authcode
#endif
			);

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", IBUNNY_API_KEY);
	ret = thttp_get(&t, IBUNNY_SERVER, uri, IBUNNY_SERVER, buf);
	if (!ret)
	code = thttp_code(&t);
	thttp_cleanup(&t);

	if (ret)
		return "系統錯誤，您可以使用復原碼或稍後再試。(Error code: 2FA-S-001)";

    	if (code != 200){
		snprintf(buf, sizeof(buf), "系統錯誤，您可以使用復原碼或稍後再試。(Error code: 2FA-S-%3d)", code);
		return buf;
	}

	return NULL;
}

int twoFA_main(char *user)
{
    FILE           *fp;
    const char *msg = NULL;
    char code[7], rev_code[9], code_input[9], buf[200], buf2[200], genbuf[3];

#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
    extern Fnv32_t  client_code;
	int	i, trusted = 0, count=0;

	setuserfile(buf, "trust.device");
	snprintf(buf2, sizeof(buf2), "%8.8X\n", client_code);
	if(fp = fopen(buf, "r+")){
		while (fgets(buf, sizeof(buf), fp)) {
			if (!strcmp(buf, buf2)){
				trusted = 1;
			}
			count++;
		}
		fclose(fp);
		if(trusted == 1)
			return NULL; //Success
	}
#endif

	clear();
	vs_hdr2(" 兩步驟認證 ", " 輸入驗證碼");

	twoFA_GenCode(code, 6);
	
	setuserfile(buf, "2fa.code");
	if (!(fp = fopen(buf, "w"))){
		move(1,0);
		outs("系統錯誤，您可以使用復原碼或稍後再試。(Error code: 2FA-F-001)");
		getdata(2, 0, "使用復原碼？ (y/N) ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
			return -1;
		}
	}
	fprintf(fp,"%s", code);
	fclose(fp);

#ifdef BETA
	msg = twoFA_Send(user,code);
#else
	msg = twoFA_Send(user,NULL);
#endif
    	if (msg){
		move(1,0);
		outs(msg);
		outs("\n  (如果您有收到驗證碼，但系統誤報錯誤，輸入R即可進入驗證程序。)");
		getdata(4, 0, "(R)使用復原碼 (T)重試發送 [C]取消登入 ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'r' && genbuf[0] != 't') {
			unlink(buf);
			return -1;
		}
		if (genbuf[0] == 't') {
#ifdef BETA
			msg = twoFA_Send(user,code);
#else
			msg = twoFA_Send(user,NULL);
#endif
		    	if (msg){
				move(1,0); clrtobot();
				outs(msg);
				getdata(3, 0, "(R)使用復原碼 [C]取消登入 ",genbuf, 3, LCECHO);
				if (genbuf[0] != 'r') {
					unlink(buf);
					return -1;
				}
			}

		}
	}
	unlink(buf);
	
	move(1,0); clrtobot();
    mvouts(2, 0, "驗證碼將直接被發送到iBunny\n");
    outs("驗證碼為6位數字、復原碼為8位英數混合。\n");

    for (int i = 3; i > 0; i--) {
		if (i < 3) {
			char buf[80];
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "驗證碼錯誤，您還有 %d 次機會。" ANSI_RESET, i);
			move(6, 0);
			outs(buf);
		}
		code_input[0] = '\0';
		getdata(5, 0, "請輸入驗證碼：", code_input,
			sizeof(code_input), DOECHO);

		size_t length = strlen(code_input);
		if(length == 6){
			if (!strcmp(code, code_input)){
#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
				clear();
				vs_hdr2(" 兩步驟認證 ", " 設定為信任的裝置？");
				move(2, 0);
				outs("設定為信任的裝置下次登入就不需要驗證。\n"
					 "提請您，請不要在公用電腦上使用此功\能。");
				getdata(5, 0, "是否將這個裝置設定為信任的裝置(y/n)？ [N]",genbuf, 3, LCECHO);
				if(genbuf[0] == 'y') {
					setuserfile(buf, "trust.device");
					log_filef(buf, LOG_CREAT,"%8.8X\n", client_code);
				}
#endif
				return NULL; //Success
			}
		}else if(length == 8){
			setuserfile(buf, "2fa.recov");
			if (!(fp = fopen(buf, "r"))){
				outs("系統錯誤，無法使用復原碼，請稍後再試。(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);
			
			if (!strcmp(rev_code, code_input)){
				unlink(buf);
				move(6, 0);
				outs(ANSI_COLOR(1;33) "使用了復原碼認證，故復原碼已失效。" ANSI_RESET);
				return NULL; //Success
			}
		}
		
		now = time(NULL);
		setuserfile(buf, "2fa.bad");
		log_filef(buf, LOG_CREAT,"%s 第%d次兩步驟驗證失敗，IP位置：%s。\n",Cdate(&now), 4 - i , fromhost);
		log_filef("2fa.bad", LOG_CREAT,"%s %s %s (%d/3)\n",Cdate(&now), cuser.userid, fromhost, 4 - i);
    }
    return -1;
}

int twoFA_genRecovCode()
{
    FILE *fp;
    char rev_code[9], buf[200], genbuf[3];
	char *user = cuser.userid;
    char passbuf[PASSLEN];

	vs_hdr("兩步驟認證復原碼");
	if(!(HasUserFlag(UF_TWOFA_LOGIN))){
		vmsg("請先在個人設定開啟兩步驟認證。");
		return 0;
	}
	
	setuserfile(buf, "2fa.recov");
	move(1, 0);

    if(isFileExist(buf) == true){
		outs("您已經有一組復原碼，每個帳戶只能擁有一組。\n");
		outs("當您繼續操作產生新復原碼，原有的就會失效。");
		getdata(4, 0, "確定繼續嗎？ (y)繼續 [N]取消 ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
			vmsg("取消操作");
			return 0;
		}
	}

	move(1, 0); clrtobot();
    outs("復原碼是當您無法使用兩步驟認證時，\n");
    outs("可以在輸入驗證碼時輸入復原碼取回帳戶存取權。\n");
    outs("另外，另外每一組復原碼只能使用一次，使用後就會失效。\n\n");
	
	outs("以下操作需要先確認您的身份。\n");
	getdata(6, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("密碼錯誤！");
		return 0;
	}
	
	twoFA_GenRevCode(rev_code, 8);
	if (!(fp = fopen(buf, "w"))){
		vmsg("系統錯誤，請稍後再試。(Error code: 2FA-F-003)");
		return 0;
	}
	fprintf(fp,"%s", rev_code);
	fclose(fp);
	
	move(10,0);
	outs("您的復原碼是：" ANSI_COLOR(1));
	outs(rev_code);
	outs(ANSI_RESET "\n\n");
	outs("復原碼共計8碼，會出現英文字母I、L、O，不會出現數字0、1。");
	mvouts(b_lines - 2,12,ANSI_COLOR(1;33) "請您記下復原碼並妥善保管，離開本視窗後就不能再重新查詢。" ANSI_RESET);
	
	pressanykey();
	return 0;
}

#endif //USE_2FALOGIN

#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)

int twoFA_RemoveTrust()
{
    FILE *fp;
    char rev_code[9], buf[200], genbuf[3];
	char *user = cuser.userid;
    char passbuf[PASSLEN];
	
	setuserfile(buf, "trust.device");
    if(isFileExist(buf) == false){
		vmsg("沒有在任何裝置上設定為信任。");
		return 0;
	}

	clear();
	vs_hdr("撤銷信任裝置");
	
	move(2, 0);
	outs("設定為信任的裝置在登入時不需要驗證。\n");
	outs("要撤銷掉所有目前設定為信任的裝置嗎？\n");
	getdata(5, 0, "確定繼續嗎？ (y)繼續 [N]取消 ",genbuf, 3, LCECHO);
	if (genbuf[0] != 'y') {
		vmsg("取消操作");
		return 0;
	}

	unlink(buf);
	vmsg("已經撤銷所有信任的裝置");
	return 0;
}

#endif //defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
