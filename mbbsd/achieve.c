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
		if(fp = fopen(buf, "r+")){
			fgets(buf, sizeof(buf), fp);
			fclose(fp);
			if(buf[strlen(buf) - 1] == '\n')
				buf[strlen(buf) - 1] = '\0';
			color = buf;
		}else
			color = "";
	}

	snprintf(buf2, sizeof(buf2), "achieve/%s.name", achieve);
	if(fp = fopen(buf2, "r+")){
		fgets(buf2, sizeof(buf2), fp);
		fclose(fp);
		if(buf2[strlen(buf2) - 1] == '\n')
			buf2[strlen(buf2) - 1] = '\0';
		name = buf2;
	}else
		return NULL;

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
	if(fp = fopen(buf, "r+")){
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
	if(fp = fopen(buf, "r+")){
		fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		return buf;
	}else{
		return NULL;
	}
}

int achieve_user()
{
	int i=0, count=0;
    char genbuf[3];
    char buf[200], buf2[200], buf3[200];
    FILE *fp;

	clear();
	vs_hdr2(" " BBSNAME " ", " 我的成就勳章");
	setuserfile(buf, "achieve");
	if(fp = fopen(buf, "r")){
		while (fgets(buf2, sizeof(buf2), fp)) {
			if(buf2[strlen(buf2) - 1] == '\n')
				buf2[strlen(buf2) - 1] = '\0';
			if(getAchName(buf2,true) != NULL){
				move(1,0);clrtobot();
				i = count + 1;
				outs("成就名稱：");outs(getAchName(buf2,false));outs("\n");
				outs("成就說明：");outs(getAchDesc(buf2));outs("\n");
				outs("成就屬性：");outs(getAchAttr(buf2));outs("\n");
				outs("\n狀態：");
				if(strcmp(cuser.achieve, buf2) == 0)
					outs(ANSI_COLOR(1;32)"使用中"ANSI_RESET);
				else
					outs("未使用");
				outs("\n");

				if(strcmp(cuser.achieve, buf2) == 0){
					getdata(b_lines - 1, 0, "[ENTER] 下一個  (U) 拔掉勳章 ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'u'){
						strlcpy(cuser.achieve, "", sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("拔掉囉！");
						return 0;
					}
				}else{
					getdata(b_lines - 1, 0, "[ENTER] 下一個  (W) 配戴勳章 ",genbuf, 3, LCECHO);
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

int achieve_buy_1stanniv(char *achieve){
    FILE *fp;
    char buf[200], date[11], genbuf[3], genbuf2[3];
    int i, dateint, start = 1071210, end = 1071216;
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
	outs("結束販售：107年12月16日 23:59\n\n");
	
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
						vmsg(MONEYNAME "不夠購買…");
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

int achieve_shop()
{
	int i;
    char genbuf[3];

VIEWLIST:
	clear();
	vs_hdr2(" " BBSNAME " ", " 成就勳章商店");
	move(1,0);
	vbarf(ANSI_REVERSE " 編號  成就名稱\t價格 \n");
	vbarf("   1.  大兔一週年站慶紀念   \[1;31m限時販售\[m\t100 " MONEYNAME " \n");
	vbarf("   2.  努力地寫\t規劃中… \n");
	vbarf("   3.  超級寫手\t規劃中… \n");
	outs("\n敬請期待更多成就勳章…\n");
	getdata(b_lines - 1, 0, "輸入編號檢視詳細說明或購買，輸入[Q]離開 ",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		achieve_buy_1stanniv("1stanniv");
		goto VIEWLIST;
	}
	
    return 0;
}

int achieve_view(char *achieve)
{
	clear();
	vs_hdr2(" " BBSNAME " ", " 查詢成就勳章詳情");
	
	move(2,0);
	outs("成就名稱：");outs(getAchName(achieve,false));outs("\n");
	outs("成就說明：");outs(getAchDesc(achieve));outs("\n");
	outs("成就屬性：");outs(getAchAttr(achieve));outs("\n");
	pressanykey();

    return 0;
}

#endif //USE_ACHIEVE