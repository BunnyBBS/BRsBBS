#include "bbs.h"

int
x_boardman(void)
{
    more("etc/topboardman", YEA);
    return 0;
}

int
x_user100(void)
{
    more("etc/topusr100", YEA);
    return 0;
}

int
x_history(void)
{
    more("etc/history", YEA);
    return 0;
}

int
x_issue(void)
{
    more("etc/day", YEA);
    return 0;
}

int
x_week(void)
{
    more("etc/week", YEA);
    return 0;
}

int
x_today(void)
{
    more("etc/today", YEA);
    return 0;
}

int
x_yesterday(void)
{
    more("etc/yesterday", YEA);
    return 0;
}

int
x_login(void)
{
    more("etc/Welcome_login.0", YEA);
    return 0;
}

int
Goodbye(void)
{
    char            genbuf[STRLEN];

    getdata(b_lines - 1, 0, "�z�T�w�n���}�i " BBSNAME " �j��(Y/N)�H[N] ",
	    genbuf, 3, LCECHO);

    if (*genbuf != 'y')
	return 0;

    adbanner_goodbye();
    show_80x24_screen("etc/Logout");
    {
	int diff = (now - login_start_time) / 60;
	snprintf(genbuf, sizeof(genbuf), "�������d�ɶ�: %d �p�� %2d ��",
		diff / 60, diff % 60);
    }
    if(!HasUserPerm(PERM_LOGINOK))
	vmsg("�|���������U�{�ǡC");
    else
	vmsg(genbuf);

    STATINC(STAT_MBBSD_EXIT);
    u_exit("EXIT ");
    return QUIT;
}
