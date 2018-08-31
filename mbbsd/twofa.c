#include "bbs.h"

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
	
	snprintf(uri, sizeof(uri), "%s?user=%s", IBUNNY_2FA_URI, user);
	THTTP t;
	thttp_init(&t);
	ret = thttp_get(&t, IBUNNY_SERVER, uri, IBUNNY_SERVER);
	if (!ret)
	code = thttp_code(&t);
	thttp_cleanup(&t);

	if (ret)
		return "伺服器連線失敗，請稍後再試，持續發生請回報為BUG。";

    if (code != 200){
		snprintf(buf, sizeof(buf), "伺服器回傳%d，請稍後再試，持續發生請回報為BUG。", code);
		return buf;
	}
	
	return NULL;
}

int twoFA_main(char *user)
{
    FILE           *fp;
    const char *msg = NULL;
    char code[7], rev_code[9], code_input[9], buf[200];

    vs_hdr("兩步驟驗證");
    move(1, 0);

    twoFA_GenCode(code, 6);
	
	snprintf(buf, sizeof(buf), "home/%c/%s/2fa.code", user[0], user);
	if (!(fp = fopen(buf, "w"))){
		outs("檔案系統錯誤，請回報為BUG。");
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

    outs("認證碼將直接被發送到iBunny\n");
    outs("一共為六位數字\n");

    for (int i = 3; i > 0; i--) {
	if (i < 3) {
	    char buf[80];
	    snprintf(buf, sizeof(buf), ANSI_COLOR(1;31)
		     "驗證碼錯誤，您還有 %d 次機會。" ANSI_RESET, i);
	    move(6, 0);
	    outs(buf);
	}
	code_input[0] = '\0';
	getdata(5, 0, "請輸入驗證碼：", code_input,
		sizeof(code_input), DOECHO);
	size_t length = strlen(code_input);
	if(length == 6){
	if (!strcmp(code, code_input))
	    return NULL;
	}else if(length == 8){
	snprintf(buf, sizeof(buf), "home/%c/%s/2fa.recov", user[0], user);
	if (!(fp = fopen(buf, "r"))){
		outs("沒有復原碼檔案，您是不是沒設定。");
		return -1;
	}
	fgets(rev_code, sizeof(rev_code), fp);
	fclose(fp);
	if (!strcmp(rev_code, code_input))
	    return NULL;
	}
    }

    return -1;
}

int twoFA_genRecovCode()
{
    FILE           *fp;
    char rev_code[9], buf[200], genbuf[3];
	char *user = cuser.userid;

	vs_hdr("兩步驟驗證復原碼");
	if(!(HasUserFlag(UF_TWOFA_LOGIN))){
		vmsg("請先在個人設定開啟兩步驟驗證。");
		return 0;
	}
	
	snprintf(buf, sizeof(buf), "home/%c/%s/2fa.recov", user[0], user);
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
    outs("復原碼是當您無法使用兩步驟驗證時\n");
    outs("可以在輸入驗證碼時輸入復原碼取回帳戶存取權\n");
	
	twoFA_GenCode(rev_code, 8);
	if (!(fp = fopen(buf, "w"))){
		vmsg("檔案系統錯誤，請回報為BUG。");
		return 0;
	}
	fprintf(fp,"%s", rev_code);
	fclose(fp);
	
	move(10,0);
	outs("您的復原碼是：");
	outs(rev_code);
	outs("\n\n");
	outs("請您記下復原碼並妥善保管，離開本視窗後就不能再重新查詢。");
	
	pressanykey();
}

