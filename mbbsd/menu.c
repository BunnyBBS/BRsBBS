#include "bbs.h"

// UNREGONLY �אּ�� BASIC �ӧP�_�O�_�� guest.

#define CheckMenuPerm(x) \
    ( (x == MENU_UNREGONLY)? \
      ((!HasUserPerm(PERM_BASIC) || HasUserPerm(PERM_LOGINOK))?0:1) :\
	((!x) ? 1 :  \
         ((x & PERM_LOGINOK) ? HasBasicUserPerm(x) : HasUserPerm(x))))

/* help & menu processring */
static int      refscreen = NA;
extern char    *boardprefix;
extern struct utmpfile_t *utmpshm;

static const char *title_tail_msgs[] = {
    "�ݪO",
    "�t�C",
    "��K",
};
static const char *title_tail_attrs[] = {
    ANSI_COLOR(37),
    ANSI_COLOR(32),
    ANSI_COLOR(36),
};
enum {
    TITLE_TAIL_BOARD = 0,
    TITLE_TAIL_SELECT,
    TITLE_TAIL_DIGEST,
};

// �ѩ���v�]���A�o�̷|�X�{�T�ؽs��:
// MODE (�w�q�� modes.h)    �O BBS ��U�إ\��b utmp ���s�� (var.c �n�[�r��)
// Menu Index (M_*)	    �O menu.c ����������n�������� mode �� index
// AdBanner Index	    �O�ʺA�ݪ��n��ܤ��򪺭�
// �q�e�o�O�Ψ�� mode map ���ഫ�� (�O�H�ݱo���Y����)
// ����� Menu Index �� AdBanner Index �X�@�A�Ш��U��������
///////////////////////////////////////////////////////////////////////
// AdBanner (SHM->notes) �e�X���O Note �O��ذϡu<�t��> �ʺA�ݪO�v(SYS)
// �ؿ��U���峹�A�ҥH�s�� Menu (M_*) �ɭn�Ө䶶�ǡG
// ��ذϽs��     => Menu Index => MODE
// (AdBannerIndex)
// ====================================
// 00�����e��     =>  M_GOODBYE
// 01�D���       =>  M_MMENU   => MMENU
// 02�t�κ��@��   =>  M_ADMIN   => ADMIN
// 03�p�H�H���   =>  M_MAIL    => MAIL
// 04�𶢲�Ѱ�   =>  M_TMENU   => TMENU
// 05�ӤH�]�w��   =>  M_UMENU   => UMENU
// 06�t�Τu���   =>  M_XMENU   => XMENU
// 07�T�ֻP��   =>  M_PMENU   => PMENU
// 08��tt�j�M��   =>  M_SREG    => SREG
// 09��tt�q�c��   =>  M_PSALE   => PSALE
// 10��tt�C�ֳ�   =>  M_AMUSE   => AMUSE
// 11��tt�Ѱ|     =>  M_CHC     => CHC
// 12�S�O�W��     =>  M_NMENU   => NMENU
///////////////////////////////////////////////////////////////////////
// �ѩ� MODE �P menu �����ǲ{�b�w���@�P (�̦��i��O�@�P��)�A�ҥH������
// �ഫ�O�a menu_mode_map �ӳB�z�C
// �n�w�q�s Menu �ɡA�Цb M_MENU_MAX ���e�[�J�s�ȡA�æb menu_mode_map
// �[�J������ MODE �ȡC �t�~�A�b Notes �U�]�n�W�[������ AdBanner �Ϥ�
// �Y���Q�[�Ϥ��h�n�ק� N_SYSADBANNER
///////////////////////////////////////////////////////////////////////

enum {
    M_GOODBYE=0,
    M_MMENU,	 M_ADMIN, M_MAIL, M_TMENU,
    M_UMENU,     M_XMENU, M_PMENU,M_SREG,
    M_PSALE,	 M_AMUSE, M_CHC,  M_NMENU,

    M_MENU_MAX,			// �o�O menu (M_*) ���̤j��
    N_SYSADBANNER = M_MENU_MAX, // �w�q M_* ��h�֦������� ADBANNER
    M_MENU_REFRESH= -1,		// �t�ΥΤ��쪺 index �� (�i��ܨ䥦���ʻP�I�q)
};

static const int menu_mode_map[M_MENU_MAX] = {
    0,
    MMENU,	ADMIN,	MAIL,	TMENU,
    UMENU,	XMENU,	PMENU,	SREG,
    PSALE,	AMUSE,	CHC,	NMENU
};

typedef struct {
    int     (*cmdfunc)();
    int     level;
    char    *desc;                   /* hotkey/description */
} commands_t;

///////////////////////////////////////////////////////////////////////

void
showtitle(const char *title, const char *mid)
{
    /* we have to...
     * - display title in left, cannot truncate.
     * - display mid message, cannot truncate
     * - display tail (board info), if possible.
     */
    int llen, rlen, mlen, mpos = 0;
    int pos = 0;
    int tail_type;
    const char *mid_attr = ANSI_COLOR(33);
    int is_currboard_special = 0;
    char buf[64];


    /* prepare mid */
#ifdef DEBUG
    {
	if(!(HasUserPerm(PERM_SYSOP))) {
		printf(ANSI_RESET "�t�ζi���˭׺��@�A�D�����T��i�J�I");
		sleep(1);
		abort_bbs(0);
	}
	sprintf(buf, "  current pid: %6d  ", getpid());
	mid = buf;
	mid_attr = ANSI_COLOR(41;5);
    }
#elif defined BETA
	/*�j�ߡG��t�ιB�Ω���վ��ɡA�sĶ�ɥ[�WBETA=1���ѼơA�åB�b�A��B�[�W�i�ѧO����r�A�קK���p�ߦb���վާ@�ɻ~�ʨ쥿���t��*/
	{
	mid = "      ���ժ�      ";
	mid_attr = ANSI_COLOR(41;5);
	}
#else
    if (ISNEWMAIL(currutmp)) {
		mid = "    �s�H����o    ";
		mid_attr = ANSI_COLOR(41;5);
    }
#endif

    /* prepare tail */
    if (currmode & MODE_SELECT)
	tail_type = TITLE_TAIL_SELECT;
    else if (currmode & MODE_DIGEST)
	tail_type = TITLE_TAIL_DIGEST;
    else
	tail_type = TITLE_TAIL_BOARD;

    if(currbid > 0)
    {
	assert(0<=currbid-1 && currbid-1<MAX_BOARD);
	is_currboard_special = (
		(getbcache(currbid)->brdattr & BRD_HIDE) &&
		(getbcache(currbid)->brdattr & BRD_POSTMASK));
    }

    /* now, calculate real positioning info */
    llen = strlen(title);
    mlen = strlen(mid);
    mpos = (t_columns -1 - mlen)/2;

    /* first, print left. */
    clear();
    outs(TITLE_COLOR "�i");
    outs(title);
    outs("�j");
    pos = llen + 4;

    /* print mid */
    while(pos++ < mpos)
	outc(' ');
    outs(mid_attr);
    outs(mid);
    pos += mlen;
    outs(TITLE_COLOR);

    /* try to locate right */
    rlen = strlen(currboard) + 4 + 4;
    if(currboard[0] && pos+rlen < t_columns)
    {
	// print right stuff
	while(pos++ < t_columns-rlen)
	    outc(' ');
	outs(title_tail_attrs[tail_type]);
	outs(title_tail_msgs[tail_type]);
	outs("�m");

	if (is_currboard_special)
	    outs(ANSI_COLOR(32));
	outs(currboard);
	outs(title_tail_attrs[tail_type]);
	outs("�n" ANSI_RESET "\n");
    } else {
	// just pad it.
	while(pos++ < t_columns)
	    outc(' ');
	outs(ANSI_RESET "\n");
    }

}

int TopBoards(void);

/* Ctrl-Z Anywhere Fast Switch, not ZG. */
static char zacmd = 0;

// ZA is waiting, hurry to the meeting stone!
int
ZA_Waiting(void)
{
    return (zacmd != 0);
}

void
ZA_Drop(void)
{
    zacmd = 0;
}

// Promp user our ZA bar and return for selection.
int
ZA_Select(void)
{
    int k;
    struct tm      ptime;

    if (!is_login_ready ||
        !HasUserPerm(PERM_BASIC) ||
        HasUserPerm(PERM_VIOLATELAW))
        return 0;

    localtime4_r(&now, &ptime);
    // TODO refresh status bar?
	move(b_lines-2, 0); clrtobot();
    vbarf(ANSI_COLOR(1;33;42)"\n  �ֳt��� "ANSI_COLOR(0;30;42)"|"ANSI_COLOR(1;37;42)" %02d"ANSI_COLOR(1;5;37;42)":"ANSI_COLOR(0;1;37;42)"%02d  \t ",ptime.tm_hour, ptime.tm_min);
	vbarf(ANSI_COLOR(0;31;47)"\n  (b)"ANSI_COLOR(0;30;47)"�峹�C��" ANSI_COLOR(0;31;47)" (c)"ANSI_COLOR(0;30;47)"���� " ANSI_COLOR(0;31;47)"(f)"ANSI_COLOR(0;30;47)"�ڪ��̷R " ANSI_COLOR(0;31;47)"(m)"ANSI_COLOR(0;30;47)"�H�c " ANSI_COLOR(0;31;47)"(u)"ANSI_COLOR(0;30;47)"�ϥΪ̦W�� \t" ANSI_COLOR(0;31;47)"(x)"ANSI_COLOR(0;30;47)"�������  "ANSI_RESET);
    k = vkey();

    if (k < ' ' || k >= 'z') return 0;
    k = tolower(k);

    if(strchr("bcfmut", k) == NULL)
	return 0;

    zacmd = k;
    return 1;
}

// The ZA processor, only invoked in menu.
void
ZA_Enter(void)
{
    char cmd = zacmd;
    while (zacmd)
    {
	cmd = zacmd;
	zacmd = 0;

	// All ZA applets must check ZA_Waiting() at every stack of event loop.
	switch(cmd) {
	    case 'b':
		Read();
		break;
	    case 'c':
		Class();
		break;
	    case 'f':
		Favorite();
		break;
	    case 'm':
		if (HasUserPerm(PERM_LOGINOK)){
			m_read();
		}
		break;
	    case 'u':
		if (HasUserPerm(PERM_LOGINOK)){
			t_users();
		}
		break;
	}
	// if user exit with new ZA assignment,
	// direct enter in next loop.
    }
}

/* �ʵe�B�z */
#define FILMROW 11
static unsigned short menu_row = 12;
static unsigned short menu_column = 20;

#ifdef EXP_ALERT_ADBANNER_USONG
static int
decide_menu_row(const commands_t *p) {
    if ((p[0].level && !HasUserPerm(p[0].level)) &&
        HasUserFlag(UF_ADBANNER_USONG) &&
        HasUserFlag(UF_ADBANNER)) {
        return menu_row + 1;
    }

    return menu_row;
}
#else
# define decide_menu_row(x) (menu_row)
#endif

static void
show_status(void)
{
    int i;
    struct tm      ptime;
    char           *myweek = "��@�G�T�|����";
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
#ifdef USE_TIANGANDIZHI
	char	*tian[]={"��","��","�A","��","�B","��","�v","��","��","��"};
	char	*di[]={"��","�l","��","�G","�f","��","�x","��","��","��","��","��"};
	int t = (ptime.tm_year + 1897) % 10;
	int d = (ptime.tm_year + 1897) % 12;
#endif
	char	*greeting[]={"���w","�Ȧw","�ߦw"};
	int g = 0;
	if(ptime.tm_hour >= 0 && ptime.tm_hour < 12)
		g = 0;
	if(ptime.tm_hour >= 12 && ptime.tm_hour < 19)
		g = 1;
	if(ptime.tm_hour >= 19 && ptime.tm_hour < 24)
		g = 2;

    move(b_lines, 0);
    vbarf(ANSI_COLOR(1;33;45) " "
#ifdef USE_MINGGUO_CALENDAR
		  "����%03d�~"
#ifdef USE_TIANGANDIZHI
		  " ����%s%s "
#endif //USE_TIANGANDIZHI
#else //USE_MINGGUO_CALENDAR
		  "�褸%04d�~"	  
#endif //USE_MINGGUO_CALENDAR
		  "%02d��%02d�� �g%c%c "
	  ANSI_COLOR(0;30;47) "  " ANSI_COLOR(1;31;47) "%s" ANSI_COLOR(0;30;47) " %s�I"
	  ANSI_COLOR(1;31;47) "\t%d" ANSI_COLOR(0;30;47) " �H�b�u "ANSI_RESET,
#ifdef USE_MINGGUO_CALENDAR
	  ptime.tm_year - 11,
#ifdef USE_TIANGANDIZHI
	  tian[t], di[d],
#endif //USE_TIANGANDIZHI
#else //USE_MINGGUO_CALENDAR
	  ptime.tm_year + 1900,  
#endif //USE_MINGGUO_CALENDAR
	  ptime.tm_mon + 1, ptime.tm_mday, myweek[i], myweek[i + 1],
	  cuser.userid, greeting[g], SHM->UTMPnumber);
}

/*
 * current caller of adbanner:
 *   xyz.c:   adbanner_goodbye();   // logout
 *   menu.c:  adbanner(cmdmode);    // ...
 *   board.c: adbanner(0);	    // ����ܦb board.c �̦ۤv�B�z(���ӬO������)
 */

void
adbanner_goodbye()
{
    adbanner(M_GOODBYE);
}

void
adbanner(int menu_index)
{
    int i = menu_index;

    // don't show if stat in class or user wants to skip adbanners
    if (currstat == CLASS || !(HasUserFlag(UF_ADBANNER)))
	return;

    // also prevent SHM busy status
    if (SHM->Pbusystate || SHM->last_film <= 0)
	return;

    if (    i != M_MENU_REFRESH &&
	    i >= 0		&&
	    i <  N_SYSADBANNER  &&
	    i <= SHM->last_film)
    {
	// use system menu - i
    } else {
	// To display ADBANNERs in slide show mode.
	// Since menu is updated per hour, the total presentation time
	// should be less than one hour. 60*60/MAX_ADBANNER[500]=7 (seconds).
	// @ Note: 60 * 60 / MAX_ADBANNER =3600/MAX_ADBANNER = "how many seconds
	// can one ADBANNER to display" to slide through every banners in one hour.
	// @ now / (3600 / MAx_ADBANNER) means "get the index of which to show".
	// syncnow();

	const int slideshow_duration = 3600 / MAX_ADBANNER,
		  slideshow_index    = now  / slideshow_duration;

	// index range: 0 =>[system] => N_SYSADBANNER    => [user esong] =>
	//              last_usong   => [advertisements] => last_film
	int valid_usong_range = (SHM->last_usong > N_SYSADBANNER &&
				 SHM->last_usong < SHM->last_film);

	if (SHM->last_film > N_SYSADBANNER) {
	    if (HasUserFlag(UF_ADBANNER_USONG) || !valid_usong_range)
		i = N_SYSADBANNER +       slideshow_index % (SHM->last_film+1-N_SYSADBANNER);
	    else
		i = SHM->last_usong + 1 + slideshow_index % (SHM->last_film - SHM->last_usong);
	}
	else
	    i = 0; // SHM->last_film;
    }

    // make it safe!
    i %= MAX_ADBANNER;

    move(1, 0);
    clrtoln(1 + FILMROW);	/* �M���W���� */
#ifdef LARGETERM_CENTER_MENU
    out_lines(SHM->notes[i], 11, (t_columns - 80)/2);	/* �u�L11��N�n */
#else
    out_lines(SHM->notes[i], 11, 0);	/* �u�L11��N�n */
#endif
    outs(ANSI_RESET);
#ifdef DEBUG
    // XXX piaip test
    move(FILMROW, 0); prints(" [ %d ] ", i);
#endif
}

static int
show_menu(int menu_index, const commands_t * p)
{
    register int    n = 0;
    register char  *s;
    int row = menu_row;

    adbanner(menu_index);

    // seems not everyone likes the menu in center.
#ifdef LARGETERM_CENTER_MENU
    // update menu column [fixed const because most items are designed as fixed)
    menu_column = (t_columns-40)/2;
    row = 12 + (t_lines-24)/2;
#endif

#ifdef EXP_ALERT_ADBANNER_USONG
    if ((p[0].level && !HasUserPerm(p[0].level)) &&
        HasUserFlag(UF_ADBANNER_USONG) &&
        HasUserFlag(UF_ADBANNER)) {
        // we have one more extra line to display ADBANNER_USONG!
        int alert_column = menu_column;
        move(row, 0);
        vpad(t_columns-2, "�w");
        if (alert_column > 2)
            alert_column -= 2;
        alert_column -= alert_column % 2;
        move(row++, alert_column);
        outs(" �W�謰�ϥΪ̤߱��I���d���ϡA���N�����߳� ");
    }
    assert(row == decide_menu_row(p));
#endif

    move(row, 0);
    while ((s = p[n].desc)) {
	if (CheckMenuPerm(p[n].level)) {
            prints("%*s  (%s%c" ANSI_RESET ")%s\n",
                   menu_column, "",
                   (HasUserFlag(UF_MENU_LIGHTBAR) ? ANSI_COLOR(36) :
                    ANSI_COLOR(1;36)), s[0], s+1);
	}
	n++;
    }
    return n - 1;
}

static void
domenu(int menu_index, const char *cmdtitle, int cmd, const commands_t cmdtable[])
{
    int             lastcmdptr, cmdmode;
    int             n, pos, total, i;
    int             err;

    assert(0 <= menu_index && menu_index < M_MENU_MAX);
    cmdmode = menu_mode_map[menu_index];

    setutmpmode(cmdmode);
    showtitle(cmdtitle, BBSName);
    total = show_menu(menu_index, cmdtable);

    show_status();
    lastcmdptr = pos = 0;

    do {
	i = -1;
	switch (cmd) {
	case Ctrl('Z'):
	    ZA_Select(); // we'll have za loop later.
	    refscreen = YEA;
	    i = lastcmdptr;
	    break;
	case Ctrl('N'):
	    New();
	    refscreen = YEA;
	    i = lastcmdptr;
	    break;
	case Ctrl('A'):
	    if (mail_man() == FULLUPDATE)
		refscreen = YEA;
	    i = lastcmdptr;
	    break;
	case KEY_DOWN:
	    i = lastcmdptr;
	case KEY_HOME:
	case KEY_PGUP:
	    do {
		if (++i > total)
		    i = 0;
	    } while (!CheckMenuPerm(cmdtable[i].level));
	    break;
	case KEY_UP:
	    i = lastcmdptr;
	case KEY_END:
	case KEY_PGDN:
	    do {
		if (--i < 0)
		    i = total;
	    } while (!CheckMenuPerm(cmdtable[i].level));
	    break;
	case KEY_LEFT:
	case 'e':
	case 'E':
	    if (cmdmode == MMENU)
		cmd = 'G';	    // to exit
	    else if ((cmdmode == MAIL) && chkmailbox())
		cmd = 'R';	    // force keep reading mail
	    else
		return;
	default:
	    if ((cmd == 's' || cmd == 'r') &&
		(cmdmode == MMENU || cmdmode == TMENU || cmdmode == XMENU)) {
		if (cmd == 's')
		    ReadSelect();
		else
		    Read();
		refscreen = YEA;
		i = lastcmdptr;
		currstat = cmdmode;
		break;
	    }
	    if (cmd == KEY_ENTER || cmd == KEY_RIGHT) {
		move(b_lines, 0);
		clrtoeol();

		currstat = XMODE;

		if ((err = (*cmdtable[lastcmdptr].cmdfunc) ()) == QUIT)
		    return;
		currutmp->mode = currstat = cmdmode;

		if (err == XEASY) {
		    refresh();
		    safe_sleep(1);
		} else if (err != XEASY + 1 || err == FULLUPDATE)
		    refscreen = YEA;

                // keep current position
		i = lastcmdptr;
                break;
	    }

	    if (cmd >= 'a' && cmd <= 'z')
		cmd = toupper(cmd);
	    while (++i <= total && cmdtable[i].desc)
		if (cmdtable[i].desc[0] == cmd)
		    break;

	    if (!CheckMenuPerm(cmdtable[i].level)) {
		for (i = 0; cmdtable[i].cmdfunc; i++)
		    if (CheckMenuPerm(cmdtable[i].level))
			break;
		if (!cmdtable[i].cmdfunc)
		    return;
	    }

	    if (cmd == 'H' && i > total){
		/* TODO: Add menu help */
	    }
	}

	// end of all commands
	if (ZA_Waiting())
	{
	    ZA_Enter();
	    refscreen = 1;
	    currstat = cmdmode;
	}

	if (i > total || !CheckMenuPerm(cmdtable[i].level))
	    continue;

	if (refscreen) {
	    showtitle(cmdtitle, BBSName);
	    // menu �]�w M_MENU_REFRESH �i�� ADBanner ��ܧO����T
	    show_menu(M_MENU_REFRESH, cmdtable);
	    show_status();
	    refscreen = NA;
	}
	cursor_clear(decide_menu_row(cmdtable) + pos, menu_column);
	n = pos = -1;
	while (++n <= (lastcmdptr = i))
	    if (CheckMenuPerm(cmdtable[n].level))
		pos++;

        // If we want to replace cursor_show by cursor_key, it must be inside
        // while(expr) othrewise calling 'continue' inside for-loop won't wait
        // for key.
	cursor_show(decide_menu_row(cmdtable) + pos, menu_column);
    } while (((cmd = vkey()) != EOF) || refscreen);

    abort_bbs(0);
}
/* INDENT OFF */

static int
view_user_money_log() {
    char userid[IDLEN+1];
    char fpath[PATHLEN];

    vs_hdr("�˵��ϥΪ̥���O��");
    usercomplete("�п�J�n�˵���ID: ", userid);
    if (!is_validuserid(userid))
        return 0;
    sethomefile(fpath, userid, FN_RECENTPAY);
    if (more(fpath, YEA) < 0)
        vmsgf("�ϥΪ� %s �L�̪����O��", userid);
    return 0;
}

static int
view_user_login_log() {
    char userid[IDLEN+1];
    char fpath[PATHLEN];

    vs_hdr("�˵��ϥΪ̳̪�W�u�O��");
    usercomplete("�п�J�n�˵���ID: ", userid);
    if (!is_validuserid(userid))
        return 0;
    sethomefile(fpath, userid, FN_RECENTLOGIN);
    if (more(fpath, YEA) < 0)
        vmsgf("�ϥΪ� %s �L�̪�W�u�O��", userid);
    return 0;
}

static int deprecate_userlist() {
    vs_hdr2(" " BBSNAME " ", " �w���ܨϥΪ̦W��");
    outs("\n"
         "���\\��w���ܨϥΪ̦W��ϡC\n"
         "�ЦܨϥΪ̦W�� (Ctrl-U) �ë��U��������C\n"
         "(�b�ϥΪ̦W��� h �|�����㻡��)\n\n"
         "�����I�s��:     Ctrl-U p\n"
         "�����N:         Ctrl-U C\n"
         "��ܤW�X�����T: Ctrl-U l\n");
    pressanykey();
    return 0;
}

void Customize(); // user.c
static int
u_customize()
{
	Customize();
	return 0;
}

static int u_view_recentlogin()
{
	char fn[PATHLEN];
	setuserfile(fn, FN_RECENTLOGIN);
	return more(fn, YEA);
}

#ifdef USE_RECENTPAY
static int u_view_recentpay()
{
	char fn[PATHLEN];
	clear();
	mvouts(10, 5, "�`�N: ���B���e�ȨѰѦҡA���" MONEYNAME
						"���ʥH���褺����Ƭ���");
	pressanykey();
	setuserfile(fn, FN_RECENTPAY);
	return more(fn, YEA);
}
#endif

static int t_aloha() {
	friend_edit(FRIEND_ALOHA);
	return 0;
}

static int t_special() {
	friend_edit(FRIEND_SPECIAL);
	return 0;
}

#ifdef HAVE_USERAGREEMENT
static int
x_agreement(void)
{
	more(HAVE_USERAGREEMENT, YEA);
	return 0;
}
#endif

#ifdef HAVE_INFO
static int
x_program(void)
{
	more("etc/version", YEA);
	return 0;
}
#endif

#ifdef HAVE_LICENSE
static int
x_gpl(void)
{
	more("etc/GPL", YEA);
	return 0;
}
#endif

#ifdef HAVE_SYSUPDATES
static int
x_sys_updates(void)
{
	more("etc/sysupdates", YEA);
	return 0;
}
#endif

#ifdef DEBUG
int _debug_reportstruct()
{
	clear();
	prints("boardheader_t:\t%d\n", sizeof(boardheader_t));
	prints("fileheader_t:\t%d\n", sizeof(fileheader_t));
	prints("userinfo_t:\t%d\n", sizeof(userinfo_t));
	prints("screenline_t:\t%d\n", sizeof(screenline_t));
	prints("SHM_t:\t%d\n", sizeof(SHM_t));
	prints("userec_t:\t%d\n", sizeof(userec_t));
	pressanykey();
	return 0;
}
#endif

#ifdef USE_BOARDTAX
// in boardtax.c
int pay_board_tax();
int set_board_tax();
int board_tax_calc();
int board_tax_log();
int set_tax_file();
int list_unpay();
#endif

int list_user_board();

/* Two Factor Auth in twofa.c */
#ifdef USE_2FALOGIN
int twoFA_genRecovCode();
#endif
#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
int twoFA_RemoveTrust();
#endif

/* Password change in user.c */
int userPass_change();
static int
u_pass_change()
{
	userPass_change(cuser, 0, usernum);
	return 0;
}

/* Mission in mission.c */
#ifdef USE_MISSION
int mission();
static int
p_mission()
{
	mission_main();
	return 0;
}
#endif

// ----------------------------------------------------------- MENU DEFINITION
// �`�N�C�� menu �̦h����P����ܶW�L 11 �� (80x24 �зǤj�p������)
// 107.08.03 ��z�{���X�[�c

static int x_admin_board(void);
static int x_admin_brdtax(void);
static int x_admin_money(void);
static int x_admin_user(void);
static int x_admin_usermenu(void);

static const commands_t      cmdlist[] = {
    {admin,				PERM_SYSOP|PERM_BBSADM|PERM_ACCOUNTS|PERM_BOARD,
										"0Admin       �r �t�κ��@�� �q"},
    {Announce,			0,				"Announce     �r ��ؤ��G�� �q"},
    {Favorite,			0,				"Favorite     �r  �ڪ��̷R  �q"},
    {Class,				0,				"Class        �r ���հQ�װ� �q"},

    {Mail,				PERM_LOGINOK,	"Mail         �r �p�H�H��� �q"},
    /*�j�ߡG�o�ӥؿ����\���ctrl-u������A�ܩ�h�H��ѫǮįq���j�A�]�S���ҥΤѨϥ\��A�]���o�ӿ���Ӱ���*/
    //{Talk,			PERM_LOGINOK,	"Talk         �r �𶢲�Ѱ� �q"},
    {Play_Play, 		PERM_LOGINOK,	"Play         �r �T�ֻP�� �q"},
    {Name_Menu, 		PERM_LOGINOK,	"Namelist     �r �s�S�O�W�� �q"},

    {u_register,    	MENU_UNREGONLY, "Register     �r ��g���U�� �q"},
#ifdef USE_MAIL_ACCOUNT_SYSOP
    {mail_account_sysop,MENU_UNREGONLY,	"Mail Admin   �r�H���b�������q"},
#endif

    {User, 				PERM_BASIC,		"User         �r �ӤH�]�w�� �q"},
    {Xyz, 				0,				"Xyz          �r �t�θ�T�� �q"},
    {Goodbye, 			0, 				"Goodbye      �r  ���ҥX�f  �q"},
    {NULL, 0, NULL}
};
int main_menu(void) {
	char init = 'C';

	if (ISNEWMAIL(currutmp))
		init = 'M';

	if (!(HasUserPerm(PERM_LOGINOK)))
		init = 'R';

    domenu(M_MMENU, "�D�\\���", init, cmdlist);
    return 0;
}
	/* 0Admin Menu */
	/* �j�ߡG�v�������Ƶ��G�����ϥΪ̸�ƪ����إu����SYSOP�PACCOUNT�ާ@�FBBSADM�p�e�@�����USYSOP���ΡA�]����L�D�ӷP�ާ@�o��BBSADM�ϥΡFBOARD�i�H�����b�o�̳]�w�ݪO�C*/
	static const commands_t adminlist[] = {
		{x_admin_board,		PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"Board Admin  �r �g�a�޲z�� �q"},
		{x_admin_usermenu,	PERM_SYSOP|PERM_ACCOUNTS|PERM_BOARD,"User Admin   �r ���F�ưȧ� �q"},
		{x_admin_money,		PERM_SYSOP|PERM_BBSADM,				"FinancAdmin  �r ���ĺʺ޸p �q"},
		{x_file,			PERM_SYSOP|PERM_BBSADM,				"SystemFile   �r  �t���ɮ�  �q"},
		{m_loginmsg,		PERM_SYSOP|PERM_BBSADM,				"LoginMsg     �r  �i�����y  �q"},
		{NULL, 0, NULL}
	};
	int
	admin(void)
	{
		char init = 'B';

		if (HasUserPerm(PERM_ACCOUNTS))
			init = 'U';

		domenu(M_ADMIN, "�������@�t��", init, adminlist);
		return 0;
	}
		static const commands_t m_admin_board[] = {
			{m_board,		PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"Set Board    �r  �]�w�ݪO  �q"},
		#ifdef USE_BOARDTAX
			{x_admin_brdtax,PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"TBoard Tax   �r �ݪO�|�޲z �q"},
		#endif
			{list_user_board,PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"ListUserMod  �r �d����O�D �q"},
			{NULL, 0, NULL}
		};
		static int x_admin_board(void)
		{
			char init = 'S';
			domenu(M_XMENU, "�g�a�޲z��", init, m_admin_board);
			return 0;
		}
		#ifdef USE_BOARDTAX
		static const commands_t m_admin_brdtax[] = {
			{board_tax_calc,PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"CTax Calc    �r  �պ�|�B  �q"},
			{set_board_tax,	PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"STax Set     �r �d�߻P�W�R �q"},
			{set_tax_file,	PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"FTax File    �r  �|�B�ɮ�  �q"},
			{board_tax_log,	PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"LTax PayLog  �r  ú�Ǭ���  �q"},
			{list_unpay,	PERM_SYSOP|PERM_BBSADM|PERM_BOARD,	"UnPay List   �r ��ú�ǦW�� �q"},
			{NULL, 0, NULL}
		};
		static int x_admin_brdtax(void)
		{
			char init = 'S';
			domenu(M_XMENU, "�ݪO�|�޲z", init, m_admin_brdtax);
			return 0;
		}
		#endif
		static const commands_t m_admin_money[] = {
			{view_user_money_log,	PERM_SYSOP|PERM_BBSADM,	"View Log     �r  ����O��  �q"},
			{give_money,			PERM_SYSOP|PERM_BBSADM,"Givemoney    �r  �o��"MONEYNAME"  �q"},
			{NULL, 0, NULL}
		};
		static int x_admin_money(void)
		{
			char init = 'V';
			domenu(M_XMENU, "���ĺʺ޸p", init, m_admin_money);
			return 0;
		}
		static const commands_t m_admin_usermenu[] = {
			{m_register,		PERM_SYSOP|PERM_ACCOUNTS,			"Register     �r �f�ֵ��U�� �q"},
			{m_user,			PERM_SYSOP|PERM_ACCOUNTS|PERM_BOARD,"User Data    �r �ϥΪ̸�� �q"},
			{x_admin_user,		PERM_SYSOP|PERM_ACCOUNTS,			"LUser Log    �r �ϥΪ̰O�� �q"},
			{search_user_bypwd,	PERM_SYSOP|PERM_ACCOUNTS,			"Search User  �r �j�M�ϥΪ� �q"},
			{NULL, 0, NULL}
		};
		static int x_admin_usermenu(void)
		{
			char init = 'U';
			domenu(M_XMENU, "���F�ưȧ�", init, m_admin_usermenu);
			return 0;
		}
		static const commands_t m_admin_user[] = {
			{view_user_money_log,	PERM_SYSOP|PERM_ACCOUNTS,	"Money Log    �r  ����O��  �q"},
			{view_user_login_log,	PERM_SYSOP|PERM_ACCOUNTS,	"Login Log    �r  �W�u�O��  �q"},
			{u_list,				PERM_SYSOP|PERM_ACCOUNTS,	"Users List   �r  ���U�W�U  �q"},
			{search_user_bybakpwd,	PERM_SYSOP|PERM_ACCOUNTS,	"Old Data     �r �d�ƥ���� �q"},
			{NULL, 0, NULL}
		};
		static int x_admin_user(void)
		{
			domenu(M_XMENU, "�ϥΪ̰O��", 'L', m_admin_user);
			return 0;
		}
	/* Mail Menu */
	static const commands_t maillist[] = {
		{m_read,			PERM_LOGINOK,			"Read         �r  �ڪ��H�c  �q"},
		{m_send,			PERM_LOGINOK,			"Send         �r  �����H�H  �q"},
		{mail_list,			PERM_LOGINOK,			"Group Mail   �r  �s�ձH�H  �q"},
		{setforward,		PERM_LOGINOK,			"Forward      �r�]�w�۰���H�q"},
		{mail_mbox, 		PERM_INTERNET,			"Zip Data     �r���]�p�H��ơq"},
		{built_mail_index,	PERM_LOGINOK,			"VSavemail    �r���ثH�c���ޡq"},
		{mail_all,			PERM_SYSOP|PERM_BBSADM,	"Mail All     �r �H���Ҧ��H �q"},
	#ifdef USE_MAIL_ACCOUNT_SYSOP
		{mail_account_sysop,PERM_LOGINOK,			"AMail Admin  �r�H���b�������q"},
	#endif
		{NULL, 0, NULL}
	};
	int
	Mail(void)
	{
		domenu(M_MAIL, "�q�l�l��", (ISNEWMAIL(currutmp) ? 'R' : 'S'), maillist);
		return 0;
	}
	/* Play Menu */
	static const commands_t moneylist[] = {
		{p_from,			0,	"Edit From    �r  �ק�G�m  �q"},
		{ordersong,			0,	"Order Song   �r�I���߱��ʺA�q"},
		{NULL, 0, NULL}
	};
	static int p_money() {
		domenu(M_PSALE, BBSMNAME2 "�W��", 'O', moneylist);
		return 0;
	};
	static const commands_t banklist[] = {
		{p_give,			0,				"Give Money   �r ���O�H" MONEYNAME" �q"},
		{save_violatelaw,	0,				"Pay Ticket   �r  ú�ǻ@��  �q"},
	#ifdef USE_BOARDTAX
		{board_tax_calc,	PERM_LOGINOK,	"CTax Calc    �r  �պ�|�B  �q"},
		{pay_board_tax,		PERM_LOGINOK,	"Board Tax    �r ú�ǬݪO�| �q"},
	#endif
		{NULL, 0, NULL}
	};
	static int p_bank() {
		domenu(M_PSALE, BBSMNAME2 "�Ȧ�", 'G', banklist);
		return 0;
	};
	static const commands_t playlist[] = {
	#ifdef USE_MISSION
		{p_mission,		0,				"IMission     �r  ���Ȥ���  �q"},
	#endif
		{p_bank,		0,				"Bank         �r  " BBSMNAME2 "�Ȧ�  �q"},
		{p_money,		PERM_LOGINOK,	"Market       �r  " BBSMNAME2 "�W��  �q"},
		{chicken_main,	PERM_LOGINOK,	"Chicken      �r " BBSMNAME2 "�i���� �q"},
	#ifndef NO_GAMBLE
		{ticket_main,	PERM_LOGINOK,	"Gamble       �r  " BBSMNAME2 "�m��  �q"},
	#endif
		//{chessroom,		PERM_LOGINOK,	"BChess      �i " BBSMNAME2 "�Ѱ|   �j"}, /*�j�ߡG�įq�C�A���ΡA���ӥi���өް�*/
		{NULL, 0, NULL}
	};
	int
	Play_Play(void)
	{
		domenu(M_PMENU, "�T�ֻP��", 'M', playlist);
		return 0;
	}
	/* Name menu */
	static const commands_t namelist[] = {
		{t_override,	PERM_LOGINOK,	"OverRide     �r  �n�ͦW��  �q"},
		{t_reject,		PERM_LOGINOK,	"Black        �r  �a�H�W��  �q"},
		{t_aloha,		PERM_LOGINOK,	"ALOHA        �r�W���q���W��q"},
		{t_fix_aloha,	PERM_LOGINOK,	"XFixALOHA    �r�ץ��W���q���q"},
		{t_special,		PERM_LOGINOK,	"Special      �r  �S�O�W��  �q"},
		{NULL, 0, NULL}
	};
	int
	Name_Menu(void)
	{
		domenu(M_NMENU, "�W��s��", 'O', namelist);
		return 0;
	}
	/* User menu */
	static const commands_t seculist[] = {
		{u_pass_change,		PERM_BASIC,		"Password     �r  �ק�K�X  �q"},
	#ifdef USE_2FALOGIN
		{twoFA_genRecovCode,PERM_BASIC,		"RecoverCode  �r ���ʹ_��X �q"},
	#endif
	#if defined(DETECT_CLIENT) && defined(USE_TRUSTDEV)
		{twoFA_RemoveTrust	,PERM_BASIC,	"TRemoveTrust �r�M�P�H���˸m�q"},
	#endif
		{NULL, 0, NULL}
	};
	static int u_security() {
		domenu(M_UMENU, "�K�X�P�w��", 'P', seculist);
		return 0;
	};
	static const commands_t userlist[] = {
		/*{u_loginview,		PERM_BASIC,     "VLogin View   ��ܶi���e��"},
		{u_myfiles,			PERM_LOGINOK,   "My Files      �i�ӤH�ɮסj (�W��,ñ�W��...)"},
		{u_mylogs,			PERM_LOGINOK,   "LMy Logs      �i�ӤH�O���j (�̪�W�u...)"},*/
		{u_info,			PERM_BASIC,		"Info         �r�ӤH��Ƴ]�w�q"},
		{u_security,		PERM_BASIC,		"Security     �r �K�X�P�w�� �q"},
		{u_customize,		PERM_BASIC,		"Customize    �r �ӤH�Ƴ]�w �q"},
		{u_editplan,		PERM_LOGINOK,   "QueryEdit    �r �s��W���� �q"},
		{u_editsig,			PERM_LOGINOK,   "NSignature   �r �s��ñ�W�� �q"},
		{u_view_recentlogin,0,				"Login Log    �r  �W���O��  �q"},
	#ifdef USE_RECENTPAY
		{u_view_recentpay,	0,				"Pay Log      �r  ����O��  �q"},
	#endif
	#ifdef ASSESS
		{u_cancelbadpost,	PERM_LOGINOK,	"Bye BadPost  �r  �R���h��  �q"},
	#endif
		{NULL, 0, NULL}
	};
	int
	User(void)
	{
		domenu(M_UMENU, "�ӤH�]�w", 'I', userlist);
		return 0;
	}
	/* XYZ tool menu */
	static const commands_t xyzlist[] = {
		/*{x_hot,				0,	"THot Topics  �r  �����ݪO  �q"},
		{x_users,				0,	"Users        �r �ϥΪ̲έp �q"},*/
	#ifndef DEBUG
		/* All these are useless in debug mode. */
	#ifdef HAVE_USERAGREEMENT
		{x_agreement,			0,	"Agreement    �r �ϥΪ̱��� �q"},
	#endif
	#ifdef  HAVE_LICENSE
		{x_gpl,					0,	"ILicense     �r  GNU ����  �q"},
	#endif
	#ifdef HAVE_INFO
		{x_program,				0,	"Program      �r  �{������  �q"},
	#endif
		{x_history,				0,	"History      �r �ڭ̪����� �q"},
		{x_login,				0,	"System       �r  ���n���i  �q"},
	#ifdef HAVE_SYSUPDATES
		{x_sys_updates,			0,	"LUpdates     �r�{����s�����q"},
	#endif

	#else // !DEBUG
		{_debug_reportstruct,	0,	"Report       �r  ���c���i  �q"},
	#endif // !DEBUG

		{p_sysinfo,				0,	"Xinfo        �r  �t�θ�T  �q"},
		{NULL, 0, NULL}
	};
	int
	Xyz(void)
	{
		domenu(M_XMENU, "�u��{��", 'X', xyzlist);
		return 0;
	}

// -----------------------------------------------------------
// �H�U�Τ���
// -----------------------------------------------------------

#ifdef PLAY_ANGEL
static const commands_t angelmenu[] = {
    {a_angelmsg, PERM_ANGEL,"Leave message �d�����p�D�H"},
    {a_angelmsg2,PERM_ANGEL,"Call screen   �I�s�e���өʯd��"},
    {angel_check_master,PERM_ANGEL,
                            "Master check  �d�ߤp�D�H���A"},
    // Cannot use R because r is reserved for Read/Mail due to TMENU.
    {a_angelreport, 0,      "PReport       �u�W�ѨϪ��A���i"},
    {NULL, 0, NULL}
};

static int menu_angelbeats() {
    domenu(M_TMENU, "Angel Beats! �ѨϤ��|", 'L', angelmenu);
    return 0;
}
#endif

/* Talk menu */
/*static const commands_t talklist[] = {
    {t_users, 0,            "Users         �u�W�ϥΪ̦C��"},
    {t_query, 0,            "Query         �d�ߺ���"},
    // PERM_PAGE - ���y���n PERM_LOGIN �F
    // �S�D�z�i�H talk ������y�C
    {t_talk, PERM_LOGINOK,  "Talk          ��H���"},
    // PERM_CHAT �D login �]���A�|���H�Φ��n�O�H�C
    {t_chat, PERM_LOGINOK,  "Chat          �i" BBSMNAME2 "�h�H��ѫǡj"},
    {deprecate_userlist, 0, "Pager         �����I�s��"},
    {t_qchicken, 0,         "Watch Pet     �d���d��"},
#ifdef PLAY_ANGEL
    {a_changeangel,
	PERM_LOGINOK,	    "AChange Angel �󴫤p�Ѩ�"},
    {menu_angelbeats, PERM_ANGEL|PERM_SYSOP,
                            "BAngel Beats! �ѨϤ��|"},
#endif
    {t_display, 0,          "Display       ��ܤW�X�����T"},
    {NULL, 0, NULL}
};*/

/*static const commands_t myfilelist[] = {
    {u_editplan,    PERM_LOGINOK,   "QueryEdit    �r�s��W���ɡq"},
    {u_editsig,	    PERM_LOGINOK,   "Signature    �r�s��ñ�W�ɡq"},
    {NULL, 0, NULL}
};

static const commands_t myuserlog[] = {
    {u_view_recentlogin, 0,   "LRecent Login  �̪�W���O��"},
#ifdef USE_RECENTPAY
    {u_view_recentpay,   0,   "PRecent Pay    �̪����O��"},
#endif
    {NULL, 0, NULL}
};

static int
u_myfiles()
{
    domenu(M_UMENU, "�ӤH�ɮ�", 'Q', myfilelist);
    return 0;
}

static int
u_mylogs()
{
    domenu(M_UMENU, "�ӤH�O��", 'L', myuserlog);
    return 0;
}*/

/* XYZ tool sub menu */
/*static const commands_t m_xyz_hot[] = {
    {x_week, 0,      "Week          �m���g���Q�j�������D�n"},
    {x_issue, 0,     "Issue         �m����Q�j�������D�n"},
    {x_boardman,0,   "Man Boards    �m�ݪO��ذϱƦ�]�n"},
    {NULL, 0, NULL}
};
static const commands_t m_xyz_user[] = {
    {x_user100 ,0,   "Users         �m�ϥΪ̦ʤj�Ʀ�]�n"},
    {topsong,PERM_LOGINOK,
	             "GTop Songs    �m�ϥΪ̤߱��I���Ʀ�n"},
    {x_today, 0,     "Today         �m����W�u�H���έp�n"},
    {x_yesterday, 0, "Yesterday     �m�Q��W�u�H���έp�n"},
    {NULL, 0, NULL}
};

static int
x_hot(void)
{
    domenu(M_XMENU, "�������D�P�ݪO", 'W', m_xyz_hot);
    return 0;
}

static int
x_users(void)
{
    domenu(M_XMENU, "�ϥΪ̲έp��T", 'U', m_xyz_user);
    return 0;
}*/

//static int chessroom();

/*static const commands_t conn6list[] = {
    {conn6_main,       PERM_LOGINOK, "1Conn6Fight    �i" ANSI_COLOR(1;33) "���l���ܧ�" ANSI_RESET "�j"},
    {conn6_personal,   PERM_LOGINOK, "2Conn6Self     �i" ANSI_COLOR(1;34) "���l�ѥ���" ANSI_RESET "�j"},
    {conn6_watch,      PERM_LOGINOK, "3Conn6Watch    �i" ANSI_COLOR(1;35) "���l���[��" ANSI_RESET "�j"},
    {NULL, 0, NULL}
};

static int conn6_menu() {
    domenu(M_CHC, BBSMNAME2 "���l��", '1', conn6list);
    return 0;
}

static const commands_t chesslist[] = {
    {chc_main,         PERM_LOGINOK, "1CChessFight   �i" ANSI_COLOR(1;33) " �H���ܧ� " ANSI_RESET "�j"},
    {chc_personal,     PERM_LOGINOK, "2CChessSelf    �i" ANSI_COLOR(1;34) " �H�ѥ��� " ANSI_RESET "�j"},
    {chc_watch,        PERM_LOGINOK, "3CChessWatch   �i" ANSI_COLOR(1;35) " �H���[�� " ANSI_RESET "�j"},
    {gomoku_main,      PERM_LOGINOK, "4GomokuFight   �i" ANSI_COLOR(1;33) "���l���ܧ�" ANSI_RESET "�j"},
    {gomoku_personal,  PERM_LOGINOK, "5GomokuSelf    �i" ANSI_COLOR(1;34) "���l�ѥ���" ANSI_RESET "�j"},
    {gomoku_watch,     PERM_LOGINOK, "6GomokuWatch   �i" ANSI_COLOR(1;35) "���l���[��" ANSI_RESET "�j"},
    {gochess_main,     PERM_LOGINOK, "7GoChessFight  �i" ANSI_COLOR(1;33) " ����ܧ� " ANSI_RESET "�j"},
    {gochess_personal, PERM_LOGINOK, "8GoChessSelf   �i" ANSI_COLOR(1;34) " ��ѥ��� " ANSI_RESET "�j"},
    {gochess_watch,    PERM_LOGINOK, "9GoChessWatch  �i" ANSI_COLOR(1;35) " ����[�� " ANSI_RESET "�j"},
    {conn6_menu,       PERM_LOGINOK, "CConnect6      �i" ANSI_COLOR(1;33) "  ���l��  " ANSI_RESET "�j"},
    {NULL, 0, NULL}
};

static int chessroom() {
    domenu(M_CHC, BBSMNAME2 "�Ѱ|", '1', chesslist);
    return 0;
}

int
Talk(void)
{
    domenu(M_TMENU, "��ѻ���", 'U', talklist);
    return 0;
}*/