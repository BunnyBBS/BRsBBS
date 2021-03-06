#include "bbs.h"

#ifdef USE_BBS2WEB

int web_sync_board(int bid, const boardheader_t *board, char *type)
{
	int ret, code = 0;
	char uri[400] = "",buf[200];

	if(type != "NEW" && type != "SYNC")
		return 2;

	/* �O�Цb�o�̦P�B�|�X���A�Ȯɤ��]�p�b�o�̦P�B */
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
	vs_hdr2(" �����A�� ", " ���U�b��");

    char passbuf[PASSLEN], buf2[8];
    move(3, 0);
    prints("�]�w�b���G%s\n", cuser.userid);
	outs("�Ъ`�N�A���\\��ȭ��z�����}�q�����b���ɨϥΡC\n");
	outs("�p�G�z���g�}�q�����b���]�t108�~7��e�µ��U�{�ǡ^�A�Ф��n�A�ϥΥ��\\��C\n");
	outs("�p�G�z�ѰO�����K�X�A�i�ϥέ��]�K�X�\\��N�����K�X���s�]�w��BBS�n�J�K�X�C\n");
	outs("�H�U�ާ@�ݭn���T�{�z�������C\n");
	getdata(9, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	snprintf(buf2, sizeof(buf2), "%s", passbuf);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("�K�X���~�I");
		return 0;
	}

	if(!strcmp(cuser.email, "x")){
		vmsg("�z�S���]�w�q�l�H�c�A�Х��h�]�w�A�ӡC");
		return 0;
	}

	outs("\n�еy��A���U���K\n");

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
		vmsg("�t�ο��~�A�еy��A�աC(Error code: WUR-R-001)");
		return 0;
	}

    if(code == 200){
		move(1,0); clrtobot();
		mvouts(10, 0, "���U��\�\\�I�֨�https://www.bunnybbs.tw/login�n�J�a�I\n�]�b���αK�X�PBBS�ۦP�^");
		pressanykey();
		return 0;
    }else{
		vmsgf("�t�ο��~�A�еy��A�աC(Error code: WUR-R-%03d)",code);
		return 0;
	}

	return 0;
}

int
web_user_resetpass(void)
{
	clear();
	vs_hdr2(" �����A�� ", " ���]�K�X");

    char passbuf[PASSLEN], buf2[8];
    move(3, 0);
    prints("�]�w�b���G%s\n", cuser.userid);
	outs("�ϥΦ��\\��|�N���]�A�������b���K�X�C\n");
	outs("�H�U�ާ@�ݭn���T�{�z�������A�п�JBBS���K�X�C\n");
	getdata(6, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	snprintf(buf2, sizeof(buf2), "%s", passbuf);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("�K�X���~�I");
		return 0;
	}

	outs("\n�еy��K\n");

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
		vmsg("�t�ο��~�A�еy��A�աC(Error code: WUP-R-001)");
		return 0;
	}

    if(code == 200){
		move(1,0); clrtobot();
		mvouts(10, 0, "���]��\�\\�I�A�������K�X�w�g���]�PBBS�ۦP�C");
		pressanykey();
		return 0;
    }else{
		vmsgf("�t�ο��~�A�еy��A�աC(Error code: WUP-R-%03d)",code);
		return 0;
	}

	return 0;
}

int
web_user_lock(void)
{
	clear();
	vs_hdr2(" �����A�� ", " ��w�b��");

    char passbuf[PASSLEN], buf2[8], genbuf[3];
    move(4, 0);
    prints("�]�w�b���G%s\n", cuser.userid);
	outs("�H�U�ާ@�ݭn���T�{�z�������C\n");
	getdata(6, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
	snprintf(buf2, sizeof(buf2), "%s", passbuf);
	passbuf[8] = '\0';
	if (!(checkpasswd(cuser.passwd, passbuf))){
		vmsg("�K�X���~�I");
		return 0;
	}

	mvouts(9, 0, "��w��A���b���N����A�s�������A�ȡA����ݬ��u�{�~�ȳB�C");
	getdata(10, 0, "�T�w�n��w�����b���F�ܡH (y/N)",genbuf, 3, LCECHO);
	if (genbuf[0] != 'y') {
		vmsg("�����ާ@�C");
		return 0;
	}

	outs("\n�еy��K\n");

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
		vmsg("�t�ο��~�A�еy��A�աC(Error code: WUL-L-001)");
		return 0;
	}

    if(code == 200){
		move(1,0); clrtobot();
		mvouts(10, 0, "�w��w�A�������b���A�p���ݸ���Ь��u�{�~�ȳB�t�Τu�{�B�C");
		pressanykey();
		return 0;
    }else{
		vmsgf("�t�ο��~�A�еy��A�աC(Error code: WUL-L-%03d)",code);
		return 0;
	}

	return 0;
}

#endif //USE_BBS2WEB