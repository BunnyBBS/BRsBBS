#include "bbs.h"
#include "daemons.h"

#define QUOTE(x) #x
#define EXPAND_AND_QUOTE(x) QUOTE(x)
#define STR_ANGELBEATS_PERF_MIN_PERIOD \
        EXPAND_AND_QUOTE(ANGELBEATS_PERF_MIN_PERIOD)
#define die(format...) { fprintf(stderr, format); exit(1); }

#define REPORT_AUTHOR   "[�ѨϤ��|]"
#define REPORT_SUBJECT  "�p�Ѩϲέp���"

#ifndef PLAY_ANGEL
int main(){ return 0; }
#else

void slurp(FILE* to, FILE* from) {
    char buf[4096]; // 4K block
    int count;

    while ((count = fread(buf, 1, sizeof(buf), from)) > 0) {
	char * p = buf;
	while (count > 0) {
	    int i = fwrite(p, 1, count, to);

	    if (i <= 0) return;

	    p += i;
	    count -= i;
	}
    }
}

void appendLogFile(FILE *output,
                   const char *filename,
                   const char *prefix,
                   int delete_file) {
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return;
    if (delete_file)
        remove(filename);

    fputs(prefix, output);
    slurp(output, fp);
    fclose(fp);
}

typedef struct {
    int uid;
    int is_angel;
    int masters;
    int masters_week;
    int masters_month;
    int masters_quarter;
    int masters_season;
    int masters_period;
} AngelRecord;

int buildMasterInfo(AngelRecord *rec, int num_recs) {
    userec_t user;
    int uid = 0, angel_uid;
    FILE *fp;
    int count = 0;
    time4_t now = time4(NULL);

    memset(rec, 0, sizeof(*rec) * num_recs);

    fp = fopen(FN_PASSWD, "rb");
    while (fread(&user, sizeof(user), 1, fp) == 1) {
        AngelRecord *r = rec + uid, *angel = NULL;
        uid++;
        r->uid = uid;
        assert(uid <= num_recs);
        if (!*user.userid)
            continue;
        if (user.role & ROLE_ANGEL_ACTIVITY)
            continue;
        if (user.userlevel & PERM_ANGEL) {
            r->is_angel = 1;
            count++;
        }
        if (!*user.myangel)
            continue;
        angel_uid = searchuser(user.myangel, NULL);
        if (!angel_uid)
            continue;
        angel = rec + (angel_uid - 1);
        angel->masters++;
        if (now - user.timeplayangel < DAY_SECONDS * 7)
            angel->masters_week++;
        if (now - user.timeplayangel < DAY_SECONDS * 30)
            angel->masters_month++;
        if (now - user.timeplayangel < DAY_SECONDS * 90)
            angel->masters_quarter++;
        if (now - user.timeplayangel < DAY_SECONDS * 120)
            angel->masters_season++;
        if (now - user.timeplayangel < DAY_SECONDS * 180)
            angel->masters_period++;
    }
    fclose(fp);
    return count;
}

int sortAngelRecord(const void *pb, const void *pa) {
    const AngelRecord *a = (const AngelRecord *)pa,
          *b = (const AngelRecord *)pb;
    assert(a->is_angel == b->is_angel);
    if (a->masters_month != b->masters_month)
        return a->masters_month - b->masters_month;
    return a->masters - b->masters;
}

int generateReport(FILE *fp, AngelRecord *rec, int num_recs, int delete_file) {
    time4_t t = time4(NULL);
    int i;

    fprintf(fp, "�@��: %s\n���D: %s\n�ɶ�: %s\n",
            REPORT_AUTHOR, REPORT_SUBJECT, ctime4(&t));

    fprintf(fp, "�{�b�����p�ѨϦ� %d ��:\n", num_recs);
    fprintf(fp,
            " (�᭱�Ʀr�������p�D�H�� |  7�Ѥ� | 30�Ѥ� | 90�Ѥ� |  120�� |  180��\n"
            "  �����D�p�D�H��(�b�Ӭq�ɶ������ǰe�T�������@�p�ѨϪ��D�H)\n"
	    "  ���`�N�ثe���D�p�D�H�Ȳέp�u�D�H���e�T���v�A�L�k�o���p�Ѩ�\n"
	    "  �O�_���� - �ҥH�аt�X��d���G�����C)\n");
    for (i = 0; i < num_recs; i++)
        fprintf(fp, "%15s | %6d | %6d | %6d | %6d | %6d | %6d\n",
                getuserid(rec[i].uid),
                rec[i].masters,
                rec[i].masters_week,
                rec[i].masters_month,
                rec[i].masters_quarter,
                rec[i].masters_season,
		rec[i].masters_period
		);
    fputs("\n", fp);

    appendLogFile(fp, "log/angel_perf.txt",
                  "\n== ���P�p�ѨϬ��ʸ�ưO�� ==\n"
                  " (����: Start �O�}�l�έp���ɶ�\n"
                  "        Duration �O�X��έp�@��\n"
                  "        Sample �����O�C���έp�ɤѨϦb�u�W������\n"
                  "        Pause1 �����O Sample �����X�����٩I�s���]����\n"
                  "        Pause2 �����O Sample �����X�����٩I�s���]����)\n",
                  delete_file);

    appendLogFile(fp, "log/change_angel_nick.log",
                  "\n== ���P�p�Ѩϼʺ��ܧ�O�� ==\n",
                  delete_file);

    appendLogFile(fp, "log/changeangel.log",
                  "\n== ���P�󴫤p�ѨϰO�� ==\n",
                  delete_file);

    fprintf(fp, "\n--\n  ����ƥ�%s%s�۰ʲ���\n\n", BBSNAME, REPORT_AUTHOR);
    return 0;
}

void usage(const char *myname) {
    fprintf(stderr, "Usage: %s [-m user][-b board]\n", myname);
    exit(1);
}

int main(int argc, char *argv[]){
    AngelRecord *rec, *angels;
    int count, i, iangel;
    int uid, bid;
    const char *myname = argv[0];
    const char *target = NULL;
    int target_is_user = 0;
    char target_name[PATHLEN];
    char output_path[PATHLEN] = "", output_dir[PATHLEN] = "";
    struct fileheader_t fhdr;
    FILE *fp = stdout;

    chdir(BBSHOME);
    attach_SHM();

    while (argc > 2) {
        if (strcmp(argv[1], "-m") == 0) {
            target = argv[2];
            target_is_user = 1;
            argc -= 2, argv += 2;
            if ((uid = searchuser(target, target_name)) < 1)
                die("Invalid user: %s\n", target);
            target = target_name;
            sethomepath(output_path, target);
            sethomedir(output_dir, target);
        } else if (strcmp(argv[1], "-b") == 0) {
            boardheader_t *bp;
            target = argv[2];
            target_is_user = 0;
            argc -= 2, argv += 2;
            if ((bid = getbnum(target)) < 1)
                die("Invalid board: %s\n", target);
            bp = getbcache(bid);
            strlcpy(target_name, bp->brdname, sizeof(target_name));
            target = target_name;
            setbpath(output_path, target);
            setbfile(output_dir, target, ".DIR");
        } else {
            usage(myname);
        }
    }
    if (argc != 1)
        usage(myname);

    if (target) {
        stampfile(output_path, &fhdr);
        fp = fopen(output_path, "wt");
        if (!fp)
            die("Failed to create: %s\n", output_path);
    }

    rec = (AngelRecord *)malloc(sizeof(AngelRecord) * MAX_USERS);
    assert(rec);
    count = buildMasterInfo(rec, MAX_USERS);
    // TODO remove expired angels.

    angels = (AngelRecord *)malloc(sizeof(AngelRecord) * count);
    assert(angels);
    for (i = 0, iangel = 0; i < MAX_USERS; i++) {
        if (!rec[i].is_angel)
            continue;
        memcpy(angels + iangel, rec + i, sizeof(AngelRecord));
        iangel++;
    }
    qsort(angels, count, sizeof(AngelRecord), sortAngelRecord);
    generateReport(fp, angels, count, target ? 1 : 0);

    if (target) {
        fclose(fp);
        strlcpy(fhdr.title, REPORT_SUBJECT, sizeof(fhdr.title));
        strlcpy(fhdr.owner, REPORT_AUTHOR, sizeof(fhdr.owner));
        append_record(output_dir, &fhdr, sizeof(fhdr));

        if (target_is_user) {
            userinfo_t *uentp = search_ulistn(uid, 1);
            if (uentp)
                uentp->alerts |= ALERT_NEW_MAIL;
        } else {
            touchbtotal(bid);
        }
    }
    return 0;
}

#endif /* defined PLAY_ANGEL */
