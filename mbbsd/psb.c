#include "bbs.h"
#include "daemons.h"

// Panty & Stocking Browser
//
// A generic framework for displaying pre-generated data by a simplified
// page-view user interface.
//
// Author: Hung-Te Lin (piaip)
// --------------------------------------------------------------------------
// Copyright (c) 2010 Hung-Te Lin <piaip@csie.ntu.edu.tw>
// All rights reserved.
// Distributed under BSD license (GPL compatible).
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// --------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////
// Constant
#define PSB_EOF (-1)
#define PSB_NA  (-2)
#define PSB_NOP (-3)

///////////////////////////////////////////////////////////////////////////
// Data Structure
typedef struct {
    int curr, total, header_lines, footer_lines;
    int key;
    int allow_pbs_version_message;
    void *ctx;
    int (*header)(void *ctx);
    int (*footer)(void *ctx);
    int (*renderer)(int i, int curr, int total, int rows, void *ctx);
    int (*cursor)(int y, int curr, void *ctx);
    int (*input_processor)(int key, int curr, int total, int rows, void *ctx);
} PSB_CTX;

static int
psb_default_header(void *ctx GCC_UNUSED) {
    vs_hdr2bar("Panty & Stocking Browser", BBSNAME);
    return 0;
}

static int
psb_default_footer(void *ctx GCC_UNUSED) {
    vs_footer(" PSB 1.0 ",
              " (��/��/PgUp/PgDn/0-9)Move (Enter/��)Select \t(q/��)Quit");
    return 0;
}

static int
psb_default_renderer(int i, int curr, int total, int rows GCC_UNUSED, void *ctx GCC_UNUSED) {
    prints("   %s(Demo) %5d / %5d Item\n", (i == curr) ? "*" : " ", i, total);
    return 0;
}

static int
psb_default_cursor(int y GCC_UNUSED, int curr GCC_UNUSED, void * ctx GCC_UNUSED) {
#ifdef USE_PFTERM
    if (HasUserFlag(UF_CURSOR_ASCII))
        outs(STR_CURSOR "\b");
    else
        outs(STR_CURSOR2 "\b");
#else
    // simulate but do not call cursor_show.
    if (HasUserFlag(UF_CURSOR_ASCII)) {
        mvouts(y, 0, STR_CURSOR);
        move(y, 0);
    } else {
        mvouts(y, 0, STR_CURSOR2);
        move(y, 1);
    }
#endif
    return 0;
}

static int
psb_default_input_processor(int key, int curr, int total, int rows, void *ctx GCC_UNUSED) {
    switch(key) {
        case 'q':
        case KEY_LEFT:
            return PSB_EOF;

        case KEY_HOME:
        case '0':
            return 0;

        case KEY_END:
        case '$':
            return total-1;

        case KEY_PGUP:
        case Ctrl('B'):
        case 'N':
            if (curr / rows > 0)
                return curr - rows;
            return 0;

        case KEY_PGDN:
        case Ctrl('F'):
        case 'P':
            if (curr + rows < total)
                return curr + rows;
            return total - 1;

        case KEY_UP:
        case Ctrl('P'):
        case 'p':
        case 'k':
            return (curr > 0) ? curr-1 : curr;

        case KEY_DOWN:
        case Ctrl('N'):
        case 'n':
        case 'j':
            return (curr + 1 < total) ? curr + 1 : curr;

        default:
            if (key >= '0' && key <= '9') {
                int newval = search_num(key, total);
                if (newval >= 0 && newval < total)
                    return newval;
                return curr;
            }
            break;
    }
    return  PSB_NA;
}

static void
psb_init_defaults(PSB_CTX *psbctx) {
    // pre-setup
    assert(psbctx);
    if (!psbctx->header)
        psbctx->header = psb_default_header;
    if (!psbctx->footer)
        psbctx->footer = psb_default_footer;
    if (!psbctx->renderer)
        psbctx->renderer = psb_default_renderer;
    if (!psbctx->cursor)
        psbctx->cursor = psb_default_cursor;

    assert(psbctx->curr >= 0 &&
           psbctx->total >= 0 &&
           psbctx->curr < psbctx->total);
    assert(psbctx->header_lines > 0 &&
           psbctx->footer_lines);
}

int
psb_main(PSB_CTX *psbctx)
{
    psb_init_defaults(psbctx);

    while (1) {
        int i;
        int rows = t_lines - psbctx->header_lines - psbctx->footer_lines;
        int base;

        assert(rows > 0);
        base = psbctx->curr / rows * rows;
        clear();
        SOLVE_ANSI_CACHE();
        psbctx->header(psbctx->ctx);
        for (i = 0; i < rows; i++) {
            move(psbctx->header_lines + i, 0);
            SOLVE_ANSI_CACHE();
            if (base + i < psbctx->total)
                psbctx->renderer(base + i, psbctx->curr, psbctx->total,
                                 rows, psbctx->ctx);
        }
        move(t_lines - psbctx->footer_lines, 0);
        SOLVE_ANSI_CACHE();
        psbctx->footer(psbctx->ctx);
        if (psbctx->allow_pbs_version_message) {
            SOLVE_ANSI_CACHE();
            prints(ANSI_COLOR(0;1;30) "%*s" ANSI_RESET, t_columns-2,
                   "-- Powered by Panty & Stocking Browser System");
        }
        i = psbctx->header_lines + psbctx->curr - base;
        move(i, 0);
        psbctx->cursor(i, psbctx->curr, psbctx->ctx);
        psbctx->key = vkey();

        i = PSB_NA;
        if (psbctx->input_processor)
            i = psbctx->input_processor(psbctx->key, psbctx->curr,
                                        psbctx->total, rows, psbctx->ctx);
        if (i == PSB_NA)
            i = psb_default_input_processor(psbctx->key, psbctx->curr,
                                            psbctx->total, rows, psbctx->ctx);
        if (i == PSB_EOF)
            break;
        if (i == PSB_NOP)
            continue;

        if (i >=0 && i < psbctx->total)
            psbctx->curr = i;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////
// Time Capsule: Edit History

#ifndef PVEH_LIMIT_NUMBER
#define PVEH_LIMIT_NUMBER   (199)
#endif

typedef struct {
    const char *subject;
    const char *filebase;
    int leave_for_recycle_bin;
    int rev_base;
    int base_as_current;
    time4_t *timestamps;
} pveh_ctx;

static int
pveh_header(void *ctx) {
    pveh_ctx *cx = (pveh_ctx*) ctx;
    vs_hdr2barf(" �i" TIME_CAPSULE_NAME ": �s����v�j \t %s", cx->subject);
    move(1, 0);
    outs("�Ъ`�N���t�Τ��|�ä[�O�d�Ҧ����s����v�C");
    outs("\n");
    return 0;
}

static int
pveh_footer(void *ctx GCC_UNUSED) {
    vs_footer(" �s����v ",
              " (����)���� (Enter/r/��)��� (x)�s�J�H�c "
              "(~)" RECYCLE_BIN_NAME
              "\t(q/��)���X");
    move(b_lines-1, 0);
    return 0;
}

static void
pveh_solve_rev_filename(int rev, int i, char *fname, size_t sz_fname,
                        pveh_ctx *cx) {
    if (cx->base_as_current && i == 0)
        strlcpy(fname, cx->filebase, sz_fname);
    else
        timecapsule_get_by_revision(
                cx->filebase, rev + cx->rev_base, fname, sz_fname);
}

static int
pveh_renderer(int i, int curr, int total, int rows GCC_UNUSED, void *ctx) {
    const char *subject = "";
    char fname[PATHLEN];
    time4_t ftime = 0;
    pveh_ctx *cx = (pveh_ctx*) ctx;
    int rev = total - i; // i/curr = 0 based, rev = 1 based

    if (cx->timestamps[i] == 0) {
        pveh_solve_rev_filename(rev, i, fname, sizeof(fname), cx);
        ftime = dasht(fname);
        if (!ftime)
            ftime++;
        cx->timestamps[i] = ftime;
    } else {
        ftime = cx->timestamps[i];
    }

    if (ftime != -1)
        subject = Cdate(&ftime);
    else
        subject = "(�O���w�L�O�d����/�w�M��)";

    prints("   %s%s  ����: ",
           (i == curr) ? ANSI_COLOR(1;41;37) : "",
           (ftime == -1) ? ANSI_COLOR(1;30) : "");
    if (cx->base_as_current && i == 0)
        outs("[�ثe����]");
    else
        prints("#%09d", rev + cx->rev_base);
    prints("  �ɶ�: %-*s" ANSI_RESET "\n", t_columns - 31, subject);
    return 0;
}

static int
pveh_input_processor(int key, int curr, int total, int rows GCC_UNUSED, void *ctx) {
    char fname[PATHLEN];
    pveh_ctx *cx = (pveh_ctx*) ctx;
    int rev = total - curr; // see renderer

    switch (key) {
        case KEY_ENTER:
        case KEY_RIGHT:
        case 'r':
            pveh_solve_rev_filename(rev, curr, fname, sizeof(fname), cx);
            more(fname, YEA);
            return PSB_NOP;

        case '~':
            cx->leave_for_recycle_bin = 1;
            return PSB_EOF;

        case 'x':
            pveh_solve_rev_filename(rev, curr, fname, sizeof(fname), cx);
            {
                char ans[3];
                getdata(b_lines-2, 0, "�T�w�n�⦹�����^�s�ܫH�c��? [y/N]: ",
                        ans, sizeof(ans), LCECHO);
                if (*ans == 'y') {
                    if (mail_log2id(cuser.userid, cx->subject,
                                    fname, RECYCLE_BIN_OWNER, 1, 0) == 0) {
                        vmsg("�x�s�����A�ЦܫH�c�ˬd�Ƨѿ��H��");
                    } else
                        vmsg("�x�s���ѡA�Ц� " BN_BUGREPORT " �ݪO���i�A����");
                }
            }
            return PSB_NOP;
    }
    return PSB_NA;
}

static int
pveh_welcome() {
    // warning screen!
    static char is_first_enter_pveh = 1;

    if (is_first_enter_pveh) {
        is_first_enter_pveh = 0;
        clear();
        move(2, 0);
        outs(ANSI_COLOR(1;31)
"  �w��ϥ� Time Capsule ���s����v�s���t��!\n\n" ANSI_RESET
"  �����z: (1) �Ҧ�����ƶȨѰѦҡA���褣�O�Ҧ��B�����㪺�q�ϰO���C\n\n"
"          (2) �Ҧ�����Ƴ��i�ण�w���Ѩt�βM�����C\n"
"              �L�s����v����N��S���s��L�A�]�i��O�Q�M���F\n\n"
"   Mini FAQ:\n\n"
"   Q: ��ˤ~�|�����v�O�� (�W�[������)?\n"
"   A: �b�t�Χ�s��C���ϥ� E �s��峹�æs�ɴN�|���O���C���夣�|�W�[�O��������\n\n"
"   Q: �q�`���v�|�O�d�h�[?\n"
"   A: �ݪO��3~4�g�A�H�c��2~3�g\n\n"
"   Q: �ɮ׳Q�R�F�]�i�H�ݾ��v��?\n"
"   A: �����٦b�ݪO�W�i�H������<����w�Q�R��>���A���M�N���i"
       RECYCLE_BIN_NAME "(~)�A��\n"
            );
        doupdate();
        pressanykey();
    }
    return 0;
}


int
psb_view_edit_history(const char *base, const char *subject,
                      int maxrev, int current_as_base) {
    pveh_ctx pvehctx = {
        .subject = subject,
        .filebase = base,
        .rev_base = 0,
        .base_as_current = current_as_base,
    };
    PSB_CTX ctx = {
        .curr = 0,
        .total = maxrev + pvehctx.base_as_current,
        .header_lines = 3,
        .footer_lines = 2,
        .allow_pbs_version_message = 1,
        .ctx = (void*)&pvehctx,
        .header = pveh_header,
        .footer = pveh_footer,
        .renderer = pveh_renderer,
        .input_processor = pveh_input_processor,
    };

    pveh_welcome();

    if (maxrev > PVEH_LIMIT_NUMBER) {
        pvehctx.rev_base = maxrev - PVEH_LIMIT_NUMBER;
        ctx.total -= pvehctx.rev_base;
    }

    pvehctx.timestamps = (time4_t*) malloc (sizeof(time4_t) * ctx.total);
    if (!pvehctx.timestamps) {
        vmsgf("�������~�A�Ц�" BN_BUGREPORT "�ݪO���i�A����");
        return FULLUPDATE;
    }
    // load on demand!
    memset(pvehctx.timestamps, 0, sizeof(time4_t) * ctx.total);

    psb_main(&ctx);
    free(pvehctx.timestamps);
    return (pvehctx.leave_for_recycle_bin ?
            RET_RECYCLEBIN :
            FULLUPDATE);
}

///////////////////////////////////////////////////////////////////////////
// Time Capsule: Recycle Bin
#ifndef PVRB_LIMIT_NUMBER
#define PVRB_LIMIT_NUMBER   (103000/10)
#endif

typedef struct {
    const char *dirbase;
    const char *subject;
    int viewbase;
    fileheader_t *records;
} pvrb_ctx;

static int
pvrb_header(void *ctx) {
    pvrb_ctx *cx = (pvrb_ctx*) ctx;
    vs_hdr2barf(" �i" TIME_CAPSULE_NAME ": " RECYCLE_BIN_NAME "�j \t %s",
                cx->subject);
    move(1, 0);
    outs("�Ъ`�N���B���ɮױN���w���M���C\n");
    vbarf(ANSI_REVERSE "    �s�� | �� �� |   �@  ��   |   ��      �D\t");
    return 0;
}

static int
pvrb_footer(void *ctx GCC_UNUSED) {
    vs_footer(" �w�R�ɮ� ",
              " (��/��/PgUp/PgDn)���� (Enter/r/��)��� (/a#n)�j�M (x)�s�J�H�c"
              "\t(q/��)���X");
    move(b_lines-1, 0);
    return 0;
}

static int
pvrb_renderer(int i, int curr, int total, int rows GCC_UNUSED, void *ctx) {
    pvrb_ctx *cx = (pvrb_ctx*) ctx;
    fileheader_t *fh = &cx->records[total - i - 1];

    // TODO make this load-on-demand
    // quick display, but lack of recommend counter...
    outs("   ");
    if (i == curr)
        // prints(ANSI_COLOR(1;40;3%d), i%8);
        outs(ANSI_COLOR(1;40;31));
    prints("%06d  %-5.5s  %-12.12s %s" ANSI_RESET "\n",
           total - i, fh->date, fh->owner, fh->title);
    return 0;
}

static int
pvrb_search(char key, int curr, int total, pvrb_ctx *cx) {
    fileheader_t *fh;
    static char search_str[FNLEN] = "";
    static char search_cmd = 0;
    const char *prompt = "";
    const char *aid_str = NULL;
    aidu_t aidu = 0;
    int need_input = 1;

    if (key == 'n') {
        key = search_cmd;
        if (key) {
            need_input = 0;
            if (curr + 1 < total)
                curr ++;
        } else
            key = '/';
    }

    if (key == '#')
        prompt = "�п�J�峹�N�X: #";
    else if (key == '/')
        prompt = "�п�J���D����r: ";
    else if (key == 'a')
        prompt = "�п�J�@������r: ";
    else {
        assert(!"unknown search command");
        return PSB_NA;
    }

    assert(sizeof(search_str) >= FNLEN);
    if (need_input &&
        getdata(b_lines-1, 0, prompt, search_str, FNLEN, DOECHO) < 1)
        return PSB_NA;

    // cache for next cache.
    search_cmd = key;

    if (key == '#') {
        // AID search is very special, we have to search from begin to end.
        curr = 0;
        aid_str = search_str;
        while (*aid_str == ' ' || *aid_str == '#')
            aid_str++;
        aidu = aidc2aidu(aid_str);
        if (!aidu)
            return PSB_NOP;
    }

    // the records was in reversed ordering
    for (; curr < total; curr++) {
        fh = &cx->records[total - curr - 1];
        if ((key == '/' && DBCS_strcasestr(fh->title, search_str)) ||
            (key == 'a' && DBCS_strcasestr(fh->owner, search_str)) ||
            (key == '#' && fn2aidu(fh->filename) == aidu)) {
            // found something. return as current index.
            return curr;
        }
    }
    return PSB_NOP;
}

static int
pvrb_input_processor(int key, int curr, int total, int rows GCC_UNUSED, void *ctx) {
    char fname[PATHLEN];
    int maxrev;
    pvrb_ctx *cx = (pvrb_ctx*) ctx;
    fileheader_t *fh = &cx->records[total - curr - 1];
    const char *err_no_rev = "��p�A������v��Ƥw�Q�t�βM���C";

    switch (key) {
        case 'x':
            setdirpath(fname, cx->dirbase, fh->filename);
            maxrev = timecapsule_get_max_revision_number(fname);
            if (maxrev < 1) {
                vmsg(err_no_rev);
            } else {
                char revfname[PATHLEN];
                char ans[3];
                timecapsule_get_by_revision(
                        fname, maxrev, revfname, sizeof(revfname));
                getdata(b_lines-2, 0, "�T�w�n�⦹�����^�s�ܫH�c��? [y/N]: ",
                        ans, sizeof(ans), LCECHO);
                if (*ans == 'y') {
                    if (mail_log2id(cuser.userid, fh->title,
                                    revfname, RECYCLE_BIN_OWNER,
                                    1, 0) == 0) {
                        vmsg("�x�s�����A�ЦܫH�c�ˬd�Ƨѿ��H��");
                    } else {
                        vmsg("�x�s���ѡA�Ц� " BN_BUGREPORT " �ݪO���i�A����");
                        return PSB_EOF;
                    }
                }
            }
            return PSB_NOP;

        case '#':
        case '/':
        case 'a':
        case 'n':
            {
                int newloc = pvrb_search(key, curr, total, cx);
                if (newloc >= 0)
                    return newloc;
                if (newloc == PSB_NOP)
                    vmsg("�䤣��ŦX����ơC");
            }
            return PSB_NOP;

        case KEY_ENTER:
        case KEY_RIGHT:
        case 'r':
            setdirpath(fname, cx->dirbase, fh->filename);
            maxrev = timecapsule_get_max_revision_number(fname);
            if (maxrev == 1) {
                char revfname[PATHLEN];
                timecapsule_get_by_revision(
                        fname, 1, revfname, sizeof(revfname));
                more(revfname, YEA);
            } else if (maxrev > 1) {
                psb_view_edit_history(fname, fh->title, maxrev, 0);
            } else {
                vmsg(err_no_rev);
            }
            return PSB_NOP;
    }
    return PSB_NA;
}

static int
pvrb_welcome() {
    // warning screen!
    static char is_first_enter_pvrb = 1;

    if (is_first_enter_pvrb) {
        is_first_enter_pvrb = 0;
        clear(); SOLVE_ANSI_CACHE();
        move(2, 0);
        outs(ANSI_COLOR(1;36)
"  �w��ϥ� " TIME_CAPSULE_NAME " " RECYCLE_BIN_NAME "!\n\n" ANSI_RESET
"  �����z: (1) �Ҧ�����ƶȨѰѦҡA���褣�O�Ҧ��B�����㪺�q�ϰO���C\n"
"          (2) �Ҧ�����Ƴ��i�ण�w���Ѩt�βM�����C\n"
"          (3) �ثe����²������V�j�M: / a # ���O�i�j�M���D/�@��/"
              "AID�峹�N�X\n"
"              n �i�H��U�@�ӲŦX�j�M������ (�ȵL�V�e�j�M)\n" ANSI_RESET
"   Mini FAQ:\n\n"
"   Q: �n��^�ӤH�H�c�u�·�! �����򤣯�@����K�I?\n"
"   A: �^�����O���Ѥֶq�ϴ��άd��ΡA��N�@�����K��j�q�^�_�H�קK�ݥΡC\n\n"
"   Q: �q�`�ɮ׷|�O�d�h�[?\n"
"   A: �ݪO��3~4�g�A�H�c��2~3�g�A�t�~�g�Ƥ]���W��(�̨t�έt���ʺA�վ�)�C\n\n"
"   Q: ���Ǧa�観�^�����i��?\n"
"   A: �ثe�}��ӤH�H�c(�Ҧ��Τ�)��ݪO/��ذϤ峹(�O�D���w)�C\n"
"      ��ذϤl�ؿ��Ȥ��䴩�C\n\n"
            );
        doupdate();
        pressanykey();
    }
    return 0;
}

int
psb_recycle_bin(const char *base, const char *title) {
    int nrecords = 0, viewbase = 0;
    pvrb_ctx pvrbctx = {
        .dirbase = base,
        .subject = title,
    };
    PSB_CTX ctx = {
        .curr = 0,
        .total = 0, // maxrev + pvrbctx.base_as_current,
        .header_lines = 3,
        .footer_lines = 2,
        .allow_pbs_version_message = 1,
        .ctx = (void*)&pvrbctx,
        .header = pvrb_header,
        .footer = pvrb_footer,
        .renderer = pvrb_renderer,
        .input_processor = pvrb_input_processor,
    };

    nrecords = timecapsule_get_max_archive_number(base, sizeof(fileheader_t));
    if (!nrecords) {
        vmsg("�ثe" RECYCLE_BIN_NAME "���L���󤺮e�C");
        return FULLUPDATE;
    }

    pvrb_welcome();

    // truncate on large size
    if (nrecords > PVRB_LIMIT_NUMBER) {
        viewbase = nrecords - PVRB_LIMIT_NUMBER;
        nrecords -= viewbase;
    }
    ctx.total = nrecords;

    pvrbctx.records = (fileheader_t*) malloc (sizeof(fileheader_t) * nrecords);
    if (!pvrbctx.records) {
        vmsgf("�������~�A�Ц�" BN_BUGREPORT "�ݪO���i�A����");
        return FULLUPDATE;
    }
    timecapsule_get_archive_blobs(base, viewbase, nrecords, pvrbctx.records,
                                  sizeof(fileheader_t));
    psb_main(&ctx);
    free(pvrbctx.records);
    return DIRCHANGED;
}

///////////////////////////////////////////////////////////////////////////
// Comment Management

#ifdef USE_COMMENTD
typedef struct {
    void *cmctx;
} pvcm_ctx;

static int
pvcm_header(void *ctx GCC_UNUSED) {
    vs_hdr2barf(" �i����޲z�j\t");
    move(1, 0);
    vbarf(ANSI_REVERSE "  %-6s|%-12.12s|%s\t", "�s ��", " �@  �� ", " ��  �e\t");
    return 0;
}

static int
pvcm_footer(void *ctx GCC_UNUSED) {
    vs_footer(" ���� ",
              " (��/��/PgUp/PgDn)���� (d)�R�� (U)�ֳt����\t(q/��)���X");
    move(b_lines-1, 0);
    return 0;
}

static int
pvcm_renderer(int i, int curr, int total GCC_UNUSED, int rows GCC_UNUSED, void *ctx) {
    pvcm_ctx *cx = (pvcm_ctx*) ctx;
    const CommentBodyReq *resp = CommentsRead(cx->cmctx, i);
    if (!resp)
        return 0;
    prints("%c %06d %-12.12s %s\n",
           (i == curr) ? '>' : ' ',
           i + 1,
           resp->userid,
           (resp->type >= 0) ? resp->msg : (ANSI_COLOR(0;30;47) "<�w�R>" ANSI_RESET));
    return 0;
}

static int
pvcm_input_processor(int key, int curr, int total GCC_UNUSED, int rows GCC_UNUSED, void *ctx) {
    pvcm_ctx *cx = (pvcm_ctx*) ctx;

    switch(key) {
        case KEY_DEL:
        case 'd':
            do {
                // See comments.c for max length of reason.
                char reason[40];
                const CommentBodyReq *resp = CommentsRead(cx->cmctx, curr);
                if (!resp || resp->type < 0)
                    break;
                if (!getdata(b_lines-2, 0, "�п�J�R����]: ",
                            reason, sizeof(reason), DOECHO))
                    break;
                if (vans("�T�w�n�R���ܡH (y/N) ") == 'y') {
                    if (CommentsDeleteFromTextFile(cx->cmctx, curr, reason)
                        != 0) {
                        vmsg("�R�����ѡC�i����w�Q�ק�C");
                    }
                }

            } while(0);

            return PSB_NOP;

        case 'U':
            do {
                const CommentKeyReq *key = CommentsGetKeyReq(cx->cmctx);
                const CommentBodyReq *resp = CommentsRead(cx->cmctx, curr);
                if (!resp)
                    break;
                edit_user_acl_for_board(resp->userid, key->board);
            } while(0);

            return PSB_NOP;
    }
    return PSB_NA;
}

static int
pvcm_welcome() {
    clear();
    vs_hdr2("�R������", "����ĵ�i");
    move(2, 0);
    // This must be a outs because we have '%' inside.
    outs(ANSI_COLOR(1;31)
"  �o�O���礤���R����ɭ��C\n\n" ANSI_RESET
"  �����z: (1) �R����ɭ���ܪ����e�O�Ӧ۩�W�ߪ���Ʈw�A�ҥH���|��\n"
"              ��@�̭פ尲�y���夺�e�����D�C���]�]���A�Y�����Q�ק�\n"
"              �ϱo���e���P��(�άO������)�h���ɭ��N�L�k�R���C\n\n"
"          (2) �R����|�q�ɮ׫e���}�l��ݰ_�ӧ@�̸򤺤�ۦP���Ĥ@���C\n"
"              �ثe�S��k100%�T�{��쥿�T����m�A���_�X����O�ۦP���C\n\n"
        "");
    pressanykey();
    return 0;
}

int
psb_comment_manager(const char *board, const char *file) {
    pvcm_ctx pvcmctx = {
        NULL,
    };
    PSB_CTX ctx = {
        .curr = 0,
        .total = 0,
        .header_lines = 2,
        .footer_lines = 2,
        .allow_pbs_version_message = 0,
        .ctx = (void*)&pvcmctx,
        .header = pvcm_header,
        .footer = pvcm_footer,
        .renderer = pvcm_renderer,
        .input_processor = pvcm_input_processor,
    };
    pvcmctx.cmctx = CommentsOpen(board, file);
    if (!pvcmctx.cmctx) {
        vmsg("�t�ο��~�A�Ц� " BN_BUGREPORT " ���i�C");
        return FULLUPDATE;
    }
    ctx.total = CommentsGetCount(pvcmctx.cmctx);
    if (ctx.total){
        pvcm_welcome();
        psb_main(&ctx);
    } else {
        vmsg("���峹�L�����ơC");
    }
    CommentsClose(pvcmctx.cmctx);
    return DIRCHANGED;
}
#endif

///////////////////////////////////////////////////////////////////////////
// Admin Edit

// Since admin edit is usually rarely used, no need to write dynamic allocation
// for it.
#ifndef MAX_PAE_ENTRIES
#define MAX_PAE_ENTRIES (256)
#endif

typedef struct {
    char *descs[MAX_PAE_ENTRIES];
    char *files[MAX_PAE_ENTRIES];
} pae_ctx;

static int
pae_header(void *ctx GCC_UNUSED) {
    vs_hdr2bar(" �i�t���ɮסj ", "  �s��t���ɮ�");
    outs("�п���n�s�誺�ɮ׫�� Enter �}�l�ק�\n");
    vbarf(ANSI_REVERSE
         "%5s %-36s%-30s", "�s��", "�W  ��", "��  �W");
    return 0;
}

static int
pae_footer(void *ctx GCC_UNUSED) {
    vs_footer(" �s��t���ɮ� ",
              " (����/0-9)���� (Enter/e/r/��)�s�� (DEL/d)�R�� \t(q/��)���X");
    move(b_lines-1, 0);
    return 0;
}

static int
pae_renderer(int i, int curr, int total GCC_UNUSED, int rows GCC_UNUSED, void *ctx) {
    pae_ctx *cx = (pae_ctx*) ctx;
    prints("  %3d %s%s%-36.36s " ANSI_COLOR(1;37) "%-30.30s" ANSI_RESET "\n",
            i+1,
            (i == curr) ? ANSI_COLOR(41) : "",
            dashf(cx->files[i]) ? ANSI_COLOR(1;36) : ANSI_COLOR(1;30),
            cx->descs[i], cx->files[i]);
    return 0;
}

static int
pae_input_processor(int key, int curr, int total GCC_UNUSED, int rows GCC_UNUSED, void *ctx) {
    int result;
    pae_ctx *cx = (pae_ctx*) ctx;

    switch(key) {
        case KEY_DEL:
        case 'd':
            if (vansf("�T�w�n�R�� %s �ܡH (y/N) ", cx->descs[curr]) == 'y')
                unlink(cx->files[curr]);
            vmsgf("�t���ɮ�[%s]: %s", cx->files[curr],
                  !dashf(cx->files[curr]) ?  "�R�����\\ " : "���R��");
            return PSB_NOP;

        case KEY_ENTER:
        case KEY_RIGHT:
        case 'r':
        case 'e':
        case 'E':
            result = veditfile(cx->files[curr]);
            // log file change
            if (result != EDIT_ABORTED)
            {
                log_filef("log/etc_edit.log",
                          LOG_CREAT,
                          "%s %s %s # %s\n",
                          Cdate(&now),
                          cuser.userid,
                          cx->files[curr],
                          cx->descs[curr]);
            }
            vmsgf("�t���ɮ�[%s]: %s",
                  cx->files[curr],
                  (result == EDIT_ABORTED) ?  "������" : "��s����");
            return PSB_NOP;
    }
    return PSB_NA;
}

int
psb_admin_edit() {
    int i;
    char buf[PATHLEN*2];
    FILE *fp;
    pae_ctx paectx = { {0}, };
    PSB_CTX ctx = {
        .curr = 0,
        .total = 0,
        .header_lines = 4,
        .footer_lines = 2,
        .allow_pbs_version_message = 1,
        .ctx = (void*)&paectx,
        .header = pae_header,
        .footer = pae_footer,
        .renderer = pae_renderer,
        .input_processor = pae_input_processor,
    };

    fp = fopen(FN_CONF_EDITABLE, "rt");
    if (!fp) {
	// you can find a sample in sample/etc/editable
	vmsgf("���]�w�i�s���ɮצC��[%s]�A�Ь��t�ί����C", FN_CONF_EDITABLE);
	return 0;
    }

    // load the editable file.
    // format: filename [ \t]* description
    while (ctx.total < MAX_PAE_ENTRIES &&
           fgets(buf, sizeof(buf), fp)) {
        char *k = buf, *v = buf;
        if (!*buf || strchr("#./ \t\n\r", *buf))
            continue;

        // change \t to ' '.
        while (*v) if (*v++ == '\t') *(v-1) = ' ';
        v = strchr(buf, ' ');
        if (v == NULL)
            continue;

        // see if someone is trying to crack
        k = strstr(buf, "..");
        if (k && k < v)
            continue;

	// reject anything outside etc/ folder.
        if (strncmp(buf, "etc/", strlen("etc/")) != 0)
            continue;

        // adjust spaces
        chomp(buf);
        k = buf; *v++ = 0;
        while (*v == ' ') v++;
        trim(k);
        trim(v);

        // add into context
        paectx.files[ctx.total] = strdup(k);
        paectx.descs[ctx.total] = strdup(v);
        ctx.total++;
    }
    if (ctx.total >= MAX_PAE_ENTRIES)
        vmsg("�`�N: �z���t�γ]�w�w�W�L�α���w�]�W���A�Ь��t�ί����[�j�]�w");

    psb_main(&ctx);

    for (i = 0; i < ctx.total; i++) {
        free(paectx.files[i]);
        free(paectx.descs[i]);
    }
    return 0;
}

