#include "bbs.h"

#ifdef USE_IBUNNY_2FALOGIN

bool isFileExist();

static void twoFA_GenCode(char *buf, size_t len)
{
	const char * const chars = "1234567890";

    for (int i = 0; i < len; i++)
	buf[i] = chars[random() % strlen(chars)];
    buf[len] = '\0';
}

static const char *
twoFA_Send(char *user)
{
	int ret, code = 0;
	char uri[320] = "",buf[200];

	snprintf(uri, sizeof(uri), "%s?user=%s"
#ifdef BETA
			 "&beta=true"
#endif
			 , IBUNNY_2FA_URI, user);

	THTTP t;
	thttp_init(&t);
	ret = thttp_get(&t, IBUNNY_SERVER, uri, IBUNNY_SERVER);
	if (!ret)
	code = thttp_code(&t);
	thttp_cleanup(&t);

	if (ret)
		return "系統錯誤，請稍後再試。(Error code: 2FA-S-001)";

    if (code != 200){
		snprintf(buf, sizeof(buf), "系統錯誤，請稍後再試。(Error code: 2FA-S-%3d)", code);
		return buf;
	}

	return NULL;
}

int twoFA_main(char *user)
{
    FILE           *fp;
    const char *msg = NULL;
    char code[7], rev_code[9], code_input[9], buf[200];

	clear();
	vs_hdr("兩步驟認證");

#ifdef BETA /* 因為測試主機與正式主機不同台，測試時無法正常取得驗證碼。 */
	snprintf(code, sizeof(code), "000000");
#else
    twoFA_GenCode(code, 6);
#endif
	
	setuserfile(buf, "2fa.code");
	if (!(fp = fopen(buf, "w"))){
		outs("系統錯誤，請稍後再試。(Error code: 2FA-F-001)");
		return -1;
	}
	fprintf(fp,"%s", code);
	fclose(fp);

    msg = twoFA_Send(user);
    if (msg){
		outs(msg);
		return -1;
	}
	unlink(buf);
	
    outs("驗證碼將直接被發送到iBunny\n");
    outs("一共為六位數字\n");

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
			if (!strcmp(code, code_input))
				return NULL; //Success
		}else if(length == 8){
			setuserfile(buf, "2fa.recov");
			if (!(fp = fopen(buf, "r"))){
				outs("系統錯誤，請稍後再試。(Error code: 2FA-F-002)");
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
		log_filef(buf, LOG_CREAT,"%s 第%d次兩步驟驗證失敗，IP位置：%s。\n",Cdate(&now), 3 - i , fromhost);
		log_filef("2fa.bad", LOG_CREAT,"%s %s %s (%d/3)\n",Cdate(&now), cuser.userid, fromhost, 3 - i);
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
	getdata(6, 0, MSG_PASSWD, passbuf, sizeof(passbuf), NOECHO);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("密碼錯誤！");
		return 0;
	}
	
	twoFA_GenCode(rev_code, 8);
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
	outs(ANSI_COLOR(1;33) "請您記下復原碼並妥善保管，離開本視窗後就不能再重新查詢。" ANSI_RESET);
	
	pressanykey();
	return 0;
}

#endif //USE_IBUNNY_2FALOGIN