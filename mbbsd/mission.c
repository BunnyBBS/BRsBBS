#include "bbs.h"

#ifdef USE_MISSION

/* 每日登入大兔 */
int mission_dailylogin(){
    FILE *fp;
    char buf[200], buf2[200], date[11], genbuf[3];
    snprintf(date, sizeof(date), "%s", Cdatedate(&now));
	
	clear();
	vs_hdr2(" 任務中心 ", " 每日登入大兔");
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

/* 每日發篇文章 */
int mission_dailypost(){
    FILE *fp;
    char buf[200], buf2[200], buf3[200], buf4[200], date[11], genbuf[3];
    snprintf(date, sizeof(date), "%s", Cdatedate(&now));
	
	clear();
	vs_hdr2(" 任務中心 ", " 每日發篇文章");
	move(2,0);
	outs("任務說明：\n"
		 "每天都來大兔說說話吧 :D\n\n");
	outs("參加資格：不限\n");
	outs("任務獎勵：每日 100 " MONEYNAME "\n\n");
	outs("＊請不要惡意洗文章數，最重可處停權處分喔！\n");
	
	setuserfile(buf, "mission.dailypost.date");
	setuserfile(buf2, "mission.dailypost.num");
	if(isFileExist(buf) == false || isFileExist(buf2) == false){
		getdata(12, 0, "接下這個任務嗎？ (y)是 [N]否 ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
			return 0;
		}
		if (!(fp = fopen(buf, "w"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F01)");
			return 0;
		}
		fprintf(fp,"%s", date);
		fclose(fp);
		if (!(fp = fopen(buf2, "w"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F02)");
			return 0;
		}
		fprintf(fp,"%d", cuser.numposts);
		fclose(fp);
		vmsg("接下任務囉～");
		return 0;
	}else{
		if (!(fp = fopen(buf, "r"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F03)");
			return 0;
		}
		fgets(buf3, sizeof(buf3), fp);
		fclose(fp);
		if (!strcmp(buf3, date)){
			if (!(fp = fopen(buf2, "r"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F04)");
				return 0;
			}
			fgets(buf4, sizeof(buf4), fp);
			fclose(fp);
			if(!strcmp(buf4, "complete")){
				mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"已經完成過任務。"ANSI_RESET);
				pressanykey();
				return 0;
			}else{
				if (cuser.numposts > atoi(buf4)){
					if (!(fp = fopen(buf2, "w"))){
						vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F05)");
						return 0;
					}
					fprintf(fp,"complete");
					fclose(fp);
					pay(-100, "完成任務：每日發篇文章。");
					mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"恭喜完成任務！"ANSI_RESET);
					pressanykey();
					return 0;
				}else{
					mvouts(b_lines - 2, 35, "任務進行中");
					pressanykey();
					return 0;
				}
			}
		}else{
			getdata(12, 0, "接下這個任務嗎？ (y)是 [N]否 ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				return 0;
			}
			if (!(fp = fopen(buf, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F06)");
				return 0;
			}
			fprintf(fp,"%s", date);
			fclose(fp);
			if (!(fp = fopen(buf2, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F07)");
				return 0;
			}
			fprintf(fp,"%d", cuser.numposts);
			fclose(fp);
			vmsg("接下任務囉～");
			return 0;
		}
	}
}

/* 每月點首歌 */
int mission_monthlyos(){
    FILE *fp;
    char buf[200], buf2[200], buf3[200], buf4[200], date[11], genbuf[3];
    struct tm      ptime;
    localtime4_r(&now, &ptime);
	snprintf(date, sizeof(date), "%03d-%02d", ptime.tm_year - 11, ptime.tm_mon + 1);
	
	clear();
	vs_hdr2(" 任務中心 ", " 每月點首歌");
	move(2,0);
	outs("任務說明：\n"
		 "每月都來點一首歌吧 :D\n\n");
	outs("參加資格：不限\n");
	outs("任務獎勵：每月 150 " MONEYNAME "\n");
	
	setuserfile(buf, "mission.monthlyos.month");
	setuserfile(buf2, "mission.monthlyos.time");
	if(isFileExist(buf) == false || isFileExist(buf2) == false){
		getdata(12, 0, "接下這個任務嗎？ (y)是 [N]否 ",genbuf, 3, LCECHO);
		if (genbuf[0] != 'y') {
			return 0;
		}
		if (!(fp = fopen(buf, "w"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F01)");
			return 0;
		}
		fprintf(fp,"%s", date);
		fclose(fp);
		if (!(fp = fopen(buf2, "w"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F02)");
			return 0;
		}
		fprintf(fp,"%11d", cuser.lastsong);
		fclose(fp);
		vmsg("接下任務囉～");
		return 0;
	}else{
		if (!(fp = fopen(buf, "r"))){
			vmsg("系統錯誤，請稍後再試。(Error code: MIS-P-F03)");
			return 0;
		}
		fgets(buf3, sizeof(buf3), fp);
		fclose(fp);
		if (!strcmp(buf3, date)){
			if (!(fp = fopen(buf2, "r"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F04)");
				return 0;
			}
			fgets(buf4, sizeof(buf4), fp);
			fclose(fp);
			if(!strcmp(buf4, "complete")){
				mvouts(b_lines - 2, 32, ANSI_COLOR(1;33)"已經完成過任務。"ANSI_RESET);
				pressanykey();
				return 0;
			}else{
				if (cuser.lastsong > atoi(buf4)){
					if (!(fp = fopen(buf2, "w"))){
						vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F05)");
						return 0;
					}
					fprintf(fp,"complete");
					fclose(fp);
					pay(-100, "完成任務：每月點首歌。");
					mvouts(b_lines - 2, 33, ANSI_COLOR(1;32)"恭喜完成任務！"ANSI_RESET);
					pressanykey();
					return 0;
				}else{
					mvouts(b_lines - 2, 35, "任務進行中");
					pressanykey();
					return 0;
				}
			}
		}else{
			getdata(12, 0, "接下這個任務嗎？ (y)是 [N]否 ",genbuf, 3, LCECHO);
			if (genbuf[0] != 'y') {
				return 0;
			}
			if (!(fp = fopen(buf, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F06)");
				return 0;
			}
			fprintf(fp,"%s", date);
			fclose(fp);
			if (!(fp = fopen(buf2, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-O-F07)");
				return 0;
			}
			fprintf(fp,"%11d", cuser.lastsong);
			fclose(fp);
			vmsg("接下任務囉～");
			return 0;
		}
	}
}

/* 修改自己的信箱 */
int mission_email(){
    FILE *fp;
    char buf[200], buf2[200], genbuf[3];
	
	clear();
	vs_hdr2(" 任務中心 ", " 修改自己的信箱");
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

/* 已過期任務。
int mission_1stanniv(){
    FILE *fp;
    char buf[200], buf2[200], date[11], genbuf[3];
    int i, dateint, start = 1071210, end = 1071211;
    struct tm      ptime;
    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	snprintf(date, sizeof(date), "%03d%02d%02d", ptime.tm_year - 11, ptime.tm_mon + 1, ptime.tm_mday);
	dateint = atoi(date);
	
	clear();
	vs_hdr2("任務中心 ", " 大兔一週年站慶大紅包");
	move(2,0);
	outs("任務說明：\n"
		 "大兔一週年站慶大紅包！\n"
		 "還依稀記得去年的12月10日嗎？那是大兔站的生日！恭喜大兔正式開站一週年了！這一年\n"
		 "來大兔也與剛開站的樣貌有所不同了，好幾位大兔國民一起辛勤的耕耘，大兔已經累計了\n"
		 "超過5000篇文章了，現在的大兔欣欣向榮，這些都是大家一起寫下的歷史！\n"
		 "感謝這一年來大家對大兔的愛護與支持，有你們大兔才有一直走下去的動力！\n\n");
	outs("參加資格：所有國民一同歡慶！\n");
	outs("任務獎勵：5000 " MONEYNAME "\n");
	outs("任務開始：107年12月10日 00:00\n");
	outs("任務期限：107年12月11日 23:59\n");
	
	if(dateint < start){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;33)"還沒開始領取喔！"ANSI_RESET);
		pressanykey();
		return 0;
	}else if(dateint > end){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"超過領取期限了！"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		setuserfile(buf, "mission.1stanniv");
		if(isFileExist(buf) == false){
			if (!(fp = fopen(buf, "w"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-1ST-F01)");
				return 0;
			}
			fprintf(fp,"complete");
			fclose(fp);

			pay(-5000, "特殊獎勵：大兔一週年站慶大紅包。");
			mvouts(b_lines - 2, 28, ANSI_COLOR(1;32) "謝謝你陪伴大兔走過這一年" ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			if (!(fp = fopen(buf, "r"))){
				vmsg("系統錯誤，請稍後再試。(Error code: MIS-1ST-F02)");
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
					vmsg("系統錯誤，請稍後再試。(Error code: MIS-1ST-F01)");
					return 0;
				}
				fprintf(fp,"complete");
				fclose(fp);

				pay(-5000, "特殊獎勵：大兔一週年站慶大紅包。");
				mvouts(b_lines - 2, 28, ANSI_COLOR(1;32) "謝謝你陪伴大兔走過這一年" ANSI_RESET);
				pressanykey();
				return 0;
			}
		}
	}
}
*/

/* 任務列表 */
int mission_main(void)
{
	int i;
    char genbuf[3];

VIEWLIST:
	clear();
	vs_hdr2(" " BBSNAME " ", " 任務中心");
	move(1,0);
	vbarf(ANSI_REVERSE " 編號  任務標題\t獎勵 \n");
	vbarf("   1.  [每日] 登入大兔\t100 " MONEYNAME "\n");
	vbarf("   2.  [每日] 發篇文章\t100 " MONEYNAME "\n");
	vbarf("   3.  [每月] 點首歌\t150 " MONEYNAME "\n");
	vbarf("   4.  [單次] 修改自己的信箱\t250 " MONEYNAME "\n");
	
	getdata(b_lines - 1, 0, "輸入任務編號檢視詳細說明、開始任務或領取獎勵，輸入[Q]離開 ",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		mission_dailylogin();
		goto VIEWLIST;
	}
	if (genbuf[0] == '2') {
		mission_dailypost();
		goto VIEWLIST;
	}
	if (genbuf[0] == '3') {
		mission_monthlyos();
		goto VIEWLIST;
	}
	if (genbuf[0] == '4') {
		mission_email();
		goto VIEWLIST;
	}
	
    return 0;
}

#endif
