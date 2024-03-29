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
    FILE *fp, *fp2;

	clear();
	vs_hdr2(" " BBSNAME " ", " 我的成就勳章");
	setuserfile(buf, "achieve");
	if(fp = fopen(buf, "r")){
		while (fgets(buf2, sizeof(buf2), fp)) {
			if(buf2[strlen(buf2) - 1] == '\n')
				buf2[strlen(buf2) - 1] = '\0';
			if(buf2[0] != '\0'){
				move(1,0);clrtobot();
				i = count + 1;
				outs("成就名稱：");outs(getAchName(buf2,false));outs("\n");
				outs("成就屬性：");outs(getAchAttr(buf2));outs("\n");

				outs("成就說明：\n");
				snprintf(buf3, sizeof(buf3), "achieve/%s.desc", buf2);
				if(fp2 = fopen(buf3, "r")){
					while(fgets(buf3, sizeof(buf3), fp2)){
						outs(buf3);
					}
				}

				mvouts(b_lines - 2, 0, "狀態：");
				if(strcmp(cuser.achieve, buf2) == 0)
					outs(ANSI_COLOR(1;32)"使用中"ANSI_RESET);
				else
					outs("未使用");
				outs("\n");

				if(strcmp(cuser.achieve, buf2) == 0){
					getdata(b_lines - 1, 0, "[N] 下一個  (U) 拔掉勳章 ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'u'){
						strlcpy(cuser.achieve, "", sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("拔掉囉！");
						return 0;
					}
				}else{
					getdata(b_lines - 1, 0, "[N] 下一個  (W) 配戴勳章 ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'w'){
						strlcpy(cuser.achieve, buf2, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("戴上囉！");
						return 0;
					}
				}
			}
			count++;
		}
		fclose(fp);
	}else{
		vmsg("你還沒有成就勳章，你可以在大兔超市裡購買。");
	}
    return 0;
}

/* 已過期。
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
	vs_hdr2("成就勳章商店 ", " ");
	move(2,0);
	outs("成就名稱：");outs(getAchName(achieve,false));outs("\n");
	outs("成就說明：");outs(getAchDesc(achieve));outs("\n");
	outs("成就屬性：");outs(getAchAttr(achieve));outs("\n");
	outs("成就價格：100 " MONEYNAME "\n\n");
	outs("開始販售：107年12月10日 00:00\n");
	outs("結束販售：107年12月11日 23:59\n\n");
	
	if(dateint < start){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;33)"還沒開始販售喔！"ANSI_RESET);
		pressanykey();
		return 0;
	}else if(dateint > end){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"超過購買期限了！"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		if(doUserOwnAch(achieve) == 1){
			mvouts(b_lines - 2, 34, ANSI_COLOR(1;32)"已購買過！"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			getdata(b_lines - 2, 0, "(B) 購買  [Q] 返回 ",genbuf, 3, LCECHO);
			if(genbuf[0] == 'b') {
				getdata(b_lines - 1, 0, "確定購買嗎？ (y/N)",genbuf, 3, LCECHO);
				if (genbuf[0] != 'y') {
					vmsg("取消支付");
					return 0;
				}else{
					reload_money();
					if (cuser.money < 100){
						vmsg(MONEYNAME "不夠購買成就…");
						return 0;
					}else{
						setuserfile(buf, "achieve");
						if(fp = fopen(buf, "a")){
							pay(100, "購買成就：%s", getAchName(achieve,true));
							fprintf(fp,"%s\n", achieve);
							fclose(fp);
							mvouts(b_lines - 2, 0, "已購買！可以在個人設定區配戴成就勳章。");
							getdata(b_lines - 1, 0, "還是你要現在配戴嗎？ (y/N)",genbuf2, 3, LCECHO);
							if(genbuf2[0] == 'y'){
								strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
								passwd_update(usernum, &cuser);
								vmsg("戴上囉！");
								return 0;
							}
							vmsg("已購買！");
							return 0;
						}else{
							vmsg("程式錯誤…");
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
	vs_hdr2(" 成就勳章商店 ", " 購買成就");
	move(2,0);
	outs("成就名稱：");outs(getAchName(achieve,false));outs("\n");
	outs("成就說明：");outs(getAchDesc(achieve));outs("\n");
	outs("成就屬性：");outs(getAchAttr(achieve));outs("\n");
	outs("取得條件：發文500篇\n\n");
	outs("＊請不要惡意洗文章數，最重可處停權處分喔！\n");

	if(doUserOwnAch(achieve) == 1){
		mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"已擁有了！"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		getdata(b_lines - 2, 0, "(B) 取得  [Q] 返回 ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'b') {
			if(cuser.numposts >= 500){
				setuserfile(buf, "achieve");
				if(fp = fopen(buf, "a")){
					fprintf(fp,"%s\n", achieve);
					fclose(fp);
					mvouts(b_lines - 2, 0, "已取得！可以在個人設定區配戴成就勳章。");
					getdata(b_lines - 1, 0, "還是你要現在配戴嗎？ (y/N)",genbuf2, 3, LCECHO);
					if(genbuf2[0] == 'y'){
						strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("戴上囉！");
						return 0;
					}
					vmsg("已購買！");
					return 0;
				}else{
					vmsg("程式錯誤…");
					return 0;
				}
			}else{
				vmsgf("文章篇數還缺%d篇喔，改天再來吧。", (500 - cuser.numposts));
				return 0;
			}
		}
	}
}

int achieve_buy_365login(char *achieve){
	FILE *fp;
	char buf[200], date[11], genbuf[3], genbuf2[3];

	clear();
	vs_hdr2(" 成就勳章商店 ", " 購買成就");
	move(2,0);
	outs("成就名稱：");outs(getAchName(achieve,false));outs("\n");
	outs("成就說明：");outs(getAchDesc(achieve));outs("\n");
	outs("成就屬性：");outs(getAchAttr(achieve));outs("\n");
	outs("取得條件：登入365次\n\n");

	if(doUserOwnAch(achieve) == 1){
		mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"已擁有了！"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		getdata(b_lines - 2, 0, "(B) 取得  [Q] 返回 ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'b') {
			if(cuser.numlogindays >= 365){
				setuserfile(buf, "achieve");
				if(fp = fopen(buf, "a")){
					fprintf(fp,"%s\n", achieve);
					fclose(fp);
					mvouts(b_lines - 2, 0, "已取得！可以在個人設定區配戴成就勳章。");
					getdata(b_lines - 1, 0, "還是你要現在配戴嗎？ (y/N)",genbuf2, 3, LCECHO);
					if(genbuf2[0] == 'y'){
						strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("戴上囉！");
						return 0;
					}
					vmsg("已購買！");
					return 0;
				}else{
					vmsg("程式錯誤…");
					return 0;
				}
			}else{
				vmsgf("登入次數還缺%d次喔，改天再來吧。", (365 - cuser.numlogindays));
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
	vs_hdr2(" " BBSNAME " ", " 成就勳章商店");
	move(1,0);
	vbarf(ANSI_REVERSE " 編號  成就名稱\t取得條件          \n");
	vbarf("   1.  大兔一週年站慶紀念 [稀有]\t100 " MONEYNAME " 已停售 \n");
	vbarf("   2.  努力的寫 [普通]\t發文500篇         \n");
	vbarf("   3.  1歲小兔子 [普通]\t登入365次         \n");
	outs("\n敬請期待更多成就勳章…\n");
	getdata(b_lines - 1, 0, "輸入編號檢視詳細說明或購買，輸入[Q]離開 ",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		move(b_lines - 3, 0); clrtobot();
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"超過購買期限了！"ANSI_RESET);
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
	FILE *fp;
	char buf[200];

	clear();
	vs_hdr2(" " BBSNAME " ", " 查詢成就勳章詳情");
	
	move(2,0);
	outs("成就名稱：");outs(getAchName(achieve,false));outs("\n");
	outs("成就屬性：");outs(getAchAttr(achieve));outs("\n");

	outs("成就說明：\n");
	snprintf(buf, sizeof(buf), "achieve/%s.desc", achieve);
	if(fp = fopen(buf, "r")){
		while(fgets(buf, sizeof(buf), fp)){
			outs(buf);
		}
	}
	pressanykey();

    return 0;
}

#endif //USE_ACHIEVE
