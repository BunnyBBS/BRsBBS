#include "bbs.h"

#ifdef USE_MISSION

/* �C��n�J�j�� */
int mission_dailylogin(){
    FILE *fp;
    char buf[200], buf2[200], date[11], genbuf[3];
    int i;
    struct tm      ptime;
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	snprintf(date, sizeof(date), "%03d-%02d-%02d", ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday);
	
	clear();
	vs_hdr2("���Ȥ��� ", " �C��n�J�j��");
	move(2,0);
	outs("���Ȼ����G\n"
		 "�@�w�n�C��W�j�߳� :)\n\n");
	outs("�ѥ[���G����\n");
	outs("���ȼ��y�G�C�� 100 " MONEYNAME "\n");
	
	setuserfile(buf, "mission.dailylogin");
	if(isFileExist(buf) == false){
		if (!(fp = fopen(buf, "w"))){
			vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-1-F01)");
			return 0;
		}
		fprintf(fp,"%s", date);
		fclose(fp);
		pay(-100, "�������ȡG�C��n�J�j�ߡC");
		mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"���ߧ������ȡI"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		if (!(fp = fopen(buf, "r"))){
			vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-1-F02)");
			return 0;
		}
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		if (!strcmp(buf2, date)){
			mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"�w�g�����L���ȡC"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if (!(fp = fopen(buf, "w"))){
				vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-1-F03)");
				return 0;
			}
			fprintf(fp,"%s", date);
			fclose(fp);
			pay(-100, "�������ȡG�C��n�J�j�ߡC");
			mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"���ߧ������ȡI"ANSI_RESET);
			pressanykey();
			return 0;
		}
	}
}

/* �ק�ۤv���H�c */
int mission_email(){
    FILE *fp;
    char buf[200], buf2[200], genbuf[3];
	
	clear();
	vs_hdr2("���Ȥ��� ", " �ק�ۤv���H�c");
	move(2,0);
	outs("���Ȼ����G\n"
		 "��Я��ɵ��U��{�������ǭק�A�������O�{���ק�ɨS�����աA\n"
		 "�ɭP��ӨϥΪ̵��U�����M����g�q�l�H�c�����|�Q�t�ά����A\n"
		 "�o�����~��ӳQ�o�{�A�]�b1.5����s���ץ��F�o�����~�C\n"
		 "���F���ϥΪ̸�Ƨ���A�ڭ̹��y�ϥΪ̦ۥD�ק�q�l�H�c�C\n\n");
	outs("�ѥ[���G�b�{���ץ��e���U�A�S���Q������H�c���ϥΪ�\n");
	outs("���ȼ��y�G250 " MONEYNAME "\n");
	outs("���ȫ��ܡG�b(U)ser �ӤH�]�w�ϡ�(I)nfo �ӤH��Ƴ]�w ���ק�\n");
	
	setuserfile(buf, "mission.email");
	if(isFileExist(buf) == false){
		if(strcmp(cuser.email, "x")){
			mvouts(b_lines - 2, 24, ANSI_COLOR(1;33)"�q�l�H�c�������`�A����ѥ[���C"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			getdata(12, 0, "���U�o�ӥ��ȶܡH (y)�O [N]�_ ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				return 0;
			}
			if (!(fp = fopen(buf, "w"))){
				vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-M-F01)");
				return 0;
			}
			now = time(NULL);
			fprintf(fp,"%s", Cdate(&now));
			fclose(fp);
			vmsg("���U�����o��");
			return 0;
		}
	}else{
		if (!(fp = fopen(buf, "r"))){
			vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-M-F02)");
			return 0;
		}
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		if (!strcmp(buf2, "complete")){
			mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"�w�g�����L���ȡC"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if(strcmp(cuser.email, "x")){
				if (!(fp = fopen(buf, "w"))){
					vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-M-F03)");
					return 0;
				}
				fprintf(fp,"complete");
				fclose(fp);
				pay(-250, "�������ȡG�ק�ۤv���H�c�C");
				mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"���ߧ������ȡI"ANSI_RESET);
				pressanykey();
				return 0;
			}else{
				mvouts(b_lines - 2, 35, "���ȶi�椤");
				pressanykey();
				return 0;
			}
		}
	}
}

int mission_1stanniv(){
    FILE *fp;
    char buf[200], buf2[200], date[11], genbuf[3];
    int i, dateint, start = 1071210, end = 1071216;
    struct tm      ptime;
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	snprintf(date, sizeof(date), "%03d%02d%02d", ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday);
	dateint = atoi(date);
	
	clear();
	vs_hdr2("���Ȥ��� ", " �j�ߤ@�g�~���y�j���]");
	move(2,0);
	outs("���Ȼ����G\n"
		 "�j�ߤ@�g�~���y�j���]�I\n"
		 "�٨̵}�O�o�h�~��12��10��ܡH���O�j�߯����ͤ�I���ߤj�ߥ����}���@�g�~�F�I�o�@�~\n"
		 "�Ӥj�ߤ]�P��}�����˻����Ҥ��P�F�A�n�X��j�߰���@�_���Ԫ��ѯСA�j�ߤw�g�֭p�F\n"
		 "�W�L5000�g�峹�F�A�{�b���j�ߪY�Y�V�a�A�o�ǳ��O�j�a�@�_�g�U�����v�I\n"
		 "�P�³o�@�~�Ӥj�a��j�ߪ��R�@�P����A���A�̤j�ߤ~���@�����U�h���ʤO�I\n\n");
	outs("�ѥ[���G�Ҧ�����@�P�w�y�I\n");
	outs("���ȼ��y�G5000 " MONEYNAME "\n");
	outs("���ȶ}�l�G107�~12��10�� 00:00\n");
	outs("���ȴ����G107�~12��16�� 23:59\n");
	
	if(dateint < start){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;33)"�٨S�}�l�����I"ANSI_RESET);
		pressanykey();
		return 0;
	}else if(dateint > end){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"�W�L��������F�I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		setuserfile(buf, "mission.1stanniv");
		if(isFileExist(buf) == false){
			if (!(fp = fopen(buf, "w"))){
				vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-1ST-F01)");
				return 0;
			}
			fprintf(fp,"complete");
			fclose(fp);

			pay(-5000, "�S����y�G�j�ߤ@�g�~���y�j���]�C");
			mvouts(b_lines - 2, 28, ANSI_COLOR(1;32) "���§A����j�ߨ��L�o�@�~" ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if (!(fp = fopen(buf, "r"))){
				vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-1ST-F02)");
				return 0;
			}
			fgets(buf2, sizeof(buf2), fp);
			fclose(fp);
			if (!strcmp(buf2, "complete")){
				mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"�w�g�����L���ȡC"ANSI_RESET);
				pressanykey();
				return 0;
			}else{
				if (!(fp = fopen(buf, "w"))){
					vmsg("�t�ο��~�A�еy��A�աC(Error code: MIS-1ST-F01)");
					return 0;
				}
				fprintf(fp,"complete");
				fclose(fp);

				pay(-5000, "�S����y�G�j�ߤ@�g�~���y�j���]�C");
				mvouts(b_lines - 2, 28, ANSI_COLOR(1;32) "���§A����j�ߨ��L�o�@�~" ANSI_RESET);
				pressanykey();
				return 0;
			}
		}
	}
}

/* ���ȦC�� */
int mission_main()
{
	int i;
    char genbuf[3];

VIEWLIST:
	clear();
	vs_hdr2(" " BBSNAME " ", " ���Ȥ���");
	move(1,0);
	vbarf(ANSI_REVERSE " �s��  ���ȼ��D\t���y \n");
	vbarf("   1.  �C��n�J�j��\t100 " MONEYNAME "\n");
	vbarf("   2.  �ק�ۤv���H�c\t250 " MONEYNAME "\n");
	vbarf("   3.  �j�ߤ@�g�~���y�j���]\t5000 " MONEYNAME "\n");
	
	getdata(b_lines - 1, 0, "��J���Ƚs���˵��Բӻ����B�}�l���ȩλ�����y�A��J[Q]���} ",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		mission_dailylogin();
		goto VIEWLIST;
	}
	if (genbuf[0] == '2') {
		mission_email();
		goto VIEWLIST;
	}
	if (genbuf[0] == '3') {
		mission_1stanniv();
		goto VIEWLIST;
	}
	
    return 0;
}

#endif
