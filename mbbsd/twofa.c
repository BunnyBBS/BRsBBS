#include "bbs.h"

#ifdef USE_TWOFA_LOGIN

#ifdef USE_TWOFA_TOTP
/* �Ъ`�N�A�ثeTOTP���Ͷ����� {BBSHOME}/bin/totp �o���{��
   �����ϥήɡA�z������ {BBSHOME}/BRsBBS/util ������ pmake totp �Ntotp�o���{���˰_��
   �_�h�ϥήɥi��|�o�ͤ��i�w�������~
   �sĶutil/totp.c�ɡA�нT�{�z���w��libssl-dev */
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
	vs_hdr2(" ��B�J�{�� ", " �{�ҳ]�w");

	move(3,0);
	prints("�]�w�b���G%s\n", user);
	prints("�z�ثe%s�}�Ҩ�B�J�{�ҡC\n", ((cuser.uflag & UF_TWOFA_LOGIN) ? ANSI_COLOR(1) "�w" ANSI_RESET : ANSI_COLOR(1;33) "��" ANSI_RESET));

	if(cuser.uflag & UF_TWOFA_LOGIN)
		getdata(6, 0, "����(y) �����ާ@[N] ",genbuf, 3, LCECHO);
	else{
		move(4, 0);
		outs("���\\��ݳz�L�z�w�n�J��iBunny");
#ifdef USE_PHONE_SMS
		outs("�αz���U��������X�ǰe���ҽX");
#endif
		outs("�A\n");
		outs("�Ϊ̱z�i�H�ϥ�Google Authenticator���ʺA���ҽX���;��C\n");
#ifdef USE_PHONE_SMS
		outs("�ϥ�Telegram��iBunny�A���]�w�ɶ��ϥ����ҽX�A���ӵn�J�ɥi�ϥ�²���{�ҡC\n\n");
		outs("�Ъ`�N�G�p�z�L������X�ǰe���ҽX�A�C�ӱb��24�p�ɤ��u���\\�o�e3�h²�T�A\n");
		outs("        ��W�L���B�ɱz�N�L�k����²�T���ҽX�A�u��z�L��L�覡���ҡC\n");
		outs("        ��ĳ�z�]�n�JiBunny�H�קK�W�L²�T���B�ӵL�k���T���ҵn�J�C\n");
		outs("        ��z���n�JiBunny�P�ɤ]�����U������X�ɡA�t�η|�u���o�e���ҽX��iBunny�C\n");
		getdata(13, 0, "(T)�ʺA���ҽX (I)iBunny (S)²�T���ҽX [N]�����ާ@ ",genbuf, 3, LCECHO);
#else
		getdata(8, 0, "(T)�ʺA���ҽX (I)iBunny [N]�����ާ@ ",genbuf, 3, LCECHO);
#endif
	}

	if (genbuf[0] != 'y' && genbuf[0] != 't' && genbuf[0] != 'i' && genbuf[0] != 's') {
		vmsg("�����ާ@�C");
		return 0;
	}

#ifndef USE_PHONE_SMS
	if (genbuf[0] == 's') {
		vmsg("�����ާ@�C");
		return 0;
	}
#endif

#ifndef USE_TWOFA_TOTP
	if (genbuf[0] == 't') {
		vmsg("�����ާ@�C");
		return 0;
	}
#endif

	clear();move(6,0);
	outs("�H�U�ާ@�ݭn���T�{�z�������C\n");
	getdata(7, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("�K�X���~�I");
		return 0;
	}

	if(cuser.uflag & UF_TWOFA_LOGIN){
		pwcuToggleUserFlag(UF_TWOFA_LOGIN);
		log_usersecurity(cuser.userid, "������B�J�{��", fromhost);
		vmsg("�w������B�J�{�ҡC");
		return 0;
	}

	/* �H�U�}�l�O�]�w��B�J�{�Ҫ��{�� */
	move(4, 0);clrtobot();

	if (genbuf[0] == 'i' || genbuf[0] == 's') {
		twoFA_GenCode(code, 6);
		setuserfile(buf, "2fa.code");
		if (!(fp = fopen(buf, "w"))){
			mvouts(b_lines - 1, 0 ,"�����\\�}�Ҩ�B�J�{�ҡC");
			vmsg("�t�ο��~�A�еy��A�աC(Error code: 2FA-S-001)");
			return 0;
		}
		fprintf(fp,"%s", code);
		fclose(fp);

		if (genbuf[0] == 'i') {
			mvouts(6, 0, "�ڭ̱N�o�e�@�h���ҽX�ܱz��iBunny�@�����աC\n");
			msg = twoFA_Send(user,code);
			if (msg != NULL){
				unlink(buf);
				mvouts(b_lines - 1, 0 ,"�����\\�}�Ҩ�B�J�{�ҡC");
				vmsgf("%s", ibunny_code2msg(msg));
				return 0;
			}
		}

#ifdef USE_PHONE_SMS
		if (genbuf[0] == 's') {
			if (strcmp(cuser.cellphone, "")){
				mvouts(6, 0, "�t�ΧY�N�o�e���ҽX�ܱz���U��������X�C\n");
				pressanykey();
				msg = twoFA_sms_Send(user,code,cuser.cellphone);
				if (msg != NULL){
					unlink(buf);
					mvouts(b_lines - 1, 0 ,"�����\\�}�Ҩ�B�J�{�ҡC");
					vmsgf("%s", ibunny_code2msg(msg));
					return 0;
				}
			}else{
				unlink(buf);
				mvouts(b_lines - 1, 0 ,"�����\\�}�Ҩ�B�J�{�ҡC");
				vmsg("�z�٥��]�w������X�I");
				return 0;
			}
		}
#endif //USE_PHONE_SMS

		unlink(buf);
		move(8, 0);outs("���ҽX����6��Ʀr�C\n");
	}
#ifdef USE_TWOFA_TOTP
	if (genbuf[0] == 't') {
		char totp_key[17];
		twoFA_GenRevCode(totp_key, 16);
		strlcpy(cuser.totp_key, totp_key, sizeof(cuser.totp_key));
		passwd_update(usernum, &cuser);
		mvouts(3, 0, "�Цb�ʺA���ҽX���;�����J�U����_�G\n\n    ");
		outs(totp_key);
		outs("\n\n�è��o�ʺA���ҽX�C\n");
		outs("�z�i�H�N���_�ƻs�� https://bunnybbs.tw/TOTP/QRcode ������QR-Code�C");
		pressanykey();
	}
#endif //USE_TWOFA_TOTP

	for (int i = 3; i > 0; i--) {
		if (i < 3) {
			char buf[80];
			move(10, 0);
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "���ҽX���~�A�z�٦� %d �����|�C" ANSI_RESET, i);
			outs(buf);
		}
#ifdef USE_TWOFA_TOTP
		if (genbuf[0] == 't') {
			totp_code(code, cuser.totp_key);
		}
		/* �z�פW�����ө�o�̡A�i�O��U���|�v�Tcode_input�CWhy? No idea...
		   �򥻤W��o�̥D�n���D�O���ͮɸ�ϥΪ̰e�X�ɦ��@�I�ɮt�ӻ{�ҥ��� */
#endif //USE_TWOFA_TOTP
		code_input[0] = '\0';
		getdata(9, 0, "�п�J���ҽX�G", code_input,
			sizeof(code_input), DOECHO);

		size_t length = strlen(code_input);
		if(length == 6){
			if (!strcmp(code, code_input)){
				pwcuToggleUserFlag(UF_TWOFA_LOGIN);
				log_usersecurity(cuser.userid, "�}�Ҩ�B�J�{��", fromhost);
				setuserfile(buf, "2fa.recov");

				if(isFileExist(buf) == true){
					vmsg("�w�}�Ҩ�B�J�{�ҡI");
					return 0;
				}

				clear();
				vs_hdr2(" ��B�J�{�� ", " ���ʹ_��X");
				outs("�w�}�Ҩ�B�J�{�ҡA�{�b�t�Υ��b���z���ͤ@�մ_��X�C\n\n");
				outs("�_��X�O��z�L�k�ϥΨ�B�J�{�ҮɡA\n");
				outs("�i�H�b��J���ҽX�ɿ�J�_��X���^�b��s���v�C\n");
				outs("�t�~�A�t�~�C�@�մ_��X�u��ϥΤ@���A�ϥΫ�N�|���ġC\n\n");

				twoFA_GenRevCode(rev_code, 8);
				if (!(fp = fopen(buf, "w"))){
					vmsg("�t�ο��~�A�еy��A�աC(Error code: 2FA-S-002)");
					return 0;
				}
				fprintf(fp,"%s", rev_code);
				fclose(fp);

				log_usersecurity(cuser.userid, "���ͨ�B�J�{�Ҵ_��X", fromhost);

				move(10,0);
				outs("�z���_��X�O�G" ANSI_COLOR(1));outs(rev_code);outs(ANSI_RESET "\n\n");
				outs("�_��X�@�p8�X�A�|�X�{�^��r��I�BL�BO�A���|�X�{�Ʀr0�B1�C");
				mvouts(b_lines - 2, 12, ANSI_COLOR(1;33) "�бz�O�U�_��X�ç����O�ޡA���}��������N����A���s�d�ߡC" ANSI_RESET);

				pressanykey();
				return 0;
			}
		}
	}

	vmsg("�����\\�}�Ҩ�B�J�{�ҡC");
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
	vs_hdr2(" ��B�J�{�� ", "");

	setuserfile(coded, "2fa.code");
	setuserfile(revcd, "2fa.recov");

	int use_sms = 0;
	int use_totp = 0;
#ifdef USE_TWOFA_TOTP
	if(strcmp(u->totp_key, "")){
		use_totp = 1;
		char totp_key[17];
		strlcpy(totp_key, u->totp_key, sizeof(totp_key));

		/* �쥻�Q���t�Φ۰ʸ����J�e���A���٬O�d�ӿ�ܦn�F */
		getdata(4, 0, "[ENTER]�ϥΰʺA���ҽX (Q)�ϥ�iBuuny��²�T ",genbuf, 3, LCECHO);
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
					mvouts(10, 35, ANSI_COLOR(1;33) "���Ҧ��\\�I" ANSI_RESET );
					return NULL; //Success
				}else if(msg == 403){
					mvouts(10, 27, ANSI_COLOR(1;31) "���ҥ��ѡG�n�J�ШD�Q�ڵ��I" ANSI_RESET );
					now = time(NULL);
					snprintf(buf2, sizeof(buf2), "[%s] ��B�J�{�ҥ��ѡG�n�J�ШD�Q�ڵ�(%s)\n",Cdate(&now), fromhost);
					setuserfile(buf, FN_BADTWOFA);
					log_filef(buf, LOG_CREAT, buf2);
					log_usersecurity(user, "��B�J�{�ҥ��ѡG�n�J�ШD�Q�ڵ�", fromhost);
					return -1;
				}else if(msg == 408){
					mvouts(10, 24, ANSI_COLOR(1;31) "���ҥ��ѡG�ާ@�O�ɡA�Э��s�n�J�I" ANSI_RESET );
					now = time(NULL);
					snprintf(buf2, sizeof(buf2), "[%s] ��B�J�{�ҥ��ѡG�ާ@�O��(%s)\n",Cdate(&now), fromhost);
					setuserfile(buf, FN_BADTWOFA);
					log_filef(buf, LOG_CREAT, buf2);
					log_usersecurity(user, "��B�J�{�ҥ��ѡG�ާ@�O��", fromhost);
					return -1;
				}else{
					move(10, 0);
					prints("  ���~�G%s\n", ibunny_code2msg(msg));
					outs("  �Y�N������ҽX�n�J�K");
					pressanykey();
					break;
				}
			}
		}/*else{
			move(10, 0);
			prints("  ���~�G%s\n", ibunny_code2msg(msg));
			outs("  �Y�N������ҽX�n�J�K");
			pressanykey();
		}*/ /* �L�k�A��²���{�Үɤ����ܡA�����~��U���y�{ */
#endif //IBUNNY_TWOFA_SIMPLE

		twoFA_GenCode(code, 6);

		if (!(fp = fopen(coded, "w"))){
			move(1,0);
			outs("�t�ο��~�A�z�i�H�ϥδ_��X�εy��A�աC(Error code: 2FA-F-001)");
			getdata(2, 0, "�ϥδ_��X�H (y/N) ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				return -1;
			}
			if (!(fp = fopen(revcd, "r"))){
				outs("�t�ο��~�A�L�k�ϥδ_��X�A�еy��A�աC(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);
			int i = 3;
			for (i = 3; i > 0; i--) {
				if (i < 3) {
					char buf[80];
					snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "�_��X���~�A�z�٦� %d �����|�C" ANSI_RESET, i);
					move(6, 0);
					outs(buf);
				}
				code_input[0] = '\0';
				getdata(5, 0, "�п�J�_��X�G", code_input, sizeof(code_input), DOECHO);
				if (!strcmp(rev_code, code_input)){
					unlink(revcd);
					move(6, 0);
					outs(ANSI_COLOR(1;33) "�ϥΤF�_��X�{�ҡA�G�_��X�w���ġC" ANSI_RESET);
					return NULL; //Success
				}
			}

			now = time(NULL);
			snprintf(buf2, sizeof(buf2), "[%s] ��B�J�{�ҥ��ѡG��%d�����ҽX���~(%s)\n",Cdate(&now), 4 - i , fromhost);
			setuserfile(buf, FN_BADTWOFA);
			log_filef(buf, LOG_CREAT, buf2);
			log_usersecurity(user, "��B�J�{�ҥ��ѡG���ҽX���~", fromhost);
			return -1;
		}
		fprintf(fp,"%s", code);
		fclose(fp);

		msg = twoFA_Send(user,code);

#ifdef USE_PHONE_SMS
		if(!strcmp(u->cellphone, ""))
			use_sms = 2; /* ��ϥΪ̨S���]�w������X�A�N��use_sms=2���ѧO */

		/* �S���n�JiBunny�N�۰ʹ���²�T */
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
			prints("\n���~�G%s", ibunny_code2msg(msg));
			outs("\n�p�G�z���������ҽX�A���t�λ~�����~�A��JR�Y�i�i�J���ҵ{�ǡC");
#ifdef USE_PHONE_SMS
			/* �S���n�JiBunny�|�۰ʵo²�T�AiBunny�t�α��F�Ψ�L���D�h���|�A���ϥΪ̦ۤv�� */
			if(use_sms == 0)
				getdata(4, 0, "(S)��Τ��²�T (R)�ϥδ_��X [C]�����n�J ",genbuf, 3, LCECHO);
			else
#endif //USE_PHONE_SMS
				getdata(4, 0, /*"(T)�A�դ@�� "*/"(R)�ϥδ_��X [C]�����n�J ",genbuf, 3, LCECHO);
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
					prints("\n���~�G%s", ibunny_code2msg(msg));
					getdata(3, 0, "(R)�ϥδ_��X [C]�����n�J ",genbuf, 3, LCECHO);
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
					prints("���~�G%s",msg);
					getdata(3, 0, "(R)�ϥδ_��X [C]�����n�J ",genbuf, 3, LCECHO);
					if (genbuf[0] != 'r') {
						unlink(buf);
						return -1;
					}
				}

			}*/
		}
		unlink(coded);
		/* �o�e�{�Ǧܦ�����A�᭱���A�����ҽX�o�e���Ʊ��C */
#ifdef USE_TWOFA_TOTP
	}
#endif //USE_TWOFA_TOTP

	move(1,0); clrtobot();
#ifdef USE_TWOFA_TOTP
	if(use_totp == 1){
		mvouts(2, 0, "�ж}�ұz���ʺA���ҽX���;����o���ҽX�C");
	}else{
#endif //USE_TWOFA_TOTP
		if(msg == NULL){
			mvouts(2, 0, "���ҽX�N�����Q�o�e��");
#ifdef USE_PHONE_SMS
			if(use_sms == 1)
				outs("�z���U��������X");
			else
#endif //USE_PHONE_SMS
				outs("�z�j�w��iBunny");
		}
#ifdef USE_TWOFA_TOTP
	}
#endif //USE_TWOFA_TOTP
	outs("\n���ҽX��6��Ʀr�B�_��X��8��^�ƲV�X�C\n");

	int y = 5;

	for (int i = 3; i > 0; i--) {
		if(i < 3){
			char buf[80];
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "���ҽX���~�A�z�٦� %d �����|�C" ANSI_RESET, i);
			move(y + 1, 0); clrtobot();
			outs(buf);
		}
#ifdef USE_TWOFA_TOTP
		if (use_totp == 1) {
			totp_code(code, cuser.totp_key);
		}
		/* �P�W�z�A��o�̥D�n���D�O�Ȳ��ͮɸ�ϥΪ̰e�X�ɦ��@�I�ɮt�ӻ{�ҥ��� */
#endif //USE_TWOFA_TOTP
		code_input[0] = '\0';
		getdata(y, 0, "�п�J���ҽX�G", code_input, sizeof(code_input), DOECHO);
		size_t length = strlen(code_input);
		if(length == 8){
			if (!(fp = fopen(revcd, "r"))){
				outs("�t�ο��~�A�L�k�ϥδ_��X�A�еy��A�աC(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);

			if (!strcmp(rev_code, code_input)){
				unlink(revcd);
				move(y + 1, 0);
				outs(ANSI_COLOR(1;33) "�ϥΤF�_��X�{�ҡA�G�_��X�w���ġC" ANSI_RESET);
				return NULL; //Success
			}
		}else{
			if (!strcmp(code, code_input)){
#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
				clear();
				vs_hdr2(" ��B�J�{�� ", " �]�w���H�����˸m�H");
				move(2, 0);
				outs("�]�w���H�����˸m�U���n�J�N���ݭn���ҡC\n"
					 "���бz�A�Ф��n�b���ιq���W�ϥΦ��\\��C");
				getdata(y, 0, "�O�_�N�o�Ӹ˸m�]�w���H�����˸m(y/n)�H [N]",genbuf, 3, LCECHO);
				if(genbuf[0] == 'y') {
					setuserfile(buf, "trust.device");
					log_filef(buf, LOG_CREAT,"%8.8X\n", client_code);
				}
#endif
				return NULL; //Success
			}
		}

		now = time(NULL);
		snprintf(buf2, sizeof(buf2), "[%s] ��B�J�{�ҥ��ѡG��%d�����ҽX���~(%s)\n",Cdate(&now), 4 - i , fromhost);
		setuserfile(buf, FN_BADTWOFA);
		log_filef(buf, LOG_CREAT, buf2);
		log_usersecurity(user, "��B�J�{�ҥ��ѡG���ҽX���~", fromhost);
	}
	return -1;
}

int twoFA_genRecovCode()
{
	FILE *fp;
	char rev_code[9], buf[200], genbuf[3];
	char *user = cuser.userid;
	char passbuf[PASSLEN];

	vs_hdr2(" ��B�J�{�� ", " ���ʹ_��X");
	if(!(HasUserFlag(UF_TWOFA_LOGIN))){
		vmsg("�Х��}�Ҩ�B�J�{�ҡC");
		return 0;
	}

	setuserfile(buf, "2fa.recov");
	move(1, 0);

	if(isFileExist(buf) == true){
		outs("�z�w�g���@�մ_��X�A�C�ӱb��u��֦��@�աC\n");
		outs("��z�~��ާ@���ͷs�_��X�A�즳���N�|���ġC");
		getdata(4, 0, "�T�w�~��ܡH (y)�~�� [N]���� ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
			vmsg("�����ާ@");
			return 0;
		}
	}

	move(1, 0); clrtobot();
	outs("�_��X�O��z�L�k�ϥΨ�B�J�{�ҮɡA\n");
	outs("�i�H�b��J���ҽX�ɿ�J�_��X���^�b��s���v�C\n");
	outs("�t�~�A�t�~�C�@�մ_��X�u��ϥΤ@���A�ϥΫ�N�|���ġC\n\n");

	outs("�H�U�ާ@�ݭn���T�{�z�������C\n");
	getdata(6, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("�K�X���~�I");
		return 0;
	}

	twoFA_GenRevCode(rev_code, 8);
	if (!(fp = fopen(buf, "w"))){
		vmsg("�t�ο��~�A�еy��A�աC(Error code: 2FA-F-003)");
		return 0;
	}
	fprintf(fp,"%s", rev_code);
	fclose(fp);

	log_usersecurity(cuser.userid, "���ͨ�B�J�{�Ҵ_��X", fromhost);

	move(10,0);
	outs("�z���_��X�O�G" ANSI_COLOR(1));
	outs(rev_code);
	outs(ANSI_RESET "\n\n");
	outs("�_��X�@�p8�X�A�|�X�{�^��r��I�BL�BO�A���|�X�{�Ʀr0�B1�C");
	mvouts(b_lines - 2,12,ANSI_COLOR(1;33) "�бz�O�U�_��X�ç����O�ޡA���}��������N����A���s�d�ߡC" ANSI_RESET);

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
		vmsg("�S���b����˸m�W�]�w���H���C");
		return 0;
	}

	clear();
	vs_hdr("�M�P�H���˸m");

	move(2, 0);
	outs("�]�w���H�����˸m�b�n�J�ɤ��ݭn���ҡC\n");
	outs("�n�M�P���Ҧ��ثe�]�w���H�����˸m�ܡH\n");
	getdata(5, 0, "�T�w�~��ܡH (y)�~�� [N]���� ",genbuf, 3, LCECHO);
	if (genbuf[0] != 'y') {
		vmsg("�����ާ@");
		return 0;
	}

	unlink(buf);
	vmsg("�w�g�M�P�Ҧ��H�����˸m");
	return 0;
}

#endif //defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)

#endif //USE_TWOFA_LOGIN
