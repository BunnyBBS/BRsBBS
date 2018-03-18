#include "bbs.h"

// Access Control List
// Holder of all access / permission related stuff
//
// Author: Hung-Te Lin (piaip)

///////////////////////////////////////////////////////////////////////////
// Bakuman: New banning system (store in user home)

#define BAKUMAN_OBJECT_TYPE_USER     'u'
#define BAKUMAN_OBJECT_TYPE_BOARD    'b'
#define BAKUMAN_REASON_LEN          (BTLEN)

static void
bakuman_make_tag_filename(const char *who, const char *object, char object_type,
                          size_t szbuf, char *buf, int create_folder) {
    char prefix[3] = { object_type, '_', 0 };

    if (!who)
        setuserfile(buf, FN_BANNED "/");
    else
        sethomefile(buf, who, FN_BANNED "/");
    if (create_folder && !dashd(buf)) {
        Mkdir(buf);
    }
    strlcat(buf, prefix, szbuf);
    strlcat(buf, object, szbuf);
}

static int
bakuman_get_info(const char *filename,
                 time4_t *pexpire, size_t szreason, char *reason) {
    FILE *fp = fopen(filename, "rt");
    int ts = 0;
    char buf[STRLEN];

    if(!fp)
        return 0;

    // banned file format:
    //  EXPIRE_TIME
    //  REASON
    buf[0] = 0;
    fgets(buf, sizeof(buf), fp);
    if (pexpire) {
        sscanf(buf, "%u", &ts);
        *pexpire = (time4_t) ts;
    }
    buf[0] = 0;
    fgets(buf, sizeof(buf), fp);
    if (szreason && reason) {
        chomp(buf);
        strlcpy(reason, buf, szreason);
    }
    fclose(fp);
    return 1;
}

static int
bakuman_set_info(const char *filename, time4_t expire, const char *reason) {
    FILE *fp = fopen(filename, "wt");
    if (!fp)
        return 0;
    if (!reason)
        reason = "";

    fprintf(fp, "%u\n%s\n", (unsigned)expire, reason);
    fclose(fp);
    return 1;
}

// return if 'who (NULL=self)' is banned by 'object'
static time4_t
is_banned_by(const char *who, const char *object, char object_type,
             size_t szreason, char *reason) {
    char tag_fn[PATHLEN];
    time4_t expire = 0;

    bakuman_make_tag_filename(who, object, object_type,
                              sizeof(tag_fn), tag_fn, 0);
    if (!dashf(tag_fn))
        return 0;

    // check expire
    if (bakuman_get_info(tag_fn, &expire, szreason, reason) && now > expire) {
        unlink(tag_fn);
        return 0;
    }
    return expire;
}

// add 'who (NULL=self)' to the ban list of 'object'
static int
ban_user_as(const char *who, const char *object, char object_type,
            time_t expire, const char *reason) {
    char tag_fn[PATHLEN];

    bakuman_make_tag_filename(who, object, object_type,
                              sizeof(tag_fn), tag_fn, 1);
    return bakuman_set_info(tag_fn, expire, reason);
}

int
get_user_banned_status_by_board(const char *user, const char *board,
                                size_t szreason, char *reason) {
    return is_banned_by(user, board, BAKUMAN_OBJECT_TYPE_BOARD,
                        szreason, reason);
}

time4_t
is_user_banned_by_board(const char *user, const char *board) {
    return is_banned_by(user, board, BAKUMAN_OBJECT_TYPE_BOARD, 0, NULL);
}

time4_t
is_banned_by_board(const char *board) {
    return is_user_banned_by_board(cuser.userid, board);
}

int
ban_user_for_board(const char *user, const char *board,
                   time4_t expire, const char *reason) {
    return ban_user_as(user, board, BAKUMAN_OBJECT_TYPE_BOARD, expire, reason);
}

int
unban_user_for_board(const char *user, const char *board) {
    char tag_fn[PATHLEN];

    bakuman_make_tag_filename(user, board, BAKUMAN_OBJECT_TYPE_BOARD,
                             sizeof(tag_fn), tag_fn, 0);
    return unlink(tag_fn) == 0;
}

static time4_t
ui_print_user_banned_status_for_board(const char *uid, const char *board) {
    char reason[STRLEN];
    time4_t expire = expire = get_user_banned_status_by_board(
        uid, board, sizeof(reason), reason);

    if (expire > now) {
        prints("�Ȱ��ϥΪ� %s �o���A�Ѱ��ɶ��|�� %d ��: %s\n�z��:%s",
               uid, (expire-now)/DAY_SECONDS+1,
               Cdatelite(&expire), reason);
    } else {
        prints("�ϥΪ� %s �ثe���b�T���W�椤�C\n",
               uid);
        expire = 0;
    }
    return expire;
}

static int
ui_ban_user_for_board(const char *uid, const char *board) {
    time4_t expire = now;
    int y, x;
    int result;
    char ans[3];
    char history_log[PATHLEN];
    char reason[STRLEN];
    char datebuf[STRLEN];

    setbfile(history_log, board, FN_BANNED_HISTORY);

    getyx(&y, &x);
    if ((expire = is_user_banned_by_board(uid, board))) {
        vmsgf("�ϥΪ̤��e�w�Q�T���A�|�� %d �ѡF�Ա��i��(S)��(L)�d��",
              (expire - now) / DAY_SECONDS+1);
        return -1;
    }
    prints("�N�ϥΪ� %s �[�J�ݪO %s ���T���W��C", uid, board);
    move(y+3, 0);
    syncnow();
    outs("�ثe�������榡�O [�Ʀr][���]�C "
         "��즳: �~(y), ��(m), ��(d)\n"
         "�d��: 3m (�T�Ӥ�), 120d (120��), 10y (10�~)\n"
         "�`�N���i�V�X��J(��:�S���T�ӥb��o�تF��,�д��⦨�Ѽ�)\n"
        );
    getdata(y+1, 0, "�ХH�Ʀr����(�w�]����)��J����: ",
            datebuf, 8, DOECHO);
    trim(datebuf);
    if (!*datebuf) {
        vmsg("����J�����A���C");
        return -1;
    } else {
        int val = atoi(datebuf);
        uint64_t long_now;
        switch(tolower(datebuf[strlen(datebuf)-1])) {
            case 'y':
                val *= 365;
                break;
            case 'm':
                val *= 30;
                break;
            case 'd':
            default:
                break;
        }
        if (val < 1) {
            vmsg("����榡��J���~�άO�p��@�ѵL�k�B�z�C");
            return -1;
        }
        long_now = (uint64_t)now + val * (uint64_t)DAY_SECONDS;
        expire = long_now;
        if ((uint64_t)expire != long_now) {
            vmsg("����L�j�εL�k�B�z�A�Э��s��J�C");
            return -1;
        }
        move(y+3, 0); clrtobot();
        // sprintf(datebuf, "%s", Cdatelite(&expire));
        sprintf(datebuf, "%d ��", val);
        prints("�����N�]�w�� %s����: %s\n",
               datebuf, Cdatelite(&expire));
        if (val > KEEP_DAYS_REGGED) {
            mvprints(y+6, 0, ANSI_COLOR(1;31)
                     "�`�N: �W�L %d �Ѫ��]�w���i��]�����@��"
                     "���W���ӾɭP�b���L���Q���s���U�A\n"
                     "      ���ɦP�W���s�b���ѩ󤣤@�w�O�P�@�H�ҥH"
                     "���|�Q�T���C\n" ANSI_RESET,
                     KEEP_DAYS_REGGED);
        }
    }

    assert(sizeof(reason) >= BAKUMAN_REASON_LEN);
    mvouts(y+5, 0, ANSI_COLOR(1;31) "�Ъ`�N���z�ѷ|�g�ѫH��q���ϥΪ̡A���Ӥ]�i��|���}��ݪO�W�C");
    // maybe race condition here, but fine.
    getdata(y+4, 0, "�п�J�z��(�ťեi�����s�W): ",
            reason, BAKUMAN_REASON_LEN, DOECHO);
    if (!*reason) {
        vmsg("����J�z�ѡA���������]�w");
        return -1;
    }

    move(y, 0); clrtobot();
    prints("\n�ϥΪ� %s �Y�N�[�J�T���W�� (����: %s)\n"
           "�z��: %s\n"
           ANSI_COLOR(1;32) "�|�H�H�q���ϥΪ�" ANSI_RESET "\n",
           uid, datebuf, reason);

    // last chance
    getdata(y+5, 0, "�T�{�H�W��ƥ������T�ܡH [y/N]: ",
            ans, sizeof(ans), LCECHO);
    if (ans[0] != 'y') {
        vmsg("�Э��s��J");
        return -1;
    }

    result = ban_user_for_board(uid, board, expire, reason);
    log_filef(history_log, LOG_CREAT,
              ANSI_COLOR(1) "%s %s" ANSI_COLOR(33) "%s" ANSI_RESET
              " �Ȱ� " ANSI_COLOR(1;31) "%s" ANSI_RESET
              " �o���A������ %s\n  �z��: %s\n",
              Cdatelite(&now),
              result ? "" :
              ANSI_COLOR(0;31)"[�t�ο��~] "ANSI_COLOR(1),
              cuser.userid, uid,  datebuf, reason);
    vmsg(result ? "�w�N�ϥΪ̥[�J�T���W��" : "���ѡA�ЦV�������i");
    if (result) {
        char xtitle[STRLEN];
        char anti_pettifogger[] =
            "�Ъ`�N�A\n"
            ANSI_COLOR(1;31) "�z�ثe�ϥΪ������A�ȤD�����L�v����,\n"
            "�����i�H�ɰ����z���ѥ������K�O�A��.\n" ANSI_RESET
            "\n"
            "�̷ӱz�L�h�w�˦ۦP�N���ϥΪ̱���,"
            "�z�w�P�N�����O�D���޲z���Ҷq,\n"
            "�o�N�z���b���]�w����ݪO�A��(����μȰ��o��)�B\n"
            "�R���z�o���S�w�峹,\n"
            "�H�ΰh�^�z�o���S�w�峹�C\n"
            "\n"
            "���D�����P�z���������Y,\n"
            "����μȰ��o���]�w�B�R��]�w�B�h��]�w��,\n"
            ANSI_COLOR(1;31)
            "�D�����̥��ƫ����פ������L�v���Ѥ��z���A��,\n"
            ANSI_RESET
            "�O�D�Ҭ������]�w�D���L�v�εL�G,\n"
            "���L�Ѧ��ߦD�k��ê�q���ϥθo�B���M�V�d�o�B�����o�C\n"
            "\n"
            "�Ф��ݦ洣�i�C\n";
        char xmsg[STRLEN*5 + sizeof(anti_pettifogger)];

        snprintf(xtitle, sizeof(xtitle), "%s �ݪO�Ȱ��o���q��(����)", board);
        snprintf(xmsg, sizeof(xmsg),
                 "%s �ݪO�w�Ȯɰ������z�o��N���C\n"
                 "�}�l�ɶ�: %s (���� %s�A��������ɶ��A�D��l�ǳW�ɶ�)\n"
                 "��]: %s\n"
                 "�䥦��T�Ь��ӬݪO�O�W�P���i�C\n\n%s",
                 board, Cdatelite(&now), datebuf, reason, anti_pettifogger);
        mail_log2id_text(uid, xtitle, xmsg, "[�t�γq��]", 1);
        sendalert(uid, ALERT_NEW_MAIL);
    }
    invalid_board_permission_cache(board);
    return 0;
}

static int
ui_unban_user_for_board(const char *uid, const char *board) {
    time4_t expire = now;
    int y, x;
    char ans[3];
    char history_log[PATHLEN];
    char reason[STRLEN];

    setbfile(history_log, board, FN_BANNED_HISTORY);

    getyx(&y, &x);
    if (!(expire = is_user_banned_by_board(uid, board))) {
        vmsg("�ϥΪ̥��b�T���W��C");
        return -1;
    }
    move(y, 0); clrtobot();
    prints("���e�Ѱ��ϥΪ� %s ��ݪO %s ���T������ (�|�� %d ��)�C",
           uid, board, (expire-now)/DAY_SECONDS+1);
    assert(sizeof(reason) >= BAKUMAN_REASON_LEN);
    getdata(y+1, 0, "�п�J�z��(�ťեi�����Ѱ�): ",
            reason, BAKUMAN_REASON_LEN, DOECHO);
    if (!*reason) {
        vmsg("����J�z�ѡA���������]�w");
        return -1;
    }

    // last chance
    getdata(y+4, 0, "�T�{�H�W��ƥ������T�ܡH [y/N]: ",
            ans, sizeof(ans), LCECHO);
    if (ans[0] != 'y') {
        vmsg("�Э��s��J");
        return -1;
    }

    unban_user_for_board(uid, board);
    log_filef(history_log, LOG_CREAT,
              ANSI_COLOR(1) "%s " ANSI_COLOR(33) "%s" ANSI_RESET
              " �Ѱ� " ANSI_COLOR(1;32) "%s" ANSI_RESET
              " ���T������ (�Z������|�� %d ��)\n  �z��: %s\n",
              Cdatelite(&now), cuser.userid, uid,
              (expire - now) / DAY_SECONDS+1, reason);
    vmsg("�ϥΪ̪��T������w�Ѱ��A�̱ߦܸӨϥΪ̭��s�W����ͮ�");
    invalid_board_permission_cache(board);
    return 0;
}

int
edit_banned_list_for_board(const char *board) {
    // TODO generalize this.
    char uid[IDLEN+1], ans[3];

    if (!board || !*board || getbnum(board) < 1)
        return 0;

    while (1) {
        clear();
        vs_hdr2f(" Bakuman �v���]�w�t�� \t"
                 " �ݪO: %s �A����: ����o��(����)�A�W��W��: ��", board);
        move(3, 0);
        outs(ANSI_COLOR(1)
        "                   �w��ϥ� Bakuman �v���]�w�t��!\n\n" ANSI_RESET
        "      ���t�δ��ѤU�C�\\��:" ANSI_COLOR(1;33)
                                 " - �L�H�ƤW�����W��]�w\n"
        "                          - �۰ʥͮĪ��ɮĭ���\n"
        "                          - �W�椺�±b���L�����s���U�ɦ۰ʥ���\n\n"
        ANSI_RESET
#if 1
        ANSI_COLOR(1;32)
        "   �����z: ���t�Ϊ��]�p�õL�N�z�A�O�D����O�Ȫ��覡�A���Y�O�D���S�O��\n"
        "           �ϥΤ�k (�p: �]�w�񤽧i�����ɭ��A�άO�W�w�n�t��H��~�o�Ѱ�)\n"
        "           �Цۦ�P�O�ͤά����էO���q�n�é��w�O�W�C�Y�]���y���ϥΪ̧�ĳ\n"
        "           �Ϊ�ĳ�ɫ�G�Цۭt�C\n"
        ANSI_RESET
#endif
        "   �p����: ���¨t�εy�����P�A�s�t�Τ��|(�]�L�k)�C�X [�{�b�W�椺�����ǤH]�A\n"
        "           ���Ĩ����O�]��K�z+�O����������: (�i�Ѿ��v�O���ۦ����)\n"
        "         - ���ɥ�(A)�s�W�ó]�n������N���ΦA�h�޳]�F���ǤH\n"
        "         - ���D�Q���e�Ѱ��εo�{�]���A���ɥi��(D)���R���M��A��(A)���s�]�w\n"
        "         - �Q�T�{�O�_�]���άd�Y�ӨϥΪ̬O���O���b�T�����A�i��(S)���ˬd\n"
        "           �t�~�]�i��(L)�ݳ]�w���v�O�� (���O����h�W�t�Τ��|�M��)\n"
        "         - �ثe�S��[�ä[�T��]���]�w�A�Y���ݭn�г]�� 10�~�� 20�~\n"
        "         - �ثe�s�W/�Ѱ����|�H�H�q���A�t�~�Ъ`�N" ANSI_COLOR(1;33)
                   "�b���Q���T���|�۰ʸѰ�\n" ANSI_RESET
        "         - �T���۰ʸѰ����|�X�{�b�O���̡A�u����ʴ��e�Ѱ����~�|\n"
ANSI_COLOR(1) "         - �Q�d�ݬY�ϥΪ̬���Q�T���i��(S)�άO(L)�A�� / �j�M\n"
        ANSI_RESET
#ifdef WATERBAN_UPGRADE_TIME_STR
        // enable and change this if you've just made an upgrade
        ANSI_COLOR(0;32)
        " �t�Χ�s��T: ���t�αҥήɤw��Ҧ��z��b�¤���W�檺�b�������]�W�F\n"
        "               " WATERBAN_UPGRADE_TIME_STR "������A���S���O���b(L)���C��̭��C�z�i�H�Ѧ�(O)���¦W��\n"
        "               �ݬݦ��S���Q�ק諸�����A�M��Q��(D)��(A)�ӽվ�C\n"
        ANSI_COLOR(1;31) "               �`�N�¤��� ID �����ܹL�j�p�g���L�k�ഫ�A�Ф�ʭ��]\n"
        ANSI_RESET
#endif
        "");

        getdata(1, 0, "(A)�W�[ (D)���e�M�� (S)���o�ثe���A "
                      "(L)�C�X�]�w���v "
#ifdef SHOW_OLD_BAN
                      "(O)�˵��¤��� "
#endif
                      "[Q]����? ",
                ans, 2, LCECHO);
        move(2, 0); clrtobot();
        if (*ans == 'q' || !*ans)
            break;

        switch (*ans) {
            case 's':
                move(1, 0);
                usercomplete(msg_uid, uid);
                if (!*uid || !searchuser(uid, uid))
                    continue;
                move(1, 0); clrtobot();
                ui_print_user_banned_status_for_board(uid, board);
                pressanykey();
                break;

            case 'a':
                move(1, 0);
                usercomplete(msg_uid, uid);
                if (!*uid || !searchuser(uid, uid))
                    continue;
                move(1, 0); clrtobot();
                if (ui_ban_user_for_board(uid, board) < 0)
                    continue;
                break;

            case 'd':
                move(1, 0);
                usercomplete(msg_uid, uid);
                if (!*uid || !searchuser(uid, uid))
                    continue;
                move(1, 0);
                if (ui_unban_user_for_board(uid, board) < 0)
                    continue;
                break;

            case 'l':
                do {
                    char history_log[PATHLEN];
                    setbfile(history_log, board, FN_BANNED_HISTORY);
                    if (more(history_log, YEA) == -1)
                        vmsg("�ثe�|�L�]�w�O���C");
                } while (0);
                break;

#ifdef SHOW_OLD_BAN
            case 'o':
                {
                    char old_log[PATHLEN];
                    setbfile(old_log, board, fn_water);
                    if (dashf(old_log)) {
                        vmsg("�Ъ`�N: ������ƶȨѰѦҡA�P�{�b��ڸT���W�槹���S�����Y�C");
                        more(old_log, YEA);
                    } else {
                        vmsg("�L�¤����ơC");
                    }
                }
                break;
#endif

            default:
                break;
        }
    }
    return 0;
}

int
edit_user_acl_for_board(const char *uid, const char *board) {
    time4_t expire;
    int finished = 0;

#define LNACLEDIT (10)

    int ytitle = b_lines - LNACLEDIT;
    grayout(0, ytitle-2, GRAYOUT_DARK);

    while (!finished) {
        move(ytitle-1, 0); clrtobot();
        outs("\n" ANSI_REVERSE);
        vbarf(" �]�w�ϥΪ� %s ��ݪO�m%s�n���v��", uid, board);

        move(ytitle+2, 0);
        expire = ui_print_user_banned_status_for_board(uid, board);

        move(ytitle+5, 0);
        prints(" " ANSI_COLOR(1;36) "%s" ANSI_RESET " - %s\n",
               expire ? "u" : "w",
               expire ? "���e�Ѱ�" : "�[�J�T���W��");

        switch (vans("�п�ܱ��i�椧�ާ@, �䥦�䵲��: ")) {
            case 'w':
                if (expire) {
                    finished = 1;
                    break;
                }
                move(ytitle-1, 0); clrtobot();
                outs("\n" ANSI_REVERSE);
                vbarf(" �T���ϥΪ�");
                move(ytitle+2, 0);
                if (ui_ban_user_for_board(uid, board) < 0)
                    continue;
                break;

            case 'u':
                if (!expire) {
                    finished = 1;
                    break;
                }
                move(ytitle-1, 0); clrtobot();
                outs("\n" ANSI_REVERSE);
                vbarf(" ���e�Ѱ��T��");
                move(ytitle+2, 0);
                if (ui_unban_user_for_board(uid, board) < 0)
                    continue;
                break;

            default:
                finished = 1;
                break;
        }
    }
    return 0;
}
