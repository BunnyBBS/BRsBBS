#include "bbs.h"

#ifdef USE_BBS2WEB
/* 這個程式是將所有的看板的設定全列出來，供網站串接同步看板設定
 * 產生格式：
 * bid, gid, isBrd, name, title, mod, hide, no_post, friend_post, no_reply, no_money, no_push, push_ip_rec, push_align
 */
void brdDetail(void)
{
    int     i, k, bid;
    boardheader_t  *bptr;
    char    BM[IDLEN * 3 + 3], *p;
    char    smallbrdname[IDLEN + 1];
    for( i = 0 ; i < MAX_BOARD ; ++i ){
	bptr = &bcache[i];
	
	if( !bptr->brdname[0] ||
	    (bptr->brdattr & BRD_TOP) ||
	    (bptr->level && !(bptr->brdattr & BRD_POSTMASK) &&
	     (bptr->level & 
	      ~(PERM_BASIC|PERM_CHAT|PERM_PAGE|PERM_POST|PERM_LOGINOK))) )
	    continue;

	for( k = 0 ; bptr->brdname[k] ; ++k )
	    smallbrdname[k] = (isupper(bptr->brdname[k]) ? 
			       tolower(bptr->brdname[k]) :
			       bptr->brdname[k]);
	smallbrdname[k] = 0;

	bid = bptr - bcache + 1;
	printf("%d,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d\n",
			bid, bptr->gid, ((bptr->brdattr & BRD_GROUPBOARD) ? 0 : 1),
			bptr->brdname, &bptr->title[7], bptr->BM,
			((bptr->brdattr & BRD_HIDE) ? 1 : 0), ((bptr->brdattr & BRD_NOPOST) ? 1 : 0),
			((bptr->brdattr & BRD_RESTRICTEDPOST) ? 1 : 0), ((bptr->brdattr & BRD_NOREPLY) ? 1 : 0), 
			((bptr->brdattr & BRD_NOCREDIT) ? 1 : 0), ((bptr->brdattr & BRD_NORECOMMEND) ? 1 : 0),
			((bptr->brdattr & BRD_IPLOGRECMD) ? 1 : 0), ((bptr->brdattr & BRD_ALIGNEDCMT) ? 1 : 0));
	}
}

int main(int argc, const char **argv)
{
    attach_SHM();
	brdDetail();
	return 0;
}
#else
int main(int argc, const char **argv)
{
	return 0;
}
#endif //USE_BBS2WEB