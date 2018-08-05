#include "bbs.h"

bool isFileExist(char *filedir){
	FILE	*file;

	if(file = fopen(filedir, "r")){
		fclose(file);
		return true;
	}
	return false;
}

int
pay_board_tax(void)
{
    FILE           *fp;
    char           *ptr, *id, *mn;
    char            buf[200] = "", genbuf[3];
    int             i, money = 0, count=0;
	int		delayday=0, delaypay=0;
	int		tax=0, shouldpay=0, paid=0;
	int		deadline=10; //ú�Ǵ������
    struct tm      ptime;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	
	if(isFileExist("etc/boardtax.txt") == false){
		vmsg("�|���]�w�|�B�ɮ�");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|ú��");
		fp = fopen("etc/boardtax.txt", "r+");
		while (fgets(buf, sizeof(buf), fp)) {
			if (!(ptr = strchr(buf, ':')))
			continue;
			*ptr = '\0';
			id = buf;
			mn = ptr + 1;
			money = atoi(mn);
			if(strcmp(id, cuser.userid) == 0){
				tax = money;
			}
			count++;
		}
		fclose(fp);

		if(tax == 0){
			vmsg("�z����ú�|");
			return 0;
		}else{
			move(1,0);clrtobot();
			snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", ptime.tm_year - 11, ptime.tm_mon + 1);
			if(fp = fopen(buf, "r+")){
				while (fgets(buf, sizeof(buf), fp)) {
					if (!(ptr = strchr(buf, ':')))
					continue;
					*ptr = '\0';
					id = buf;
					mn = ptr + 1;
					if(strcmp(id, cuser.userid) == 0){
						move(b_lines-3, 0);
						prints("ú�Ǭ����G");
						prints("%s", mn);
						pressanykey();
						return 0;
					}
					count++;
				}
				fclose(fp);
			}
			outs("\n�̾ڳW�w�����D���ȬݪO���O�D����C��10��e��I���ݪO�|�C\n"
				 "����W�w������ú�ǬݪO�N�Q�Ȯɦ��d��H�W�ݪO���d�ϡC\n"
				 "�p�G�z���|�B���~�Φ�����ðݽЦV�g�a�޲z���������ߡC\n\n"
				 "�|�B�p�⤽���G�D�Ш|���ݪO�Ĳֶi�|�B�A�Ĥ@�ӬݪO200�B�ĤG��300�H���֥[�C\n"
				 "              �Ш|���ݪO�ƱĩT�w�|�B�A�Ĥ@�ӬݪO�K�|�B�ĤG�Ӱ_�C�ݪO50�C\n"
				 "              �D�Ш|���ݪO��=X�F�Ш|���ݪO��=Y\n"
				 "              TAX = 50X^2+150X+50Y (�YY>0�A�hTAX-50)\n"
				 "���Ǫ������G���11��_ú�Ǫ̡A�C�ѥ[��25���C\n");
			if((int)ptime.tm_mday > deadline){
				delayday = (int)ptime.tm_mday - deadline;
				delaypay = delayday * 25;
				move(b_lines-6, 0);
				prints("%03d�~%02d��ݪO�|\n", ptime.tm_year - 11, ptime.tm_mon + 1);
				prints("��ú�ݪO�| %d\n", tax);
				prints("��ú���Ǫ� %d\n", delaypay);
				shouldpay = tax + delaypay;
				prints("�`�p��ú�|�� %d\n", shouldpay);
			}else{
				move(b_lines-4, 0);
				prints("%03d�~%02d��ݪO�|\n", ptime.tm_year - 11, ptime.tm_mon + 1);
				prints("���I�ݪO�| %d\n", tax);
				shouldpay = tax;
			}
			getdata(b_lines - 1, 0, "�z�Q��I���몺�ݪO�|�F�ܡH (y/N)",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				vmsg("������I");
				return 0;
			}else{
				reload_money();
				if (cuser.money < shouldpay){
					vmsg(MONEYNAME "����ú�ǵ|�B...");
					return 0;
				}else{
					if(delaypay > 0){
						pay(shouldpay, "ú��%03d�~%02d��ݪO�|%d���]�t���Ǫ�%d���^�C", ptime.tm_year - 11, ptime.tm_mon + 1, shouldpay, delaypay);
						log_filef(buf, LOG_CREAT,"%s:%02d/%02d %02d:%02d�wú��%03d�~%02d��ݪO�|%d%s�]�t���Ǫ�%d���^�C\n",
								  cuser.userid, ptime.tm_mon + 1, ptime.tm_mday,
								  ptime.tm_hour, ptime.tm_min, 
								  ptime.tm_year - 11, ptime.tm_mon + 1,
								  shouldpay, MONEYNAME, delaypay);
					}else{
						pay(shouldpay, "ú��%03d�~%02d��ݪO�|%d���C", ptime.tm_year - 11, ptime.tm_mon + 1, shouldpay);
						log_filef(buf, LOG_CREAT,"%s:%02d/%02d %02d:%02d�wú��%03d�~%02d��ݪO�|%d%s�C\n",
								  cuser.userid, ptime.tm_mon + 1, ptime.tm_mday,
								  ptime.tm_hour, ptime.tm_min, 
								  ptime.tm_year - 11, ptime.tm_mon + 1,
								  shouldpay, MONEYNAME);
					}
					clear();
					move(b_lines-2, 0);
					prints("�wú��%03d�~%02d��ݪO�|�C\n", ptime.tm_year - 11, ptime.tm_mon + 1);
					pressanykey();
					return 0;
				}
			}
		}
	}
}

int
brdTaxCalcFunc(int x,int y)
{
    int		tax;
	tax = (50*x*x)+(150*x)+(50*y);
	if(y > 0){
		tax = tax - 50;
	}
	return tax;
}

int
set_board_tax(void)
{
    FILE           *fp, *fp2;
    char           *ptr, *id, *mn;
    char            buf[200] = "", genbuf[3];
    int             money = 0, count=0, shouldpay=0, change=0;
    int		x, y;
    char userid[IDLEN+1];
	
	if(isFileExist("etc/boardtax.txt") == false){
		vmsg("�|���]�w�|�B�ɮסA���pô�u�{�~�ȳB�C");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|�]�w");
		getdata(1, 0, "�d�� (Q)  �s�W/�ק� (A)  �R�� (D)  ���� [C] ",genbuf, 3, LCECHO);
		if (genbuf[0] == 'q' || genbuf[0] == 'a' || genbuf[0] == 'd'){
			move(1,0);
			usercomplete("�п�J�n�]�w��ID ", userid);
			if (!is_validuserid(userid)){
				vmsg("�d�LID");
				return 0;
			}

			if (genbuf[0] == 'q'){
				fp = fopen("etc/boardtax.txt", "r+");
				while (fgets(buf, sizeof(buf), fp)) {
					if (!(ptr = strchr(buf, ':')))
					continue;
					*ptr = '\0';
					id = buf;
					mn = ptr + 1;
					money = atoi(mn);
					if(strcmp(id, userid) == 0){
						shouldpay = money;
					}
					count++;
				}
				fclose(fp);
				
				move(b_lines-4, 0);clrtobot();
				prints("�d��ID�G%s\n���I�ݪO�| %d", userid, shouldpay);
				pressanykey();
			}else if (genbuf[0] == 'a'){
				getdata(2, 0, "�D�Ш|���ݪO�� ", buf, 20, DOECHO);
				x = atoi(buf);
				if (x < 0) {
					vmsg("��J���~!!");
					return 0;
				}

				getdata(3, 0, "�Ш|���ݪO��   ", buf, 20, DOECHO);
				y = atoi(buf);
				if (y < 0) {
					vmsg("��J���~!!");
					return 0;
				}

				shouldpay = brdTaxCalcFunc(x,y);
				
				system("cp etc/boardtax.txt etc/boardtax.tmp");
				fp = fopen("etc/boardtax.tmp", "r+");
				fp2 = fopen("etc/boardtax.txt", "w");
				while (fgets(buf, sizeof(buf), fp)) {
					if (!(ptr = strchr(buf, ':')))
					continue;
					*ptr = '\0';
					id = buf;
					mn = ptr + 1;
					money = atoi(mn);
					if(strcmp(id, userid) == 0){
						if(shouldpay > 0){
							fprintf(fp2,"%s:%d\n", id, shouldpay);
						}
						change = 1;
					}else{
						fprintf(fp2,"%s:%d\n", id, money);
					}
					count++;
				}
				if(change == 0 && shouldpay > 0){
					fprintf(fp2,"%s:%d\n", userid, shouldpay);
				}
				fclose(fp);
				fclose(fp2);
				
				move(b_lines-6, 0);clrtobot();
				prints("�w�����]�w�I\n�]�wID�G%s\n�D�Ш|���ݪO�ơG%d\n�Ш|���ݪO��  �G%d\n���I�ݪO�| $%d", userid, x, y, shouldpay);
				pressanykey();
			}else if (genbuf[0] == 'd'){
				system("cp etc/boardtax.txt etc/boardtax.tmp");
				fp = fopen("etc/boardtax.tmp", "r+");
				fp2 = fopen("etc/boardtax.txt", "w");
				while (fgets(buf, sizeof(buf), fp)) {
					if (!(ptr = strchr(buf, ':')))
					continue;
					*ptr = '\0';
					id = buf;
					mn = ptr + 1;
					money = atoi(mn);
					if(strcmp(id, userid) == 0){
						change = 1;
					}else{
						fprintf(fp2,"%s:%d\n", id, money);
					}
					count++;
				}
				fclose(fp);
				fclose(fp2);
				
				vmsg("�R������");
			}
			return 0;
		}else{
			return 0;
		}
	}
}

int
board_tax_calc(void)
{
    int		x, y, tax;
    char	buf[200] = "";
	
	clear();
	vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|�պ�");
	
	getdata(2, 0, "�D�Ш|���ݪO�� ", buf, 20, DOECHO);
	x = atoi(buf);
	if (x < 0) {
	    vmsg("��J���~!!");
	    return 0;
	}
	
	getdata(3, 0, "�Ш|���ݪO��   ", buf, 20, DOECHO);
	y = atoi(buf);
	if (y < 0) {
	    vmsg("��J���~!!");
	    return 0;
	}
	
	tax = brdTaxCalcFunc(x,y);
	
	move(b_lines-4, 0);
	prints("�D�Ш|���ݪO�ơG%d\n�Ш|���ݪO��  �G%d\n���I�ݪO�| $%d", x, y, tax);
	pressanykey();
	return 0;
}

int
board_tax_log(void)
{
    char	buf[200] = "", genbuf[3];
    int		year, month;
	clear();
	vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|�ɮ�");
	getdata(2, 0, "���d�ݪ�����~�� ", buf, 20, DOECHO);
	year = atoi(buf);
	if (year < 0) {
		vmsg("��J���~!!");
		return 0;
	}
	getdata(3, 0, "���d�ݪ����     ", buf, 20, DOECHO);
	month = atoi(buf);
	if (month < 0) {
		vmsg("��J���~!!");
		return 0;
	}
	snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", year, month);
	
	if(isFileExist(buf) == false){
		vmsg("�o�Ӥ�S��ú�Ǭ���");
		return 0;
	}else{
		getdata(b_lines-2, 0, "�˵� (V)  �s�� (E)  ���� [Q] ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'v'){
			more(buf, YEA);
		}else if(genbuf[0] == 'e'){
			veditfile(buf);
		}
	}
	return 0;
}

int
set_tax_file(void)
{
    char genbuf[3];
	clear();
	vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|ú�Ǭ���");
	getdata(b_lines-2, 0, "�˵� (V)  �s�� (E)  ���� [Q] ",genbuf, 3, LCECHO);
	if(genbuf[0] == 'v'){
		more("etc/boardtax.txt", YEA);
	}else if(genbuf[0] == 'e'){
		veditfile("etc/boardtax.txt");
	}
	return 0;
}