#include "bbs.h"

#ifdef USE_BOARDTAX

#define BOARD_TAX_DEADLINE (15)
#define BOARD_TAX_DISDATE (25)

#if defined(BOARD_TAX_DEADLINE) && !defined(BOARD_TAX_LATEPAYRATE)
#define BOARD_TAX_LATEPAYRATE (25)
#endif

int
getBoardTax(char *userid){
    struct tm      ptime;
    FILE           *fp;
    char           *ptr, *id, *mn;
    char            buf[200] = "";
    int             i, tax=0, money = 0, count=0;

	fp = fopen("etc/boardtax.txt", "r+");
	while (fgets(buf, sizeof(buf), fp)) {
		if (!(ptr = strchr(buf, ':')))
		continue;
		*ptr = '\0';
		id = buf;
		mn = ptr + 1;
		money = atoi(mn);
		if(strcmp(id, userid) == 0){
			tax = money;
		}
		count++;
	}
	fclose(fp);

	if(tax == 0){
		return 0;
	}else{
		return tax;
	}
}

char *
isBrdTaxPaid(char *userid){
    struct tm      ptime;
    FILE           *fp;
    char           *ptr, *id, *mn;
    char            buf[200] = "", buf2[200] = "";
    int             i, tax=0, money = 0, count=0, paid=0;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;

	snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", ptime.tm_year - 11, ptime.tm_mon + 1);
	if(fp = fopen(buf, "r+")){
		while (fgets(buf, sizeof(buf), fp)) {
			if (!(ptr = strchr(buf, ':')))
			continue;
			*ptr = '\0';
			id = buf;
			mn = ptr + 1;
			if(strcmp(id, userid) == 0){
				snprintf(buf2, sizeof(buf2), "%s", mn);
				paid = 1;
			}
			count++;
		}
		fclose(fp);
	}
	if(paid == 0){
		return 0;
	}else if(paid == 1){
		return buf2;
	}
	return 0;
}

int
pay_board_tax(void)
{
    FILE           *fp;
    char           *ptr, *id, *mn;
    char            buf[200] = "", buf2[200] = "", genbuf[3];
    int             i, money = 0, count=0;
	int		lateDay=0, latePay=0;
	int		tax=0, shouldpay=0, paid=0;
    struct tm      ptime;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	
	if(isFileExist("etc/boardtax.txt") == false){
		vmsg("�|���]�w�|�B�ɮ�");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|ú��");
		tax = getBoardTax(cuser.userid);

		if(tax == 0){
			vmsg("�z����ú�|");
			return 0;
		}else{
			move(1,0);clrtobot();
			snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", ptime.tm_year - 11, ptime.tm_mon + 1);
			if(isBrdTaxPaid(cuser.userid) != 0){
				vmsg("�����wú��");
				return 0;
			}
			outs("\n�̾ڳW�w�����D���ȬݪO���O�D����C��10��e��I���ݪO�|�A\n"
				 "����W�w������ú�ǬݪO�N�Q�Ȯɦ��d��H�W�ݪO���d�ϡC\n"
				 "�p�G�z���|�B���~�Φ�����ðݽЦV�g�a�޲z���������ߡC\n\n"
				 "�|�B�p�⤽���G�D�Ш|���ݪO�Ĳֶi�|�B�A�Ĥ@�ӬݪO200�B�ĤG��300�H���֥[�C\n"
				 "              �Ш|���ݪO�ƱĩT�w�|�B�A�Ĥ@�ӬݪO�K�|�B�ĤG�Ӱ_�C�ݪO50�C\n"
				 "              �D�Ш|���ݪO��=X�F�Ш|���ݪO��=Y\n"
				 "              TAX = 50X^2+150X+50Y (�YY>0�A�hTAX-50)\n\n");
#ifdef BOARD_TAX_DEADLINE
			prints("���Ǫ������G���%d��_ú�Ǫ̡A�C�ѥ[��%d���C\n",BOARD_TAX_DEADLINE,BOARD_TAX_LATEPAYRATE);
#endif
#ifdef BOARD_TAX_DISDATE
			prints("���Ϥg�a�޲z���K��]�w�U���|���ɮסA���%d��_�ܦ���1��e���}��ú�ǡC\n",BOARD_TAX_DISDATE);
			if((int)ptime.tm_mday > BOARD_TAX_DISDATE){
				vmsg("�����w���iú��");
				return 0;
			}else{
#endif
#ifdef BOARD_TAX_DEADLINE
				if((int)ptime.tm_mday > BOARD_TAX_DEADLINE){
					lateDay = (int)ptime.tm_mday - BOARD_TAX_DEADLINE;
					latePay = lateDay * BOARD_TAX_LATEPAYRATE;
					move(b_lines-6, 0);
					prints("%03d�~%02d��ݪO�|\n", ptime.tm_year - 11, ptime.tm_mon + 1);
					prints("��ú�ݪO�| %d\n", tax);
					prints("��ú���Ǫ� %d\n", latePay);
					shouldpay = tax + latePay;
					prints("�`�p��ú�|�� %d\n", shouldpay);
				}else{
#endif
					move(b_lines-4, 0);
					prints("%03d�~%02d��ݪO�|\n", ptime.tm_year - 11, ptime.tm_mon + 1);
					prints("���I�ݪO�| %d\n", tax);
					shouldpay = tax;
#ifdef BOARD_TAX_DEADLINE
				}
#endif
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
#ifdef BOARD_TAX_DEADLINE
						if(latePay > 0){
							snprintf(buf2, sizeof(buf2), "�]�t���Ǫ�%d�^", latePay);
						}
#endif
						pay(shouldpay, "ú��%03d�~%02d��ݪO�|%d��%s�C", ptime.tm_year - 11, ptime.tm_mon + 1, shouldpay, buf2);
						log_filef(buf, LOG_CREAT,"%s:%02d/%02d %02d:%02d�wú��%03d�~%02d��ݪO�|%d%s%s�C\n",
								  cuser.userid, ptime.tm_mon + 1, ptime.tm_mday,
								  ptime.tm_hour, ptime.tm_min, 
								  ptime.tm_year - 11, ptime.tm_mon + 1,
								  shouldpay, buf2, MONEYNAME);
						clear();
						move(b_lines-2, 0);
						prints("�wú��%03d�~%02d��ݪO�|�C\n", ptime.tm_year - 11, ptime.tm_mon + 1);
						pressanykey();
						return 0;
					}
				}
#ifdef BOARD_TAX_DISDATE
			}
#endif
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
    int             money = 0, count=0, shouldpay=0, lateDay=0, latePay=0, change=0;
    int		i, x, y;
    char userid[IDLEN+1];
    struct tm      ptime;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	
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
				move(b_lines-5, 0);clrtobot();
				prints("�d��ID�G%s\n", userid);
				shouldpay = getBoardTax(userid);
				if(shouldpay == 0)
					outs("����ú��");
				else{
					char *paid = isBrdTaxPaid(userid);
					if(paid == 0){
						prints(ANSI_COLOR(1;31) "����|��ú��" ANSI_RESET "\n��ú�ݪO�| %d\n", shouldpay);
#ifdef BOARD_TAX_DEADLINE
						if((int)ptime.tm_mday > BOARD_TAX_DEADLINE){
							lateDay = (int)ptime.tm_mday - BOARD_TAX_DEADLINE;
							latePay = lateDay * BOARD_TAX_LATEPAYRATE;
							prints("��ú���Ǫ� %d\n", latePay);
							prints("�`�p��ú�|�� %d\n", shouldpay + latePay);
						}
#endif
					}else{
						prints(ANSI_COLOR(1;32) "����w�gú�ǬݪO�|" ANSI_RESET "\n%s\n", paid);
					}
				}
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
	
	move(2,0);
	prints("�w��ϥάݪO�|�|�B�պ�t�ΡC\n");
	prints("��J�z���ݪO�ƶq��A�t�η|�۰ʸպ�z���|�B�C\n");
	prints("�D�Ш|���ݪO�ơG���]�ߩ���_�j�H�B���Ϥ��J�B���ϧO�ֻP���s��a���ݪO�C\n�Ш|���ݪO��  �G���]�ߩ�Ш|�S�ϨåH�}��Ш|���ت����ݪO�C");
	
	getdata(7, 0, "�D�Ш|���ݪO�� ", buf, 20, DOECHO);
	x = atoi(buf);
	if (x < 0) {
	    vmsg("��J���~!!");
	    return 0;
	}
	
	getdata(8, 0, "�Ш|���ݪO��   ", buf, 20, DOECHO);
	y = atoi(buf);
	if (y < 0) {
	    vmsg("��J���~!!");
	    return 0;
	}
	
	tax = brdTaxCalcFunc(x,y);
	
	move(b_lines-4, 0);clrtobot();
	prints("�D�Ш|���ݪO�ơG%d\n�Ш|���ݪO��  �G%d\n���I�ݪO�| $%d/��", x, y, tax);
	pressanykey();
	return 0;
}

int
board_tax_log(void)
{
    char	buf[200] = "", genbuf[3];
    int		i, year, month;
    struct tm      ptime;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	
	clear();
	vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|�ɮ�");
	getdata(2, 0, "���� [T]  ���w��� (D)  ���� (Q) ",genbuf, 3, LCECHO);
	if(genbuf[0] == 'q'){
		return 0;
	}else if(genbuf[0] == 'd'){
		getdata(3, 0, "���d�ݪ�����~�� ", buf, 20, DOECHO);
		year = atoi(buf);
		if (year < 0) {
			vmsg("��J���~!!");
			return 0;
		}
		getdata(4, 0, "���d�ݪ����     ", buf, 20, DOECHO);
		month = atoi(buf);
		if (month < 0) {
			vmsg("��J���~!!");
			return 0;
		}
	}else{
		year = ptime.tm_year - 11;
		month = ptime.tm_mon + 1;
	}
	
	snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", year, month);
	
	if(isFileExist(buf) == false){
		vmsgf("%03d�~%02d��S��ú�Ǭ���", year, month);
		return 0;
	}else{
		if(HasUserPerm(PERM_SYSOP)){
			getdata(b_lines-2, 0, "�˵� [V]  �s�� (E)  ���� (Q) ",genbuf, 3, LCECHO);
			if(genbuf[0] == 'r'){
				return 0;
			}else if(genbuf[0] == 'e'){
				veditfile(buf);
			}else{
				more(buf, YEA);
			}
		}else{
			more(buf, YEA);
		}
	}
	return 0;
}

int
set_tax_file(void)
{
    char genbuf[3];
	clear();
	vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|�|�B�ɮ�");
	getdata(b_lines-2, 0, "�˵� [V]  �s�� (E)  ���� (Q) ",genbuf, 3, LCECHO);
	if(genbuf[0] == 'q'){
		return 0;
	}else if(genbuf[0] == 'e'){
		veditfile("etc/boardtax.txt");
	}else{
		more("etc/boardtax.txt", YEA);
	}
	return 0;
}

int
list_unpay(void)
{
    FILE           *fp, *fp2;
    char           *ptr, *id, *mn;
    char            buf[200] = "";
    int             money = 0, count=0, exist=0;
	
	if(isFileExist("etc/boardtax.txt") == false){
		vmsg("�|���]�w�|�B�ɮ�");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "�Ȧ� ", " �ݪO�|��ú�ǦW��");

		move(4, 0);
		prints("�W���z��...");

		fp = fopen("etc/boardtax.txt", "r+");
		fp2 = fopen("etc/boardtaxunpay", "w");
		while (fgets(buf, sizeof(buf), fp)) {
			if (!(ptr = strchr(buf, ':')))
			continue;
			*ptr = '\0';
			id = buf;
			mn = ptr + 1;
			money = atoi(mn);
			if(isBrdTaxPaid(id) == 0){
				fprintf(fp2,"%s:%d\n", id, money);
				exist = 1;
			}
			count++;
		}
		fclose(fp);
		fclose(fp2);

		if(exist == 1){
			clear();
			more("etc/boardtaxunpay", YEA);
		}else{
			move(1,0);clrtobot();
			mvouts(b_lines-2, 0, "�Ҧ��H��ú�ڤF�C");
			pressanykey();
		}
		return 0;
	}
}

#endif //USE_BOARDTAX