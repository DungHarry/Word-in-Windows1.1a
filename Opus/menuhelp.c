/* menuhelp.c */

#define RSHDEFS

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "status.h"
#include "prompt.h"
#include "cmdtbl.h"
#include "opuscmd.h"
#include "screen.h"
#include "disp.h"


#ifdef PROTOTYPE
#include "menuhelp.cpt"
#endif /* PROTOTYPE */

#ifdef PCJ
#define DMENUHELP
#endif /* PCJ */

extern int wwCur;
extern struct PPR **vhpprPRPrompt;
extern HWND vhwndStatLine;
extern struct SCI vsci;
extern HMENU vhMenu;
extern struct PREF vpref;
extern CHAR szEmpty[];
extern CHAR vstPrompt[];
extern int vfSysMenu;
char *PchGetStr();

/* This file is generated by a series of processes from OpusCmd.CMD 
*  it contains the help prompt strings in compressed form.
*/
#define CODE csconst
#include "menuhelp.h"


/* Helper strings for the actual menus. */
csconst char csrgstMenuBarHelp [] [] =
{
	StSharedKey("Move, size or close application window",MenuBarWinPosQuit),
			StSharedKey("Move, size or close document window",MenuBarWinPosClose),
			StSharedKey("Create, open, save, print documents or quit Word",MenuBarCreateOpenSave),
			StSharedKey("Undo, delete, copy, insert, search, replace and Go To",MenuBarUndoDeleteSearch),
			StSharedKey("Choose alternative views, on-screen formatting tools or menu mode",MenuBarViewsAids),
			StSharedKey("Insert various types of text and graphics",MenuBarInsert),
			StSharedKey("Format text and graphics",MenuBarFormat),
			StSharedKey("Check, polish, and finish text and graphics",MenuBarPolish),
			StSharedKey("Create, edit, run, and assign macros",MenuBarMacros),
#define imnuMacro 8
			StSharedKey("Rearrange windows or activate specified window",MenuBarSwitch),
#ifdef DEBUG
			StShared("Debug tests and settings"),
#endif
			StSharedKey("Get help or start interactive lessons",MenuBarHelp)
};


#define imnuFile        2
#ifdef DEBUG
#define imnuHelp        11
#define imnuMax         12
#else
#define imnuHelp        10
#define imnuMax         11
#endif



/* D R A W  M E N U  H E L P */
/* Called from WM_MENUSELECT handler... */
/* %%Function:DrawMenuHelp %%Owner:krishnam */
DrawMenuHelp(hMenu, bcm, mf)
HMENU hMenu;
BCM bcm;
int mf;
{
	char FAR * lpst;
	int cch;
	int imnu;

	Assert(vhwndStatLine != NULL);

#ifdef DMENUHELP
	CommSzNum(SzShared("DrawMenuHelp: "), hMenu);
	CommSzNum(SzShared("         bcm: "), bcm);
	CommSzNum(SzShared("          mf: "), mf);
	CommSzNum(SzShared("      vhMenu: "), vhMenu);
#endif

	Assert(vhpprPRPrompt == hNil);
	vstPrompt[0] = '\0';

#ifdef RSH
	StartSubTimer(uasMenu);
#endif /* RSH */

	if (hMenu == NULL)
		{
		RestorePrompt();
		return;
		}
	else  if ((mf & MF_POPUP) != 0)
		{
/* Put up description of the menu */
		/* NOTE: This line has been split to prevent opus bug
			6162.  If the following two lines are converted to
			one then the cs compiler will first save &csrgstMenuBarHelp
			away before calling ImnuFromHMenu.  Since it in
			turn might call GetSubMenu and potentially move this
			module the saved far pointer might be invalidated and
			we could have a garbage string on our hands.
			Brad Verheiden - 4-12-89.
		*/
		imnu = ImnuFromHMenu(hMenu, bcm);
		lpst = csrgstMenuBarHelp[imnu];
		cch = min (*lpst+1, cchPromptMax);
		bltbx(lpst, (char FAR *) vstPrompt, cch);
		vstPrompt[0] = cch-1;
		goto LHaveString;
		}
	else  if (mf & MF_SYSMENU)
		bcm = BcmFromSysMenu(bcm);

		{
		CHAR szT[256];
		GetMenuHelpSz(bcm, szT);
		SzToStInPlace(szT);
		*szT = min (*szT, cchPromptMax-1);
		CopySt (szT, vstPrompt);
		}

LHaveString:
	DisplayStatLinePrompt (pdcMenuHelp);
}


/* %%Function:ImnuFromMenu %%Owner:krishnam */
int ImnuFromHMenu(hMenu, bcm)
HMENU hMenu;
{
	int imnu = 0;
	HMENU hMenuT;

	if (hMenu != vhMenu)
		return 0;    /* it's the system menu */

	while ((hMenuT = GetSubMenu(vhMenu, imnu++)) != NULL)
		{
		if (hMenuT == bcm)
			{
			if (!vfSysMenu)
				imnu++;

			if (imnu >= imnuMacro && vpref.fShortMenus)
				imnu++;

			if (wwCur == wwNil && imnu > imnuFile)
				imnu = imnuHelp;

			Assert(imnu < imnuMax);
			return imnu;
			}
		}

	/* get here iff doc !maximized and on hyphen menu (doc control) */
	return 1;
}


/* Convert standard system menu things to bcm's */
/* %%Function:BcmFromSysMenu %%Owner:krishnam */
int BcmFromSysMenu(bcm)
int bcm;
{
	switch (bcm & 0xfff0)
		{
	case SC_SIZE:
		return bcmSizeWnd;

	case SC_MOVE:
		return bcmMoveWnd;

	case SC_MINIMIZE:
		return bcmAppMinimize;

	case SC_MAXIMIZE:
		return bcmZoomWnd;

	case SC_CLOSE:
		return bcmExit;

	case SC_NEXTWINDOW:
	case SC_PREVWINDOW:
	case SC_VSCROLL:
	case SC_HSCROLL:
	case SC_MOUSEMENU:
	case SC_KEYMENU:
		return bcmNil;

	case SC_ARRANGE:
		return bcmArrangeWnd;

	case SC_RESTORE:
		return bcmRestoreWnd;
		}
	return bcm;
}


/* %%Function:GetMenuHelpSz %%Owner:krishnam */
GetMenuHelpSz(bcm, sz)
BCM bcm;
char * sz;
{
	extern struct STTB ** hsttbMenuHelp;
	extern HPSYT vhpsyt;

	HPSY hpsy;

	*sz = '\0';

	if ((int) bcm >= 0)
		{
		char HUGE * hpst;
		char rgchSy [cbMaxSy];

		hpsy = (HPSY) (vhpsyt->grpsy + bcm);
		if (hpsy->stName[0] == 0)
			{
			hpsy = (HPSY) (vhpsyt->grpsy + 
				*((uns HUGE *) (hpsy->stName + 1)));
			}

		switch (hpsy->mct)
			{
		case mctCmd:
		case mctSdm:
			if (hpsy->iidstr == iidstrNil)
				break;
			*PchGetStr((char far *)rgsz[hpsy->iidstr], sz) = '\0';
			break;

		case mctMacro:
		case mctGlobalMacro:
		case mctNil:
			if (hsttbMenuHelp != hNil)
				{
				int ibstP1;
				
				if ((ibstP1 = hpsy->ibstMenuHelpP1) != 0)
					{
					GetSzFromSttb(hsttbMenuHelp, ibstP1-1,
							sz);
					}
				}
			break;
			}
		}
}


#define chTokenMin      (256-cTokenMax)

/* PchGetStr - convert tokenized string to non-tokenized form. This routine
*  is recursive as token strings can themselves have tokens in them.
*
*  Tokens are represented in strings as a one byte code with the high bit set.
*  The lower 7 bits is the index into rgszToken. This means we have a problem
*  with international characters (think big etc...). What we do is map common
*  international characters to control codes (i.e. below cExtMacChar which is
*  kept below SPACE). If there are more than can be handled this way the
*  remainder are placed as a two bytes sequence, the first being NULL. If there
*  are no extended characters then stringpp will not generate the mapping 
*  tables.
*/
/* %%Function:PchGetStr %%Owner:krishnam */
char *PchGetStr(lpch, pch)
char far *lpch;
char *pch;
{
	unsigned int ch;
	int ich;

	for (ich = *lpch++; ich > 0; ich--)
		{
		if ((ch = *pch++ = *lpch++) & 0x80)
			{
			/* recurse to get token within a token */
			pch = PchGetStr((char far *)rgszToken[ch-chTokenMin], pch - 1);
			}
#ifdef cExtCharMac
		else  if (ch < cExtCharMac)
			{
			if (ch == 0)
				{
				/* two byte extended character */
				ich--;
				ch = *lpch++;
				}
			else
				ch = rgExtChar[ch];
			*(pch-1) = ch;
			}
#endif /* cExtCharMac */
		}

	return(pch);    /*      pch points to byte after last character.
							no terminating zero is added.   */
}


