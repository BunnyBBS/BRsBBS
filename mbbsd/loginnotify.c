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
    vs_hdr2(" 登入通知訊息 ", " 通知設定");

    move(3,0);
    prints("設定帳號：%s\n", user);
    prints("您目前%s開啟登入通知訊息。\n", ((cuser.uflag & UF_NOTIFY_LOGIN) ? ANSI_COLOR(1) "已" ANSI_RESET : ANSI_COLOR(1;33) "未" ANSI_RESET));

    if(cuser.uflag & UF_NOTIFY_LOGIN)
        getdata(6, 0, "關閉(y) 取消操作[N] ",genbuf, 3, LCECHO);
    else{
        mvouts(6, 0, "本功\能需搭配使用iBunny，請先確定您有登入iBunny。");
        getdata(7, 0, "開啟(y) 取消操作[N] ",genbuf, 3, LCECHO);
    }
    if (genbuf[0] != 'y') {
        vmsg("取消操作。");
        return 0;
    }

    move(6,0);clrtobot();
    outs("以下操作需要先確認您的身份。\n");
    getdata(7, 0, MSG_PASSWD, passbuf, PASS_INPUT_LEN + 1, PASSECHO);
    passbuf[8] = '\0';
    if (!(checkpasswd(cuser.passwd, passbuf))){
        vmsg("密碼錯誤！");
        return 0;
    }

    if(cuser.uflag & UF_NOTIFY_LOGIN){
        pwcuToggleUserFlag(UF_NOTIFY_LOGIN);
        vmsg("已關閉登入通知訊息。");
        return 0;
    }

    move(4, 0);clrtobot();
    mvouts(6, 0, "我們將發送一則訊息至您的iBunny作為測試。\n");

    msg = notiLogin_Send(user);

    if (msg){
        if(msg == 400){
            mvouts(b_lines - 1, 0 ,"未成功\開啟登入通知訊息。");
            vmsg("您未登入iBunny！請先登入再來設定。(Error code: LNO-S-400)");
            return 0;
        }else if(msg == 401){
            mvouts(b_lines - 1, 0 ,"未成功\開啟登入通知訊息。");
            vmsg("API串接出錯，請聯繫工程業務處協助。(Error code: LNO-S-401)");
            return 0;
        }else if(msg == 500){
            mvouts(b_lines - 1, 0 ,"未成功\開啟登入通知訊息。");
            vmsg("伺服器出錯，請聯繫工程業務處協助。(Error code: LNO-S-500)");
            return 0;
        }else{
            mvouts(b_lines - 1, 0 ,"未成功\開啟登入通知訊息。");
            vmsgf("系統錯誤，請稍後再試。(Error code: LNO-S-%3d)", msg);
            return 0;
        }
    }
    unlink(buf);

    outs("我們已發送一則測試訊息至您的iBunny！");

    getdata(9, 0, "請問您是否有正確收到訊息？ (y)有 [N]沒有 ",genbuf, 3, LCECHO);
    if (genbuf[0] == 'y') {
        pwcuToggleUserFlag(UF_NOTIFY_LOGIN);
        vmsg("已開啟登入通知訊息。");
        return 0;
    }

    vmsg("未成功\開啟，如持續發生請聯繫工程業務處協助。(Error code: LNO-S-001)");
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