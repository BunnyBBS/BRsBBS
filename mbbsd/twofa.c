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
	if(!ret)
		code = thttp_code(&t);
	thttp_cleanup(&t);

	if(code == 200)
		return NULL;

	if(code == 400)
		return "�b���Ӹ`���~�A�L�k�o�e���ҽX�C(2FA-BS400)";
	if(code == 401)
		return "API�걵�X���A���pô�u�{�~�ȳB��U�C(2FA-BS401)";
	if(code == 410)
		return "�b�����B�w���A�L�k�o�e���ҽX�C(2FA-BS410)";
	if(code == 500)
		return "���A���X���A���pô�u�{�~�ȳB��U�C(2FA-BS500)";
	if(ret)
		return "�t�ο��~�A���pô�u�{�~�ȳB��U�C(2FA-BS001)";
}

static const char *
twoFA_sms_Send(char *user, char *authcode, char *cellphone)
{
	int ret, code = 0;
	char uri[320] = "",buf[200];

	snprintf(uri, sizeof(uri), "/%s?user=%s&cellphone=%s"
#ifdef BETA
			 "&beta=true&code=%s"
#endif
			 , SMS_TWOFA_URI, user, cellphone
#ifdef BETA
			 , authcode
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

	if(code == 400)
		return "�b���Ӹ`���~�A�L�k�o�e���ҽX�C(2FA-MS400)";
	if(code == 401)
		return "API�걵�X���A���pô�u�{�~�ȳB��U�C(2FA-MS401)";
	if(code == 410)
		return "�b�����B�w���A�L�k�o�e���ҽX�C(2FA-MS410)";
	if(code == 500)
		return "���A���X���A���pô�u�{�~�ȳB��U�C(2FA-MS500)";
	if(ret)
		return "�t�ο��~�A���pô�u�{�~�ȳB��U�C(2FA-MS001)";
}

int twoFA_setting(void)
{
    FILE           *fp;
    const char *msg = NULL;
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
        outs("���\\��ݳz�L�z�w�n�J��iBunny�αz���U��������X�ǰe���ҽX�C\n");
        outs("�Ъ`�N�G�p�z�L������X�ǰe���ҽX�A�C�ӱb��24�p�ɤ��u��\�\\�o�e3�h²�T�A\n");
        outs("        ��W�L���B�ɱz�N�L�k����²�T���ҽX�A�u��z�LiBunny�����Ψϥδ_��X�C\n");
        outs("        ��ĳ�z�]�n�JiBunny�H�קK�W�L²�T���B�ӵL�k���T���ҵn�J�C\n");
        outs("        ��z���n�JiBunny�P�ɤ]�����U������X�ɡA�t�η|�u���o�e���ҽX��iBunny�C\n");
        getdata(10, 0, "�}��(y) �����ާ@[N] ",genbuf, 3, LCECHO);
    }
	if (genbuf[0] != 'y') {
		vmsg("�����ާ@�C");
		return 0;
	}

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
		vmsg("�w������B�J�{�ҡC");
		return 0;
	}

	move(4, 0);clrtobot();
	mvouts(6, 0, "�ڭ̱N�o�e�@�h���ҽX�ܱz��iBunny�@�����աC\n");

	twoFA_GenCode(code, 6);

	setuserfile(buf, "2fa.code");
	if (!(fp = fopen(buf, "w"))){
		mvouts(b_lines - 1, 0 ,"�����\\�}�Ҩ�B�J�{�ҡC");
		vmsg("�t�ο��~�A�еy��A�աC(Error code: 2FA-S-001)");
		return 0;
	}
	fprintf(fp,"%s", code);
	fclose(fp);

	msg = twoFA_Send(user,code);

	if (msg != NULL && strcmp(cuser.cellphone, "")){
		msg = NULL;
		move(7, 0);
        outs("�z�S���n�JiBunny�A�t�ΧY�N�o�e���ҽX�ܱz���U��������X�C\n");
        pressanykey();
		msg = twoFA_sms_Send(user,code,cuser.cellphone);
	}

    if (msg != NULL){
		mvouts(b_lines - 1, 0 ,"�����\\�}�Ҩ�B�J�{�ҡC");
		vmsgf("%s", msg);
		return 0;
	}
	unlink(buf);

    move(8, 0);outs("���ҽX�w�g�o�e�����A���ҽX��6��Ʀr�C\n");

    for (int i = 3; i > 0; i--) {
		if (i < 3) {
			char buf[80];
			move(10, 0);
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "���ҽX���~�A�z�٦� %d �����|�C" ANSI_RESET, i);
			outs(buf);
		}
		code_input[0] = '\0';
		getdata(9, 0, "�п�J���ҽX�G", code_input,
			sizeof(code_input), DOECHO);

		size_t length = strlen(code_input);
		if(length == 6){
			if (!strcmp(code, code_input)){
				pwcuToggleUserFlag(UF_TWOFA_LOGIN);
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
    FILE           *fp;
    const char *msg = NULL;
    char code[7], rev_code[9], code_input[9], buf[200], buf2[200], genbuf[3];
    char *user = u->userid;

    int use_sms = 0;
    if(!strcmp(cuser.cellphone, ""))
    	use_sms = 2;

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
	vs_hdr2(" ��B�J�{�� ", " ��J���ҽX");

	twoFA_GenCode(code, 6);

	setuserfile(buf, "2fa.code");
	if (!(fp = fopen(buf, "w"))){
		move(1,0);
		outs("�t�ο��~�A�z�i�H�ϥδ_��X�εy��A�աC(Error code: 2FA-F-001)");
		getdata(2, 0, "�ϥδ_��X�H (y/N) ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
			return -1;
		}
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
			setuserfile(buf, "2fa.recov");
			if (!(fp = fopen(buf, "r"))){
				outs("�t�ο��~�A�L�k�ϥδ_��X�A�еy��A�աC(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);
			if (!strcmp(rev_code, code_input)){
				unlink(buf);
				move(6, 0);
				outs(ANSI_COLOR(1;33) "�ϥΤF�_��X�{�ҡA�G�_��X�w���ġC" ANSI_RESET);
				return NULL; //Success
			}
		}

		now = time(NULL);
		setuserfile(buf, "2fa.bad");
		log_filef(buf, LOG_CREAT,"%s ��%d����B�J���ҥ��ѡAIP��m�G%s�C\n",Cdate(&now), 4 - i , fromhost);
		log_filef("2fa.bad", LOG_CREAT,"%s %s %s (%d/3)\n",Cdate(&now), cuser.userid, fromhost, 4 - i);
		return -1;
	}
	fprintf(fp,"%s", code);
	fclose(fp);

	msg = twoFA_Send(user,code);

	if(msg != NULL && use_sms == 0){
		use_sms = 1;
		msg = NULL;
		msg = twoFA_sms_Send(user,code,u->cellphone);
	}
   	if(msg != NULL){
		move(1, 0);
		prints("���~�G%s",msg);
		outs("\n  (�p�G�z���������ҽX�A���t�λ~�����~�A��JR�Y�i�i�J���ҵ{�ǡC)");
		if(use_sms == 0)
			getdata(4, 0, "(T)�A�դ@�� (R)�ϥδ_��X (S)��Τ��²�T [C]�����n�J ",genbuf, 3, LCECHO);
		else
			getdata(4, 0, "(T)�A�դ@�� (R)�ϥδ_��X [C]�����n�J ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'r' && genbuf[0] != 't') {
			unlink(buf);
			return -1;
		}
		if (use_sms != 0 && genbuf[0] == 's') {
			unlink(buf);
			return -1;
		}
		if (genbuf[0] == 's') {
			msg = twoFA_sms_Send(user,code,u->cellphone);
		    if (msg != NULL){
				move(1,0); clrtobot();
				prints("���~�G%s",msg);
				getdata(3, 0, "(R)�ϥδ_��X [C]�����n�J ",genbuf, 3, LCECHO);
				if (genbuf[0] != 'r') {
					unlink(buf);
					return -1;
				}
			}
		}
		if (genbuf[0] == 't') {
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

		}
	}
	unlink(buf);

	move(1,0); clrtobot();
    mvouts(2, 0, "���ҽX�N�����Q�o�e��");
    if(use_sms == 1)
    	outs("�z���U��������X");
    else
    	outs("�z�j�w��iBunny");
    outs("\n���ҽX��6��Ʀr�B�_��X��8��^�ƲV�X�C\n");

    if(use_sms == 0)
    	outs("�p�n���²�T���ҡA�п�Js\n");

    for (int i = 3; i > 0; i--) {
    	int skip_log = 0;
		if (i < 3) {
			char buf[80];
			snprintf(buf, sizeof(buf), ANSI_COLOR(1;31) "���ҽX���~�A�z�٦� %d �����|�C" ANSI_RESET, i);
			move(6, 0);
			outs(buf);
		}
		code_input[0] = '\0';
		getdata(5, 0, "�п�J���ҽX�G", code_input,
			sizeof(code_input), DOECHO);

		size_t length = strlen(code_input);
		if(length == 1 && !strcmp("s", code_input)){
			msg = NULL;
			if(use_sms == 0){
				setuserfile(buf, "2fa.code");
				fp = fopen(buf, "w+");
				fprintf(fp,"%s", code);
				fclose(fp);
				use_sms = 1;
				msg = twoFA_sms_Send(user,code,u->cellphone);
				unlink(buf);
				i++;
			}
			move(1,0); clrtobot();
		    mvouts(2, 0, "���ҽX�N�����Q�o�e��");
		    if(use_sms == 1 && msg == NULL)
		    	outs("�z���U��������X");
		    else
		    	outs("�z�j�w��iBunny");
		    outs("\n���ҽX��6��Ʀr�B�_��X��8��^�ƲV�X�C\n");
		    if(use_sms == 0)
		    	outs("�p�n���²�T���ҡA�п�Js\n");
		    if (msg != NULL){
				move(6,0);
				prints("���~�G%s",msg);
			}
			skip_log = 1;
		}else if(length == 6){
			if (!strcmp(code, code_input)){
#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
				clear();
				vs_hdr2(" ��B�J�{�� ", " �]�w���H�����˸m�H");
				move(2, 0);
				outs("�]�w���H�����˸m�U���n�J�N���ݭn���ҡC\n"
					 "���бz�A�Ф��n�b���ιq���W�ϥΦ��\\��C");
				getdata(5, 0, "�O�_�N�o�Ӹ˸m�]�w���H�����˸m(y/n)�H [N]",genbuf, 3, LCECHO);
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
				outs("�t�ο��~�A�L�k�ϥδ_��X�A�еy��A�աC(Error code: 2FA-F-002)");
				return -1;
			}
			fgets(rev_code, sizeof(rev_code), fp);
			fclose(fp);

			if (!strcmp(rev_code, code_input)){
				unlink(buf);
				move(6, 0);
				outs(ANSI_COLOR(1;33) "�ϥΤF�_��X�{�ҡA�G�_��X�w���ġC" ANSI_RESET);
				return NULL; //Success
			}
		}

		if(skip_log = 1){
			now = time(NULL);
			setuserfile(buf, "2fa.bad");
			log_filef(buf, LOG_CREAT,"%s ��%d����B�J���ҥ��ѡAIP��m�G%s�C\n",Cdate(&now), 4 - i , fromhost);
			log_filef("2fa.bad", LOG_CREAT,"%s %s %s (%d/3)\n",Cdate(&now), cuser.userid, fromhost, 4 - i);
		}
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

#endif //USE_2FALOGIN
