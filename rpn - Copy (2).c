#undef MIN_AMS

#define MIN_AMS 101
#define OPTIMIZE_ROM_CALLS
#define MLINK_FORMAT_RELOCS
#define OPTIMIZE_CALC_CONSTS
#define NO_CALC_DETECT

#include <tigcclib.h>



void _main() {
#if 0
  TRY
    push_END ();
    push_parse_text ("sqrt(x)");
    NG_rationalESI (top_estack);
    WinClr (DeskTop);
    Print2DExpr (Parse2DExpr (top_estack, FALSE), DeskTop, 0, 50);
  ONERR
    DrawStr (20, 20, "Error!", A_NORMAL);
  ENDTRY
  ngetchx ();
#endif
#if 0
	ESI x = (ESI) bottom_estack;
	clrscr();
	while (x != top_estack+1) {
		printf("%02x ",*x++);
	}
	ngetchx();
  TRY
    WinClr (DeskTop);
    Print2DExpr (Parse2DExpr (top_estack, FALSE), DeskTop, 0, 50);
  ONERR
    DrawStr (20, 20, "Error!", A_NORMAL);
  ENDTRY
  ngetchx ();
#endif
#if 0
	FILE *f = fopen("aaa", "wb");
	fputs("asdfasdfasdf",f);
	fputc(TEXT, f);
#endif
#if 1
TRY
#undef KEY_DIAMOND
#define KEY_DIAMOND 16384
	short ****cur_page = PAGES;
	short ***cur_group = *cur_page;
	short **cur_expr = *cur_group;
	for (;;) {
		unsigned short key;
		switch (key=ngetchx()) {
			case '+':return;

			case KEY_F3: // increment page
				cur_expr = *(cur_group = *(cur_page = cur_page==(PAGES+NUM_PAGES-1) ? PAGES : cur_page+1));
				break;
			case KEY_DIAMOND|KEY_F3:
				cur_expr = *(cur_group = *(cur_page = cur_page==PAGES ? (PAGES+NUM_PAGES-1) : cur_page-1));
				break;

			case KEY_F4: // increment group
				cur_expr = *(cur_group = cur_group==*(cur_page+1)-1 ? *cur_page : cur_group+1);
				break;
			case KEY_DIAMOND|KEY_F4: // decrement group
				cur_expr = *(cur_group = cur_group==*cur_page ? *(cur_page+1)-1 : cur_group-1);
				break;

			case KEY_F5: // increment expression
				cur_expr = cur_expr==*(cur_group+1)-1 ? *cur_group : cur_expr+1;
				break;
			case KEY_DIAMOND|KEY_F5: // decrement expression
				cur_expr = cur_expr==*cur_group ? *(cur_group+1)-1 : cur_expr-1;
				break;
		}
		TRY
		top_estack = (ESI) bottom_estack+1;
		for (short *p=*cur_expr; p!=*(cur_expr+1); ++p) {
			if (*p < 0) {
				push_quantum(*p>-18 ? VAR_A_TAG + ~*p : VAR_R_TAG + ~*p-17);
			} else if (*p & 0x100) {
				push_shortint(*p & ~0x100);
			} else if (*p & 0x200) {
				push_quantum(*p & ~0x200);
				push_quantum(1);
				push_quantum(1);
				push_quantum(1);
				push_quantum(POSFRAC_TAG);
			} else {
				push_quantum(*p);
			}
		}
		ONERR clrscr(); printf("push error"); ngetchx(); ENDTRY
		TRY
			WinClr(DeskTop);
			Print2DExpr(Parse2DExpr(top_estack,0), DeskTop, 0/*xpos*/, 50);
		ONERR clrscr(); printf("print error"); ngetchx(); ENDTRY
	}
ONERR clrscr(); printf("general error"); ngetchx(); ENDTRY
#endif
}
// limit vars to [a-q] instead of [a-z]