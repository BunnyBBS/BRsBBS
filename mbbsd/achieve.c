#include "bbs.h"

#ifdef USE_ACHIEVE

int
doUserOwnAch(char *achieve){
	int count=0,owned=0;
    char buf[200], buf2[200];
    FILE *fp;

	setuserfile(buf, "achieve");
	if(fp = fopen(buf, "r")){
		while (fgets(buf2, sizeof(buf2), fp)) {
			if(buf2[strlen(buf2) - 1] == '\n')
				buf2[strlen(buf2) - 1] = '\0';
			if(strcmp(achieve, buf2) == 0)
				owned = 1;
			count++;
		}
		fclose(fp);
	}

	return owned;
}

const char *
getAchName(char *achieve, bool noColor){
    FILE *fp;
    char *name, *color, buf[200] = "", buf2[200] = "", output[200] = "";

    if(noColor == false){
		snprintf(buf, sizeof(buf), "achieve/%s.color", achieve);
		if(fp = fopen(buf, "r")){
			fgets(buf, sizeof(buf), fp);
			fclose(fp);
			if(buf[strlen(buf) - 1] == '\n')
				buf[strlen(buf) - 1] = '\0';
			color = buf;
		}else{
			noColor = true;
			color = "";
		}
	}

	snprintf(buf2, sizeof(buf2), "achieve/%s.name", achieve);
	if(fp = fopen(buf2, "r")){
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		if(buf2[strlen(buf2) - 1] == '\n')
			buf2[strlen(buf2) - 1] = '\0';
		name = buf2;
	}else
		name = achieve;

    if(noColor == false)
		snprintf(output, sizeof(output), "\[%sm%s\[m", color, name);
	else
		snprintf(output, sizeof(output), "%s", name);

	return output;
}

const char *
getAchDesc(char *achieve){
    FILE *fp;
    char buf[200] = "";

	snprintf(buf, sizeof(buf), "achieve/%s.desc", achieve);
	if(fp = fopen(buf, "r")){
		fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		return buf;
	}else{
		return NULL;
	}
}

const char *
getAchAttr(char *achieve){
    FILE *fp;
    char buf[200] = "";

	snprintf(buf, sizeof(buf), "achieve/%s.attr", achieve);
	if(fp = fopen(buf, "r")){
		fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		return buf;
	}else{
		return NULL;
	}
}

int achieve_user(void)
{
	int i=0, count=0;
    char genbuf[3];
    char buf[200], buf2[200], buf3[200];
    FILE *fp;

	clear();
	vs_hdr2(" " BBSNAME " ", " �ڪ����N����");
	setuserfile(buf, "achieve");
	if(fp = fopen(buf, "r")){
		while (fgets(buf2, sizeof(buf2), fp)) {
			if(buf2[strlen(buf2) - 1] == '\n')
				buf2[strlen(buf2) - 1] = '\0';
			if(buf2[0] != '\0'){
				move(1,0);clrtobot();
				i = count + 1;
				outs("���N�W�١G");outs(getAchName(buf2,false));outs("\n");
				outs("���N�����G");outs(getAchDesc(buf2));outs("\n");
				outs("���N�ݩʡG");outs(getAchAttr(buf2));outs("\n");
				outs("\n���A�G");
				if(strcmp(cuser.achieve, buf2) == 0)
					outs(ANSI_COLOR(1;32)"�ϥΤ�"ANSI_RESET);
				else
					outs("���ϥ�");
				outs("\n");

				if(strcmp(cuser.achieve, buf2) == 0){
					getdata(b_lines - 1, 0, "[ENTER] �U�@��  (U) �ޱ����� ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'u'){
						strlcpy(cuser.achieve, "", sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("�ޱ��o�I");
						return 0;
					}
				}else{
					getdata(b_lines - 1, 0, "[ENTER] �U�@��  (W) �t������ ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'w'){
						strlcpy(cuser.achieve, buf2, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("���W�o�I");
						return 0;
					}
				}
			}
			count++;
		}
		fclose(fp);
	}else{
		vmsg("�A�٨S�����N�����A�A�i�H�b�j�߶W�����ʶR�C");
	}
    return 0;
}

/* �w�L���C
int achieve_buy_1stanniv(char *achieve){
    FILE *fp;
    char buf[200], date[11], genbuf[3], genbuf2[3];
    int i, dateint, start = 1071210, end = 1071211;
    struct tm      ptime;
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	snprintf(date, sizeof(date), "%03d%02d%02d", ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday);
	dateint = atoi(date);
	
	clear();
	vs_hdr2("���N�����ө� ", " ");
	move(2,0);
	outs("���N�W�١G");outs(getAchName(achieve,false));outs("\n");
	outs("���N�����G");outs(getAchDesc(achieve));outs("\n");
	outs("���N�ݩʡG");outs(getAchAttr(achieve));outs("\n");
	outs("���N����G100 " MONEYNAME "\n\n");
	outs("�}�l�c��G107�~12��10�� 00:00\n");
	outs("�����c��G107�~12��11�� 23:59\n\n");
	
	if(dateint < start){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;33)"�٨S�}�l�c���I"ANSI_RESET);
		pressanykey();
		return 0;
	}else if(dateint > end){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"�W�L�ʶR�����F�I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		if(doUserOwnAch(achieve) == 1){
			mvouts(b_lines - 2, 34, ANSI_COLOR(1;32)"�w�ʶR�L�I"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			getdata(b_lines - 2, 0, "(B) �ʶR  [Q] ��^ ",genbuf, 3, LCECHO);
			if(genbuf[0] == 'b') {
				getdata(b_lines - 1, 0, "�T�w�ʶR�ܡH (y/N)",genbuf, 3, LCECHO);
				if (genbuf[0] != 'y') {
					vmsg("������I");
					return 0;
				}else{
					reload_money();
					if (cuser.money < 100){
						vmsg(MONEYNAME "�����ʶR���N�K");
						return 0;
					}else{
						setuserfile(buf, "achieve");
						if(fp = fopen(buf, "a")){
							pay(100, "�ʶR���N�G%s", getAchName(achieve,true));
							fprintf(fp,"%s\n", achieve);
							fclose(fp);
							mvouts(b_lines - 2, 0, "�w�ʶR�I�i�H�b�ӤH�]�w�ϰt�����N�����C");
							getdata(b_lines - 1, 0, "�٬O�A�n�{�b�t���ܡH (y/N)",genbuf2, 3, LCECHO);
							if(genbuf2[0] == 'y'){
								strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
								passwd_update(usernum, &cuser);
								vmsg("���W�o�I");
								return 0;
							}
							vmsg("�w�ʶR�I");
							return 0;
						}else{
							vmsg("�{�����~�K");
							return 0;
						}
					}
				}
			}
		}
	}
}
*/

int achieve_buy_500post(char *achieve){
	FILE *fp;
	char buf[200], date[11], genbuf[3], genbuf2[3];

	clear();
	vs_hdr2(" ���N�����ө� ", " �ʶR���N");
	move(2,0);
	outs("���N�W�١G");outs(getAchName(achieve,false));outs("\n");
	outs("���N�����G");outs(getAchDesc(achieve));outs("\n");
	outs("���N�ݩʡG");outs(getAchAttr(achieve));outs("\n");
	outs("���o����G�o��500�g\n\n");
	outs("���Ф��n�c�N�~�峹�ơA�̭��i�B���v�B����I\n");

	if(doUserOwnAch(achieve) == 1){
		mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"�w�֦��F�I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		getdata(b_lines - 2, 0, "(B) ���o  [Q] ��^ ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'b') {
			if(cuser.numposts >= 500){
				setuserfile(buf, "achieve");
				if(fp = fopen(buf, "a")){
					fprintf(fp,"%s\n", achieve);
					fclose(fp);
					mvouts(b_lines - 2, 0, "�w���o�I�i�H�b�ӤH�]�w�ϰt�����N�����C");
					getdata(b_lines - 1, 0, "�٬O�A�n�{�b�t���ܡH (y/N)",genbuf2, 3, LCECHO);
					if(genbuf2[0] == 'y'){
						strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("���W�o�I");
						return 0;
					}
					vmsg("�w�ʶR�I");
					return 0;
				}else{
					vmsg("�{�����~�K");
					return 0;
				}
			}else{
				vmsgf("�峹�g���ٯ�%d�g��A��ѦA�ӧa�C", (500 - cuser.numposts));
				return 0;
			}
		}
	}
}

int achieve_buy_365login(char *achieve){
	FILE *fp;
	char buf[200], date[11], genbuf[3], genbuf2[3];

	clear();
	vs_hdr2(" ���N�����ө� ", " �ʶR���N");
	move(2,0);
	outs("���N�W�١G");outs(getAchName(achieve,false));outs("\n");
	outs("���N�����G");outs(getAchDesc(achieve));outs("\n");
	outs("���N�ݩʡG");outs(getAchAttr(achieve));outs("\n");
	outs("���o����G�n�J365��\n\n");

	if(doUserOwnAch(achieve) == 1){
		mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"�w�֦��F�I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		getdata(b_lines - 2, 0, "(B) ���o  [Q] ��^ ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'b') {
			if(cuser.numlogindays >= 365){
				setuserfile(buf, "achieve");
				if(fp = fopen(buf, "a")){
					fprintf(fp,"%s\n", achieve);
					fclose(fp);
					mvouts(b_lines - 2, 0, "�w���o�I�i�H�b�ӤH�]�w�ϰt�����N�����C");
					getdata(b_lines - 1, 0, "�٬O�A�n�{�b�t���ܡH (y/N)",genbuf2, 3, LCECHO);
					if(genbuf2[0] == 'y'){
						strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("���W�o�I");
						return 0;
					}
					vmsg("�w�ʶR�I");
					return 0;
				}else{
					vmsg("�{�����~�K");
					return 0;
				}
			}else{
				vmsgf("�n�J�����ٯ�%d����A��ѦA�ӧa�C", (365 - cuser.numlogindays));
				return 0;
			}
		}
	}
}

int achieve_shop(void)
{
	int i;
    char genbuf[3];

VIEWLIST:
	clear();
	vs_hdr2(" " BBSNAME " ", " ���N�����ө�");
	move(1,0);
	vbarf(ANSI_REVERSE " �s��  ���N�W��\t���o����          \n");
	vbarf("   1.  �j�ߤ@�g�~���y���� [�}��]\t100 " MONEYNAME " �w���� \n");
	vbarf("   2.  �V�O���g [���q]\t�o��500�g         \n");
	vbarf("   3.  1���p�ߤl [���q]\t�n�J365��         \n");
	outs("\n�q�д��ݧ�h���N�����K\n");
	getdata(b_lines - 1, 0, "��J�s���˵��Բӻ������ʶR�A��J[Q]���} ",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		move(b_lines - 3, 0); clrtobot();
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"�W�L�ʶR�����F�I"ANSI_RESET);
		pressanykey();
		goto VIEWLIST;
	}
	if (genbuf[0] == '2') {
		achieve_buy_500post("500post");
		goto VIEWLIST;
	}
	if (genbuf[0] == '3') {
		achieve_buy_365login("365login");
		goto VIEWLIST;
	}
	
    return 0;
}

int achieve_view(char *achieve)
{
	clear();
	vs_hdr2(" " BBSNAME " ", " �d�ߦ��N�����Ա�");
	
	move(2,0);
	outs("���N�W�١G");outs(getAchName(achieve,false));outs("\n");
	outs("���N�����G");outs(getAchDesc(achieve));outs("\n");
	outs("���N�ݩʡG");outs(getAchAttr(achieve));outs("\n");
	pressanykey();

    return 0;
}

#endif //USE_ACHIEVE
