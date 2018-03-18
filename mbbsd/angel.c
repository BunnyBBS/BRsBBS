#include "bbs.h"
#ifdef PLAY_ANGEL

// PTT-BBS Angel System

#include "daemons.h"
#define FN_ANGELMSG     "angelmsg"
#define FN_ANGELMSG2    "angelmsg2"

// �I�s�ѨϮ���ܪ�����
#define FN_ANGEL_USAGE   "etc/angel_usage"
// �I�s�ѨϮ���ܪ�����(���ۭq�T��)
#define FN_ANGEL_USAGE2     "etc/angel_usage2"
// �ѨϤ��b�u�W�ɪ�����(���ۭq�T��)
#define FN_ANGEL_OFFLINE2   "etc/angel_offline2"

#ifndef ANGEL_INACTIVE_DAYS
#define ANGEL_INACTIVE_DAYS (180)
#endif

static const char
*PROMPT_ANGELBEATS = " Angel Beats! �ѨϤ��| ",
*ERR_CONNECTION = "��p�A�L�k�s�u�ܤѨϤ��|�A�еy��A�աC\n"
                  "�Y����o�ͽЦ� " BN_BUGREPORT " �ݪO�q������޲z�H���C\n",
*ERR_PROTOCOL = "��p�A�ѨϤ��|�s�u���`�A�еy��A�աC\n"
                "�Y����o�ͽЦ� " BN_BUGREPORT " �ݪO�q������޲z�H���C\n",
*ERR_PROTOCOL2 = "��p�A�ѨϤ��|���G�w��s�A�Э��s�n�J�C\n"
                 "�Y�n�J�ᤴ���~�Ц� " BN_BUGREPORT " �ݪO�q������޲z�H���C\n";

/////////////////////////////////////////////////////////////////////////////
// Angel Beats! Client

// this works for simple requests: (return 1/angel_uid for success, 0 for fail
static int
angel_beats_do_request(int op, int master_uid, int angel_uid) {
    int fd;
    int ret = 1;
    angel_beats_data req = {0};

    req.cb = sizeof(req);
    req.operation = op;
    req.master_uid = master_uid;
    req.angel_uid = angel_uid;
    assert(op != ANGELBEATS_REQ_INVALID);

    if ((fd = toconnect(ANGELBEATS_ADDR)) < 0)
        return -1;

    assert(req.operation != ANGELBEATS_REQ_INVALID);

    if (towrite(fd, &req, sizeof(req)) < 0 ||
        toread (fd, &req, sizeof(req)) < 0 ||
        req.cb != sizeof(req)) {
        ret = -2;
    } else {
        ret = (req.angel_uid > 0) ? req.angel_uid : 1;
    }

    close(fd);
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Local Angel Service

void
angel_register_new(const char *userid) {
    angel_beats_do_request(ANGELBEATS_REQ_REG_NEW, usernum,
                           searchuser(userid, NULL));
}

void
angel_notify_activity(const char *userid) {
    int master;
    static time4_t t = 0;
    time4_t tick;

    // tick: every 1 minutes.
    syncnow();
    tick = now - now % (1 * 60);

    // ping daemon only in different ticks.
    if (tick == t)
        return;

    master = searchuser(userid, NULL);
    t = tick;

    angel_beats_do_request(ANGELBEATS_REQ_HEARTBEAT, master, usernum);
}

void
angel_toggle_pause()
{
    if (!HasUserPerm(PERM_ANGEL) || !currutmp)
	return;
    currutmp->angelpause ++;
    currutmp->angelpause %= ANGELPAUSE_MODES;
    if (cuser.uflag & UF_NEW_ANGEL_PAGER) {
        // pmore_QuickRawModePref-like conf
        currutmp->angelpause = vs_quick_pref(
            currutmp->angelpause % ANGELPAUSE_MODES,
            "�]�w�p�Ѩϯ��٩I�s��(�i�������Ʀr���,��K�]�w����]�קK�~��)",
            "�п�����٩I�s�����s���A: ",
            "�}��\t����\t����",
            NULL) % ANGELPAUSE_MODES;
    }
}

void
angel_parse_nick_fp(FILE *fp, char *nick, int sznick)
{
    char buf[PATHLEN];
    // should be in first line
    rewind(fp);
    *buf = 0;
    if (fgets(buf, sizeof(buf), fp))
    {
	// verify first line
	if (buf[0] == '%' && buf[1] == '%' && buf[2] == '[')
	{
	    chomp(buf+3);
	    strlcpy(nick, buf+3, sznick);
	}
    }
}

void
angel_load_my_fullnick(char *buf, int szbuf)
{
    char fn[PATHLEN];
    FILE *fp = NULL;
    static char mynick[IDLEN + 1] = "";
    static time4_t touched = 0;
    time4_t modtime = 0;

    *buf = 0;
    setuserfile(fn, FN_ANGELMSG);
    modtime = dasht(fn);
    if (modtime != touched) {
        touched = modtime;
        *mynick = 0;
        // reload file
        if ((fp = fopen(fn, "rt")))
        {
            angel_parse_nick_fp(fp, mynick, sizeof(mynick));
            fclose(fp);
        }
    }
    strlcpy(buf, mynick, szbuf);
    strlcat(buf, "�p�Ѩ�", szbuf);
}

// cache my angel's nickname
static char _myangel[IDLEN+1] = "",
	    _myangel_nick[IDLEN+1] = "";
static time4_t _myangel_touched = 0;
static char _valid_angelmsg = 0;

void
angel_reload_nick()
{
    char reload = 0;
    char fn[PATHLEN];
    time4_t ts = 0;
    FILE *fp = NULL;

    fn[0] = 0;
    // see if we have angel id change (reload whole)
    if (strcmp(_myangel, cuser.myangel) != 0)
    {
	strlcpy(_myangel, cuser.myangel, sizeof(_myangel));
	reload = 1;
    }
    // see if we need to check file touch date
    if (!reload && _myangel[0] && _myangel[0] != '-')
    {
	sethomefile(fn, _myangel, FN_ANGELMSG);
	ts = dasht(fn);
	if (ts != -1 && ts > _myangel_touched)
	    reload = 1;
    }
    // if no need to reload, reuse current data.
    if (!reload)
    {
	// vmsg("angel_data: no need to reload.");
	return;
    }

    // reset cache
    _myangel_touched = ts;
    _myangel_nick[0] = 0;
    _valid_angelmsg = 0;

    // quick check
    if (_myangel[0] == '-' || !_myangel[0])
	return;

    // do reload data.
    if (!fn[0])
    {
	sethomefile(fn, _myangel, FN_ANGELMSG);
	ts = dasht(fn);
	_myangel_touched = ts;
    }

    assert(*fn);
    // complex load
    fp = fopen(fn, "rt");
    if (fp)
    {
	_valid_angelmsg = 1;
	angel_parse_nick_fp(fp, _myangel_nick, sizeof(_myangel_nick));
	fclose(fp);
    }
}

const char *
angel_get_nick()
{
    angel_reload_nick();
    return _myangel_nick;
}

int
select_angel() {
    angel_beats_uid_list list = {0};
    angel_beats_data req = {0};
    int i;
    int fd;

    vs_hdr2(PROMPT_ANGELBEATS, " ����Ѩ� ");
    outs("\n");

    if ((fd = toconnect(ANGELBEATS_ADDR)) < 0) {
        outs(ERR_CONNECTION);
        pressanykey();
        return 0;
    }

    req.cb = sizeof(req);
    req.operation = ANGELBEATS_REQ_GET_ONLINE_LIST;
    req.master_uid = usernum;

    if (towrite(fd, &req, sizeof(req)) < 0 ||
        toread(fd, &list, sizeof(list)) < 0 ||
        list.cb != sizeof(list)) {
        close(fd);
        outs(ERR_PROTOCOL);
        pressanykey();
        return 0;
    }
    close(fd);

    if (!list.angels) {
        vmsg("��p�A�ثe�S���i�I�s���ѨϦb�u�W�C");
        return 0;
    }

    // list all angels
    for (i = 0; i < list.angels; i++) {
        char fn[PATHLEN];
        char nick[IDLEN + 1] = "";
        int uid = list.uids[i];
        const char *userid = getuserid(uid);
        FILE *fp = NULL;
        userinfo_t *uinfo = search_ulist_userid(userid);
        const char *pause_msg = "";

        sethomefile(fn, userid, FN_ANGELMSG);
        if ((fp = fopen(fn, "rt")) != NULL) {
            angel_parse_nick_fp(fp, nick, sizeof(nick));
            strlcat(nick, "�p�Ѩ�", sizeof(nick));
            fclose(fp);
        } else {
            strlcpy(nick, "(���]�w�ʺ�)", sizeof(nick));
        }
        if (uinfo && uinfo->angelpause == 1)
            pause_msg = ANSI_COLOR(1;32) "(�����s���D/�s�D�H) " ANSI_RESET;
        else if (uinfo && uinfo->angelpause == 2)
            pause_msg = ANSI_COLOR(1;31) "(�����I�s��) " ANSI_RESET;
        prints(" %3i. %s %s [UID: %d]\n", i + 1, nick, pause_msg, uid);
    }
    while (list.angels) {
        char ans[5];
        int idx;

        if (!getdata(b_lines - 1, 0, "�аݭn�������p�Ѩ� (��J�Ʀr): ",
                     ans, sizeof(ans), NUMECHO)) {
            vmsg("������p�ѨϡC");
            return 0;
        }
        idx = atoi(ans);
        if (idx < 1 || idx > list.angels) {
            vmsg("�Ʀr�����T�C");
            return 0;
        }
        // No need to tell AngelBeats since this is only for ANGEL_CIA_ACCOUNT.
        pwcuSetMyAngel(getuserid(list.uids[idx - 1]));
	log_filef(BBSHOME "/log/changeangel.log",LOG_CREAT,
                  "%s �~�� %s ��� %s �p�Ѩ�\n",
                  Cdatelite(&now), cuser.userid, cuser.myangel);
        vmsg("�p�ѨϤw�󴫧����C");
        break;
    }
    return 0;
}

static int
do_changeangel(int force) {
    char buf[4];
    const char *prompt = "�n�O�����A�U���I�s�ɷ|�q�W�u���ѨϤ���X�s���p�Ѩ�";
    static int is_bad_master = -1;

    /* cuser.myangel == "-" means banned for calling angel */
    if (cuser.myangel[0] == '-')
        return 0;

    if (HasUserRole(ROLE_ANGEL_CIA))
        return select_angel();

    if (!cuser.myangel[0]) {
        vmsg(prompt);
        return 0;
    }

    // get/cache "bad_master" info
    if (is_bad_master < 0) {
        char bad_master_file[PATHLEN];
        setuserfile(bad_master_file, ".bad_master");
        is_bad_master = dashf(bad_master_file);
        if (is_bad_master &&
            dasht(bad_master_file) < (now - ANGEL_INACTIVE_DAYS * DAY_SECONDS)) {
            log_filef("log/bad_master.log", LOG_CREAT,
                      "%s %s removed from bad master list (%d)\n",
                      Cdatelite(&now), cuser.userid, dasht(bad_master_file));
            remove(bad_master_file);
        }
    }

    if (!(force || HasUserPerm(PERM_ADMIN)))
    {
#ifdef ANGEL_CHANGE_TIMELIMIT_MINS
        int duration = ANGEL_CHANGE_TIMELIMIT_MINS;
        if (is_bad_master)
            duration *= 3;
        if (cuser.timesetangel &&
            (now - cuser.timesetangel < duration * 60)) {
            vmsgf("�C���󴫤p�Ѩϳֶ̤��j %d �����C", duration);
            return 0;
        }
#endif
       if (is_bad_master)
           log_filef("log/bad_master.log", LOG_CREAT,
                     "%s %s change angel.\n", Cdatelite(&now), cuser.userid);
        if (is_bad_master &&
            !verify_captcha("���קK�j�q�D���`�󴫤p�ѨϡA\n"))
            return 0;
    }

    getdata(b_lines - 1, 0, "�T�w�n�󴫤p�ѨϡH [y/N]", buf, 3, LCECHO);
    if (buf[0] == 'y') {
	log_filef(BBSHOME "/log/changeangel.log",LOG_CREAT,
                  "%s �p�D�H %s ���� %s �p�Ѩ�\n",
                  Cdatelite(&now), cuser.userid, cuser.myangel);
        angel_beats_do_request(ANGELBEATS_REQ_REMOVE_LINK,
                               usernum, searchuser(cuser.myangel, NULL));
	pwcuSetMyAngel("");
        vmsg(prompt);
    }
    return 0;
}

int a_changeangel(void) {
    return do_changeangel(0);
}

const char *
angel_order_song(char *receiver, size_t sz_receiver) {
    userec_t udata;
    char prompt[STRLEN], ans[3];
    const char *angel_nick = NULL;

    if (!*cuser.myangel)
        return NULL;

#ifdef ANGEL_ORDER_SONG_DAY
    // check day
    {
        struct tm tm;
        localtime4_r(&now, &tm);
        if (tm.tm_mday != ANGEL_ORDER_SONG_DAY)
            return NULL;
    }
#endif

    // ensure if my angel is still valid.
    if (passwd_load_user(cuser.myangel, &udata) <= 0 ||
        !(udata.userlevel & PERM_ANGEL))
        return NULL;

    angel_nick = angel_get_nick();
    snprintf(prompt, sizeof(prompt), "�n�d�����A��%s�p�Ѩ϶�? [y/N]: ",
             angel_nick);
    if (getdata(20, 0, prompt, ans, sizeof(ans), LCECHO) && *ans == 'y') {
        snprintf(receiver, sz_receiver, "%s�p�Ѩ�", angel_nick);
        return angel_nick;
    }
    return NULL;
}

int angel_check_master(void) {
    char uid[IDLEN + 1];
    userec_t xuser;
    int is_my_master;

    vs_hdr2(PROMPT_ANGELBEATS, " �d�ߥD�H���A ");
    usercomplete("�Q�d�ߪ��D�H ID: ", uid);
    move(2, 0); clrtobot();
    if (!*uid)
        return 0;
    if (getuser(uid, &xuser) < 1) {
        vmsg("�� ID ���s�b�C");
        return 0;
    }
    wait_penalty(1);
    is_my_master = (strcasecmp(xuser.myangel, cuser.userid) == 0);
    move(7, 0);
    if (is_my_master) {
        prints(ANSI_COLOR(1;32) "%s �O�A���D�H�C" ANSI_RESET "\n",
               xuser.userid);
        if (xuser.timesetangel)
            prints("�p�ѨϻP�D�H�����Y�w�����F %d �ѡC\n",
                   (now - xuser.timesetangel) / DAY_SECONDS + 1);
        if (xuser.timeplayangel && xuser.timeplayangel > xuser.timesetangel)
            prints("�p�D�H�̫�P�ѨϤ���(hh���\\�I�s���I�q)���ɶ�: %s\n",
                   Cdatelite(&xuser.timeplayangel));
        else if (xuser.timesetangel)
            prints("���p�D�H���G�q�ӨS�P�A����(���\\�I�s���I�q)�L\n"
                   " (�`����~�Ѩϩλ~�����D�H)�C\n");
    } else {
        prints(ANSI_COLOR(1;31) "%s ���O�A���p�D�H�C" ANSI_RESET "\n",
               xuser.userid);
    }
    log_filef("log/angel_query_master.log", LOG_CREAT,
             "%s [%s] query [%s]\n", Cdatelite(&now), cuser.userid, uid);
    pressanykey();
    return 0;
}

void
angel_log_order_song(const char *angel_nick) {
    char angel_exp[STRLEN];

    syncnow();
    if (cuser.timesetangel && now >= cuser.timesetangel)
        snprintf(angel_exp, sizeof(angel_exp),
                 "%d��", (now - cuser.timesetangel) / DAY_SECONDS + 1);
    else
        strlcpy(angel_exp, "�ܤ[", sizeof(angel_exp));

    log_filef("log/osong_angel.log", LOG_CREAT,
              "%s %*s �I�q�� %*s�p�Ѩ� (%s - %s)\n",
              Cdatelite(&now), IDLEN, cuser.userid,
              IDLEN - 6, angel_nick, fromhost, angel_exp);
    pwcuPlayAngel();
}

void
angel_log_msg_to_angel(void) {
    if (cuser.timeplayangel > cuser.timesetangel) {
        // Try to avoid mass logs
        if ((now - cuser.timeplayangel) >= ANGEL_CHANGE_TIMELIMIT_MINS * 60)
            return;
    }
    pwcuPlayAngel();
}

int a_angelreport() {
    angel_beats_report rpt = {0};
    angel_beats_data   req = {0};
    int fd;

    vs_hdr2(PROMPT_ANGELBEATS, " �ѨϪ��A���i ");
    outs("\n");

    if ((fd = toconnect(ANGELBEATS_ADDR)) < 0) {
        outs(ERR_CONNECTION);
        pressanykey();
        return 0;
    }

    req.cb = sizeof(req);
    req.operation = ANGELBEATS_REQ_REPORT;
    req.master_uid = usernum;

    if (HasUserPerm(PERM_ANGEL))
        req.angel_uid = usernum;

    if (towrite(fd, &req, sizeof(req)) < 0 ||
        toread(fd, &rpt, sizeof(rpt)) < 0) {
        outs(ERR_PROTOCOL);
    } else if (rpt.cb != sizeof(rpt)) {
        outs(ERR_PROTOCOL2);
    } else {
        prints(
            "\t �{�b�ɶ�: %s\n"
            "\t �t�Τ��w�n�O���ѨϬ� %d ��C\n"
            "\t �ثe�� %d ��ѨϦb�u�W�A�䤤 %d �쯫�٩I�s�����]�w�}��F\n",
            Cdatelite(&now),
            rpt.total_angels,
            rpt.total_online_angels,
            rpt.total_active_angels);

       if (!rpt.inactive_days)
           rpt.inactive_days = ANGEL_INACTIVE_DAYS;

        prints(
            "\t �W�u�ѨϤ��A�֦����D�p�D�H�ƥس̤֬� %d ��A�̦h�� %d ��\n"
            "\t �W�u�B�}�񦬥D�H���ѨϤ��A���D�D�H�̤� %d ��A�̦h %d ��\n"
            "\t ���D�p�D�H�w�q�� %d �Ѥ�������@(�]�t�e��)�p�Ѩ϶ǹL�T��\n",
            rpt.min_masters_of_online_angels,
            rpt.max_masters_of_online_angels,
            rpt.min_masters_of_active_angels,
            rpt.max_masters_of_active_angels,
            rpt.inactive_days);
#ifdef ANGELBEATS_ACTIVE_MASTER_RECORD_START
       int days = (now - ANGELBEATS_ACTIVE_MASTER_RECORD_START) / DAY_SECONDS;
       days += 1;
       if (days < rpt.inactive_days)
           prints("\t (�ѩ��D�p�D�H�O�s�W���έp���ءA"
                   "�ثe��ڥu�� %d �Ѥ����ƾ�)\n", days);
#endif

       prints("\n\t " ANSI_COLOR(1;33) "�����z�H�U�ƾڦ��]�t�z�p�H���O���A"
               "���N���G�i��|���S�����C\n" ANSI_RESET);

#ifdef ANGEL_REPORT_INDEX
        if (HasUserPerm(PERM_ANGEL)) {
            if (currutmp->angelpause != ANGELPAUSE_NONE)
                prints("\t �ѩ�z�ثe�ڦ��p�D�H�ҥH�L�����T\n");
            else if (rpt.my_active_index == 0)
                prints("\t �z���G���䥦�n�J�����Ωڦ��ҥH�ثe�L�Ѩ϶���\n");
            else
                prints("\t �z���u�W�p�Ѩ϶��쬰 %d�C\n"
                       "\t ������i��|�]�䥦�p�ѨϤW�u�Χ��ܩI�s�����ܤj\n",
                       rpt.my_active_index);
        }
#endif
        prints("\t �z�ثe�j���� %d �쬡�D�p�D�H�C\n", rpt.my_active_masters);

        if (rpt.last_assigned_master > 0) {
            // TODO check if last_assigned is already invalid.
            prints("\t �A�̫᦬�쪺�s�p�D�H�O %s (%s)\n",
                   getuserid(rpt.last_assigned_master),
                   Cdatelite(&rpt.last_assigned));
           prints("\n"
                  "\t (�ܦh�s���p�D�H�i��O�~���ά~�Ѩ��`�������|�e�T���A\n"
                  "\t  �p�G�A���ܤ[���S�ݨ�s�D�H���T���A�N�i�H���լ��󤧫e\n"
                  "\t  �|�H�����S���s�D�H�A���O���������褣����)\n");
        }
#ifdef ANGEL_ASSIGN_DOCUMENT
       prints("\n\t �Yı�o�ܤ[���S����s�D�H�i�� [a] ��d�ݤ��t������C\n");
#endif
    }
    close(fd);
#ifdef ANGEL_ASSIGN_DOCUMENT
    if (tolower(pressanykey()) == 'a') {
       more(ANGEL_ASSIGN_DOCUMENT, YEA);
    }
#else
    pressanykey();
#endif
    wait_penalty(1);
    return 0;
}

inline int
angel_reject_me(userinfo_t * uin){
    int* iter = uin->reject;
    int unum;
    while ((unum = *iter++)) {
	if (unum == currutmp->uid) {
            // �W�Ŧn��?
            if (intbsearch(unum, uin->myfriend, uin->nFriends))
                return 0;
	    return 1;
	}
    }
    return 0;
}

static void
angel_display_message(const char *template_fn,
                      const char *message_fn,
                      int skip_lines,
                      int row, int col,
                      int date_row, int date_col) {
    char buf[ANSILINELEN];
    FILE *fp = fopen(message_fn, "rt");
    time4_t ts = dasht(message_fn);

    show_file(template_fn, vgety(), b_lines - vgety(), SHOWFILE_ALLOW_ALL);
    while (skip_lines-- > 0)
        fgets(buf, sizeof(buf), fp);

    while (fgets(buf, sizeof(buf), fp))
    {
	chomp(buf);
        move_ansi(row++, col);
        outs(buf);
    }
    fclose(fp);
    move_ansi(date_row, date_col);
    outs(ANSI_RESET);
    outs(Cdatelite(&ts));
}

enum ANGEL_MSG_FORMAT {
    FORMAT_NICK_MSG = 1,
    FORMAT_PLAIN_MSG,
};

int
angel_edit_msg(const char *prompt, const char *filename,
               enum ANGEL_MSG_FORMAT format) {
    char nick[IDLEN - 6 + 1] = ""; // 6=strlen("�p�Ѩ�")
    char old_nick[IDLEN] = "";
    char msg[3][STRLEN] = {"", "", ""};
    char fpath[PATHLEN];
    char buf[512];
    FILE *fp;
    int i, do_delete_file = 0;

    vs_hdr2(PROMPT_ANGELBEATS, prompt);
    setuserfile(fpath, filename);

    outs("��]�w: \n");
    fp = fopen(fpath, "r");
    if (fp) {
        if (format == FORMAT_NICK_MSG) {
            fgets(buf, sizeof(buf), fp);
            if (strstr(buf, "%%[") == buf) {
                chomp(buf);
                strlcpy(nick, buf + 3, sizeof(nick));
                strlcpy(old_nick, nick, sizeof(old_nick));
                prints(" �ʺ�: %s�p�Ѩ�\n", nick);
            }
        }
        for (i = 0; i < 3; i++) {
            if (!fgets(msg[i], sizeof(msg[i]), fp))
                break;
            outs(" : ");
            outs(msg[i]);
            chomp(msg[i]);
        }

        fclose(fp);
    } else {
        outs("(�ثe�L�]�w)\n");
    }
    mvouts(10, 0, "�s�]�w:\n");
    if (format == FORMAT_NICK_MSG) {
        getdata_buf(11, 0, " �p�Ѩϼʺ١G", nick, sizeof(nick), DOECHO);
        if (!*nick) {
            mvouts(12, 0, "�ťձN�ɭP�R���p�ѨϼʺٻP�T���C\n");
            do_delete_file = 1;
        }
    }
    if (!do_delete_file) {
        mvouts(12, 0, " �s��T�� (�̦h�T��A��[ENTER]����):\n");
        for (i = 0; i < 3; i++) {
            if (!getdata_buf(13 + i, 0, " : ", msg[i], 73, DOECHO)) {
                for (i++; i < 3; i++)
                    msg[i][0] = 0;
                break;
            }
        }
        if (format == FORMAT_PLAIN_MSG && !*msg[0]) {
            mvouts(15, 0, "�ťձN�ɭP�R���p�ѨϰT���C\n");
            do_delete_file = 1;
        }
    }

    if (!getdata(20, 0, "�T�w�x�s�H [y/N]: ", buf, 3, LCECHO) ||
        buf[0] != 'y') {
        return 0;
    }

    if (strcmp(nick, old_nick) != 0) {
        log_filef("log/change_angel_nick.log", LOG_CREAT,
                  "%s %s (%s�p�Ѩ�)�󴫼ʺ٬��u%s�p�Ѩϡv\n",
                  Cdatelite(&now), cuser.userid, old_nick, nick);
    }

    if (do_delete_file) {
        if (dashf(fpath) && remove(fpath) != 0)
            vmsg("�t�ο��~ - �L�k�R���C");
        return 1;
    }

    // write file
    fp = fopen(fpath, "w");
    if (!fp) {
        vmsg("�t�ο��~ - �L�k�g�J�C");
        return 0;
    }
    if (format == FORMAT_NICK_MSG) {
        fputs("%%[", fp);
        fputs(nick, fp);
        fputs("\n", fp);
    }
    for (i = 0; i < 3; i++) {
        fputs(msg[i], fp);
        fputc('\n', fp);
    }

    fclose(fp);
    return 1;
}

int
a_angelmsg(){
    return angel_edit_msg("�s��p�ѨϼʺٻP���u�T��", FN_ANGELMSG,
                          FORMAT_NICK_MSG);
}

int
a_angelmsg2(){
    return angel_edit_msg("�s��p�ѨϩI�s�e���өʯd��", FN_ANGELMSG2,
                          FORMAT_PLAIN_MSG);
}

static int
FindAngel(void){

    int angel_uid = 0;
    userinfo_t *angel = NULL;
    int retries = 2;

    do {
        angel_uid = angel_beats_do_request(ANGELBEATS_REQ_SUGGEST_AND_LINK,
                                           usernum, 0);
        if (angel_uid < 0)  // connection failure
            return -1;
        if (angel_uid > 0)
            angel = search_ulist(angel_uid);
    } while ((retries-- < 1) && !(angel && (angel->userlevel & PERM_ANGEL)));

    if (angel) {
        // got new angel
        pwcuSetMyAngel(angel->userid);
        return 1;
    }

    return 0;
}

#ifdef BN_NEWBIE
static inline void
GotoNewHand(){
    char old_board[IDLEN + 1] = "";
    int canRead = 1;

    if (currutmp && currutmp->mode == EDITING)
	return;

    // usually crashed as 'assert(currbid == brc_currbid)'
    if (currboard[0]) {
	strlcpy(old_board, currboard, IDLEN + 1);
	currboard = ""; // force enter_board
    }

    if (enter_board(BN_NEWBIE) == 0)
	canRead = 1;

    if (canRead)
	Read();

    if (canRead && old_board[0])
	enter_board(old_board);
}
#endif


static inline void
NoAngelFound(const char* msg){
    // don't worry about the screen -
    // it should have been backuped before entering here.

    grayout(0, b_lines-3, GRAYOUT_DARK);
    move(b_lines-4, 0); clrtobot();
    outs(msg_separator);
    move(b_lines-2, 0);
    if (!msg)
	msg = "�A���p�Ѩϲ{�b���b�u�W";
    outs(msg);
#ifdef BN_NEWBIE
    if (currutmp == NULL || currutmp->mode != EDITING)
	outs("�A�Х��b�s��O�W�M�䵪�שΫ� Ctrl-P �o��");
    if (vmsg("�Ы����N���~��A�Y�Q�����i�J�s��O�o��Ы� p") == 'p')
	GotoNewHand();
#else
    pressanykey();
#endif
}

static inline void
AngelNotOnline(){
    char msg_fn[PATHLEN];

    // use cached angel data (assume already called before.)
    // angel_reload_nick();
    if (!_valid_angelmsg)
    {
	NoAngelFound(NULL);
	return;
    }

    // valid angelmsg is ready for being loaded.
    sethomefile(msg_fn, cuser.myangel, FN_ANGELMSG);
    if (dashs(msg_fn) < 1) {
	NoAngelFound(NULL);
	return;
    }

    showtitle("�p�Ѩϯd��", BBSNAME);
    move(2, 0);
    prints("�z��%s�p�Ѩϲ{�b���b�u�W�A͢�d�����A�G\n", _myangel_nick);
    angel_display_message(FN_ANGEL_OFFLINE2, msg_fn, 1, 5, 4, 9, 53);

    // Query if user wants to go to newbie board
    switch(tolower(vmsg("�Q�����ثe�b�u�W���p�ѨϽЫ� h, "
#ifdef BN_NEWBIE
                    "�i�s��O�Ы� p, "
#endif
                    "�䥦���N�����}"))) {
        case 'h':
            move(b_lines - 4, 0); clrtobot();
            do_changeangel(1);
            break;
#ifdef BN_NEWBIE
        case 'p':
            GotoNewHand();
            break;
#endif
    }
}

static void
TalkToAngel(){
    static char AngelPermChecked = 0;
    static userinfo_t* lastuent = NULL;
    userinfo_t *uent;
    int supervisor = 0;
    char msg_fn[PATHLEN];

    if (strcmp(cuser.myangel, "-") == 0){
	NoAngelFound("�A�S���p�Ѩ�");
	return;
    }

    if (HasUserRole(ROLE_ANGEL_CIA))
        supervisor = 1;

    // �Y�ϥΪ̦h���n�J�ä��}�I�s�h�i��|�s�򴫦h�Ӥp�ѨϡC
    // �ҥH !AngelPermChecked �� reload user data.
    // ť�ӫܸ��ݦ��O���G�u�����ϥΪ̷|�o�ˡC
    if (!AngelPermChecked) {
        pwcuReload();
    }
    if (cuser.myangel[0] && !AngelPermChecked) {
	userec_t xuser = {0};
	if (getuser(cuser.myangel, &xuser) < 1 ||
            !(xuser.userlevel & PERM_ANGEL)) {
	    pwcuSetMyAngel("");
#ifdef USE_FREE_ANGEL_FOR_INACTIVE_MASTER
        } else if (!supervisor &&
                   (now - cuser.timeplayangel >
                    ANGEL_INACTIVE_DAYS * DAY_SECONDS)) {
            // Inactive master.
            uent = search_ulist_userid(cuser.myangel);
            if (uent == NULL || angel_reject_me(uent) ||
                uent->angelpause || uent->mode == DEBUGSLEEPING) {
                log_filef("log/auto_change_angel.log", LOG_CREAT,
                          "%s master %s (%d days), angel %s, state (%s)\n",
                          Cdatelite(&now), cuser.userid,
                          (now - cuser.timeplayangel) / DAY_SECONDS,
                          cuser.myangel,
                          !uent ? "not online" : angel_reject_me(uent) ?
                          "reject" : uent->angelpause ? "pause" : "debugsleep");
                pwcuSetMyAngel("");
                angel_beats_do_request(
                    ANGELBEATS_REQ_REMOVE_LINK, usernum,
                    searchuser(cuser.myangel, NULL));
            }
#endif
        }
    }
    AngelPermChecked = 1;

    if (cuser.myangel[0] == 0) {
        int ret = FindAngel();
        if (ret <= 0) {
            lastuent = NULL;
            NoAngelFound(
                (ret < 0) ? "��p�A�ѨϨt�άG�١A�Ц�" BN_BUGREPORT "�^���C" :
                            "�{�b�S���p�ѨϦb�u�W");
            return;
        }
    }

    // now try to load angel data.
    // This MUST be done before calling AngelNotOnline,
    // because it relies on this data.
    angel_reload_nick();

    uent = search_ulist_userid(cuser.myangel);
    if (uent == NULL || (!supervisor && angel_reject_me(uent)) ||
        uent->mode == DEBUGSLEEPING){
	lastuent = NULL;
	AngelNotOnline();
	return;
    }

    // check angelpause: if talked then should accept.
    if (supervisor && uent->angelpause == ANGELPAUSE_REJNEW) {
        // The only case to override angelpause.
    } else if (uent == lastuent) {
	// we've talked to angel.
	// XXX what if uentp reused by other? chance very, very low...
	if (uent->angelpause >= ANGELPAUSE_REJALL)
	{
	    AngelNotOnline();
	    return;
	}
    } else {
	if (uent->angelpause) {
	    // lastuent = NULL;
	    AngelNotOnline();
	    return;
	}
    }

    sethomefile(msg_fn, cuser.myangel, FN_ANGELMSG2);
    if (dashs(msg_fn) > 0) {
        // render per-user message
        move(1, 0);
        clrtobot();
        angel_display_message(FN_ANGEL_USAGE2, msg_fn, 0, 2, 4, 6, 24);
    } else {
        more(FN_ANGEL_USAGE, NA);
    }

    {
	char xnick[IDLEN+1], prompt[IDLEN*2];
	snprintf(xnick, sizeof(xnick), "%s�p�Ѩ�", _myangel_nick);
	snprintf(prompt, sizeof(prompt), "��%s�p�Ѩ�: ", _myangel_nick);
	// if success, record uent.
	if (my_write(uent->pid, prompt, xnick, WATERBALL_ANGEL, uent)) {
	    lastuent = uent;
        }
    }
}

void
CallAngel(){
    static int      entered = 0;
    screen_backup_t old_screen;

    if (entered)
	return;
    entered = 1;
    scr_dump(&old_screen);
    TalkToAngel();
    scr_restore(&old_screen);
    entered = 0;
}

void
pressanykey_or_callangel(){
    int w = t_columns - 2; // see vtuikit.c, SAFE_MAX_COL

    if (!HasBasicUserPerm(PERM_LOGINOK) ||
        strcmp(cuser.myangel, "-") == 0) {
	pressanykey();
	return;
    }

    move(b_lines, 0); clrtoeol();

    // message string length = 38
    outs(VCLR_PAUSE_PAD " ");
    w -= 1 + 38;
    vpad(w / 2, VMSG_PAUSE_PAD);
    outs(VCLR_PAUSE     " �Ы� " ANSI_COLOR(36) "�ť���"
           VCLR_PAUSE     " �~��A�� " ANSI_COLOR(36) "H"
           VCLR_PAUSE     " �I�s�p�ѨϨ�U " VCLR_PAUSE_PAD);
    vpad(w - w / 2, VMSG_PAUSE_PAD);
    outs(" " ANSI_RESET);

    if (tolower(vkey()) == 'h')
        CallAngel();

    move(b_lines, 0);
    clrtoeol();
}

#endif // PLAY_ANGEL
