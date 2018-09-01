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
		return "�t�ο��~�A�еy��A�աC(Error code: 2FA-S-001)";

    if (code != 200){
		snprintf(buf, sizeof(buf), "�t�ο��~�A�еy��A�աC(Error code: 2FA-S-%3d)", code);
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
	vs_hdr("��B�J�{��");

#ifdef BETA /* �]�����եD���P�����D�����P�x�A���ծɵL�k���`���o���ҽX�C */
	snprintf(code, sizeof(code), "000000");
#else
    twoFA_GenCode(code, 6);
#endif
	
	setuserfile(buf, "2fa.code");
	if (!(fp = fopen(buf, "w"))){
		outs("�t�ο��~�A�еy��A�աC(Error code: 2FA-F-001)");
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
	
    outs("���ҽX�N�����Q�o�e��iBunny\n");
    outs("�@�@������Ʀr\n");

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
			if (!strcmp(code, code_input))
				return NULL; //Success
		}else if(length == 8){
			setuserfile(buf, "2fa.recov");
			if (!(fp = fopen(buf, "r"))){
				outs("�t�ο��~�A�еy��A�աC(Error code: 2FA-F-002)");
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
		log_filef(buf, LOG_CREAT,"%s ��%d����B�J���ҥ��ѡAIP��m�G%s�C\n",Cdate(&now), 3 - i , fromhost);
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
	getdata(6, 0, MSG_PASSWD, passbuf, sizeof(passbuf), NOECHO);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("�K�X���~�I");
		return 0;
	}
	
	twoFA_GenCode(rev_code, 8);
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
	outs(ANSI_COLOR(1;33) "�бz�O�U�_��X�ç����O�ޡA���}��������N����A���s�d�ߡC" ANSI_RESET);
	
	pressanykey();
	return 0;
}

#endif //USE_IBUNNY_2FALOGIN