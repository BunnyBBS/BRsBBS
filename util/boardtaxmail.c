#include "bbs.h"

int main(void)
{
    FILE           *fp, *fp2;
    char           *ptr, *id, *mn;
    char            buf[200] = "", cmd[200];
    int             i, money = 0, count=0;
	int		delayday=0, delaypay=0;
	int		tax=0, shouldpay=0, paid=0;
    struct tm      ptime;

    localtime4_r(&now, &ptime);
    i = ptime.tm_wday << 1;
	
	if(!(fp = fopen(BBSHOME "/etc/boardtax.txt", "r+"))){
		return 0;
	}else{
		while (fgets(buf, sizeof(buf), fp)) {
			if (!(ptr = strchr(buf, ':')))
			continue;
			*ptr = '\0';
			id = buf;
			mn = ptr + 1;
			money = atoi(mn);
			sprintf(cmd, BBSHOME "/bin/mmail %s \"本月看板稅開徵通知\" \"[金融管理署]\" \"" BBSHOME "/etc/boardtaxmail.txt\"", id);
			system(cmd);
			count++;
		}
		fclose(fp);
		return 0;
	}
}