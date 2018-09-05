#include "bbs.h"

#ifdef USE_MISSION

/* 0904~0905停機補償 */
int mission_0904offline(){
    FILE *fp;
    char buf[200], buf2[200], date[11], genbuf[3];
    int i, dateint, deadline = 1070907;
    struct tm      ptime;
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	snprintf(date, sizeof(date), "%03d%02d%02d", ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday);
	dateint = atoi(date);
	
	clear();
	vs_hdr2("任務中心 ", " 0904~0905停機補償");
	move(2,0);
	outs("任務說明：\n"
		 "09/04~09/05因為伺服器錯誤造成所有服務都沒有開機自啟動，\n"
		 "將近24小時後才重新開機恢復服務，\n"
		 "特發放400兔幣作為補償，每人限領一次。\n\n");
	outs("參加資格：不限\n");
	outs("任務獎勵：400 " MONEYNAME "\n");
	outs("任務期限：107年09月07日 23:59前\n");
	
	if(dateint > deadline){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"超過領取期限了！"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		setuserfile(buf, "mission.0904offline");
		if(isFileExist(buf) == false){
			if (!(fp = fopen(buf, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F01)");
				return 0;
			}
			fprintf(fp,"complete");
			fclose(fp);
			pay(-400, "特殊獎勵：0904~0905停機補償。");
			mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"已領取獎勵！"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if (!(fp = fopen(buf, "r"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F02)");
				return 0;
			}
			fgets(buf2, sizeof(buf2), fp);
			fclose(fp);
			if (!strcmp(buf2, "complete")){
				mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"已經完成過任務。"ANSI_RESET);
				pressanykey();
				return 0;
			}else{
				if (!(fp = fopen(buf, "w"))){
					vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F03)");
					return 0;
				}
				fprintf(fp,"complete");
				fclose(fp);
				pay(-400, "特殊獎勵：0904~0905停機補償。");
				mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"已領取獎勵！"ANSI_RESET);
				pressanykey();
				return 0;
			}
		}
	}
}

/* 每日登入大兔 */
int mission_dailylogin(){
    FILE *fp;
    char buf[200], buf2[200], date[11], genbuf[3];
    int i;
    struct tm      ptime;
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	snprintf(date, sizeof(date), "%03d-%02d-%02d", ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday);
	
	clear();
	vs_hdr2("任務中心 ", " 每日登入大兔");
	move(2,0);
	outs("任務說明：\n"
		 "一定要每日上大兔喔 :)\n\n");
	outs("參加資格：不限\n");
	outs("任務獎勵：每日 100 " MONEYNAME "\n");
	
	setuserfile(buf, "mission.dailylogin");
	if(isFileExist(buf) == false){
		if (!(fp = fopen(buf, "w"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-1-F01)");
			return 0;
		}
		fprintf(fp,"%s", date);
		fclose(fp);
		pay(-100, "完成任務：每日登入大兔。");
		mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"恭喜完成任務！"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		if (!(fp = fopen(buf, "r"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-1-F02)");
			return 0;
		}
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		if (!strcmp(buf2, date)){
			mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"已經完成過任務。"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if (!(fp = fopen(buf, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-1-F03)");
				return 0;
			}
			fprintf(fp,"%s", date);
			fclose(fp);
			pay(-100, "完成任務：每日登入大兔。");
			mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"恭喜完成任務！"ANSI_RESET);
			pressanykey();
			return 0;
		}
	}
}

/* 修改自己的信箱 */
int mission_email(){
    FILE *fp;
    char buf[200], buf2[200], genbuf[3];
	
	clear();
	vs_hdr2("任務中心 ", " 修改自己的信箱");
	move(2,0);
	outs("任務說明：\n"
		 "初創站時註冊單程式有做些修改，不幸的是程式修改時沒有測試，\n"
		 "導致後來使用者註冊單雖然有填寫電子信箱但不會被系統紀錄，\n"
		 "這項錯誤後來被發現，也在1.5版更新中修正了這項錯誤。\n"
		 "為了讓使用者資料完整，我們鼓勵使用者自主修改電子信箱。\n\n");
	outs("參加資格：在程式修正前註冊，沒有被紀錄到信箱的使用者\n");
	outs("任務獎勵：250 " MONEYNAME "\n");
	outs("任務指示：在(U)ser 個人設定區→(I)nfo 個人資料設定 中修改\n");
	
	setuserfile(buf, "mission.email");
	if(isFileExist(buf) == false){
		if(strcmp(cuser.email, "x")){
			mvouts(b_lines - 2, 24, ANSI_COLOR(1;33)"電子信箱紀錄正常，不具參加資格。"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			getdata(12, 0, "接下這個任務嗎？ (y)是 [N]否 ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				return 0;
			}
			if (!(fp = fopen(buf, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-M-F01)");
				return 0;
			}
			now = time(NULL);
			fprintf(fp,"%s", Cdate(&now));
			fclose(fp);
			vmsg("接下任務囉～");
			return 0;
		}
	}else{
		if (!(fp = fopen(buf, "r"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-M-F02)");
			return 0;
		}
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		if (!strcmp(buf2, "complete")){
			mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"已經完成過任務。"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if(strcmp(cuser.email, "x")){
				if (!(fp = fopen(buf, "w"))){
					vmsg("系統錯誤，請稍後再試。(Error code: MIS-M-F03)");
					return 0;
				}
				fprintf(fp,"complete");
				fclose(fp);
				pay(-250, "完成任務：修改自己的信箱。");
				mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"恭喜完成任務！"ANSI_RESET);
				pressanykey();
				return 0;
			}else{
				mvouts(b_lines - 2, 35, "任務進行中");
				pressanykey();
				return 0;
			}
		}
	}
}

/* 任務列表 */
int mission_main()
{
	int i;
    char genbuf[3];
	
	clear();
	vs_hdr2(" " BBSNAME " ", " 任務中心");
	move(1,0);
	vbarf(ANSI_REVERSE " 編號  任務標題\t獎勵 \n");
	vbarf("   1.  每日登入大兔\t100 " MONEYNAME "\n");
	vbarf("   2.  修改自己的信箱\t250 " MONEYNAME "\n");
	vbarf("   3.  0904~0905停機補償\t400 " MONEYNAME "\n");
	
	getdata(b_lines - 1, 0, "輸入任務編號檢視詳細說明、開始任務或領取獎勵：",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		mission_dailylogin();
		return 0;
	}
	if (genbuf[0] == '2') {
		mission_email();
		return 0;
	}
	if (genbuf[0] == '3') {
		mission_0904offline();
		return 0;
	}
	
    return 0;
}

#endif
