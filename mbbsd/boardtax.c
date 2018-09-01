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
		vmsg("尚未設定稅額檔案");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅繳納");
		tax = getBoardTax(cuser.userid);

		if(tax == 0){
			vmsg("您不須繳稅");
			return 0;
		}else{
			move(1,0);clrtobot();
			snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", ptime.tm_year - 11, ptime.tm_mon + 1);
			if(isBrdTaxPaid(cuser.userid) != 0){
				vmsg("本期已繳款");
				return 0;
			}
			outs("\n依據規定持有非公務看板的板主應於每月10日前支付當月看板稅，\n"
				 "未於規定期間內繳納看板將被暫時扣留於違規看板扣留區。\n"
				 "如果您的稅額有誤或有任何疑問請向土地管理局局長洽詢。\n\n"
				 "稅額計算公式：非教育類看板採累進稅額，第一個看板200、第二個300以此累加。\n"
				 "              教育類看板數採固定稅額，第一個看板免稅、第二個起每看板50。\n"
				 "              非教育類看板數=X；教育類看板數=Y\n"
				 "              TAX = 50X^2+150X+50Y (若Y>0，則TAX-50)\n\n");
#ifdef BOARD_TAX_DEADLINE
			prints("滯納金說明：當月%d日起繳納者，每天加收%d元。\n",BOARD_TAX_DEADLINE,BOARD_TAX_LATEPAYRATE);
#endif
#ifdef BOARD_TAX_DISDATE
			prints("為使土地管理局便於設定下期稅款檔案，當月%d日起至次月1日前不開放繳納。\n",BOARD_TAX_DISDATE);
			if((int)ptime.tm_mday > BOARD_TAX_DISDATE){
				vmsg("本期已不可繳納");
				return 0;
			}else{
#endif
#ifdef BOARD_TAX_DEADLINE
				if((int)ptime.tm_mday > BOARD_TAX_DEADLINE){
					lateDay = (int)ptime.tm_mday - BOARD_TAX_DEADLINE;
					latePay = lateDay * BOARD_TAX_LATEPAYRATE;
					move(b_lines-6, 0);
					prints("%03d年%02d月看板稅\n", ptime.tm_year - 11, ptime.tm_mon + 1);
					prints("應繳看板稅 %d\n", tax);
					prints("應繳滯納金 %d\n", latePay);
					shouldpay = tax + latePay;
					prints("總計應繳稅金 %d\n", shouldpay);
				}else{
#endif
					move(b_lines-4, 0);
					prints("%03d年%02d月看板稅\n", ptime.tm_year - 11, ptime.tm_mon + 1);
					prints("應付看板稅 %d\n", tax);
					shouldpay = tax;
#ifdef BOARD_TAX_DEADLINE
				}
#endif
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
#ifdef BOARD_TAX_DEADLINE
						if(latePay > 0){
							snprintf(buf2, sizeof(buf2), "（含滯納金%d）", latePay);
						}
#endif
						pay(shouldpay, "繳納%03d年%02d月看板稅%d元%s。", ptime.tm_year - 11, ptime.tm_mon + 1, shouldpay, buf2);
						log_filef(buf, LOG_CREAT,"%s:%02d/%02d %02d:%02d已繳納%03d年%02d月看板稅%d%s%s。\n",
								  cuser.userid, ptime.tm_mon + 1, ptime.tm_mday,
								  ptime.tm_hour, ptime.tm_min, 
								  ptime.tm_year - 11, ptime.tm_mon + 1,
								  shouldpay, buf2, MONEYNAME);
						clear();
						move(b_lines-2, 0);
						prints("已繳納%03d年%02d月看板稅。\n", ptime.tm_year - 11, ptime.tm_mon + 1);
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
				move(b_lines-5, 0);clrtobot();
				prints("查詢ID：%s\n", userid);
				shouldpay = getBoardTax(userid);
				if(shouldpay == 0)
					outs("不須繳納");
				else{
					char *paid = isBrdTaxPaid(userid);
					if(paid == 0){
						prints(ANSI_COLOR(1;31) "本月尚未繳納" ANSI_RESET "\n應繳看板稅 %d\n", shouldpay);
#ifdef BOARD_TAX_DEADLINE
						if((int)ptime.tm_mday > BOARD_TAX_DEADLINE){
							lateDay = (int)ptime.tm_mday - BOARD_TAX_DEADLINE;
							latePay = lateDay * BOARD_TAX_LATEPAYRATE;
							prints("應繳滯納金 %d\n", latePay);
							prints("總計應繳稅金 %d\n", shouldpay + latePay);
						}
#endif
					}else{
						prints(ANSI_COLOR(1;32) "本月已經繳納看板稅" ANSI_RESET "\n%s\n", paid);
					}
				}
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
	
	move(2,0);
	prints("歡迎使用看板稅稅額試算系統。\n");
	prints("輸入您的看板數量後，系統會自動試算您的稅額。\n");
	prints("非教育類看板數：指設立於帝寶大廈、市區公寓、郊區別墅與高山營地的看板。\n教育類看板數  ：指設立於教育特區並以開放教育為目的的看板。");
	
	getdata(7, 0, "非教育類看板數 ", buf, 20, DOECHO);
	x = atoi(buf);
	if (x < 0) {
	    vmsg("輸入錯誤!!");
	    return 0;
	}
	
	getdata(8, 0, "教育類看板數   ", buf, 20, DOECHO);
	y = atoi(buf);
	if (y < 0) {
	    vmsg("輸入錯誤!!");
	    return 0;
	}
	
	tax = brdTaxCalcFunc(x,y);
	
	move(b_lines-4, 0);clrtobot();
	prints("非教育類看板數：%d\n教育類看板數  ：%d\n應付看板稅 $%d/月", x, y, tax);
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
	vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅檔案");
	getdata(2, 0, "本月 [T]  指定月份 (D)  取消 (Q) ",genbuf, 3, LCECHO);
	if(genbuf[0] == 'q'){
		return 0;
	}else if(genbuf[0] == 'd'){
		getdata(3, 0, "欲查看的民國年份 ", buf, 20, DOECHO);
		year = atoi(buf);
		if (year < 0) {
			vmsg("輸入錯誤!!");
			return 0;
		}
		getdata(4, 0, "欲查看的月份     ", buf, 20, DOECHO);
		month = atoi(buf);
		if (month < 0) {
			vmsg("輸入錯誤!!");
			return 0;
		}
	}else{
		year = ptime.tm_year - 11;
		month = ptime.tm_mon + 1;
	}
	
	snprintf(buf, sizeof(buf), "log/taxpaid/board_%03d_%02d.log", year, month);
	
	if(isFileExist(buf) == false){
		vmsgf("%03d年%02d月沒有繳納紀錄", year, month);
		return 0;
	}else{
		if(HasUserPerm(PERM_SYSOP)){
			getdata(b_lines-2, 0, "檢視 [V]  編修 (E)  取消 (Q) ",genbuf, 3, LCECHO);
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
	vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅稅額檔案");
	getdata(b_lines-2, 0, "檢視 [V]  編修 (E)  取消 (Q) ",genbuf, 3, LCECHO);
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
		vmsg("尚未設定稅額檔案");
		return 0;
	}else{
		vs_hdr2(" " BBSMNAME2 "銀行 ", " 看板稅未繳納名單");

		move(4, 0);
		prints("名單整理中...");

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
			mvouts(b_lines-2, 0, "所有人都繳款了。");
			pressanykey();
		}
		return 0;
	}
}

#endif //USE_BOARDTAX