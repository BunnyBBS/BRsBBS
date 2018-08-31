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
		return "���A���s�u���ѡA�еy��A�աA����o�ͽЦ^����BUG�C";

    if (code != 200){
		snprintf(buf, sizeof(buf), "���A���^��%d�A�еy��A�աA����o�ͽЦ^����BUG�C", code);
		return buf;
	}
	
	return NULL;
}

int twoFA_main(char *user)
{
    FILE           *fp;
    const char *msg = NULL;
    char code[7], rev_code[9], code_input[9], buf[200];

    vs_hdr("��B�J����");
    move(1, 0);

    twoFA_GenCode(code, 6);
	
	snprintf(buf, sizeof(buf), "home/%c/%s/2fa.code", user[0], user);
	if (!(fp = fopen(buf, "w"))){
		outs("�ɮרt�ο��~�A�Ц^����BUG�C");
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

    outs("�{�ҽX�N�����Q�o�e��iBunny\n");
    outs("�@�@������Ʀr\n");

    for (int i = 3; i > 0; i--) {
	if (i < 3) {
	    char buf[80];
	    snprintf(buf, sizeof(buf), ANSI_COLOR(1;31)
		     "���ҽX���~�A�z�٦� %d �����|�C" ANSI_RESET, i);
	    move(6, 0);
	    outs(buf);
	}
	code_input[0] = '\0';
	getdata(5, 0, "�п�J���ҽX�G", code_input,
		sizeof(code_input), DOECHO);
	size_t length = strlen(code_input);
	if(length == 6){
	if (!strcmp(code, code_input))
	    return NULL;
	}else if(length == 8){
	snprintf(buf, sizeof(buf), "home/%c/%s/2fa.recov", user[0], user);
	if (!(fp = fopen(buf, "r"))){
		outs("�S���_��X�ɮסA�z�O���O�S�]�w�C");
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

	vs_hdr("��B�J���Ҵ_��X");
	if(!(HasUserFlag(UF_TWOFA_LOGIN))){
		vmsg("�Х��b�ӤH�]�w�}�Ҩ�B�J���ҡC");
		return 0;
	}
	
	snprintf(buf, sizeof(buf), "home/%c/%s/2fa.recov", user[0], user);
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
    outs("�_��X�O��z�L�k�ϥΨ�B�J���Ү�\n");
    outs("�i�H�b��J���ҽX�ɿ�J�_��X���^�b��s���v\n");
	
	twoFA_GenCode(rev_code, 8);
	if (!(fp = fopen(buf, "w"))){
		vmsg("�ɮרt�ο��~�A�Ц^����BUG�C");
		return 0;
	}
	fprintf(fp,"%s", rev_code);
	fclose(fp);
	
	move(10,0);
	outs("�z���_��X�O�G");
	outs(rev_code);
	outs("\n\n");
	outs("�бz�O�U�_��X�ç����O�ޡA���}��������N����A���s�d�ߡC");
	
	pressanykey();
}

