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
		return "�t�ο��~�A�z�i�H�ϥδ_��X�εy��A�աC(Error code: 2FA-S-001)";

    	if (code != 200){
		snprintf(buf, sizeof(buf), "�t�ο��~�A�z�i�H�ϥδ_��X�εy��A�աC(Error code: 2FA-S-%3d)", code);
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
		outs("\n  (�p�G�z���������ҽX�A���t�λ~�����~�A��JR�Y�i�i�J���ҵ{�ǡC)");
		getdata(4, 0, "(R)�ϥδ_��X (T)���յo�e [C]�����n�J ",genbuf, 3, LCECHO);
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
    mvouts(2, 0, "���ҽX�N�����Q�o�e��iBunny\n");
    outs("���ҽX��6��Ʀr�B�_��X��8��^�ƲV�X�C\n");

    for (int i = 3; i > 0; i--) {
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
		if(length == 6){
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
		
		now = time(NULL);
		setuserfile(buf, "2fa.bad");
		log_filef(buf, LOG_CREAT,"%s ��%d����B�J���ҥ��ѡAIP��m�G%s�C\n",Cdate(&now), 4 - i , fromhost);
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

	vs_hdr("��B�J�{�Ҵ_��X");
	if(!(HasUserFlag(UF_TWOFA_LOGIN))){
		vmsg("�Х��b�ӤH�]�w�}�Ҩ�B�J�{�ҡC");
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
