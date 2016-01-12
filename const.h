#define cast

#define TRUE  1
#define FALSE 0

#define TITLE_STR	"Assassin - 6809 assembler V1.7"
#define CMD_NAME	"as"

#define	BLOCK			0x01
#define	ABS			0x02
#define	REL			0x04
#define	EQU			0x08
#define VALUE			(ABS | REL | EQU)
#define MACRO			0x10
#define	REDEF			0x80

#define	BLOCK_STACK_SIZE	30
#define	GEN_BUFF_SIZE		256
#define	TEXT_BUFF_SIZE		256
#define MAX_MACRO_PARAMS	16

#define BYTELOAD  1
#define BYTESTORE 2
#define WORDLOAD  3
#define WORDSTORE 4

#define SHORT 1
#define LONG  2
#define OPT   3

#define	INV 0
#define IMM 1
#define DIR 2
#define IND 3
#define EXT 4

#define NUL  '\0'
#define	TAB  9
#define SP   32
#define	COMMA		','
#define	COLON		':'
#define	SEMICOLON	';'
#define	OPENPAREN	'('
#define	CLOSEPAREN	')'
#define SINGLEQUOTE	'\''
#define DOUBLEQUOTE	'\"'

#define SIZE 80
#define ERR  0XFF
#define CASEMASK 0xDFDF

#define BASE 1
#define BDEC 2
#define WDEC 3
#define BINC 4
#define WINC 5

#define AN 0x4E41
#define AB 0x4241
#define AD 0x4441
#define CA 0x4143
#define CB 0x4243
#define DA 0x4144
#define MA 0x414D
#define DB 0x4244
#define DD 0x4444
#define AS 0x5341
#define BI 0x4942
#define TA 0x4154
#define TB 0x4254
#define PA 0x4150
#define PB 0x4250
#define PD 0x4450
#define PS 0x5350
#define PU 0x5550
#define PX 0x5850
#define PY 0x5950
#define CM 0x4D43
#define CO 0x4F43
#define CL 0x4C43
#define CW 0x5743
#define AI 0x4941
#define DE 0x4544
#define EO 0x4F45
#define RA 0x4152
#define RB 0x4252
#define EX 0x5845
#define IN 0x4E49
#define JS 0x534A
#define JM 0x4D4A
#define LD 0x444C
#define LS 0x534C
#define LE 0x454C
#define AX 0x5841
#define AY 0x5941
#define AU 0x5541
#define LB 0x424C
#define MU 0x554D
#define NE 0x454E
#define NO 0x4F4E
#define OR 0x524F
#define CC 0x4343
#define LU 0x554C
#define HU 0x5548
#define HS 0x5348
#define RO 0x4F52
#define RT 0x5452
#define SU 0x5553
#define SB 0x4253
#define ST 0x5453
#define SE 0x4553
#define SW 0x5753
#define SY 0x5953
#define TS 0x5354
#define TF 0x4654
#define BA 0x4142
#define BB 0x4242
#define BD 0x4442
#define I2 0X1249
#define I3 0X1349
#define NC 0x434E
#define LA 0x414C
#define DP 0x5044
#define PC 0x4350
#define SR 0x5253
#define RN 0x4E52
#define HI 0x4948
#define LS 0x534C
#define CS 0x5343
#define LO 0x4F4C
#define EQ 0x5145
#define VC 0x4356
#define VS 0x5356
#define PL 0x4C50
#define MI 0x494D
#define GE 0x4547
#define LT 0x544C
#define GT 0x5447
#define BE 0x4542
#define EN 0x4E45
#define DW 0x5744
#define FW 0x5746
#define FB 0x4246
#define FS 0x5346
#define DS 0x5344
#define EQ 0x5145
#define RE 0x4552
