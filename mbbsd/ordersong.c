#include "bbs.h"

#define lockreturn(unmode, state)  if(lockutmpmode(unmode, state)) return
#define lockreturn0(unmode, state) if(lockutmpmode(unmode, state)) return 0
#define lockbreak(unmode, state)   if(lockutmpmode(unmode, state)) break
#define SONGBOOK  "etc/SONGBOOK"
#define OSONGPATH "etc/SONGO"

#ifndef ORDERSONG_MIN_NUMPOST
#define ORDERSONG_MIN_NUMPOST   (3)
#endif
#ifndef ORDERSONG_MAX_BADPOST
#define ORDERSONG_MAX_BADPOST   (1)
#endif
//#ifndef ORDERSONG_MIN_NUMLOGINDAYS
//#define ORDERSONG_MIN_NUMLOGINDAYS   (30)
//#endif
#ifndef ORDERSONG_PAYMONEY
#define ORDERSONG_PAYMONEY   (100)
#endif

#define MAX_SONGS (MAX_ADBANNER-100) // (400) XXX MAX_SONGS should be fewer than MAX_ADBANNER.

static void sortsong(void);

static int
do_order_song(void)
{
    char            sender[IDLEN + 1], receiver[IDLEN + 1] = "", buf[200],
		    genbuf[200], filename[256], say[51];
    char            trans_buffer[PATHLEN];
    char            address[IDLEN+1];
    FILE           *fp, *fp1;
    fileheader_t    mail;
    int             nsongs;
    char save_title[STRLEN];
    const char *override_receiver = NULL;

#if defined(ORDERSONG_MAX_BADPOST) && defined(ASSESS)
    if (cuser.badpost > ORDERSONG_MAX_BADPOST) {
        vmsgf("���קK�ݥΡA�d���e�Х������h��O���� %d �g�H�U",
                ORDERSONG_MAX_BADPOST);
        return 0;
    }
#endif
	
#ifdef ORDERSONG_MIN_NUMLOGINDAYS
    if (cuser.numlogindays < ORDERSONG_MIN_NUMLOGINDAYS) {
        vmsgf("���קK�ݥΡA�d���e�n����%s %d %s",
                STR_LOGINDAYS, ORDERSONG_MIN_NUMLOGINDAYS, STR_LOGINDAYS_QTY);
        return 0;
    }
#endif

    strlcpy(buf, Cdatedate(&now), sizeof(buf));
    lockreturn0(OSONG, LOCK_MULTI);
    pwcuReload();

    /* Jaky �@�H�@���I�@�� */
    if (!strcmp(buf, Cdatedate(&cuser.lastsong)) && !HasUserPerm(PERM_SYSOP)) {
	move(22, 0);
	vmsg("�A���Ѥw�g�d�L���o�A���ѦA�ӧa....");
	unlockutmpmode();
	return 0;
    }

    while (1) {
	char ans[4];
	clear();
	vs_hdr2(" " BBSMNAME2 "�W�� ", " �I���ʺA");
	prints("�˷R�� %s �w��Ө�߱��I���d���t��\n\n", cuser.userid);
	outs(ANSI_COLOR(1) "�`�N�߱��I�����e�ФůA����| �H������ �T��"
	     "���M�V�d ����\n"
	     "�Y���W�z�H�W���ΡA����N�O�d�M�w�O�_���}���e���v�Q\n"
         "�B�H�W�̱N�����ΦW�O�@(�� ID �i�Q���G�󤽶}�ݪO)\n"
	     "�p���P�N�Ы� Q ���}�C" ANSI_RESET "\n");
	getdata(6, 0,"�}�l�d��(Y)  ���I�q��(S)  ���}[Q] ", ans, sizeof(ans), LCECHO);

	if (ans[0] == 'y'){
	    break;
	}else if (ans[0] == 's') {
	    a_menu("�d���d��", SONGBOOK, 0, 0, NULL, NULL);
	    clear();
	}else if (ans[0] == 'q') {
	    unlockutmpmode();
	    return 0;
    }else{
	    unlockutmpmode();
	    return 0;
	}
	}

    getdata_str(7, 0, "�d����(�i�ΦW): ", sender, sizeof(sender), DOECHO,
                cuser.userid);

#ifdef USE_ANGEL_SONG
    override_receiver = angel_order_song(receiver, sizeof(receiver));
#endif

    if (!*receiver)
        getdata(8, 0, "�d����(�i�ΦW): ", receiver, sizeof(receiver), DOECHO);

    do {
	getdata(9, 0, "�Q�n�n��L/�o��..:", say, sizeof(say), DOECHO);
	reduce_blank(say, say);
	if (!say[0]) {
	    bell();
	    mvouts(10, 0, "�Э��s��J�Q�������e");
	}
    } while (!say[0]);

    snprintf(save_title, sizeof(save_title), "%s:%s", sender, say);

    if (override_receiver) {
        *address = 0;
    } else do {
        getdata_str(11, 0, "�H��֪��H�c(�����u��ID)?",
                    address, sizeof(address), LCECHO, receiver);
        if (!*address)
            break;
        if (searchuser(address, address)) {
            if (is_rejected(address)) {
                vmsg("���ڦ�");
                continue;
            } else {
                break;
            }
        }
        vmsg("�п�J���� ID �Ϊ��� ENTER");
    } while (1);

#ifdef ORDERSONG_PAYMONEY
	move(b_lines-2, 0);
	prints("�Y�N��I�I���O�� %d %s", ORDERSONG_PAYMONEY, MONEYNAME);
	getdata(b_lines - 1, 0, "�T�w�n��I�F�ܡH (y/N)",genbuf, 3, LCECHO);
	if (genbuf[0] != 'y') {
	    unlockutmpmode();
	    return 0;
	}

	reload_money();
	if (cuser.money < (int)ORDERSONG_PAYMONEY){
		vmsg(MONEYNAME "����ú���I���O��...");
		unlockutmpmode();
		return 0;
	}else{
		pay((int)ORDERSONG_PAYMONEY, "ú���I���O�ΡC");
#endif

    vmsg("���ۭn��d���o...");
    a_menu("�d���d��", SONGBOOK, 0, 0, trans_buffer, NULL);
    if (!trans_buffer[0] || strstr(trans_buffer, "home") ||
	strstr(trans_buffer, "boards") || !(fp = fopen(trans_buffer, "r"))) {
	unlockutmpmode();
	return 0;
    }
#ifdef DEBUG
    vmsg(trans_buffer);
#endif
    strlcpy(filename, OSONGPATH, sizeof(filename));
    stampfile(filename, &mail);
    unlink(filename);

    if (!(fp1 = fopen(filename, "w"))) {
	fclose(fp);
	unlockutmpmode();
	return 0;
    }
    strlcpy(mail.owner, "[�߱��I��]", sizeof(mail.owner));
    snprintf(mail.title, sizeof(mail.title), "�� %s �d���� %s ", sender,
             receiver);

    while (fgets(buf, sizeof(buf), fp)) {
	char           *po;
	if (!strncmp(buf, "���D: ", 6)) {
	    clear();
	    move(10, 10);
	    outs(buf);
	    pressanykey();
	    fclose(fp);
	    fclose(fp1);
	    unlockutmpmode();
	    return 0;
	}
	while ((po = strstr(buf, "<~Src~>"))) {
	    const char *dot = "";
	    if (is_validuserid(sender) && strcmp(sender, cuser.userid) != 0)
		dot = ".";
	    po[0] = 0;
	    snprintf(genbuf, sizeof(genbuf), "%s%s%s%s", buf, sender, dot, po + 7);
	    strlcpy(buf, genbuf, sizeof(buf));
	}
	while ((po = strstr(buf, "<~Des~>"))) {
            const char *r = receiver;
#ifdef PLAY_ANGEL
            if (strstr(po, "�p�Ѩ�") && strstr(receiver, "�p�Ѩ�") &&
                override_receiver) {
                r = override_receiver;
            }
#endif
	    po[0] = 0;
	    snprintf(genbuf, sizeof(genbuf), "%s%s%s", buf, r, po + 7);
	    strlcpy(buf, genbuf, sizeof(buf));
	}
	while ((po = strstr(buf, "<~Say~>"))) {
	    po[0] = 0;
	    snprintf(genbuf, sizeof(genbuf), "%s%s%s", buf, say, po + 7);
	    strlcpy(buf, genbuf, sizeof(buf));
	}
	fputs(buf, fp1);
    }
    fclose(fp1);
    fclose(fp);

    log_filef("etc/osong.log",  LOG_CREAT,
              "id: %-12s �� %s �d���� %s : \"%s\", ��H�� %s\n",
              cuser.userid, sender, receiver, say, address);

    LOG_IF(LOG_CONF_OSONG_VERBOSE,
           log_filef("log/osong_verbose.log",  LOG_CREAT,
                     "%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
                     Cdate(&now), cuser.userid, trans_buffer, address,
                     sender, receiver, say));

#ifdef USE_ANGEL_SONG
    if (override_receiver)
        angel_log_order_song(override_receiver);
#endif

    if (append_record(OSONGPATH "/" FN_DIR, &mail, sizeof(mail)) != -1) {
	pwcuSetLastSongTime(now);
	/* Jaky �W�L MAX_ADBANNER ���q�N�}�l�� */
	// XXX ���J�����Ƿ|���o���O:
	// 3. �� <�t��> �ʺA�ݪO   SYSOP [01/23/08]
	// 4. �� <�I�q> �ʺA�ݪO   Ptt   [08/26/09]
	// 5. �� <�s�i> �ʺA�ݪO   SYSOP [08/22/09]
	// 6. �� <�ݪO> �ʺA�ݪO   SYSOP [04/16/09]
	// �ѩ��I�q������O�����J���A���ઽ���� MAX_ADBANNER ���M�᭱���S�o���C
	nsongs = get_num_records(OSONGPATH "/" FN_DIR, sizeof(mail));
	if (nsongs > MAX_SONGS) {
	    // XXX race condition
	    delete_range(OSONGPATH "/" FN_DIR, 1, nsongs - MAX_SONGS);
	}
	snprintf(genbuf, sizeof(genbuf), "%s says \"%s\" to %s.",
		sender, say, receiver);
	log_usies("OSONG", genbuf);
    }
    snprintf(save_title, sizeof(save_title), "%s:%s", sender, say);
    hold_mail(filename, receiver, save_title);
    if (*address) {
	bsmtp(filename, save_title, address, NULL);
    }

    clear();
    outs(
	 "\n\n  ���߱z�����o...\n"
	 "  �@�p�ɤ��ʺA�ݪO�|�۰ʭ��s��s�A\n"
	 "  �j�a�N�i�H�ݨ�z���߱��I���d���o�I\n\n"
	 "  ��������D�i�H�� " BN_NOTE " �O����ذϧ䵪�סA\n"
	 "  �]�i�b " BN_NOTE " �O��ذϬݨ�ۤv���d���O���C\n"
	 "  �������_�Q���N���]�w��� " BN_NOTE " �ݪO���X�A\n"
	 "  ���ˤ����O�D���z�A�ȡC\n");
    pressanykey();
    sortsong();
    topsong();

    unlockutmpmode();
    return 1;
#ifdef ORDERSONG_PAYMONEY
    }
#endif
}

int
ordersong(void)
{
    do_order_song();
    return 0;
}

// topsong

#define QCAST     int (*)(const void *, const void *)

typedef struct songcmp_t {
    char            name[100];
    char            cname[100];
    int             count;
}               songcmp_t;


static int
count_cmp(songcmp_t * b, songcmp_t * a)
{
    return (a->count - b->count);
}

int
topsong(void)
{
    more(FN_TOPSONG, YEA);
    return 0;
}


static void
sortsong(void)
{
    FILE           *fo, *fp = fopen(BBSHOME "/" FN_USSONG, "r");
    songcmp_t       songs[MAX_SONGS + 1];
    int             n;
    char            buf[256], cbuf[256];
    int totalcount = 0;

    memset(songs, 0, sizeof(songs));
    if (!fp)
	return;
    if (!(fo = fopen(FN_TOPSONG, "w"))) {
	fclose(fp);
	return;
    }
    totalcount = 0;
    /* XXX: ���F�e MAX_SONGS ��, �ѤU���|�Ƨ� */
    while (fgets(buf, 200, fp)) {
	chomp(buf);
	strip_blank(cbuf, buf);
	if (!cbuf[0] || !isprint2((int)cbuf[0]))
	    continue;

	for (n = 0; n < MAX_SONGS && songs[n].name[0]; n++)
	    if (!strcmp(songs[n].cname, cbuf))
		break;
	strlcpy(songs[n].name, buf, sizeof(songs[n].name));
	strlcpy(songs[n].cname, cbuf, sizeof(songs[n].cname));
	songs[n].count++;
	totalcount++;
    }
    qsort(songs, MAX_SONGS, sizeof(songcmp_t), (QCAST) count_cmp);
    fprintf(fo,
	    "    " ANSI_COLOR(36) "�w�w" ANSI_COLOR(37) "�W��" ANSI_COLOR(36) "�w�w�w�w�w�w" ANSI_COLOR(37)
	    "�d��" ANSI_COLOR(36) "�w�w�w�w�w�w�w�w�w�w�w" ANSI_COLOR(37) "����" ANSI_COLOR(36)
	    "�w�w" ANSI_COLOR(32) "�@%d��" ANSI_COLOR(36) "�w�w" ANSI_RESET "\n", totalcount);
    for (n = 0; n < 100 && songs[n].name[0]; n++) {
	fprintf(fo, "      %5d. %-38.38s %4d " ANSI_COLOR(32) "[%.2f]" ANSI_RESET "\n", n + 1,
		songs[n].name, songs[n].count,
		(float)songs[n].count / totalcount);
    }
    fclose(fp);
    fclose(fo);
}
