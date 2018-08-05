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
	int		deadline=10; //繳納期限日期
    struct tm      ptime;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	
	if(isFileExist("etc/boardtax.txt") == false){
		vmsg("尚未設定稅額檔案");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅繳納");
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
			vmsg("您不須繳稅");
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
						prints("繳納紀錄：");
						prints("%s", mn);
						pressanykey();
						return 0;
					}
					count++;
				}
				fclose(fp);
			}
			outs("\n依據規定持有非公務看板的板主應於每月10日前支付當月看板稅。\n"
				 "未於規定期間內繳納看板將被暫時扣留於違規看板扣留區。\n"
				 "如果您的稅額有誤或有任何疑問請向土地管理局局長洽詢。\n\n"
				 "稅額計算公式：非教育類看板採累進稅額，第一個看板200、第二個300以此累加。\n"
				 "              教育類看板數採固定稅額，第一個看板免稅、第二個起每看板50。\n"
				 "              非教育類看板數=X；教育類看板數=Y\n"
				 "              TAX = 50X^2+150X+50Y (若Y>0，則TAX-50)\n"
				 "滯納金說明：當月11日起繳納者，每天加收25元。\n");
			if((int)ptime.tm_mday > deadline){
				delayday = (int)ptime.tm_mday - deadline;
				delaypay = delayday * 25;
				move(b_lines-6, 0);
				prints("%03d年%02d月看板稅\n", ptime.tm_year - 11, ptime.tm_mon + 1);
				prints("應繳看板稅 %d\n", tax);
				prints("應繳滯納金 %d\n", delaypay);
				shouldpay = tax + delaypay;
				prints("總計應繳稅金 %d\n", shouldpay);
			}else{
				move(b_lines-4, 0);
				prints("%03d年%02d月看板稅\n", ptime.tm_year - 11, ptime.tm_mon + 1);
				prints("應付看板稅 %d\n", tax);
				shouldpay = tax;
			}
			getdata(b_lines - 1, 0, "您想支付本月的看板稅了嗎？ (y/N)",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				vmsg("取消支付");
				return 0;
			}else{
				reload_money();
				if (cuser.money < shouldpay){
					vmsg(MONEYNAME "不夠繳納稅額...");
					return 0;
				}else{
					if(delaypay > 0){
						pay(shouldpay, "繳納%03d年%02d月看板稅%d元（含滯納金%d元）。", ptime.tm_year - 11, ptime.tm_mon + 1, shouldpay, delaypay);
						log_filef(buf, LOG_CREAT,"%s:%02d/%02d %02d:%02d已繳納%03d年%02d月看板稅%d%s（含滯納金%d元）。\n",
								  cuser.userid, ptime.tm_mon + 1, ptime.tm_mday,
								  ptime.tm_hour, ptime.tm_min, 
								  ptime.tm_year - 11, ptime.tm_mon + 1,
								  shouldpay, MONEYNAME, delaypay);
					}else{
						pay(shouldpay, "繳納%03d年%02d月看板稅%d元。", ptime.tm_year - 11, ptime.tm_mon + 1, shouldpay);
						log_filef(buf, LOG_CREAT,"%s:%02d/%02d %02d:%02d已繳納%03d年%02d月看板稅%d%s。\n",
								  cuser.userid, ptime.tm_mon + 1, ptime.tm_mday,
								  ptime.tm_hour, ptime.tm_min, 
								  ptime.tm_year - 11, ptime.tm_mon + 1,
								  shouldpay, MONEYNAME);
					}
					clear();
					move(b_lines-2, 0);
					prints("已繳納%03d年%02d月看板稅。\n", ptime.tm_year - 11, ptime.tm_mon + 1);
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
		vmsg("尚未設定稅額檔案，請聯繫工程業務處。");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅設定");
		getdata(1, 0, "查詢 (Q)  新增/修改 (A)  刪除 (D)  取消 [C] ",genbuf, 3, LCECHO);
		if (genbuf[0] == 'q' || genbuf[0] == 'a' || genbuf[0] == 'd'){
			move(1,0);
			usercomplete("請輸入要設定的ID ", userid);
			if (!is_validuserid(userid)){
				vmsg("查無ID");
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
				prints("查詢ID：%s\n應付看板稅 %d", userid, shouldpay);
				pressanykey();
			}else if (genbuf[0] == 'a'){
				getdata(2, 0, "非教育類看板數 ", buf, 20, DOECHO);
				x = atoi(buf);
				if (x < 0) {
					vmsg("輸入錯誤!!");
					return 0;
				}

				getdata(3, 0, "教育類看板數   ", buf, 20, DOECHO);
				y = atoi(buf);
				if (y < 0) {
					vmsg("輸入錯誤!!");
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
				prints("已完成設定！\n設定ID：%s\n非教育類看板數：%d\n教育類看板數  ：%d\n應付看板稅 $%d", userid, x, y, shouldpay);
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
				
				vmsg("刪除完成");
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
	vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅試算");
	
	getdata(2, 0, "非教育類看板數 ", buf, 20, DOECHO);
	x = atoi(buf);
	if (x < 0) {
	    vmsg("輸入錯誤!!");
	    return 0;
	}
	
	getdata(3, 0, "教育類看板數   ", buf, 20, DOECHO);
	y = atoi(buf);
	if (y < 0) {
	    vmsg("輸入錯誤!!");
	    return 0;
	}
	
	tax = brdTaxCalcFunc(x,y);
	
	move(b_lines-4, 0);
	prints("非教育類看板數：%d\n教育類看板數  ：%d\n應付看板稅 $%d", x, y, tax);
	pressanykey();
	return 0;
}

int
board_tax_log(void)
{
    char	buf[200] = "", genbuf[3];
    int		year, month;
	clear();
	vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅檔案");
	getdata(2, 0, "欲查看的民國年份 ", buf, 20, DOECHO);
	year = atoi(buf);
	if (year < 0) {
		vmsg("輸入錯誤!!");
		return 0;
	}
	getdata(3, 0, "欲查看的月份     ", buf, 20, DOECHO);
	month = atoi(buf);
	if (month < 0) {
		vmsg("輸入錯誤!!");
		return 0;
	}
	snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", year, month);
	
	if(isFileExist(buf) == false){
		vmsg("這個月沒有繳納紀錄");
		return 0;
	}else{
		getdata(b_lines-2, 0, "檢視 (V)  編修 (E)  取消 [Q] ",genbuf, 3, LCECHO);
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
	vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅繳納紀錄");
	getdata(b_lines-2, 0, "檢視 (V)  編修 (E)  取消 [Q] ",genbuf, 3, LCECHO);
	if(genbuf[0] == 'v'){
		more("etc/boardtax.txt", YEA);
	}else if(genbuf[0] == 'e'){
		veditfile("etc/boardtax.txt");
	}
	return 0;
}