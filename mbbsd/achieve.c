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
	vs_hdr2(" " BBSNAME " ", " §Úªº¦¨´N¾±³¹");
	setuserfile(buf, "achieve");
	if(fp = fopen(buf, "r")){
		while (fgets(buf2, sizeof(buf2), fp)) {
			if(buf2[strlen(buf2) - 1] == '\n')
				buf2[strlen(buf2) - 1] = '\0';
			if(buf2[0] != '\0'){
				move(1,0);clrtobot();
				i = count + 1;
				outs("¦¨´N¦WºÙ¡G");outs(getAchName(buf2,false));outs("\n");
				outs("¦¨´N»¡©ú¡G");outs(getAchDesc(buf2));outs("\n");
				outs("¦¨´NÄÝ©Ê¡G");outs(getAchAttr(buf2));outs("\n");
				outs("\nª¬ºA¡G");
				if(strcmp(cuser.achieve, buf2) == 0)
					outs(ANSI_COLOR(1;32)"¨Ï¥Î¤¤"ANSI_RESET);
				else
					outs("¥¼¨Ï¥Î");
				outs("\n");

				if(strcmp(cuser.achieve, buf2) == 0){
					getdata(b_lines - 1, 0, "[ENTER] ¤U¤@­Ó  (U) ©Þ±¼¾±³¹ ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'u'){
						strlcpy(cuser.achieve, "", sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("©Þ±¼Åo¡I");
						return 0;
					}
				}else{
					getdata(b_lines - 1, 0, "[ENTER] ¤U¤@­Ó  (W) °tÀ¹¾±³¹ ",genbuf, 3, LCECHO);
					if(genbuf[0] == 'w'){
						strlcpy(cuser.achieve, buf2, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("À¹¤WÅo¡I");
						return 0;
					}
				}
			}
			count++;
		}
		fclose(fp);
	}else{
		vmsg("§AÁÙ¨S¦³¦¨´N¾±³¹¡A§A¥i¥H¦b¤j¨ß¶W¥«¸ÌÁÊ¶R¡C");
	}
    return 0;
}

/* ¤w¹L´Á¡C
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
	vs_hdr2("¦¨´N¾±³¹°Ó©± ", " ");
	move(2,0);
	outs("¦¨´N¦WºÙ¡G");outs(getAchName(achieve,false));outs("\n");
	outs("¦¨´N»¡©ú¡G");outs(getAchDesc(achieve));outs("\n");
	outs("¦¨´NÄÝ©Ê¡G");outs(getAchAttr(achieve));outs("\n");
	outs("¦¨´N»ù®æ¡G100 " MONEYNAME "\n\n");
	outs("¶}©l³c°â¡G107¦~12¤ë10¤é 00:00\n");
	outs("µ²§ô³c°â¡G107¦~12¤ë11¤é 23:59\n\n");
	
	if(dateint < start){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;33)"ÁÙ¨S¶}©l³c°â³á¡I"ANSI_RESET);
		pressanykey();
		return 0;
	}else if(dateint > end){
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"¶W¹LÁÊ¶R´Á­­¤F¡I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		if(doUserOwnAch(achieve) == 1){
			mvouts(b_lines - 2, 34, ANSI_COLOR(1;32)"¤wÁÊ¶R¹L¡I"ANSI_RESET);
			pressanykey();
			return 0;
		}else{
			getdata(b_lines - 2, 0, "(B) ÁÊ¶R  [Q] ªð¦^ ",genbuf, 3, LCECHO);
			if(genbuf[0] == 'b') {
				getdata(b_lines - 1, 0, "½T©wÁÊ¶R¶Ü¡H (y/N)",genbuf, 3, LCECHO);
				if (genbuf[0] != 'y') {
					vmsg("¨ú®ø¤ä¥I");
					return 0;
				}else{
					reload_money();
					if (cuser.money < 100){
						vmsg(MONEYNAME "¤£°÷ÁÊ¶R¦¨´N¡K");
						return 0;
					}else{
						setuserfile(buf, "achieve");
						if(fp = fopen(buf, "a")){
							pay(100, "ÁÊ¶R¦¨´N¡G%s", getAchName(achieve,true));
							fprintf(fp,"%s\n", achieve);
							fclose(fp);
							mvouts(b_lines - 2, 0, "¤wÁÊ¶R¡I¥i¥H¦b­Ó¤H³]©w°Ï°tÀ¹¦¨´N¾±³¹¡C");
							getdata(b_lines - 1, 0, "ÁÙ¬O§A­n²{¦b°tÀ¹¶Ü¡H (y/N)",genbuf2, 3, LCECHO);
							if(genbuf2[0] == 'y'){
								strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
								passwd_update(usernum, &cuser);
								vmsg("À¹¤WÅo¡I");
								return 0;
							}
							vmsg("¤wÁÊ¶R¡I");
							return 0;
						}else{
							vmsg("µ{¦¡¿ù»~¡K");
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
	vs_hdr2(" ¦¨´N¾±³¹°Ó©± ", " ÁÊ¶R¦¨´N");
	move(2,0);
	outs("¦¨´N¦WºÙ¡G");outs(getAchName(achieve,false));outs("\n");
	outs("¦¨´N»¡©ú¡G");outs(getAchDesc(achieve));outs("\n");
	outs("¦¨´NÄÝ©Ê¡G");outs(getAchAttr(achieve));outs("\n");
	outs("¨ú±o±ø¥ó¡Gµo¤å500½g\n\n");
	outs("¡¯½Ð¤£­n´c·N¬~¤å³¹¼Æ¡A³Ì­«¥i³B°±Åv³B¤À³á¡I\n");

	if(doUserOwnAch(achieve) == 1){
		mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"¤w¾Ö¦³¤F¡I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		getdata(b_lines - 2, 0, "(B) ¨ú±o  [Q] ªð¦^ ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'b') {
			if(cuser.numposts >= 500){
				setuserfile(buf, "achieve");
				if(fp = fopen(buf, "a")){
					fprintf(fp,"%s\n", achieve);
					fclose(fp);
					mvouts(b_lines - 2, 0, "¤w¨ú±o¡I¥i¥H¦b­Ó¤H³]©w°Ï°tÀ¹¦¨´N¾±³¹¡C");
					getdata(b_lines - 1, 0, "ÁÙ¬O§A­n²{¦b°tÀ¹¶Ü¡H (y/N)",genbuf2, 3, LCECHO);
					if(genbuf2[0] == 'y'){
						strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("À¹¤WÅo¡I");
						return 0;
					}
					vmsg("¤wÁÊ¶R¡I");
					return 0;
				}else{
					vmsg("µ{¦¡¿ù»~¡K");
					return 0;
				}
			}else{
				vmsgf("¤å³¹½g¼ÆÁÙ¯Ê%d½g³á¡A§ï¤Ñ¦A¨Ó§a¡C", (500 - cuser.numposts));
				return 0;
			}
		}
	}
}

int achieve_buy_365login(char *achieve){
	FILE *fp;
	char buf[200], date[11], genbuf[3], genbuf2[3];

	clear();
	vs_hdr2(" ¦¨´N¾±³¹°Ó©± ", " ÁÊ¶R¦¨´N");
	move(2,0);
	outs("¦¨´N¦WºÙ¡G");outs(getAchName(achieve,false));outs("\n");
	outs("¦¨´N»¡©ú¡G");outs(getAchDesc(achieve));outs("\n");
	outs("¦¨´NÄÝ©Ê¡G");outs(getAchAttr(achieve));outs("\n");
	outs("¨ú±o±ø¥ó¡Gµn¤J365¦¸\n\n");

	if(doUserOwnAch(achieve) == 1){
		mvouts(b_lines - 2, 35, ANSI_COLOR(1;32)"¤w¾Ö¦³¤F¡I"ANSI_RESET);
		pressanykey();
		return 0;
	}else{
		getdata(b_lines - 2, 0, "(B) ¨ú±o  [Q] ªð¦^ ",genbuf, 3, LCECHO);
		if(genbuf[0] == 'b') {
			if(cuser.numlogindays >= 365){
				setuserfile(buf, "achieve");
				if(fp = fopen(buf, "a")){
					fprintf(fp,"%s\n", achieve);
					fclose(fp);
					mvouts(b_lines - 2, 0, "¤w¨ú±o¡I¥i¥H¦b­Ó¤H³]©w°Ï°tÀ¹¦¨´N¾±³¹¡C");
					getdata(b_lines - 1, 0, "ÁÙ¬O§A­n²{¦b°tÀ¹¶Ü¡H (y/N)",genbuf2, 3, LCECHO);
					if(genbuf2[0] == 'y'){
						strlcpy(cuser.achieve, achieve, sizeof(cuser.achieve));
						passwd_update(usernum, &cuser);
						vmsg("À¹¤WÅo¡I");
						return 0;
					}
					vmsg("¤wÁÊ¶R¡I");
					return 0;
				}else{
					vmsg("µ{¦¡¿ù»~¡K");
					return 0;
				}
			}else{
				vmsgf("µn¤J¦¸¼ÆÁÙ¯Ê%d¦¸³á¡A§ï¤Ñ¦A¨Ó§a¡C", (365 - cuser.numlogindays));
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
	vs_hdr2(" " BBSNAME " ", " ¦¨´N¾±³¹°Ó©±");
	move(1,0);
	vbarf(ANSI_REVERSE " ½s¸¹  ¦¨´N¦WºÙ\t¨ú±o±ø¥ó          \n");
	vbarf("   1.  ¤j¨ß¤@¶g¦~¯¸¼y¬ö©À [µ}¦³]\t100 " MONEYNAME " ¤w°±°â \n");
	vbarf("   2.  §V¤Oªº¼g [´¶³q]\tµo¤å500½g         \n");
	vbarf("   3.  1·³¤p¨ß¤l [´¶³q]\tµn¤J365¦¸         \n");
	outs("\n·q½Ð´Á«Ý§ó¦h¦¨´N¾±³¹¡K\n");
	getdata(b_lines - 1, 0, "¿é¤J½s¸¹ÀËµø¸Ô²Ó»¡©ú©ÎÁÊ¶R¡A¿é¤J[Q]Â÷¶} ",genbuf, 3, LCECHO);
	
	if (genbuf[0] == '1') {
		move(b_lines - 3, 0); clrtobot();
		mvouts(b_lines - 2, 31, ANSI_COLOR(1;31)"¶W¹LÁÊ¶R´Á­­¤F¡I"ANSI_RESET);
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
	vs_hdr2(" " BBSNAME " ", " ¬d¸ß¦¨´N¾±³¹¸Ô±¡");
	
	move(2,0);
	outs("¦¨´N¦WºÙ¡G");outs(getAchName(achieve,false));outs("\n");
	outs("¦¨´N»¡©ú¡G");outs(getAchDesc(achieve));outs("\n");
	outs("¦¨´NÄÝ©Ê¡G");outs(getAchAttr(achieve));outs("\n");
	pressanykey();

    return 0;
}

#endif //USE_ACHIEVE
