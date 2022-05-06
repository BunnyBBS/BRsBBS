#include "bbs.h"

#ifdef USE_TWOFA_LOGIN

#ifdef USE_TWOFA_TOTP
/* 請注意，目前TOTP產生須仰賴 {BBSHOME}/bin/totp 這隻程式
   首次使用時，您須先至 {BBSHOME}/BRsBBS/util 中執行 pmake totp 將totp這隻程式裝起來
   否則使用時可能會發生不可預期的錯誤
   編譯util/totp.c時，請確認您有安裝libssl-dev */
static void totp_code(char *buf2, const char * pubkey)
{
   FILE           *fp;
   char            buf[200];
   snprintf(buf, sizeof(buf), "%s/bin/totp %s", BBSHOME, pubkey);

   fp = popen(buf, "r");
   fgets(buf2, sizeof(buf2), fp);
   pclose(fp);

   buf2[7] = '\0';
}
#endif //USE_TWOFA_TOTP

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

	snprintf(uri, sizeof(uri), "/%s?user=%s&code=%s"
			 , TWOFA_CODE_URI, user
#ifdef BETA
			 , authcode
#else
			 , ""
#endif
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

	return code;
}

#ifdef IBUNNY_TWOFA_SIMPLE
static void twoFA_GenToken(char *buf, size_t len)
{
	const char * const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

	for (int i = 0; i < len; i++)
	buf[i] = chars[random() % strlen(chars)];
	buf[len] = '\0';
}

static const char *
twoFA_Simple(char *user, char *authcode)
{
	int ret, code = 0;
	char uri[320] = "",buf[200];

	snprintf(uri, sizeof(uri), "/%s?user=%s&token=%s"
			 , TWOFA_SIMPLE_URI, user, authcode);

	THTTP t;
	thttp_init(&t);
	snprintf(buf, sizeof(buf), "Bearer %s", IBUNNY_API_KEY);
	ret = thttp_get(&t, IBUNNY_SERVER, uri, IBUNNY_SERVER, buf);
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(code == 200)
		return NULL;

	return code;
}
#endif //IBUNNY_TWOFA_SIMPLE

#ifdef USE_PHONE_SMS
static const char *
twoFA_sms_Send(char *user, char *authcode, char *cellphone)
{
	int ret, code = 0;
	char uri[320] = "",buf[200];

	snprintf(uri, sizeof(uri), "/%s?user=%s&cellphone=%s&code=%s"
			 , SMS_TWOFA_URI, user, cellphone
#ifdef BETA
			 , authcode
#else
			 , ""
#endif
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

	return code;
}
#endif //USE_PHONE_SMS

int twoFA_setting(void)
{
	FILE		   *fp;
	int msg;
	char code[7], rev_code[9], code_input[7], buf[200], buf2[200], genbuf[3];
	char *user = cuser.userid;
	char passbuf[PASSLEN];

	clear();
	vs_hdr2(" 兩步驟認證 ", " 認證設定");

	move(3,0);
	prints("設定帳號：%s\n", user);
	prints("您目前%s開啟兩步驟認證。\n", ((cuser.uflag & UF_TWOFA_LOGIN) ? ANSI_COLOR(1) "已" ANSI_RESET : ANSI_COLOR(1;33) "未" ANSI_RESET));

	if(cuser.uflag & UF_TWOFA_LOGIN)
		getdata(6, 0, "關閉(y) 取消操作[N] ",genbuf, 3, LCECHO);
	else{
		move(4, 0);
		outs("本功\能需透過您已登入的iBunny");
#ifdef USE_PHONE_SMS
		outs("或您註冊的手機號碼傳送驗證碼");
#endif
		outs("，\n");
		outs("或者您可以使用Google Authenticator等動態驗證碼產生器。\n");
#ifdef USE_PHONE_SMS
		outs("使用Telegram版iBunny，除設定時須使用驗證碼，未來登入時可使用簡易認證。\n\n");
		outs("請注意：如透過手機號碼傳送驗證碼，每個帳號24小時內只允許\發送3則簡訊，\n");
		outs("        當超過限額時您將無法收到簡訊驗證碼，只能透過其他方式驗證。\n");
		outs("        建議您也登入iBunny以避免超過簡訊限額而無法正確驗證登入。\n");
		outs("        當您有登入iBunny同時也有註冊手機號碼時，系統會優先發送驗證碼至iBunny。\n");
		getdata(13, 0, "(T)動態驗證碼 (I)iBunny (S)簡訊驗證碼 [N]取消操作 ",genbuf, 3, LCECHO);
#else
		getdata(8, 0, "(T)動態驗證碼 (I)iBunny [N]取消操作 ",genbuf, 3, LCECHO);
#endif
	}

	if (genbuf[0] != 'y' && genbuf[0] != 't' && genbuf[0] != 'i' && genbuf[0] != 's') {
		vmsg("取消操作。");
		return 0;
	}

#ifndef USE_PHONE_SMS
	if (genbuf[0] == 's') {
		vmsg("取消操作。");
		return 0;
	}
#endif

#ifndef USE_TWOFA_TOTP
	if (genbuf[0] == 't') {
		vmsg("取消操作。");
		return 0;
	}
#endif

	clear();move(6,0);
	outs("以下操作需要先確認您的身份。\n");
	getdata(7, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("密碼錯誤！");
		return 0;
	}

	if(cuser.uflag & UF_TWOFA_LOGIN){
		pwcuToggleUserFlag(UF_TWOFA_LOGIN);
		log_usersecurity(cuser.userid, "關閉兩步驟認證", fromhost);
		vmsg("已關閉兩步驟認證。");
		return 0;
	}

	/* 以下開始是設定兩步驟認證的程序 */
	move(4, 0);clrtobot();

	if (genbuf[0] == 'i' || genbuf[0] == 's') {
		twoFA_GenCode(code, 6);
		setuserfile(buf, "2fa.code");
		if (!(fp = fopen(buf, "w"))){
			mvouts(b_lines - 1, 0 ,"未成功\開啟兩步驟認證。");
			vmsg("系統錯誤，請稍後再試。(Error code: 2FA-S-001)");
			return 0;
		}
		fprintf(fp,"%s", code);
		fclose(fp);

		if (genbuf[0] == 'i') {
			mvouts(6, 0, "我們將發送一則驗證碼至您的iBunny作為測試。\n");
			msg = twoFA_Send(user,code);
			if (msg != NULL){
				unlink(buf);
				mvouts(b_lines - 1, 0 ,"未成功\開啟兩步驟認證。");
				vmsgf("%s", ibunny_code2msg(msg));
				return 0;
			}
		}

#ifdef USE_PHONE_SMS
		if (genbuf[0] == 's') {
			if (strcmp(cuser.cellphone, "")){
				mvouts(6, 0, "系統即將發送驗證碼至您註冊的手機號碼。\n");
				pressanykey();
				msg = twoFA_sms_Send(user,code,cuser.cellphone);
				if (msg != NULL){
					unlink(buf);
					mvouts(b_lines - 1, 0 ,"未成功\開啟兩步驟認證。");
					vmsgf("%s", ibunny_code2msg(msg));
					return 0;
				}
			}else{
				unlink(buf);
				mvouts(b_lines - 1, 0 ,"未成功\開啟兩步驟認證。");
				vmsg("您還未設定手機號碼！");
				return 0;
			}
		}
#endif //USE_PHONE_SMS

		unlink(buf);
		move(8, 0);outs("驗證碼應為6位數字。\n");
	}
#ifdef USE_TWOFA_TOTP
	if (genbuf[0] == 't') {
		char totp_key[17];
		twoFA_GenRevCode(totp_key, 16);
		strlcpy(cuser.totp_key, totp_key, sizeof(cuser.totp_key));
		passwd_update(usernum, &cuser);
		mvouts(3, 0, "請在動態驗證碼產生器中輸入下方金鑰：\n\n    ");
		outs(totp_key);
		outs("\n\n並取得動態驗證碼。\n");
		outs("您可以將金鑰複製至 https://bunnybbs.tw/TOTP/QRcode 中產生QR-Code。");
		pressanykey();
	}
#endif //USE_TWOFA_TOTP

	for (int i = 3; i > 0; i--) {
		if (i < 3) {
			char buf[80];
			move(10, 0);
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "驗證碼錯誤，您還有 %d 次機會。" ANSI_RESET, i);
			outs(buf);
		}
#ifdef USE_TWOFA_TOTP
		if (genbuf[0] == 't') {
			totp_code(code, cuser.totp_key);
		}
		/* 理論上不應該放這裡，可是放下面會影響code_input。Why? No idea...
		   基本上放這裡主要問題是產生時跟使用者送出時有一點時差而認證失敗 */
#endif //USE_TWOFA_TOTP
		code_input[0] = '\0';
		getdata(9, 0, "請輸入驗證碼：", code_input,
			sizeof(code_input), DOECHO);

		size_t length = strlen(code_input);
		if(length == 6){
			if (!strcmp(code, code_input)){
				pwcuToggleUserFlag(UF_TWOFA_LOGIN);
				log_usersecurity(cuser.userid, "開啟兩步驟認證", fromhost);
				setuserfile(buf, "2fa.recov");

				if(isFileExist(buf) == true){
					vmsg("已開啟兩步驟認證！");
					return 0;
				}

				clear();
				vs_hdr2(" 兩步驟認證 ", " 產生復原碼");
				outs("已開啟兩步驟認證，現在系統正在為您產生一組復原碼。\n\n");
				outs("復原碼是當您無法使用兩步驟認證時，\n");
				outs("可以在輸入驗證碼時輸入復原碼取回帳戶存取權。\n");
				outs("另外，另外每一組復原碼只能使用一次，使用後就會失效。\n\n");

				twoFA_GenRevCode(rev_code, 8);
				if (!(fp = fopen(buf, "w"))){
					vmsg("系統錯誤，請稍後再試。(Error code: 2FA-S-002)");
					return 0;
				}
				fprintf(fp,"%s", rev_code);
				fclose(fp);

				log_usersecurity(cuser.userid, "產生兩步驟認證復原碼", fromhost);

				move(10,0);
				outs("您的復原碼是：" ANSI_COLOR(1));outs(rev_code);outs(ANSI_RESET "\n\n");
				outs("復原碼共計8碼，會出現英文字母I、L、O，不會出現數字0、1。");
				mvouts(b_lines - 2, 12, ANSI_COLOR(1;33) "請您記下復原碼並妥善保管，離開本視窗後就不能再重新查詢。" ANSI_RESET);

				pressanykey();
				return 0;
			}
		}
	}

	vmsg("未成功\開啟兩步驟認證。");
	return 0;
}

int twoFA_main(const userec_t *u)
{
	FILE		   *fp;
	const char *msg = NULL;
	char code[7], rev_code[9], code_input[9], buf[200], buf2[200], genbuf[3];
	char coded[200], revcd[200];
	char *user = u->userid;

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
	vs_hdr2(" 兩步驟認證 ", "");

	setuserfile(coded, "2fa.code");
	setuserfile(revcd, "2fa.recov");

	int use_sms = 0;
	int use_totp = 0;
#ifdef USE_TWOFA_TOTP
	if(strcmp(u->totp_key, "")){
		use_totp = 1;
		char totp_key[17];
		strlcpy(totp_key, u->totp_key, sizeof(totp_key));

		/* 原本想讓系統自動跳到輸入畫面，但還是留個選擇好了 */
		getdata(4, 0, "[ENTER]使用動態驗證碼 (Q)使用iBuuny或簡訊 ",genbuf, 3, LCECHO);
		if (genbuf[0] == 'q') {
			use_totp = 0;
		}
	}

	if(use_totp == 0){	
#endif //USE_TWOFA_TOTP
#ifdef IBUNNY_TWOFA_SIMPLE
		char token[33];
		twoFA_GenToken(token, 32);
		msg = twoFA_Simple(user,token);
		if(msg == NULL){
			while(true){
				msg = twoFA_Simple(user,token);
				if(msg == 404){
					sleep(5);
					continue;
				}else if(msg == NULL){
					mvouts(10, 35, ANSI_COLOR(1;33) "驗證成功\！" ANSI_RESET );
					return NULL; //Success
				}else if(msg == 403){
					mvouts(10, 27, ANSI_COLOR(1;31) "驗證失敗：登入請求被拒絕！" ANSI_RESET );
					now = time(NULL);
					snprintf(buf2, sizeof(buf2), "[%s] 兩步驟認證失敗：登入請求被拒絕(%s)\n",Cdate(&now), fromhost);
					setuserfile(buf, FN_BADTWOFA);
					log_filef(buf, LOG_CREAT, buf2);
					log_usersecurity(user, "兩步驟認證失敗：登入請求被拒絕", fromhost);
					return -1;
				}else if(msg == 408){
					mvouts(10, 24, ANSI_COLOR(1;31) "驗證失敗：操作逾時，請重新登入！" ANSI_RESET );
					now = time(NULL);
					snprintf(buf2, sizeof(buf2), "[%s] 兩步驟認證失敗：操作逾時(%s)\n",Cdate(&now), fromhost);
					setuserfile(buf, FN_BADTWOFA);
					log_filef(buf, LOG_CREAT, buf2);
					log_usersecurity(user, "兩步驟認證失敗：操作逾時", fromhost);
					return -1;
				}else{
					move(10, 0);
					prints("  錯誤：%s\n", ibunny_code2msg(msg));
					outs("  即將改用驗證碼登入…");
					pressanykey();
					break;
				}
			}
		}/*else{
			move(10, 0);
			prints("  錯誤：%s\n", ibunny_code2msg(msg));
			outs("  即將改用驗證碼登入…");
			pressanykey();
		}*/ /* 無法適用簡易認證時不提示，直接繼續下面流程 */
#endif //IBUNNY_TWOFA_SIMPLE

		twoFA_GenCode(code, 6);

		if (!(fp = fopen(coded, "w"))){
			move(1,0);
			outs("系統錯誤，您可以使用復原碼或稍後再試。(Error code: 2FA-F-001)");
			getdata(2, 0, "使用復原碼？ (y/N) ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				return -1;
			}
			if (!(fp = fopen(revcd, "r"))){
				outs("系統錯誤，無法使用復原碼，請稍後再試。(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);
			int i = 3;
			for (i = 3; i > 0; i--) {
				if (i < 3) {
					char buf[80];
					snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "復原碼錯誤，您還有 %d 次機會。" ANSI_RESET, i);
					move(6, 0);
					outs(buf);
				}
				code_input[0] = '\0';
				getdata(5, 0, "請輸入復原碼：", code_input, sizeof(code_input), DOECHO);
				if (!strcmp(rev_code, code_input)){
					unlink(revcd);
					move(6, 0);
					outs(ANSI_COLOR(1;33) "使用了復原碼認證，故復原碼已失效。" ANSI_RESET);
					return NULL; //Success
				}
			}

			now = time(NULL);
			snprintf(buf2, sizeof(buf2), "[%s] 兩步驟認證失敗：第%d次驗證碼錯誤(%s)\n",Cdate(&now), 4 - i , fromhost);
			setuserfile(buf, FN_BADTWOFA);
			log_filef(buf, LOG_CREAT, buf2);
			log_usersecurity(user, "兩步驟認證失敗：驗證碼錯誤", fromhost);
			return -1;
		}
		fprintf(fp,"%s", code);
		fclose(fp);

		msg = twoFA_Send(user,code);

#ifdef USE_PHONE_SMS
		if(!strcmp(u->cellphone, ""))
			use_sms = 2; /* 當使用者沒有設定手機號碼，就用use_sms=2來識別 */

		/* 沒有登入iBunny就自動嘗試簡訊 */
		if(msg == 400 && use_sms == 0){
			msg = NULL; use_sms = 1;
			msg = twoFA_sms_Send(user, code, u->cellphone);
			if(msg != NULL){
				use_sms = -1;
			}
		}
#endif //USE_PHONE_SMS

	   	if(msg != NULL){
			move(1, 0); clrtobot();
			prints("\n錯誤：%s", ibunny_code2msg(msg));
			outs("\n如果您有收到驗證碼，但系統誤報錯誤，輸入R即可進入驗證程序。");
#ifdef USE_PHONE_SMS
			/* 沒有登入iBunny會自動發簡訊，iBunny系統掛了或其他問題則不會，讓使用者自己選 */
			if(use_sms == 0)
				getdata(4, 0, "(S)改用手機簡訊 (R)使用復原碼 [C]取消登入 ",genbuf, 3, LCECHO);
			else
#endif //USE_PHONE_SMS
				getdata(4, 0, /*"(T)再試一次 "*/"(R)使用復原碼 [C]取消登入 ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'r' && genbuf[0] != 's') {
				unlink(coded);
				return -1;
			}
#ifdef USE_PHONE_SMS
			if(use_sms == 0 && genbuf[0] == 's'){
				use_sms = 1;
				msg = twoFA_sms_Send(user,code,u->cellphone);
				if(msg != NULL){
					use_sms = -1;
					move(1,0); clrtobot();
					prints("\n錯誤：%s", ibunny_code2msg(msg));
					getdata(3, 0, "(R)使用復原碼 [C]取消登入 ",genbuf, 3, LCECHO);
					if (genbuf[0] != 'r') {
						unlink(coded);
						return -1;
					}
				}
			}
#else //USE_PHONE_SMS
			if(genbuf[0] == 's'){
				unlink(coded);
				return -1;
			}
#endif //USE_PHONE_SMS
			/*if(genbuf[0] == 't'){
				msg = twoFA_Send(user,code);
				if (msg != NULL){
					move(1,0); clrtobot();
					prints("錯誤：%s",msg);
					getdata(3, 0, "(R)使用復原碼 [C]取消登入 ",genbuf, 3, LCECHO);
					if (genbuf[0] != 'r') {
						unlink(buf);
						return -1;
					}
				}

			}*/
		}
		unlink(coded);
		/* 發送程序至此為止，後面不再管驗證碼發送的事情。 */
#ifdef USE_TWOFA_TOTP
	}
#endif //USE_TWOFA_TOTP

	move(1,0); clrtobot();
#ifdef USE_TWOFA_TOTP
	if(use_totp == 1){
		mvouts(2, 0, "請開啟您的動態驗證碼產生器取得驗證碼。");
	}else{
#endif //USE_TWOFA_TOTP
		if(msg == NULL){
			mvouts(2, 0, "驗證碼將直接被發送到");
#ifdef USE_PHONE_SMS
			if(use_sms == 1)
				outs("您註冊的手機號碼");
			else
#endif //USE_PHONE_SMS
				outs("您綁定的iBunny");
		}
#ifdef USE_TWOFA_TOTP
	}
#endif //USE_TWOFA_TOTP
	outs("\n驗證碼為6位數字、復原碼為8位英數混合。\n");

	int y = 5;

	for (int i = 3; i > 0; i--) {
		if(i < 3){
			char buf[80];
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "驗證碼錯誤，您還有 %d 次機會。" ANSI_RESET, i);
			move(y + 1, 0); clrtobot();
			outs(buf);
		}
#ifdef USE_TWOFA_TOTP
		if (use_totp == 1) {
			totp_code(code, cuser.totp_key);
		}
		/* 同上述，放這裡主要問題是怕產生時跟使用者送出時有一點時差而認證失敗 */
#endif //USE_TWOFA_TOTP
		code_input[0] = '\0';
		getdata(y, 0, "請輸入驗證碼：", code_input, sizeof(code_input), DOECHO);
		size_t length = strlen(code_input);
		if(length == 8){
			if (!(fp = fopen(revcd, "r"))){
				outs("系統錯誤，無法使用復原碼，請稍後再試。(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);

			if (!strcmp(rev_code, code_input)){
				unlink(revcd);
				move(y + 1, 0);
				outs(ANSI_COLOR(1;33) "使用了復原碼認證，故復原碼已失效。" ANSI_RESET);
				return NULL; //Success
			}
		}else{
			if (!strcmp(code, code_input)){
#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
				clear();
				vs_hdr2(" 兩步驟認證 ", " 設定為信任的裝置？");
				move(2, 0);
				outs("設定為信任的裝置下次登入就不需要驗證。\n"
					 "提請您，請不要在公用電腦上使用此功\能。");
				getdata(y, 0, "是否將這個裝置設定為信任的裝置(y/n)？ [N]",genbuf, 3, LCECHO);
				if(genbuf[0] == 'y') {
					setuserfile(buf, "trust.device");
					log_filef(buf, LOG_CREAT,"%8.8X\n", client_code);
				}
#endif
				return NULL; //Success
			}
		}

		now = time(NULL);
		snprintf(buf2, sizeof(buf2), "[%s] 兩步驟認證失敗：第%d次驗證碼錯誤(%s)\n",Cdate(&now), 4 - i , fromhost);
		setuserfile(buf, FN_BADTWOFA);
		log_filef(buf, LOG_CREAT, buf2);
		log_usersecurity(user, "兩步驟認證失敗：驗證碼錯誤", fromhost);
	}
	return -1;
}

int twoFA_genRecovCode()
{
	FILE *fp;
	char rev_code[9], buf[200], genbuf[3];
	char *user = cuser.userid;
	char passbuf[PASSLEN];

	vs_hdr2(" 兩步驟認證 ", " 產生復原碼");
	if(!(HasUserFlag(UF_TWOFA_LOGIN))){
		vmsg("請先開啟兩步驟認證。");
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

	log_usersecurity(cuser.userid, "產生兩步驟認證復原碼", fromhost);

	move(10,0);
	outs("您的復原碼是：" ANSI_COLOR(1));
	outs(rev_code);
	outs(ANSI_RESET "\n\n");
	outs("復原碼共計8碼，會出現英文字母I、L、O，不會出現數字0、1。");
	mvouts(b_lines - 2,12,ANSI_COLOR(1;33) "請您記下復原碼並妥善保管，離開本視窗後就不能再重新查詢。" ANSI_RESET);

	pressanykey();
	return 0;
}

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

#endif //USE_TWOFA_LOGIN
