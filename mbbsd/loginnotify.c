#include "bbs.h"

#ifdef USE_NOTILOGIN
static const char *
notiLogin_Send(char *user)
{
    int ret, code = 0;
    char uri[320] = "",buf[200];

    snprintf(uri, sizeof(uri), "/%s?user=%s", NOTILOGIN_URI, user);

    THTTP t;
    thttp_init(&t);
    snprintf(buf, sizeof(buf), "Bearer %s", IBUNNY_API_KEY);
    ret = thttp_get(&t, IBUNNY_SERVER, uri, IBUNNY_SERVER, buf);
    if(!ret)
        code = thttp_code(&t);
    thttp_cleanup(&t);

    if(ret)
        return 1;

    if(code == 200)
        return NULL;
    else
        return code;

}

int notiLogin_setting(void)
{
    FILE           *fp;
    const char *msg = NULL;
    char code[7], rev_code[9], code_input[7], buf[200], buf2[200], genbuf[3];
    char *user = cuser.userid;
    char passbuf[PASSLEN];

    clear();
    vs_hdr2(" �n�J�q���T�� ", " �q���]�w");

    move(3,0);
    prints("�]�w�b���G%s\n", user);
    prints("�z�ثe%s�}�ҵn�J�q���T���C\n", ((cuser.uflag & UF_NOTIFY_LOGIN) ? ANSI_COLOR(1) "�w" ANSI_RESET : ANSI_COLOR(1;33) "��" ANSI_RESET));

    if(cuser.uflag & UF_NOTIFY_LOGIN)
        getdata(6, 0, "����(y) �����ާ@[N] ",genbuf, 3, LCECHO);
    else{
        mvouts(6, 0, "���\\��ݷf�t�ϥ�iBunny�A�Х��T�w�z���n�JiBunny�C");
        getdata(7, 0, "�}��(y) �����ާ@[N] ",genbuf, 3, LCECHO);
    }
    if (genbuf[0] != 'y') {
        vmsg("�����ާ@�C");
        return 0;
    }

    move(6,0);clrtobot();
    outs("�H�U�ާ@�ݭn���T�{�z�������C\n");
    getdata(7, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
    passbuf[8] = '\0';
    if (!(checkpasswd(cuser.passwd, passbuf))){
        vmsg("�K�X���~�I");
        return 0;
    }

    if(cuser.uflag & UF_NOTIFY_LOGIN){
        pwcuToggleUserFlag(UF_NOTIFY_LOGIN);
        vmsg("�w�����n�J�q���T���C");
        return 0;
    }

    move(4, 0);clrtobot();
    mvouts(6, 0, "�ڭ̱N�o�e�@�h�T���ܱz��iBunny�@�����աC\n");

    msg = notiLogin_Send(user);

    if (msg){
        if(msg == 400){
            mvouts(b_lines - 1, 0 ,"�����\\�}�ҵn�J�q���T���C");
            vmsg("�z���n�JiBunny�I�Х��n�J�A�ӳ]�w�C(Error code: LNO-S-400)");
            return 0;
        }else if(msg == 401){
            mvouts(b_lines - 1, 0 ,"�����\\�}�ҵn�J�q���T���C");
            vmsg("API�걵�X���A���pô�u�{�~�ȳB��U�C(Error code: LNO-S-401)");
            return 0;
        }else if(msg == 500){
            mvouts(b_lines - 1, 0 ,"�����\\�}�ҵn�J�q���T���C");
            vmsg("���A���X���A���pô�u�{�~�ȳB��U�C(Error code: LNO-S-500)");
            return 0;
        }else{
            mvouts(b_lines - 1, 0 ,"�����\\�}�ҵn�J�q���T���C");
            vmsgf("�t�ο��~�A�еy��A�աC(Error code: LNO-S-%3d)", msg);
            return 0;
        }
    }
    unlink(buf);

    outs("�ڭ̤w�o�e�@�h���հT���ܱz��iBunny�I");

    getdata(9, 0, "�аݱz�O�_�����T����T���H (y)�� [N]�S�� ",genbuf, 3, LCECHO);
    if (genbuf[0] == 'y') {
        pwcuToggleUserFlag(UF_NOTIFY_LOGIN);
        vmsg("�w�}�ҵn�J�q���T���C");
        return 0;
    }

    vmsg("�����\\�}�ҡA�p����o�ͽ��pô�u�{�~�ȳB��U�C(Error code: LNO-S-001)");
    return 0;
}

void
notiLogin_main(char *user){
    const char *msg = NULL;
    msg = notiLogin_Send(user);
    if(msg != NULL){
        time4_t     dtime = time(0);
        LOG_IF(LOG_CONF_POST,
               log_filef("log/notilogin.bad", LOG_CREAT,
                         "time: %d, user: %s, error: %3d \n",
                         (int)(++dtime), user, msg));
    }
}
#endif //USE_NOTILOGIN