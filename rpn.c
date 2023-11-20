#undef MIN_AMS

#define MIN_AMS 101
#define OPTIMIZE_ROM_CALLS
#define MLINK_FORMAT_RELOCS
#define OPTIMIZE_CALC_CONSTS
#define NO_CALC_DETECT

#include <tigcclib.h>

#if 1
	void err(const char *func_name, unsigned short errCode) {
			char str[40];
			WinClr(DeskTop);
			sprintf(str, "error in %s", func_name);
			DrawStr(0, 30, str, A_NORMAL);
			sprintf(str, "code %d", errCode);
			DrawStr(0, 50, str, A_NORMAL);
			sprintf(str, "%s", find_error_message(errCode));
			DrawStr(0, 70, str, A_NORMAL);
			ngetchx();
			exit(0); }
#else
	#undef TRY
	#undef ONERR
	#undef ENDTRY
	#define TRY {
	#define ONERR }if(0){
	#define ENDTRY }
	#define err(func_name,errCode) {}
#endif

// keys used by RPN, as represented by kbd_queue()/ngetchx()
// some are already defined in <kbd.h>
#undef KEY_DIAMOND
#undef KEY_DOWN
#undef KEY_UP
#undef KEY_RIGHT
#undef KEY_LEFT
#define KEY_2ND                    4096
#define KEY_DIAMOND               16384
#define KEY_EE                      149
#define KEY_PI                      140
#define KEY_SIN        (KEY_2ND    |'Y')
#define KEY_COS        (KEY_2ND    |'Z')
#define KEY_TAN        (KEY_2ND    |'T')
#define KEY_ASIN       (KEY_DIAMOND|'Y')
#define KEY_ACOS       (KEY_DIAMOND|'Z')
#define KEY_ATAN       (KEY_DIAMOND|'T')
#define KEY_LN         (KEY_2ND    |'X')
#define KEY_EXP        (KEY_DIAMOND|'X')
#define KEY_LOG                     '\''
#define KEY_EXP10      (KEY_DIAMOND|'=')
#define KEY_RECIP                   ']'
#define KEY_SQRT       (KEY_2ND    |'*')
#define KEY_SQ         (KEY_DIAMOND|'*')
#define KEY_NEG        (KEY_DIAMOND|KEY_SIGN)
#define KEY_DOWN                    340
#define KEY_UP                      337
#define KEY_PUSH_VAR                KEY_STO
#define KEY_RIGHT                   344
#define KEY_LEFT                    338
#define KEY_EVAL                    '='
#define KEY_POP                     KEY_BACKSPACE
#define KEY_CLEAR_ALL  (KEY_DIAMOND|KEY_CLEAR)
#define KEY_POP_ALL    (KEY_DIAMOND|KEY_POP)
#define KEY_RESET                   KEY_ESC
#define KEY_CHANGEMODE (KEY_DIAMOND|KEY_ENTER)
// #define KEY_HOME 277
inline unsigned short alias(unsigned short key) {
	switch (key) {
		case '(': return KEY_RECIP;
		case ')': return KEY_SQRT;
		case ',': return KEY_SQ;
		case 'x': return KEY_LN;
		case 'y': return KEY_EXP;
		case 'z': return KEY_LOG;
		case 't': return KEY_EXP10;
		default: return key; } }

#define MAX_FLOATS 17 // a-q
#define MAX_FLOAT_STR_LENGTH 16
#define CHAR_WIDTH 6
#define FLOAT_STR_Y 85
typedef struct {
	float f;
	char chars[MAX_FLOAT_STR_LENGTH], *end; } float_str;
float_str float_strs[MAX_FLOATS], *fs, *const fs_end=float_strs+MAX_FLOATS;
void fs_init(float_str *fs) {
	fs->f = *(fs->end=fs->chars) = 0; }
void fs_init_all() {
	for (fs=fs_end; fs--!=float_strs; fs_init(fs));
	fs = float_strs; }
void fs_push_cur(char c) {
	if (fs->end != fs->chars + MAX_FLOAT_STR_LENGTH)
		DrawChar(CHAR_WIDTH*(fs->end-fs->chars+2), FLOAT_STR_Y, c, A_NORMAL),
		*fs->end++=c,
		*fs->end = 0,
		fs->f = atof(fs->chars); }
void fs_draw_cur(unsigned short attr) {
	DrawChar(0, FLOAT_STR_Y, 'a'+fs-float_strs, attr);
	DrawChar(CHAR_WIDTH, FLOAT_STR_Y, '=', attr);
	DrawStr(CHAR_WIDTH<<1, FLOAT_STR_Y, fs->chars, attr); }
inline void fs_next() {
	fs_draw_cur(A_XOR);
	fs = fs == fs_end - 1 ? float_strs : fs+1;
	fs_draw_cur(A_NORMAL); }
inline void fs_prev() {
	fs_draw_cur(A_XOR);
	fs = fs == float_strs ? fs_end - 1 : fs - 1;
	fs_draw_cur(A_NORMAL); }

#define MAX_RPN_SIZE 64
#define DEFAULT_X_OFFSET 8
#define DEFAULT_Y_OFFSET 17
#define X_INCREMENT 80
unsigned short op_args(unsigned short op) { switch (op) {
	case '+': case '-': case '*': case '/': case '^':
		return 2;
	case KEY_SIN  : case KEY_COS  : case KEY_TAN :
	case KEY_ASIN : case KEY_ACOS : case KEY_ATAN:
	case KEY_LN   : case KEY_EXP  :
	case KEY_LOG  : case KEY_EXP10:
	case KEY_RECIP: case KEY_SQRT : case KEY_SQ  :
	case KEY_NEG  :
		return 1;
	default:
		return 0; } }
// RPN stack, not to be confused with estack.h
short rpn[MAX_RPN_SIZE], *rpn_top=rpn, *const rpn_end=rpn+MAX_RPN_SIZE;
unsigned short vars; // used to count/index RPN variables
unsigned short num_exprs; // used to ignore operators with too many arguments
inline void rpn_init() { rpn_top=rpn; vars=num_exprs=0; }
inline void rpn_push_var() { if (rpn_top!=rpn_end && vars<MAX_FLOATS) *rpn_top++=~vars++, ++num_exprs; }
void rpn_push_op(unsigned short op) {
	unsigned short args = op_args(op);
	if (rpn_top!=rpn_end && num_exprs>=args)
		*rpn_top++ = op,
		num_exprs += 1 - args; }
inline void rpn_pop() {
	if (rpn_top != rpn) {
		if (*--rpn_top < 0) --vars;
		num_exprs -= 1 - op_args(*rpn_top); } }

#define NUM_PAGES 7
#define MINUS MINUS_TAG
#define ADD ADD_TAG
#define SUB SUB_TAG
#define MUL MUL_TAG
#define DIV DIV_TAG
#define POW POW_TAG
#define SQRT SQRT_TAG
#define EXPF EXPF_TAG
#define LN LN_TAG
#define LOG LOG_TAG
#define SIN SIN_TAG
#define COS COS_TAG
#define TAN TAN_TAG
#define ASIN ASIN_TAG
#define ATAN ATAN_TAG
// ~i denotes the i-th variable
// 0x1DD denotes the integer 0xDD
// 0x2DD denotes the fraction 1/0xDD
const short QUANTA[2119] = {
	~0,~1,SUB,~2,DIV,
	~0,~1,ADD,~2,MUL,
	~0,~1,DIV,~2,ADD,
	~0,~1,MUL,~2,ADD,
	~0,~1,ADD,~2,SUB,

	~0,~1,SUB,~2,DIV,~3,ADD,
	~0,~1,DIV,~2,ADD,~3,SUB,
	~0,~1,ADD,~2,MUL,~3,SUB,
	~0,~1,ADD,~2,SUB,~3,MUL,
	~0,~1,MUL,~2,~3,SUB,SUB,

	~0,~1,SUB,~2,ADD,~3,ADD,~4,DIV,
	~0,~1,SUB,~2,SUB,~3,ADD,~4,MUL,
	~0,~1,SUB,~2,ADD,~3,MUL,~4,SUB,
	~0,~1,ADD,~2,SUB,~3,DIV,~4,ADD,
	~0,~1,MUL,~2,MUL,~3,DIV,~4,ADD,

	~0,~1,~2,ADD,~3,SUB,MUL,~4,MUL,~5,ADD,
	~0,~1,~2,SUB,~3,ADD,MUL,~4,~5,MUL,DIV,
	~0,~1,SUB,~2,ADD,~3,MUL,~4,MUL,~5,SUB,
	~0,~1,SUB,~2,~3,DIV,DIV,~4,~5,SUB,ADD,

	~0,~1,SUB,~2,MUL,~3,~4,DIV,DIV,~5,~6,SUB,SUB,
	~0,~1,ADD,~2,SUB,~3,MUL,~4,~5,MUL,~6,MUL,DIV,
	~0,~1,SUB,~2,ADD,~3,DIV,~4,~5,MUL,~6,DIV,DIV,
	~0,~1,ADD,~2,~3,MUL,~4,MUL,DIV,~5,ADD,~6,SUB,
	~0,~1,ADD,~2,SUB,~3,~4,ADD,~5,~6,MUL,DIV,ADD,

	~0,~1,ADD,~2,~3,ADD,MUL,~4,~5,MUL,~6,~7,SUB,MUL,DIV,
	~0,~1,MUL,~2,~3,MUL,SUB,~4,ADD,~5,~6,~7,MUL,ADD,DIV,
	~0,~1,MUL,~2,~3,MUL,ADD,~4,~5,ADD,~6,~7,MUL,SUB,DIV,
	~0,~1,ADD,~2,~3,SUB,DIV,~4,~5,ADD,~6,~7,SUB,DIV,ADD,
	~0,~1,ADD,~2,~3,SUB,~4,ADD,MUL,~5,~6,MUL,~7,SUB,DIV,

	~0,~1,ADD,~2,~3,MUL,~4,MUL,DIV,~5,~6,ADD,~7,~8,SUB,MUL,ADD,
	~0,~1,~2,MUL,~3,MUL,ADD,~4,~5,ADD,~6,MUL,~7,~8,ADD,MUL,DIV,
	~0,~1,~2,ADD,MUL,~3,~4,SUB,~5,MUL,DIV,~6,~7,~8,SUB,DIV,SUB,
	~0,~1,ADD,~2,SUB,~3,MUL,~4,MUL,~5,~6,SUB,~7,~8,SUB,MUL,DIV,
	~0,~1,MUL,~2,~3,ADD,~4,MUL,SUB,~5,~6,ADD,~7,ADD,~8,MUL,DIV,

	~0,~1,~2,SUB,MUL,~3,~4,~5,MUL,SUB,MUL,~6,~7,ADD,~8,~9,SUB,MUL,DIV,
	~0,~1,ADD,~2,SUB,~3,ADD,~4,ADD,~5,~6,~7,ADD,MUL,~8,~9,ADD,MUL,DIV,
	~0,~1,ADD,~2,~3,ADD,MUL,~4,ADD,~5,MUL,~6,~7,~8,ADD,MUL,~9,MUL,DIV,
	~0,~1,ADD,~2,~3,MUL,~4,ADD,DIV,~5,~6,SUB,~7,ADD,~8,~9,MUL,DIV,ADD,
	~0,~1,SUB,~2,~3,~4,MUL,ADD,MUL,~5,~6,~7,ADD,MUL,~8,MUL,~9,MUL,DIV,

	~0,~1,ADD,~2,SUB,~3,~4,ADD,~5,SUB,MUL,~6,~7,SUB,~8,MUL,~9,~10,SUB,MUL,DIV,
	~0,~1,DIV,~2,~3,ADD,~4,SUB,~5,~6,SUB,DIV,ADD,~7,~8,ADD,~9,~10,DIV,DIV,ADD,
	~0,~1,ADD,~2,~3,ADD,MUL,~4,ADD,~5,SUB,~6,~7,SUB,~8,~9,ADD,~10,SUB,MUL,DIV,
	~0,~1,ADD,~2,SUB,~3,~4,MUL,DIV,~5,~6,~7,ADD,MUL,~8,~9,ADD,~10,SUB,DIV,SUB,
	~0,~1,ADD,~2,~3,ADD,MUL,~4,~5,SUB,MUL,~6,~7,ADD,~8,MUL,~9,~10,DIV,MUL,DIV,

	~0,~1,ADD,~2,SUB,~3,~4,SUB,~5,SUB,MUL,~6,~7,MUL,~8,MUL,~9,~10,ADD,~11,ADD,MUL,DIV,
	~0,~1,ADD,~2,~3,SUB,DIV,~4,~5,~6,ADD,DIV,ADD,~7,~8,~9,SUB,MUL,~10,~11,MUL,DIV,ADD,
	~0,~1,ADD,~2,~3,ADD,~4,~5,SUB,MUL,SUB,~6,~7,MUL,~8,MUL,~9,~10,SUB,~11,ADD,MUL,DIV,

	0x102,~0,~1,SUB,SQRT,~2,DIV,~3,~4,DIV,ADD,POW,
	0x101,~0,~1,ADD,DIV,0x101,~2,~3,SUB,DIV,ADD,0x101,~4,DIV,ADD,
	~0,0x101,~1,DIV,ADD,0x101,~2,DIV,~3,ADD,DIV,0x101,~4,DIV,ADD,
	0x102,~0,~1,MUL,~2,DIV,~3,ADD,POW,~4,SQRT,ADD,
	~0,~1,MUL,~2,~3,ADD,DIV,SQRT,~4,ADD,

	~0,~1,ADD,~2,~3,DIV,ADD,SQRT,~4,~5,ADD,DIV,
	~0,~1,MUL,~2,~3,DIV,SQRT,ADD,0x102,~4,~5,MUL,POW,ADD,
	~0,~1,MUL,0x102,~2,POW,~3,DIV,SQRT,MUL,0x101,~4,~5,ADD,SQRT,DIV,ADD,
	0x102,~0,~1,SQRT,ADD,POW,0x102,~2,~3,ADD,POW,MUL,~4,~5,DIV,SQRT,MUL,
	0x102,~0,~1,ADD,~2,~3,DIV,SQRT,ADD,~4,~5,ADD,DIV,POW,

	0x102,~0,~1,ADD,POW,~2,~3,SUB,SQRT,DIV,~4,~5,~6,ADD,SQRT,DIV,ADD,
	0x101,~0,~1,SUB,DIV,~2,0x102,~3,~4,ADD,POW,MUL,DIV,SQRT,0x102,~5,POW,~6,MUL,ADD,
	~0,~1,~2,ADD,SQRT,DIV,SQRT,0x101,0x102,~4,~5,SUB,POW,DIV,0x101,0x102,~6,~7,ADD,POW,DIV,ADD,MUL,
	0x101,~0,DIV,0x101,~1,SQRT,DIV,ADD,0x102,~2,~3,ADD,~4,SUB,POW,~5,~6,SUB,SQRT,DIV,ADD,
	~0,~1,~2,ADD,DIV,~3,ADD,~4,0x102,~5,POW,ADD,~6,SQRT,SUB,MUL,

	0x102,~0,POW,~1,~2,ADD,MUL,~3,~4,~5,MUL,ADD,DIV,0x101,0x101,~6,DIV,0x101,~7,DIV,ADD,DIV,ADD,
	0x202,~0,~1,SUB,~2,~3,DIV,MUL,POW,0x102,~4,POW,0x102,~5,~6,ADD,POW,ADD,~7,ADD,DIV,
	0x202,~0,~1,~2,ADD,DIV,0x101,~3,DIV,ADD,POW,0x102,~4,~5,ADD,POW,~6,~7,SUB,SQRT,MUL,DIV,
	0x102,~0,POW,~1,SQRT,ADD,~2,0x102,~3,POW,MUL,SQRT,DIV,~4,~5,MUL,SQRT,SQRT,~6,~7,ADD,DIV,ADD,
	~0,~1,~2,SQRT,DIV,DIV,SQRT,~3,~4,~5,MUL,ADD,DIV,0x202,~6,~7,ADD,POW,ADD,

	0x102,~0,~1,ADD,POW,0x102,~2,~3,SUB,POW,SUB,~4,~5,MUL,0x102,~6,~7,ADD,~8,SUB,POW,MUL,SQRT,DIV,
	~0,~1,ADD,SQRT,~2,~3,MUL,0x102,~4,POW,ADD,DIV,~5,~6,ADD,SQRT,0x102,~7,~8,SUB,POW,ADD,DIV,
	0x101,~0,DIV,~1,0x102,~2,~3,ADD,POW,DIV,ADD,~4,SQRT,0x102,~5,POW,DIV,SUB,0x102,~6,~7,ADD,POW,~8,ADD,DIV,
	0x102,~0,~1,ADD,~2,~3,ADD,DIV,POW,~4,~5,ADD,~6,SQRT,DIV,SQRT,ADD,0x102,~7,~8,DIV,POW,DIV,
	0x102,~0,~1,DIV,POW,~2,~3,MUL,~4,DIV,SQRT,ADD,~5,ADD,~6,~7,~8,MUL,SQRT,ADD,DIV,

	~0,~1,SUB,~2,~3,ADD,DIV,MINUS,0x10A,POW,
	~0,~1,MUL,~2,~3,DIV,0x10A,POW,MUL,
	~0,~1,SUB,MINUS,0x10A,POW,~2,~3,ADD,DIV,

	~0,~1,EXPF,MUL,~2,~3,EXPF,MUL,ADD,
	~0,EXPF,~1,EXPF,ADD,~2,~3,ADD,DIV,
	~0,~1,DIV,0x101,~2,~3,MUL,MINUS,EXPF,SUB,MUL,

	~0,~1,SUB,~2,~3,MUL,LN,MUL,
	~0,~1,~2,SUB,LOG,MUL,~3,DIV,
	~0,~1,~2,0x101,~3,DIV,ADD,MUL,LOG,MUL,
	~0,~1,ADD,~2,SUB,LN,~3,DIV,
	~0,~1,~2,~3,SUB,LN,MUL,ADD,
	~0,~1,SUB,~2,~3,ADD,LOG,DIV,

	0x203,~0,~1,ADD,POW,0x101,~3,~2,POW,DIV,ADD,
	~2,~3,ADD,MINUS,~0,~1,ADD,POW,
	0x103,~0,POW,~3,~1,~2,SUB,POW,ADD,

	0x15A,~0,~1,DIV,MUL,SIN,~2,~3,SUB,COS,ADD,
	~0,~1,SIN,MUL,~2,~3,COS,MUL,MUL,
	~0,~1,DIV,COS,~2,~3,SUB,SIN,DIV,

	~0,~1,~2,SUB,MINUS,0x10A,POW,MUL,~3,~4,ADD,DIV,
	~0,0x10A,POW,~1,MINUS,0x10A,POW,ADD,0x202,~2,~3,DIV,0x10A,POW,~4,0x10A,POW,SUB,POW,ADD,
	~0,0x10A,POW,~1,MINUS,0x10A,POW,MUL,~2,ADD,~3,~4,ADD,0x10A,POW,DIV,

	0x101,~0,~1,~2,MUL,ADD,EXPF,ADD,~3,~4,~5,EXPF,SUB,MUL,DIV,
	~0,~1,SUB,~2,~3,MUL,EXPF,MUL,~4,~5,SUB,MINUS,EXPF,DIV,
	~0,~1,~2,ADD,EXPF,ADD,~3,~4,~5,SUB,MINUS,EXPF,SUB,DIV,

	~0,~1,ADD,LN,~2,DIV,~3,LN,~4,~5,SUB,DIV,ADD,
	~0,~1,~2,ADD,LOG,MUL,~3,LOG,~4,~5,MUL,SUB,DIV,
	~0,~1,ADD,0x101,~2,DIV,LOG,MUL,~3,~4,~5,ADD,DIV,LOG,DIV,
	~0,~1,~2,MUL,ADD,LOG,~3,~4,~5,DIV,LOG,SUB,DIV,
	~0,~1,MUL,~2,MUL,LN,~3,~4,~5,LN,MUL,ADD,DIV,
	~0,~1,~2,~3,MUL,ADD,~4,~5,ADD,DIV,LN,MUL,

	0x101,~1,~0,POW,DIV,~4,~5,SUB,~2,~3,ADD,POW,ADD,
	~2,~0,~1,ADD,POW,~4,~5,ADD,MINUS,~3,POW,DIV,
	~1,~0,POW,~3,~2,POW,SUB,~4,~5,ADD,DIV,
	~2,~0,~1,SUB,POW,~4,~5,ADD,MINUS,~3,POW,DIV,

	~0,~1,MUL,~2,~3,MUL,DIV,ATAN,~4,~5,MUL,ADD,
	~0,~1,~2,MUL,ADD,ATAN,~3,~4,ADD,~5,DIV,ASIN,DIV,
	~0,~1,MUL,~2,DIV,ASIN,~3,~4,~5,MUL,ADD,DIV,

	~0,LOG,~1,LOG,ADD,~2,LOG,ADD,~3,~4,DIV,LOG,ADD,
	~0,0x10A,POW,~1,0x10A,POW,MUL,~2,0x10A,POW,MUL,~3,~4,MUL,0x10A,POW,DIV,
	0x102,~0,EXPF,~0,MINUS,EXPF,ADD,POW,~1,~2,MUL,EXPF,0x101,~3,EXPF,DIV,MUL,SQRT,DIV,
	~0,0x10A,POW,~1,0x10A,POW,~2,0x10A,POW,MUL,~3,0x10A,POW,~4,0x10A,POW,MUL,DIV,SQRT,MUL,
	~1,~0,POW,LOG,~2,~3,~0,POW,LOG,MUL,ADD,
	0x102,0x102,~0,POW,0x102,~0,MUL,~1,MUL,SUB,0x102,~1,POW,ADD,0x102,~2,POW,DIV,POW,LN,
	0x102,~0,~1,MUL,~2,MUL,0x102,~3,POW,0x102,~4,POW,MUL,DIV,SQRT,LOG,MUL,
	~0,~1,ADD,MINUS,EXPF,SQRT,0x102,~2,~3,SUB,EXPF,POW,DIV,0x203,0x102,~4,POW,POW,MUL,
	0x103,~0,~1,MUL,~2,DIV,POW,LN,0x103,~3,~4,DIV,POW,LN,ADD,
	0x103,~0,0x10A,POW,~1,~2,MUL,0x10A,POW,MUL,~3,~4,0x10A,POW,MUL,DIV,POW,

	~0,SIN,~0,COS,DIV,0x101,0x102,~1,~2,MUL,SIN,POW,SUB,SQRT,MUL,
	~0,~1,SUB,COS,~0,~1,ADD,COS,SUB,
	~0,SIN,~1,COS,MUL,~0,COS,~1,SIN,MUL,SUB,
	0x102,~0,~1,SUB,POW,~2,~3,ADD,~4,LN,EXPF,MUL,ADD,
	~0,~1,MUL,LN,EXPF,~2,~3,MUL,LOG,0x10A,POW,ADD,
	~0,~1,~2,MUL,LOG,0x10A,POW,MUL,0x202,~3,~4,MUL,POW,ADD,

	~0,~1,COS,~1,DIV,~2,COS,~2,DIV,ADD,MUL,
	~0,0x106,~1,MUL,DIV,0x105,~2,~3,~4,SIN,MUL,ADD,POW,MUL,
	0x101,0x102,~0,COS,~0,SIN,DIV,POW,ADD,SQRT,~1,COS,~1,SIN,DIV,MUL,
	~0,SIN,~1,COS,MUL,~0,COS,~1,SIN,MUL,ADD,
	0x102,~0,COS,POW,0x102,~0,SIN,POW,SUB,~0,TAN,0x101,0x102,~0,TAN,POW,SUB,DIV,MUL,
	0x101,~0,~1,MUL,DIV,~2,~3,~4,SIN,MUL,ADD,LN,MUL,

	0x101,~0,DIV,MINUS,0x101,0x103,0x103,~0,POW,MUL,DIV,ADD,0x101,0x105,0x105,~0,POW,MUL,DIV,SUB,0x101,0x107,0x107,~0,POW,MUL,DIV,ADD,
	0x101,~0,DIV,0x101,0x103,0x103,~0,POW,MUL,DIV,ADD,0x101,0x105,0x105,~0,POW,MUL,DIV,ADD,0x101,0x107,0x107,~0,POW,MUL,DIV,ADD,
	~0,0x102,~0,POW,0x102,DIV,SUB,0x103,~0,POW,0x103,DIV,ADD,0x104,~0,POW,0x104,DIV,SUB,
	0x101,0x104,~0,POW,0x102,DIV,ADD,0x106,~0,POW,0x106,DIV,SUB,0x108,~0,POW,0x118,DIV,ADD,0x10A,~0,POW,0x178,DIV,SUB,
	0x101,~0,ADD,0x102,~0,POW,0x102,DIV,ADD,0x103,~0,POW,0x106,DIV,ADD,0x104,~0,POW,0x118,DIV,ADD,
	0x101,~0,ADD,0x102,~0,POW,ADD,0x104,~0,POW,0x108,DIV,ADD,0x105,~0,POW,0x10F,DIV,SUB,

	~0,~1,SQRT,DIV,0x102,~2,POW,~3,ADD,SQRT,~4,SQRT,ADD,~5,SQRT,~6,~7,MUL,ADD,DIV,LN,MUL,
	0x101,0x102,~0,POW,~1,SUB,SQRT,DIV,~2,0x102,~0,POW,~1,SUB,SQRT,SUB,~2,0x102,~0,POW,~1,SUB,SQRT,ADD,DIV,LN,MUL,
	~0,~1,MUL,~2,~3,~4,EXPF,MUL,ADD,LN,SUB,~5,~6,~7,ADD,DIV,ASIN,DIV,
	~0,~1,DIV,~2,0x102,~3,POW,DIV,0x102,~4,POW,~5,ADD,~6,~7,SQRT,ADD,DIV,LN,MUL,SUB,
	~0,~1,MUL,MINUS,EXPF,~2,~3,DIV,SQRT,MUL,ATAN,~4,~5,~6,MUL,~7,MUL,SQRT,MUL,DIV,
	~0,EXPF,~1,~2,SIN,MUL,~3,~4,COS,MUL,SUB,~5,0x102,~6,POW,0x102,~7,POW,ADD,SQRT,MUL,DIV,MUL,

};
const short *EXPRS[144] = {
	QUANTA+   0,QUANTA+   5,QUANTA+  10,QUANTA+  15,QUANTA+  20,
	QUANTA+  25,QUANTA+  32,QUANTA+  39,QUANTA+  46,QUANTA+  53,
	QUANTA+  60,QUANTA+  69,QUANTA+  78,QUANTA+  87,QUANTA+  96,
	QUANTA+ 105,QUANTA+ 116,QUANTA+ 127,QUANTA+ 138,
	QUANTA+ 149,QUANTA+ 162,QUANTA+ 175,QUANTA+ 188,QUANTA+ 201,
	QUANTA+ 214,QUANTA+ 229,QUANTA+ 244,QUANTA+ 259,QUANTA+ 274,
	QUANTA+ 289,QUANTA+ 306,QUANTA+ 323,QUANTA+ 340,QUANTA+ 357,
	QUANTA+ 374,QUANTA+ 393,QUANTA+ 412,QUANTA+ 431,QUANTA+ 450,
	QUANTA+ 469,QUANTA+ 490,QUANTA+ 511,QUANTA+ 532,QUANTA+ 553,
	QUANTA+ 574,QUANTA+ 597,QUANTA+ 620,
	QUANTA+ 643,QUANTA+ 655,QUANTA+ 670,QUANTA+ 685,QUANTA+ 697,
	QUANTA+ 707,QUANTA+ 719,QUANTA+ 733,QUANTA+ 750,QUANTA+ 767,
	QUANTA+ 781,QUANTA+ 798,QUANTA+ 818,QUANTA+ 841,QUANTA+ 862,
	QUANTA+ 878,QUANTA+ 901,QUANTA+ 922,QUANTA+ 944,QUANTA+ 967,
	QUANTA+ 986,QUANTA+1010,QUANTA+1033,QUANTA+1059,QUANTA+1082,
	QUANTA+1103,QUANTA+1113,QUANTA+1122,
	QUANTA+1132,QUANTA+1141,QUANTA+1150,
	QUANTA+1161,QUANTA+1169,QUANTA+1177,QUANTA+1187,QUANTA+1195,QUANTA+1203,
	QUANTA+1211,QUANTA+1222,QUANTA+1230,
	QUANTA+1239,QUANTA+1250,QUANTA+1259,
	QUANTA+1268,QUANTA+1280,QUANTA+1300,
	QUANTA+1316,QUANTA+1331,QUANTA+1345,
	QUANTA+1359,QUANTA+1372,QUANTA+1385,QUANTA+1400,QUANTA+1413,QUANTA+1426,
	QUANTA+1438,QUANTA+1451,QUANTA+1463,QUANTA+1474,
	QUANTA+1486,QUANTA+1498,QUANTA+1511,
	QUANTA+1523,QUANTA+1536,QUANTA+1553,QUANTA+1572,QUANTA+1592,QUANTA+1603,QUANTA+1623,QUANTA+1640,QUANTA+1659,QUANTA+1674,
	QUANTA+1691,QUANTA+1706,QUANTA+1715,QUANTA+1726,QUANTA+1739,QUANTA+1751,
	QUANTA+1765,QUANTA+1776,QUANTA+1790,QUANTA+1806,QUANTA+1817,QUANTA+1836,
	QUANTA+1849,QUANTA+1877,QUANTA+1904,QUANTA+1923,QUANTA+1948,QUANTA+1969,
	QUANTA+1988,QUANTA+2010,QUANTA+2037,QUANTA+2055,QUANTA+2076,QUANTA+2096,
	QUANTA+2119,
};
const short **PROBGRPS[31] = {
	EXPRS+  0,EXPRS+  5,EXPRS+ 10,EXPRS+ 15,EXPRS+ 19,
	EXPRS+ 24,EXPRS+ 29,EXPRS+ 34,EXPRS+ 39,EXPRS+ 44,
	EXPRS+ 47,EXPRS+ 52,
	EXPRS+ 57,EXPRS+ 62,EXPRS+ 67,
	EXPRS+ 72,EXPRS+ 75,EXPRS+ 78,EXPRS+ 84,EXPRS+ 87,
	EXPRS+ 90,EXPRS+ 93,EXPRS+ 96,EXPRS+102,EXPRS+106,
	EXPRS+109,EXPRS+119,EXPRS+125,EXPRS+131,EXPRS+137,
	EXPRS+143,
};
const short ***PAGES[8] = {
	PROBGRPS+0,PROBGRPS+5,PROBGRPS+10,PROBGRPS+12,PROBGRPS+15,PROBGRPS+20,PROBGRPS+25,PROBGRPS+30,
};
#undef MINUS
#undef ADD
#undef SUB
#undef MUL
#undef DIV
#undef POW
#undef SQRT
#undef EXPF
#undef LN
#undef LOG
#undef SIN
#undef COS
#undef TAN
#undef ASIN
#undef ATAN
short ****cur_page  = (short****) PAGES;
short  ***cur_group = (short*** ) PROBGRPS;
short   **cur_expr  = (short**  ) EXPRS;

short x_offset=DEFAULT_X_OFFSET, Top;
void estack_swap() {
	ESI e = next_expression_index(top_estack);
	move_between_to_top(next_expression_index(e), e); }
inline void estack_push_rpn() {
	for (short *t=rpn; t!=rpn_top; ++t) {
		if (*t < 0)
			if (float_strs[~*t].f)
				if (float_strs[~*t].chars[0] == (char) KEY_PI)
					push_quantum(PI_TAG);
				else
					push_Float(float_strs[~*t].f);
			else
				push_quantum(VAR_A_TAG + ~*t);
		else switch (*t) {
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
			case KEY_NEG  :                                   push_quantum(MINUS_TAG); break; } } }
inline void estack_push_cur() {
	for (short *t=*cur_expr; t!=*(cur_expr+1); ++t) {
		if (*t < 0) {
			if (float_strs[~*t].f)
				if (float_strs[~*t].chars[0] == (char) KEY_PI)
					push_quantum(PI_TAG);
				else
					push_Float(float_strs[~*t].f);
			else
				push_quantum(VAR_A_TAG + ~*t); }
		else if (*t & 0x100)
			push_shortint(*t & ~0x100);
		else if (*t & 0x200) {
			push_quantum(*t & ~0x200);
			push_quantum(1);
			push_quantum(1);
			push_quantum(1);
			push_quantum(POSFRAC_TAG); }
		else
			push_quantum(*t); } }
void draw(unsigned short reparse, unsigned short eval) {
	if (reparse) {
		top_estack = (ESI) bottom_estack + 1;
		switch (reparse) {
			case 1: estack_push_rpn(); break;
			case 2: estack_push_cur(); break; }
		if (eval && top_estack != bottom_estack + 1)
			NG_approxESI(top_estack);
		Parms2D(Parse2DExpr(top_estack, 0), &Top, &Top, &Top);
		x_offset = DEFAULT_X_OFFSET; }
	WinClr(DeskTop);
	Print2DExpr(top_estack, DeskTop, eval?DEFAULT_X_OFFSET:x_offset, DEFAULT_Y_OFFSET+Top);
	if (reparse == 2)
		DrawChar(CHAR_WIDTH*0, FLOAT_STR_Y-8, cur_page-(short****)PAGES+'0', A_NORMAL),
		DrawChar(CHAR_WIDTH*1, FLOAT_STR_Y-8, cur_group-*cur_page+'1', A_NORMAL),
		DrawChar(CHAR_WIDTH*2, FLOAT_STR_Y-8, cur_expr-*cur_group+'a', A_NORMAL);
	fs_draw_cur(A_NORMAL); }

#define TIMER_CONSTANT (1ul<<20)

void _main() { TRY
	WinClr(DeskTop);
	fs_init_all();
	fs_draw_cur(A_NORMAL);
	void *kq = kbd_queue();
	unsigned short use_expr = 0;
	unsigned short key;
	for (unsigned long timer=TIMER_CONSTANT; --timer; )
		if (!OSdequeue(&key,kq)) { timer=TIMER_CONSTANT; switch (key=alias(key)) {
			case KEY_CHANGEMODE:
				use_expr = !use_expr;
				draw(1+use_expr, 0);
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q':
				fs_draw_cur(A_XOR);
				fs = float_strs + key - 'a';
				fs_draw_cur(A_NORMAL);
				break;
			case KEY_PI: case KEY_SIGN: case '.': case KEY_EE:
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				fs_push_cur(key);
				draw(1+use_expr, 0);
				break;
			case '+': case '-': case '*': case '/': case '^': case KEY_DIAMOND|'^':
				if (use_expr) {
					switch (key) {
						case '+': // increment expression
							cur_expr = cur_expr==*(cur_group+1)-1 ? *cur_group : cur_expr+1;
							break;
						case '-': // decrement expression
							cur_expr = cur_expr==*cur_group ? *(cur_group+1)-1 : cur_expr-1;
							break;
						case '*': // increment group
							cur_expr = *(cur_group = cur_group==*(cur_page+1)-1 ? *cur_page : cur_group+1);
							break;
						case '/': // decrement group
							cur_expr = *(cur_group = cur_group==*cur_page ? *(cur_page+1)-1 : cur_group-1);
							break;
						case '^': // increment page
							cur_expr = *(cur_group = *(cur_page = cur_page==(short****)(PAGES+NUM_PAGES-1) ? (short****)PAGES : cur_page+1));
							break;
						case KEY_DIAMOND | '^': // decrement page
							cur_expr = *(cur_group = *(cur_page = cur_page==(short****)PAGES ? (short****)(PAGES+NUM_PAGES-1) : cur_page-1));
							break;
					}
					fs_draw_cur(A_XOR);
					fs = float_strs;
					fs_draw_cur(A_NORMAL);
				} else if (key != (KEY_DIAMOND|'^')) rpn_push_op(key);
				draw(1+use_expr, 0);
				break;
			case KEY_MODE:
				MO_modeDialog();
				draw(1+use_expr, 0);
				break;
			case KEY_SIN  : case KEY_COS  : case KEY_TAN :
			case KEY_ASIN : case KEY_ACOS : case KEY_ATAN:
			case KEY_LN   : case KEY_EXP  :
			case KEY_LOG  : case KEY_EXP10:
			case KEY_RECIP: case KEY_SQRT : case KEY_SQ  :
			case KEY_NEG  :
				rpn_push_op(key);
				draw(1+use_expr, 0);
				break;
			case KEY_ENTER:
			case KEY_DOWN:
				fs_next();
				draw(1+use_expr, 0);
				break;
			case KEY_UP:
				fs_prev();
				draw(1+use_expr, 0);
				break;
			case KEY_PUSH_VAR:
				rpn_push_var();
				draw(1+use_expr, 0);
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
				draw(1+use_expr, 1);
				break;
			case KEY_CLEAR:
				fs_init(fs);
				draw(1+use_expr, 0);
				break;
			case KEY_POP:
				rpn_pop();
				draw(1+use_expr, 0);
				break;
			case KEY_CLEAR_ALL:
				fs_init_all();
				draw(1+use_expr, 0);
				break;
			case KEY_POP_ALL:
				rpn_init();
				draw(1+use_expr, 0);
				break;
			case KEY_RESET:
				fs_init_all(); rpn_init();
				draw(1+use_expr, 0);
				break;
			case KEY_QUIT:
				return;
		} }
ONERR err("_main", errCode); ENDTRY }