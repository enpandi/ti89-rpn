// this version was used at district contest

#undef MIN_AMS

#define MIN_AMS 101
#define OPTIMIZE_ROM_CALLS
#define MLINK_FORMAT_RELOCS
#define OPTIMIZE_CALC_CONSTS
#define NO_CALC_DETECT

#include <tigcclib.h>

#if 1
	void err(const char *funcname, unsigned short errCode) {
			char str[40];
			WinClr(DeskTop);
			sprintf(str, "error in %s", funcname);
			DrawStr(0, 30, str, A_NORMAL);
			sprintf(str, "code %d", errCode);
			DrawStr(0, 50, str, A_NORMAL);
			sprintf(str, "%s", find_error_message(errCode));
			DrawStr(0, 70, str, A_NORMAL);
			ngetchx();
			exit(0);
	}
#else
	#undef TRY
	#undef ONERR
	#undef ENDTRY
	#define TRY if(1){
	#define ONERR }else if(0){
	#define ENDTRY }
	#define err(funcname,errCode) if(0);
#endif

// keys used by RPN, as represented by kbd_queue()/ngetchx()
// some are already defined in <kbd.h>
#undef KEY_DIAMOND
#undef KEY_DOWN
#undef KEY_UP
#undef KEY_RIGHT
#undef KEY_LEFT
#define KEY_2ND                   4096
#define KEY_DIAMOND              16384
#define KEY_EE                     149
#define KEY_SIN       (KEY_2ND    |'Y')
#define KEY_COS       (KEY_2ND    |'Z')
#define KEY_TAN       (KEY_2ND    |'T')
#define KEY_ASIN      (KEY_DIAMOND|'Y')
#define KEY_ACOS      (KEY_DIAMOND|'Z')
#define KEY_ATAN      (KEY_DIAMOND|'T')
#define KEY_LN        (KEY_2ND    |'X')
#define KEY_EXP       (KEY_DIAMOND|'X')
#define KEY_LOG                    '\''
#define KEY_EXP10     (KEY_DIAMOND|'=')
#define KEY_RECIP                  ']'
#define KEY_SQRT      (KEY_2ND    |'*')
#define KEY_SQ        (KEY_DIAMOND|'*')
#define KEY_NEG       (KEY_DIAMOND|KEY_SIGN)
#define KEY_PI                     140
#define KEY_DOWN                   340
#define KEY_UP                     337
#define KEY_PUSH_VAR               KEY_STO
#define KEY_RIGHT                  344
#define KEY_LEFT                   338
#define KEY_EVAL                   '='
#define KEY_POP                    KEY_BACKSPACE
#define KEY_CLEAR_ALL (KEY_DIAMOND|KEY_CLEAR)
#define KEY_POP_ALL   (KEY_DIAMOND|KEY_POP)
#define KEY_RESET                  KEY_ESC
//#define KEY_HOME 277
inline unsigned short alias(unsigned short key) {
	switch (key) {
		case '(': return KEY_RECIP;
		case ')': return KEY_SQRT;
		case ',': return KEY_SQ;
		case 'x': return KEY_LN;
		case 'y': return KEY_EXP;
		case 'z': return KEY_LOG;
		case 't': return KEY_EXP10;
		default: return key;
	}
}
unsigned short op_args(unsigned short op) {
	switch (op) {
		case '+': case '-': case '*': case '/': case '^':
			return 2;
		case KEY_SIN  : case KEY_COS  : case KEY_TAN :
		case KEY_ASIN : case KEY_ACOS : case KEY_ATAN:
		case KEY_LN   : case KEY_EXP  :
		case KEY_LOG  : case KEY_EXP10:
		case KEY_RECIP: case KEY_SQRT : case KEY_SQ  :
		case KEY_NEG  :
			return 1;
	}
	return 0;
}

#define MAX_STACK_SIZE 64
#define MAX_FLOATS 26 // a-z
#define MAX_FLOAT_STR_LENGTH 32
#define FONT_CHAR_WIDTH 6
#define FLOAT_STR_Y 85
#define DEFAULT_X_OFFSET 8
#define DEFAULT_Y_OFFSET 17
#define X_INCREMENT 80

typedef struct {
	float f;
	char chars[MAX_FLOAT_STR_LENGTH], *end;
} float_str;
float_str float_strs[MAX_FLOATS], *fs, *const fs_end=float_strs+MAX_FLOATS;
inline void fs_init_all() { for (fs=float_strs+MAX_FLOATS; fs--!=float_strs; fs->end = fs->chars); ++fs; }
inline void fs_push_cur(char c) {
	if (fs->end != fs->chars + MAX_FLOAT_STR_LENGTH)
		DrawChar(FONT_CHAR_WIDTH*(fs->end-fs->chars+1), FLOAT_STR_Y, *fs->end++=c, A_NORMAL),
		*fs->end = 0,
		fs->f = atof(fs->chars); }
void fs_clear(float_str *fs) {
	*fs->end = 0;
	DrawStr(FONT_CHAR_WIDTH<<1, FLOAT_STR_Y, fs->chars, A_XOR);
	fs->f = *(fs->end=fs->chars) = 0;
}
void fs_draw_cur(unsigned short attr) {
	DrawChar(0, FLOAT_STR_Y, 'a'+fs-float_strs, attr);
	DrawChar(FONT_CHAR_WIDTH, FLOAT_STR_Y, '=', attr);
	*fs->end = 0;
	DrawStr(FONT_CHAR_WIDTH<<1, FLOAT_STR_Y, fs->chars, attr);
}
void fs_clear_all() { for (fs=float_strs+MAX_FLOATS; fs--!=float_strs;) fs_clear(fs); fs=float_strs; }
inline void fs_next() {
	fs_draw_cur(A_XOR);
	fs = fs == fs_end - 1 ? float_strs : fs+1;
	fs_draw_cur(A_NORMAL);
}
inline void fs_prev() {
	fs_draw_cur(A_XOR);
	fs = fs == float_strs ? fs_end - 1 : fs - 1;
	fs_draw_cur(A_NORMAL);
}


// RPN stack
short stack[MAX_STACK_SIZE], *st_top=stack, *const st_end=stack+MAX_STACK_SIZE;
unsigned short vars; // used to count/index RPN variables
unsigned short num_exprs; // used to ignore operators with too many arguments
inline void st_push_var() { if (st_top != st_end && vars < MAX_FLOATS) *st_top++ = ~vars++, ++num_exprs; }
void st_push_op(unsigned short op) {
	unsigned short args = op_args(op);
	if (st_top != st_end && num_exprs >= args)
		*st_top++ = op,
		num_exprs += 1 - args;
}
inline void st_pop() {
	if (st_top != stack) {
		if (*--st_top < 0) --vars;
		num_exprs -= 1 - op_args(*st_top);
	}
}
inline void st_pop_all() { st_top = stack; vars = num_exprs = 0; }

short x_offset=DEFAULT_X_OFFSET, Top;
void estack_swap() {
	ESI e = next_expression_index(top_estack);
	move_between_to_top(next_expression_index(e), e);
}
void draw(unsigned short reparse, unsigned short eval) {
	if (reparse) {
		top_estack = (ESI) bottom_estack + 1;
		for (short *e=stack; e!=st_top; ++e) {
			if (*e < 0) {
				if (float_strs[~*e].f)
					push_Float(float_strs[~*e].f);
				else
					push_quantum(*e>-18 ? VAR_A_TAG + ~*e : VAR_R_TAG + ~*e-17);
			} else switch (*e) {
				case '+'      :                                   push_quantum(  ADD_TAG); break;
				case '-'      :                                   push_quantum(  SUB_TAG); break;
				case '*'      :                                   push_quantum(  MUL_TAG); break;
				case '/'      :                                   push_quantum(  DIV_TAG); break;
				case '^'      :                    estack_swap(); push_quantum(  POW_TAG); break;
				case KEY_SIN  :                                   push_quantum(  SIN_TAG); break;
				case KEY_COS  :                                   push_quantum(  COS_TAG); break;
				case KEY_TAN  :                                   push_quantum(  TAN_TAG); break;
				case KEY_ASIN :                                   push_quantum( ASIN_TAG); break;
				case KEY_ACOS :                                   push_quantum( ACOS_TAG); break;
				case KEY_ATAN :                                   push_quantum( ATAN_TAG); break;
				case KEY_LN   :                                   push_quantum(   LN_TAG); break;
				case KEY_EXP  :                                   push_quantum( EXPF_TAG); break;
				case KEY_LOG  :                                   push_quantum(  LOG_TAG); break;
				case KEY_EXP10: push_shortint(10);                push_quantum(  POW_TAG); break;
				case KEY_RECIP: push_shortint( 1); estack_swap(); push_quantum(  DIV_TAG); break;
				case KEY_SQRT :                                   push_quantum( SQRT_TAG); break;
				case KEY_SQ   : push_shortint( 2); estack_swap(); push_quantum(  POW_TAG); break;
				case KEY_NEG  :                                   push_quantum(MINUS_TAG); break;
				case KEY_PI   :                                   push_quantum(   PI_TAG); break;
			}
		}
		if (eval && top_estack != bottom_estack + 1)
			NG_approxESI(top_estack);
		Parms2D(Parse2DExpr(top_estack, 0), &Top, &Top, &Top);
		x_offset = DEFAULT_X_OFFSET;
	}
	WinClr(DeskTop);
	Print2DExpr(top_estack, DeskTop, eval?DEFAULT_X_OFFSET:x_offset, DEFAULT_Y_OFFSET+Top);
	fs_draw_cur(A_REPLACE);
}

#if 0
unsigned short disp() {
	clrscr();
	printf("\nvars=%d num_exprs=%d\n%c", vars, num_exprs, '[');
	for (short *p=stack; p!=st_top; ++p)
		if (*p<0) printf("%d ",*p);
		else printf("%c ", *p);
	printf("]\n[");
	for (int i=0; i<26; ++i) printf("%f ", float_strs[i].f);
	return 1;
}

void bruh() {
	clrscr();
for(ESI x = (ESI) bottom_estack; x!=top_estack+1; ++x)
	printf("%02x ",*x);
	ngetchx();
}
#endif

void _main() { TRY
	fs_init_all();
	WinClr(DeskTop);
	fs_draw_cur(A_NORMAL);
	void *kq = kbd_queue();
	unsigned short key;
	for (;;) if (!OSdequeue(&key,kq)) { switch (key=alias(key)) {
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
			fs_draw_cur(A_XOR);
			fs = float_strs + key - 'a';
			fs_draw_cur(A_NORMAL);
			break;
		case KEY_SIGN: case '.': case KEY_EE:
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			fs_push_cur(key);
			draw(1, 0);
			break;
		case '+': case '-': case '*': case '/': case '^':
			st_push_op(key);
			draw(1, 0);
			break;
		case KEY_SIN  : case KEY_COS  : case KEY_TAN :
		case KEY_ASIN : case KEY_ACOS : case KEY_ATAN:
		case KEY_LN   : case KEY_EXP  :
		case KEY_LOG  : case KEY_EXP10:
		case KEY_RECIP: case KEY_SQRT : case KEY_SQ  :
		case KEY_NEG  :
			st_push_op(key);
			draw(1, 0);
			break;
		case KEY_PI:
			st_push_op(key);
			draw(1, 0);
			break;
		case KEY_MODE:
			MO_modeDialog();
			draw(1, 0);
			break;
		case KEY_ENTER:
		case KEY_DOWN:
			fs_next();
			break;
		case KEY_UP:
			fs_prev();
			break;
		case KEY_PUSH_VAR:
			st_push_var();
			draw(1, 0);
			break;
		case KEY_RIGHT:
			x_offset -= X_INCREMENT;
			draw(0, 0);
			break;
		case KEY_LEFT:
			if (x_offset != DEFAULT_X_OFFSET)
				x_offset += X_INCREMENT;
			draw(0, 0);
			break;
		case KEY_EVAL:
			draw(1, 1);
			break;
		case KEY_CLEAR:
			fs_clear(fs);
			draw(1, 0);
			break;
		case KEY_POP:
			st_pop();
			draw(1, 0);
			break;
		case KEY_CLEAR_ALL:
			fs_clear_all();
			draw(1, 0);
			break;
		case KEY_POP_ALL:
			st_pop_all();
			draw(1, 0);
			break;
		case KEY_RESET:
			fs_clear_all(); st_pop_all();
			draw(1, 0);
			break;
		case KEY_QUIT:
			return;
	} }//disp(); }
ONERR err("_main", errCode); ENDTRY }