
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 6 "yacc_sql.y"


#include "str.h"
extern char * position;
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
extern int yylex(void);
//获取子串
char *substr(const char *s,int n1,int n2)/*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
{
	char *sp=malloc(sizeof(char)*(n2-n1+2));
	int i,j=0;
	for (i=n1; i<=n2; i++) {
		sp[j++]=s[i];
	}
	sp[j]=0;
	return sp;
}
  sqlstr *ssql;
  Condition wherecondi[MAX_NUM];

	/*临时中间变量*/
int whereleng=0; 
int fromleng=0;
int selectleng=0;
int valueleng=0;
int i;
char get_id[MAX_NUM];
char *temp;

Value valueT[MAX_NUM];

int compOpT;
void yyerror(const char *str) {

	ssql->flag=0;
	whereleng=0; 
  fromleng=0;
  selectleng=0;
	valueleng=0;
  ssql->sstr.ins.nValues=0;
	ssql->sstr.errors=position;

}



/* Line 189 of yacc.c  */
#line 122 "yacc_sql.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SEMICOLON = 258,
     CREATE = 259,
     DROP = 260,
     TABLE = 261,
     INDEX = 262,
     SELECT = 263,
     INSERT = 264,
     DELETE = 265,
     UPDATE = 266,
     LBRACE = 267,
     RBRACE = 268,
     COMMA = 269,
     INT_T = 270,
     STRING_T = 271,
     FLOAT_T = 272,
     HELP = 273,
     EXIT = 274,
     DOT = 275,
     INTO = 276,
     VALUES = 277,
     FROM = 278,
     WHERE = 279,
     AND = 280,
     SET = 281,
     ON = 282,
     EQ = 283,
     LT = 284,
     GT = 285,
     LE = 286,
     GE = 287,
     NE = 288,
     NUMBER = 289,
     FLOAT = 290,
     ID = 291,
     SSS = 292,
     STAR = 293,
     STRING_V = 294
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 59 "yacc_sql.y"

    struct _Attr *attr;
	  struct _Condition *condition1;
  	struct _Value *value1;
    char *string;
  //  char *ssss;
    int number;
    float floats;




/* Line 214 of yacc.c  */
#line 210 "yacc_sql.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 222 "yacc_sql.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   126

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  27
/* YYNRULES -- Number of rules.  */
#define YYNRULES  62
/* YYNRULES -- Number of states.  */
#define YYNSTATES  133

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    30,    33,    38,    48,    53,
      62,    63,    67,    73,    76,    78,    80,    82,    84,    86,
      96,    97,   101,   103,   105,   107,   113,   122,   130,   132,
     135,   140,   141,   145,   151,   152,   156,   157,   161,   162,
     166,   170,   174,   178,   182,   188,   194,   202,   204,   206,
     208,   210,   212
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      41,     0,    -1,    -1,    41,    42,    -1,    59,    -1,    54,
      -1,    58,    -1,    57,    -1,    48,    -1,    45,    -1,    46,
      -1,    47,    -1,    44,    -1,    43,    -1,    19,     3,    -1,
      18,     3,    -1,     5,     6,    36,     3,    -1,     4,     7,
      36,    27,    36,    12,    36,    13,     3,    -1,     5,     7,
      36,     3,    -1,     4,     6,    36,    12,    50,    49,    13,
       3,    -1,    -1,    14,    50,    49,    -1,    53,    52,    12,
      51,    13,    -1,    53,    52,    -1,    34,    -1,    15,    -1,
      16,    -1,    17,    -1,    36,    -1,     9,    21,    36,    22,
      12,    56,    55,    13,     3,    -1,    -1,    14,    56,    55,
      -1,    34,    -1,    35,    -1,    37,    -1,    10,    23,    36,
      63,     3,    -1,    11,    36,    26,    36,    28,    56,    63,
       3,    -1,     8,    60,    23,    36,    62,    63,     3,    -1,
      38,    -1,    36,    61,    -1,    36,    20,    36,    61,    -1,
      -1,    14,    36,    61,    -1,    14,    36,    20,    36,    61,
      -1,    -1,    14,    36,    62,    -1,    -1,    24,    65,    64,
      -1,    -1,    25,    65,    64,    -1,    36,    66,    56,    -1,
      56,    66,    56,    -1,    36,    66,    36,    -1,    56,    66,
      36,    -1,    36,    20,    36,    66,    56,    -1,    56,    66,
      36,    20,    36,    -1,    36,    20,    36,    66,    36,    20,
      36,    -1,    28,    -1,    29,    -1,    30,    -1,    31,    -1,
      32,    -1,    33,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    85,    85,    87,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   118,   123,   127,   132,   141,   147,
     157,   159,   163,   170,   179,   182,   183,   184,   187,   197,
     212,   214,   220,   226,   232,   245,   257,   272,   293,   296,
     300,   305,   307,   311,   317,   319,   323,   325,   329,   331,
     336,   348,   361,   372,   385,   397,   409,   422,   423,   424,
     425,   426,   427
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SEMICOLON", "CREATE", "DROP", "TABLE",
  "INDEX", "SELECT", "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE",
  "COMMA", "INT_T", "STRING_T", "FLOAT_T", "HELP", "EXIT", "DOT", "INTO",
  "VALUES", "FROM", "WHERE", "AND", "SET", "ON", "EQ", "LT", "GT", "LE",
  "GE", "NE", "NUMBER", "FLOAT", "ID", "SSS", "STAR", "STRING_V",
  "$accept", "commands", "command", "exit", "help", "drop_table",
  "create_index", "drop_index", "create_table", "attr_def_list",
  "attr_def", "number", "type", "ID_get", "insert", "value_list", "value",
  "delete", "update", "select", "select_attr", "attr_list", "rel_list",
  "where", "condition_list", "condition", "comOp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    40,    41,    41,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    43,    44,    45,    46,    47,    48,
      49,    49,    50,    50,    51,    52,    52,    52,    53,    54,
      55,    55,    56,    56,    56,    57,    58,    59,    60,    60,
      60,    61,    61,    61,    62,    62,    63,    63,    64,    64,
      65,    65,    65,    65,    65,    65,    65,    66,    66,    66,
      66,    66,    66
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     4,     9,     4,     8,
       0,     3,     5,     2,     1,     1,     1,     1,     1,     9,
       0,     3,     1,     1,     1,     5,     8,     7,     1,     2,
       4,     0,     3,     5,     0,     3,     0,     3,     0,     3,
       3,     3,     3,     3,     5,     5,     7,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     3,    13,    12,     9,    10,    11,     8,     5,     7,
       6,     4,     0,     0,     0,     0,    41,    38,     0,     0,
       0,     0,    15,    14,     0,     0,     0,     0,     0,     0,
      39,     0,     0,    46,     0,     0,     0,    16,    18,    41,
      41,    44,     0,     0,     0,     0,    28,    20,     0,     0,
       0,    42,    40,     0,    46,     0,    32,    33,     0,    34,
       0,    48,    35,     0,     0,     0,    25,    26,    27,    23,
       0,    41,    44,     0,    30,     0,    57,    58,    59,    60,
      61,    62,     0,     0,     0,    47,    46,    20,     0,     0,
       0,    43,    45,    37,     0,     0,     0,    52,    50,    53,
      51,    48,     0,    21,    19,    24,     0,     0,    30,     0,
       0,     0,    49,    36,    22,    17,    31,    29,     0,    54,
      55,     0,    56
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    11,    12,    13,    14,    15,    16,    17,    75,
      57,   116,    79,    58,    18,   105,    70,    19,    20,    21,
      28,    40,    64,    54,    95,    71,    92
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -70
static const yytype_int8 yypact[] =
{
     -70,     5,   -70,    19,    65,    32,   -14,    -1,    -7,    35,
      66,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,
     -70,   -70,    38,    39,    40,    41,    -8,   -70,    50,    42,
      43,     9,   -70,   -70,    68,    54,    79,    80,    48,    49,
     -70,    51,    64,    67,    52,    53,    56,   -70,   -70,    -3,
      76,    81,    82,   -16,    90,    69,   -70,    84,    37,    87,
      60,   -70,   -70,    70,    67,    -4,   -70,   -70,    12,   -70,
      18,    75,   -70,    -4,    53,    88,   -70,   -70,   -70,    91,
      71,    76,    81,    99,    94,    73,   -70,   -70,   -70,   -70,
     -70,   -70,    22,    26,   -16,   -70,    67,    84,   101,    77,
      92,   -70,   -70,   -70,    -4,    97,    18,   -70,   -70,    93,
     -70,    75,   109,   -70,   -70,   -70,   102,   111,    94,   113,
      30,    83,   -70,   -70,   -70,   -70,   -70,   -70,    98,   -70,
     -70,    85,   -70
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,    20,
      46,   -70,   -70,   -70,   -70,     4,   -65,   -70,   -70,   -70,
     -70,   -47,    44,   -60,    13,    29,   -69
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      84,    93,    61,    62,    83,     2,    38,    29,    96,     3,
       4,    38,    39,     5,     6,     7,     8,    60,    66,    67,
      68,    69,    30,     9,    10,    22,    23,   108,   110,    31,
      66,    67,    85,    69,   101,    44,   112,   120,    32,   118,
      86,    87,    88,    89,    90,    91,    86,    87,    88,    89,
      90,    91,    76,    77,    78,   129,    66,    67,   107,    69,
      66,    67,   109,    69,    66,    67,   128,    69,    26,    33,
      27,    24,    25,    41,    34,    35,    36,    37,    42,    43,
      45,    46,    47,    48,    49,    50,    52,    51,    55,    56,
      38,    53,    59,    72,    65,    63,    81,    73,    74,    80,
      94,    98,   103,    99,   114,   117,    82,   100,   104,   106,
     119,   115,   123,   121,   125,   124,   127,   113,   131,   130,
      97,   132,   126,   111,   122,     0,   102
};

static const yytype_int8 yycheck[] =
{
      65,    70,    49,    50,    64,     0,    14,    21,    73,     4,
       5,    14,    20,     8,     9,    10,    11,    20,    34,    35,
      36,    37,    23,    18,    19,     6,     7,    92,    93,    36,
      34,    35,    20,    37,    81,    26,    96,   106,     3,   104,
      28,    29,    30,    31,    32,    33,    28,    29,    30,    31,
      32,    33,    15,    16,    17,   120,    34,    35,    36,    37,
      34,    35,    36,    37,    34,    35,    36,    37,    36,     3,
      38,     6,     7,    23,    36,    36,    36,    36,    36,    36,
      12,    27,     3,     3,    36,    36,    22,    36,    36,    36,
      14,    24,    36,     3,    12,    14,    36,    28,    14,    12,
      25,    13,     3,    12,     3,    13,    36,    36,    14,    36,
      13,    34,     3,    20,     3,    13,     3,    97,    20,    36,
      74,    36,   118,    94,   111,    -1,    82
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    41,     0,     4,     5,     8,     9,    10,    11,    18,
      19,    42,    43,    44,    45,    46,    47,    48,    54,    57,
      58,    59,     6,     7,     6,     7,    36,    38,    60,    21,
      23,    36,     3,     3,    36,    36,    36,    36,    14,    20,
      61,    23,    36,    36,    26,    12,    27,     3,     3,    36,
      36,    36,    22,    24,    63,    36,    36,    50,    53,    36,
      20,    61,    61,    14,    62,    12,    34,    35,    36,    37,
      56,    65,     3,    28,    14,    49,    15,    16,    17,    52,
      12,    36,    36,    63,    56,    20,    28,    29,    30,    31,
      32,    33,    66,    66,    25,    64,    56,    50,    13,    12,
      36,    61,    62,     3,    14,    55,    36,    36,    56,    36,
      56,    65,    63,    49,     3,    34,    51,    13,    56,    13,
      66,    20,    64,     3,    13,     3,    55,     3,    36,    56,
      36,    20,    36
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 14:

/* Line 1455 of yacc.c  */
#line 118 "yacc_sql.y"
    {
        ssql->flag=10;//"exit";
    ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 123 "yacc_sql.y"
    {
        ssql->flag=9;//"help";
    ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 127 "yacc_sql.y"
    {
        ssql->flag=6;//"drop_table";
        ssql->sstr.drt.relName=(yyvsp[(3) - (4)].string);
    ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 132 "yacc_sql.y"
    {
																							        ssql->flag=7;//"create_index";
																							        ssql->sstr.crei.indexName=(yyvsp[(3) - (9)].string);
																							        ssql->sstr.crei.relName=(yyvsp[(5) - (9)].string);
																							        ssql->sstr.crei.attrName=(yyvsp[(7) - (9)].string);
																							    ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 141 "yacc_sql.y"
    {
											        ssql->flag=8;//"drop_index";
											        ssql->sstr.dri.indexName=(yyvsp[(3) - (4)].string);
											    ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 147 "yacc_sql.y"
    {
	
																																		ssql->flag=5;//"create_table";
																																		ssql->sstr.cret.relName=(yyvsp[(3) - (8)].string);
																																		ssql->sstr.cret.attrCount=valueleng;
																																		//临时变量清零	
																																		valueleng=0;
																																		
																																	    ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 159 "yacc_sql.y"
    {    ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 163 "yacc_sql.y"
    {
																ssql->sstr.cret.attributes[valueleng].attrName=(char*)malloc(sizeof(char));
																strcpy(ssql->sstr.cret.attributes[valueleng].attrName,get_id); 
																ssql->sstr.cret.attributes[valueleng].attrType=(yyvsp[(2) - (5)].number);  
																ssql->sstr.cret.attributes[valueleng].attrLength=(yyvsp[(4) - (5)].number);
																valueleng++;
															    ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 170 "yacc_sql.y"
    {
															ssql->sstr.cret.attributes[valueleng].attrName=(char*)malloc(sizeof(char));
															strcpy(ssql->sstr.cret.attributes[valueleng].attrName,get_id); 
															ssql->sstr.cret.attributes[valueleng].attrType=(yyvsp[(2) - (2)].number);  
															ssql->sstr.cret.attributes[valueleng].attrLength=4;
															valueleng++;
														    ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 179 "yacc_sql.y"
    {(yyval.number) = (yyvsp[(1) - (1)].number);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 182 "yacc_sql.y"
    { (yyval.number)=1; ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 183 "yacc_sql.y"
    { (yyval.number)=0; ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 184 "yacc_sql.y"
    { (yyval.number)=2; ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 187 "yacc_sql.y"
    {
				 temp=(yyvsp[(1) - (1)].string); 
				for(i=0;i<MAX_NUM;i++){
				get_id[i]=*(temp+i);
				}
			;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 197 "yacc_sql.y"
    {
 																																valueT[valueleng++] = *(yyvsp[(6) - (9)].value1);
																																
																																ssql->flag=2;//"insert";
																																ssql->sstr.ins.relName=(yyvsp[(3) - (9)].string);	
																																ssql->sstr.ins.nValues=valueleng;
																																for(i=0;i<valueleng;i++){
																																ssql->sstr.ins.values[i] = valueT[i];
																												}
																																//临时变量清零	
																																valueleng=0;
																															//	free(valueT);
	
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 214 "yacc_sql.y"
    { 
  													 valueT[valueleng++] = *(yyvsp[(2) - (3)].value1);
	;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 220 "yacc_sql.y"
    {	
					(yyval.value1) = (Value*)malloc(sizeof(Value));
					(yyval.value1)->data=(int*)malloc((sizeof(int)));
					(yyval.value1)->type =1; 					                      // "ints";
					*((int*)((yyval.value1)->data)) = (yyvsp[(1) - (1)].number);
					;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 226 "yacc_sql.y"
    {								
					(yyval.value1)=(Value*)malloc(sizeof(Value));
					(yyval.value1)->data=malloc(MAX_NUM*(sizeof(char)));
					(yyval.value1)->type =2; 					                     	//"floats"
					*((float*)((yyval.value1)->data)) = (yyvsp[(1) - (1)].floats);
					;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 232 "yacc_sql.y"
    {																		
					(yyval.value1)=(Value*)malloc(sizeof(Value));				
					(yyval.value1)->data=malloc(MAX_NUM*(sizeof(char)));		
    			(yyval.value1)->type = 0;					// "chars";						
    			(yyvsp[(1) - (1)].string) = substr((yyvsp[(1) - (1)].string),1,strlen((yyvsp[(1) - (1)].string))-2);
    			sprintf((char*)((yyval.value1)->data), "%s", (yyvsp[(1) - (1)].string)); 			
    		
					
    			 ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 245 "yacc_sql.y"
    {
	
	ssql->flag=4;//"delete";
	ssql->sstr.del.relName=(yyvsp[(3) - (5)].string);
	for(i=0;i<whereleng;i++){
	ssql->sstr.del.conditions[i]=wherecondi[i];
	}
	ssql->sstr.del.nConditions=whereleng;
	whereleng=0;	
    ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 257 "yacc_sql.y"
    {
	
											ssql->flag=3;//"update";
											ssql->sstr.upd.relName=(yyvsp[(2) - (8)].string);
											ssql->sstr.upd.value=*(yyvsp[(6) - (8)].value1);
											ssql->sstr.upd.attrName=(yyvsp[(4) - (8)].string);
											for(i=0;i<whereleng;i++){
											ssql->sstr.upd.conditions[i]=wherecondi[i];
											}
											ssql->sstr.upd.nConditions=whereleng;
											whereleng=0;
									
										    ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 272 "yacc_sql.y"
    {
      
																												ssql->sstr.sel.relations[fromleng++]=(yyvsp[(4) - (7)].string);
																											
																												for(i=0;i<whereleng;i++){
																												ssql->sstr.sel.conditions[i]=wherecondi[i];
																												}
																											
																												ssql->flag=1;//"select";
																												ssql->sstr.sel.nSelAttrs=selectleng;
																												ssql->sstr.sel.nRelations=fromleng;	
																												ssql->sstr.sel.nConditions=whereleng;
																											
																												//临时变量清零
																												whereleng=0;	
																												fromleng=0;
																												selectleng=0;
   																										 ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 293 "yacc_sql.y"
    { ssql->sstr.sel.selAttrs[selectleng]->attrName="*";	
    			ssql->sstr.sel.selAttrs[selectleng++]->relName=NULL;
    	;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 296 "yacc_sql.y"
    {
										ssql->sstr.sel.selAttrs[selectleng]->attrName=(yyvsp[(1) - (2)].string);
										ssql->sstr.sel.selAttrs[selectleng++]->relName=NULL;
  									;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 300 "yacc_sql.y"
    {
  						ssql->sstr.sel.selAttrs[selectleng]->attrName=(yyvsp[(3) - (4)].string);
							ssql->sstr.sel.selAttrs[selectleng++]->relName=(yyvsp[(1) - (4)].string);
  			;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 307 "yacc_sql.y"
    {
     	  ssql->sstr.sel.selAttrs[selectleng]->relName = NULL;
        ssql->sstr.sel.selAttrs[selectleng++]->attrName=(yyvsp[(2) - (3)].string);
    ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 311 "yacc_sql.y"
    {
  						ssql->sstr.sel.selAttrs[selectleng]->attrName=(yyvsp[(4) - (5)].string);
							ssql->sstr.sel.selAttrs[selectleng++]->relName=(yyvsp[(2) - (5)].string);
  			;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 319 "yacc_sql.y"
    {	
									        ssql->sstr.sel.relations[fromleng++]=(yyvsp[(2) - (3)].string);
									    	;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 325 "yacc_sql.y"
    {	
																		wherecondi[whereleng++]=*(yyvsp[(2) - (3)].condition1);
																	    ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 331 "yacc_sql.y"
    {
																		wherecondi[whereleng++]=*(yyvsp[(2) - (3)].condition1);
													   			;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 336 "yacc_sql.y"
    {
									(yyval.condition1) = ( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 1;
									(yyval.condition1)->lhsAttr.relName = NULL;
									(yyval.condition1)->lhsAttr.attrName = (yyvsp[(1) - (3)].string);
									(yyval.condition1)->op = compOpT;
									(yyval.condition1)->bRhsIsAttr = 0;
									(yyval.condition1)->rhsAttr.relName = NULL;
									(yyval.condition1)->rhsAttr.attrName = NULL;
									(yyval.condition1)->rhsValue = *(yyvsp[(3) - (3)].value1);

								    ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 348 "yacc_sql.y"
    {
									(yyval.condition1) = ( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 0;
									(yyval.condition1)->lhsAttr.relName=NULL;
									(yyval.condition1)->lhsAttr.attrName=NULL;
									(yyval.condition1)->lhsValue = *(yyvsp[(1) - (3)].value1);
									(yyval.condition1)->op = compOpT;
									(yyval.condition1)->bRhsIsAttr = 0;
									(yyval.condition1)->rhsAttr.relName = NULL;
									(yyval.condition1)->rhsAttr.attrName = NULL;
									(yyval.condition1)->rhsValue = *(yyvsp[(3) - (3)].value1);

								    ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 361 "yacc_sql.y"
    {
									(yyval.condition1)=( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 1;
									(yyval.condition1)->lhsAttr.relName=NULL;
									(yyval.condition1)->lhsAttr.attrName=(yyvsp[(1) - (3)].string);
									(yyval.condition1)->op=compOpT;
									(yyval.condition1)->bRhsIsAttr = 1;
									(yyval.condition1)->rhsAttr.relName=NULL;
									(yyval.condition1)->rhsAttr.attrName=(yyvsp[(3) - (3)].string);

								    ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 372 "yacc_sql.y"
    {
									(yyval.condition1)=( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 0;
									(yyval.condition1)->lhsAttr.relName=NULL;
									(yyval.condition1)->lhsAttr.attrName=NULL;
									(yyval.condition1)->lhsValue = *(yyvsp[(1) - (3)].value1);
									(yyval.condition1)->op=compOpT;
									
									(yyval.condition1)->bRhsIsAttr = 1;
									(yyval.condition1)->rhsAttr.relName=NULL;
									(yyval.condition1)->rhsAttr.attrName=(yyvsp[(3) - (3)].string);
								
								    ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 385 "yacc_sql.y"
    {
									(yyval.condition1)=( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 1;
									(yyval.condition1)->lhsAttr.relName=(yyvsp[(1) - (5)].string);
									(yyval.condition1)->lhsAttr.attrName=(yyvsp[(3) - (5)].string);
									(yyval.condition1)->op=compOpT;
									(yyval.condition1)->bRhsIsAttr = 0;   //属性值
									(yyval.condition1)->rhsAttr.relName=NULL;
									(yyval.condition1)->rhsAttr.attrName=NULL;
									(yyval.condition1)->rhsValue=*(yyvsp[(5) - (5)].value1);			
							
    											;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 397 "yacc_sql.y"
    {
									(yyval.condition1)=( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 0;//属性值
									(yyval.condition1)->lhsAttr.relName=NULL;
									(yyval.condition1)->lhsAttr.attrName=NULL;
									(yyval.condition1)->lhsValue = *(yyvsp[(1) - (5)].value1);
									(yyval.condition1)->op=compOpT;
									(yyval.condition1)->bRhsIsAttr = 1;//属性
									(yyval.condition1)->rhsAttr.relName = (yyvsp[(3) - (5)].string);
									(yyval.condition1)->rhsAttr.attrName = (yyvsp[(5) - (5)].string);
									
    						;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 409 "yacc_sql.y"
    {
									(yyval.condition1)=( Condition *)malloc(sizeof( Condition));
									(yyval.condition1)->bLhsIsAttr = 1;		//属性
									(yyval.condition1)->lhsAttr.relName=(yyvsp[(1) - (7)].string);
									(yyval.condition1)->lhsAttr.attrName=(yyvsp[(3) - (7)].string);
									(yyval.condition1)->op=compOpT;
									(yyval.condition1)->bRhsIsAttr = 1;		//属性
									(yyval.condition1)->rhsAttr.relName=(yyvsp[(5) - (7)].string);
									(yyval.condition1)->rhsAttr.attrName=(yyvsp[(7) - (7)].string);								
    											;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 422 "yacc_sql.y"
    { compOpT = 0; ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 423 "yacc_sql.y"
    { compOpT = 3; ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 424 "yacc_sql.y"
    { compOpT = 5; ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 425 "yacc_sql.y"
    { compOpT = 1; ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 426 "yacc_sql.y"
    { compOpT = 4; ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 427 "yacc_sql.y"
    { compOpT = 2; ;}
    break;



/* Line 1455 of yacc.c  */
#line 2034 "yacc_sql.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 430 "yacc_sql.y"

//_____________________________________________________________________
int hust_parse( sqlstr *sqls){
	ssql=sqls;
	return yyparse();
}

