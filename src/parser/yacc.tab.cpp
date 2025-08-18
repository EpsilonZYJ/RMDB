/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"

#include "ast.h"
#include "yacc.tab.h"
#include <iostream>
#include <memory>

int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);

void yyerror(YYLTYPE *locp, const char* s) {
    std::cerr << "Parser Error at line " << locp->first_line << " column " << locp->first_column << ": " << s << std::endl;
}
using namespace ast;
std::vector<std::shared_ptr<ast::JoinExpr>> current_joins;

#line 86 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SHOW = 3,                       /* SHOW  */
  YYSYMBOL_TABLES = 4,                     /* TABLES  */
  YYSYMBOL_CREATE = 5,                     /* CREATE  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_DROP = 7,                       /* DROP  */
  YYSYMBOL_DESC = 8,                       /* DESC  */
  YYSYMBOL_INSERT = 9,                     /* INSERT  */
  YYSYMBOL_INTO = 10,                      /* INTO  */
  YYSYMBOL_VALUES = 11,                    /* VALUES  */
  YYSYMBOL_DELETE = 12,                    /* DELETE  */
  YYSYMBOL_FROM = 13,                      /* FROM  */
  YYSYMBOL_ASC = 14,                       /* ASC  */
  YYSYMBOL_ORDER = 15,                     /* ORDER  */
  YYSYMBOL_BY = 16,                        /* BY  */
  YYSYMBOL_WHERE = 17,                     /* WHERE  */
  YYSYMBOL_UPDATE = 18,                    /* UPDATE  */
  YYSYMBOL_SET = 19,                       /* SET  */
  YYSYMBOL_SELECT = 20,                    /* SELECT  */
  YYSYMBOL_INT = 21,                       /* INT  */
  YYSYMBOL_CHAR = 22,                      /* CHAR  */
  YYSYMBOL_FLOAT = 23,                     /* FLOAT  */
  YYSYMBOL_DATE = 24,                      /* DATE  */
  YYSYMBOL_INDEX = 25,                     /* INDEX  */
  YYSYMBOL_AND = 26,                       /* AND  */
  YYSYMBOL_JOIN = 27,                      /* JOIN  */
  YYSYMBOL_ON = 28,                        /* ON  */
  YYSYMBOL_EXIT = 29,                      /* EXIT  */
  YYSYMBOL_HELP = 30,                      /* HELP  */
  YYSYMBOL_TXN_BEGIN = 31,                 /* TXN_BEGIN  */
  YYSYMBOL_TXN_COMMIT = 32,                /* TXN_COMMIT  */
  YYSYMBOL_TXN_ABORT = 33,                 /* TXN_ABORT  */
  YYSYMBOL_TXN_ROLLBACK = 34,              /* TXN_ROLLBACK  */
  YYSYMBOL_ORDER_BY = 35,                  /* ORDER_BY  */
  YYSYMBOL_ENABLE_NESTLOOP = 36,           /* ENABLE_NESTLOOP  */
  YYSYMBOL_ENABLE_SORTMERGE = 37,          /* ENABLE_SORTMERGE  */
  YYSYMBOL_EXPLAIN = 38,                   /* EXPLAIN  */
  YYSYMBOL_SEMI = 39,                      /* SEMI  */
  YYSYMBOL_ANTI = 40,                      /* ANTI  */
  YYSYMBOL_COUNT = 41,                     /* COUNT  */
  YYSYMBOL_MAX = 42,                       /* MAX  */
  YYSYMBOL_MIN = 43,                       /* MIN  */
  YYSYMBOL_SUM = 44,                       /* SUM  */
  YYSYMBOL_AVG = 45,                       /* AVG  */
  YYSYMBOL_AS = 46,                        /* AS  */
  YYSYMBOL_GROUP = 47,                     /* GROUP  */
  YYSYMBOL_HAVING = 48,                    /* HAVING  */
  YYSYMBOL_LIMIT = 49,                     /* LIMIT  */
  YYSYMBOL_STATIC_CHECKPOINT = 50,         /* STATIC_CHECKPOINT  */
  YYSYMBOL_LOAD = 51,                      /* LOAD  */
  YYSYMBOL_LEQ = 52,                       /* LEQ  */
  YYSYMBOL_NEQ = 53,                       /* NEQ  */
  YYSYMBOL_GEQ = 54,                       /* GEQ  */
  YYSYMBOL_T_EOF = 55,                     /* T_EOF  */
  YYSYMBOL_IDENTIFIER = 56,                /* IDENTIFIER  */
  YYSYMBOL_VALUE_STRING = 57,              /* VALUE_STRING  */
  YYSYMBOL_VALUE_INT = 58,                 /* VALUE_INT  */
  YYSYMBOL_VALUE_FLOAT = 59,               /* VALUE_FLOAT  */
  YYSYMBOL_VALUE_DATE = 60,                /* VALUE_DATE  */
  YYSYMBOL_VALUE_BOOL = 61,                /* VALUE_BOOL  */
  YYSYMBOL_OP_PLUS = 62,                   /* OP_PLUS  */
  YYSYMBOL_OP_MINUS = 63,                  /* OP_MINUS  */
  YYSYMBOL_OP_TIMES = 64,                  /* OP_TIMES  */
  YYSYMBOL_OP_DIVIDE = 65,                 /* OP_DIVIDE  */
  YYSYMBOL_VALUE_PATH = 66,                /* VALUE_PATH  */
  YYSYMBOL_67_ = 67,                       /* ';'  */
  YYSYMBOL_68_ = 68,                       /* '='  */
  YYSYMBOL_69_ = 69,                       /* '('  */
  YYSYMBOL_70_ = 70,                       /* ')'  */
  YYSYMBOL_71_ = 71,                       /* ','  */
  YYSYMBOL_72_ = 72,                       /* '.'  */
  YYSYMBOL_73_ = 73,                       /* '<'  */
  YYSYMBOL_74_ = 74,                       /* '>'  */
  YYSYMBOL_75_ = 75,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 76,                  /* $accept  */
  YYSYMBOL_start = 77,                     /* start  */
  YYSYMBOL_stmt = 78,                      /* stmt  */
  YYSYMBOL_txnStmt = 79,                   /* txnStmt  */
  YYSYMBOL_dbStmt = 80,                    /* dbStmt  */
  YYSYMBOL_setStmt = 81,                   /* setStmt  */
  YYSYMBOL_ddl = 82,                       /* ddl  */
  YYSYMBOL_dml = 83,                       /* dml  */
  YYSYMBOL_file_path = 84,                 /* file_path  */
  YYSYMBOL_fieldList = 85,                 /* fieldList  */
  YYSYMBOL_colNameList = 86,               /* colNameList  */
  YYSYMBOL_field = 87,                     /* field  */
  YYSYMBOL_type = 88,                      /* type  */
  YYSYMBOL_valueList = 89,                 /* valueList  */
  YYSYMBOL_value = 90,                     /* value  */
  YYSYMBOL_condition = 91,                 /* condition  */
  YYSYMBOL_optWhereClause = 92,            /* optWhereClause  */
  YYSYMBOL_whereClause = 93,               /* whereClause  */
  YYSYMBOL_col = 94,                       /* col  */
  YYSYMBOL_colList = 95,                   /* colList  */
  YYSYMBOL_op = 96,                        /* op  */
  YYSYMBOL_expr = 97,                      /* expr  */
  YYSYMBOL_setClauses = 98,                /* setClauses  */
  YYSYMBOL_setClause = 99,                 /* setClause  */
  YYSYMBOL_asClause = 100,                 /* asClause  */
  YYSYMBOL_select_item = 101,              /* select_item  */
  YYSYMBOL_select_list = 102,              /* select_list  */
  YYSYMBOL_tableList = 103,                /* tableList  */
  YYSYMBOL_opt_order_clause = 104,         /* opt_order_clause  */
  YYSYMBOL_order_items = 105,              /* order_items  */
  YYSYMBOL_order_item = 106,               /* order_item  */
  YYSYMBOL_opt_asc_desc = 107,             /* opt_asc_desc  */
  YYSYMBOL_group_by_clause = 108,          /* group_by_clause  */
  YYSYMBOL_agg_expr = 109,                 /* agg_expr  */
  YYSYMBOL_having_clause = 110,            /* having_clause  */
  YYSYMBOL_having_clauses = 111,           /* having_clauses  */
  YYSYMBOL_opt_limit_clause = 112,         /* opt_limit_clause  */
  YYSYMBOL_set_knob_type = 113,            /* set_knob_type  */
  YYSYMBOL_tbName = 114,                   /* tbName  */
  YYSYMBOL_colName = 115,                  /* colName  */
  YYSYMBOL_alias = 116                     /* alias  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  57
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   251

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  117
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  256

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   321


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      69,    70,    75,     2,    71,     2,    72,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    67,
      73,    68,    74,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    69,    69,    74,    79,    84,    92,    93,    94,    95,
      96,   100,   104,   108,   112,   119,   123,   127,   134,   141,
     145,   149,   153,   157,   164,   168,   172,   176,   182,   189,
     195,   197,   201,   205,   212,   216,   223,   230,   234,   238,
     242,   249,   253,   260,   264,   268,   272,   276,   283,   290,
     291,   298,   302,   309,   313,   320,   324,   331,   335,   339,
     343,   347,   351,   358,   362,   369,   373,   380,   384,   388,
     392,   396,   403,   408,   414,   418,   422,   426,   430,   434,
     438,   452,   456,   460,   467,   471,   475,   479,   499,   515,
     534,   538,   542,   547,   555,   569,   570,   571,   575,   580,
     586,   590,   594,   598,   602,   606,   613,   617,   624,   629,
     635,   640,   646,   647,   651,   655,   662,   664
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SHOW", "TABLES",
  "CREATE", "TABLE", "DROP", "DESC", "INSERT", "INTO", "VALUES", "DELETE",
  "FROM", "ASC", "ORDER", "BY", "WHERE", "UPDATE", "SET", "SELECT", "INT",
  "CHAR", "FLOAT", "DATE", "INDEX", "AND", "JOIN", "ON", "EXIT", "HELP",
  "TXN_BEGIN", "TXN_COMMIT", "TXN_ABORT", "TXN_ROLLBACK", "ORDER_BY",
  "ENABLE_NESTLOOP", "ENABLE_SORTMERGE", "EXPLAIN", "SEMI", "ANTI",
  "COUNT", "MAX", "MIN", "SUM", "AVG", "AS", "GROUP", "HAVING", "LIMIT",
  "STATIC_CHECKPOINT", "LOAD", "LEQ", "NEQ", "GEQ", "T_EOF", "IDENTIFIER",
  "VALUE_STRING", "VALUE_INT", "VALUE_FLOAT", "VALUE_DATE", "VALUE_BOOL",
  "OP_PLUS", "OP_MINUS", "OP_TIMES", "OP_DIVIDE", "VALUE_PATH", "';'",
  "'='", "'('", "')'", "','", "'.'", "'<'", "'>'", "'*'", "$accept",
  "start", "stmt", "txnStmt", "dbStmt", "setStmt", "ddl", "dml",
  "file_path", "fieldList", "colNameList", "field", "type", "valueList",
  "value", "condition", "optWhereClause", "whereClause", "col", "colList",
  "op", "expr", "setClauses", "setClause", "asClause", "select_item",
  "select_list", "tableList", "opt_order_clause", "order_items",
  "order_item", "opt_asc_desc", "group_by_clause", "agg_expr",
  "having_clause", "having_clauses", "opt_limit_clause", "set_knob_type",
  "tbName", "colName", "alias", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-213)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-115)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      80,    13,    21,     6,   -14,    35,    34,   -14,    54,    52,
    -213,  -213,  -213,  -213,  -213,  -213,    32,   -18,  -213,    70,
     -13,  -213,  -213,  -213,  -213,  -213,  -213,    62,   -14,   -14,
    -213,   -14,   -14,    28,  -213,   -14,   -14,    82,  -213,  -213,
      18,    48,    53,    59,    65,    72,   -32,  -213,    87,  -213,
      -6,    76,  -213,    52,  -213,  -213,   139,  -213,  -213,   -14,
      81,   108,  -213,   109,  -213,   168,   169,   131,   127,   -41,
     133,   133,   133,   133,   134,  -213,   -14,    95,   131,    -5,
     -14,  -213,   131,   131,   131,   123,   133,  -213,  -213,   -12,
    -213,   125,  -213,   121,   124,   126,   128,   129,   130,  -213,
    -213,    -4,  -213,  -213,  -213,   -14,  -213,   -21,  -213,   122,
      36,  -213,    45,   115,  -213,   171,    51,   131,  -213,    96,
      87,    87,    87,    87,    87,    87,   -14,   174,   175,   -14,
     148,    -4,  -213,   131,  -213,   135,  -213,  -213,  -213,  -213,
     131,  -213,  -213,  -213,  -213,  -213,  -213,    50,  -213,   133,
    -213,  -213,  -213,  -213,  -213,  -213,   102,  -213,  -213,    67,
    -213,  -213,  -213,  -213,  -213,  -213,   177,   -14,   -14,  -213,
     187,   158,   148,  -213,   149,  -213,  -213,   115,  -213,  -213,
    -213,  -213,   115,   115,   115,   115,   133,   189,   190,   133,
     140,   200,   158,   150,  -213,  -213,  -213,  -213,  -213,  -213,
     133,   133,  -213,   151,   152,   154,   155,   156,   157,    51,
     193,   211,   179,   200,  -213,  -213,  -213,   133,   -34,   133,
     133,   133,   133,   102,   140,   133,   172,  -213,   179,  -213,
     159,   161,   162,   163,   164,   165,  -213,    51,    64,   166,
    -213,  -213,  -213,  -213,  -213,  -213,  -213,  -213,  -213,   102,
    -213,  -213,  -213,   133,  -213,  -213
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     3,    11,    12,    13,    14,     0,     0,     5,     0,
       0,     9,     6,    10,     7,     8,    15,     0,     0,     0,
      17,     0,     0,   114,    21,     0,     0,     0,   112,   113,
       0,     0,     0,     0,     0,     0,   116,    81,    73,    82,
       0,     0,    54,     0,    30,    31,     0,     1,     2,     0,
       0,     0,    20,     0,   115,     0,    49,     0,     0,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
       0,    16,     0,     0,     0,     0,     0,    25,   116,    49,
      65,     0,    18,     0,     0,     0,     0,     0,     0,   117,
      72,    49,    84,    83,    53,     0,    29,     0,    32,     0,
       0,    34,     0,     0,    51,    50,     0,     0,    26,     0,
      73,    73,    73,    73,    73,    73,     0,     0,     0,     0,
      99,    49,    19,     0,    37,     0,    39,    40,    36,    22,
       0,    23,    45,    43,    44,    47,    46,     0,    41,     0,
      61,    60,    62,    57,    58,    59,     0,    66,    67,     0,
      75,    76,    77,    78,    79,    80,    86,     0,     0,    85,
       0,   109,    99,    33,     0,    35,    24,     0,    52,    63,
      64,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    91,   109,     0,    42,    68,    69,    71,    70,    87,
       0,     0,    55,    98,     0,     0,     0,     0,     0,     0,
     108,     0,   111,    91,    38,    88,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,   111,    56,
       0,     0,     0,     0,     0,     0,   106,     0,    97,    90,
      92,   110,    28,   100,   101,   102,   103,   104,   105,     0,
      96,    95,    94,     0,   107,    93
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -213,  -213,  -213,  -213,  -213,  -213,  -213,  -213,  -213,  -213,
     167,   103,  -213,  -213,  -103,  -143,   -80,  -213,    -9,  -213,
    -207,  -212,  -213,   132,    46,   170,   185,   136,    26,  -213,
     -11,  -213,    71,    22,  -213,    56,    17,  -213,    -3,   -64,
    -213
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,    24,    25,    56,   107,
     110,   108,   138,   147,   179,   114,    87,   115,   116,   203,
     156,   181,    89,    90,    75,    49,    50,   101,   212,   239,
     240,   252,   171,   209,   210,   191,   227,    40,    51,    52,
     100
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      48,    34,   223,    91,    37,    86,   178,    76,   105,   118,
     148,   236,    31,    86,   104,    46,   158,    26,   109,   111,
     111,   130,    46,   126,    64,    60,    61,    28,    62,    63,
     249,    32,    65,    66,    93,   127,   128,   254,    27,    54,
    -114,   230,    33,   199,    48,    35,    29,    36,    55,   132,
     133,   172,    53,    91,    58,   159,    81,   215,   216,   117,
      94,    95,    96,    97,    98,    77,    77,   129,    48,   109,
      57,    30,   250,   102,   194,    59,   175,   106,   251,   195,
     196,   197,   198,     1,    64,     2,    68,     3,     4,     5,
      38,    39,     6,    41,    42,    43,    44,    45,     7,     8,
       9,    67,   102,   150,   151,   152,   139,   140,    46,    10,
      11,    12,    13,    14,    15,   141,   140,    69,    16,   153,
     176,   177,    70,   166,   154,   155,   169,    47,    71,   182,
     183,    17,   184,    74,    72,    18,    41,    42,    43,    44,
      45,    73,   185,   134,   135,   136,   137,   180,    78,    80,
      82,    46,    88,   142,   143,   144,   145,   146,    46,   142,
     143,   144,   145,   146,   187,   188,   160,   161,   162,   163,
     164,   165,   142,   143,   144,   145,   146,    83,    84,    85,
     202,   204,   205,   206,   207,   208,    86,    88,    92,    46,
      99,   120,   113,   119,   121,   170,   122,   149,   123,   124,
     125,   167,   168,   189,   174,   186,   190,   193,   229,   231,
     232,   233,   234,   235,   180,   211,   238,   200,   201,   224,
     214,   218,   217,   219,   220,   221,   222,   225,   226,   243,
     241,   244,   245,   246,   247,   248,   173,   253,    79,   228,
     180,   131,   255,   192,   238,   242,   237,   103,   213,   157,
       0,   112
};

static const yytype_int16 yycheck[] =
{
       9,     4,   209,    67,     7,    17,   149,    13,    13,    89,
     113,   223,     6,    17,    78,    56,   119,     4,    82,    83,
      84,   101,    56,    27,    56,    28,    29,     6,    31,    32,
     237,    25,    35,    36,    75,    39,    40,   249,    25,    57,
      72,    75,    56,   186,    53,    10,    25,    13,    66,    70,
      71,   131,    20,   117,    67,   119,    59,   200,   201,    71,
      69,    70,    71,    72,    73,    71,    71,    71,    77,   133,
       0,    50,     8,    76,   177,    13,   140,    80,    14,   182,
     183,   184,   185,     3,    56,     5,    68,     7,     8,     9,
      36,    37,    12,    41,    42,    43,    44,    45,    18,    19,
      20,    19,   105,    52,    53,    54,    70,    71,    56,    29,
      30,    31,    32,    33,    34,    70,    71,    69,    38,    68,
      70,    71,    69,   126,    73,    74,   129,    75,    69,    62,
      63,    51,    65,    46,    69,    55,    41,    42,    43,    44,
      45,    69,    75,    21,    22,    23,    24,   156,    72,    10,
      69,    56,    56,    57,    58,    59,    60,    61,    56,    57,
      58,    59,    60,    61,   167,   168,   120,   121,   122,   123,
     124,   125,    57,    58,    59,    60,    61,    69,    69,    11,
     189,    41,    42,    43,    44,    45,    17,    56,    61,    56,
      56,    70,    69,    68,    70,    47,    70,    26,    70,    70,
      70,    27,    27,    16,    69,    28,    48,    58,   217,   218,
     219,   220,   221,   222,   223,    15,   225,    28,    28,    26,
      70,    69,    71,    69,    69,    69,    69,    16,    49,    70,
      58,    70,    70,    70,    70,    70,   133,    71,    53,   213,
     249,   105,   253,   172,   253,   228,   224,    77,   192,   117,
      -1,    84
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     7,     8,     9,    12,    18,    19,    20,
      29,    30,    31,    32,    33,    34,    38,    51,    55,    77,
      78,    79,    80,    81,    82,    83,     4,    25,     6,    25,
      50,     6,    25,    56,   114,    10,    13,   114,    36,    37,
     113,    41,    42,    43,    44,    45,    56,    75,    94,   101,
     102,   114,   115,    20,    57,    66,    84,     0,    67,    13,
     114,   114,   114,   114,    56,   114,   114,    19,    68,    69,
      69,    69,    69,    69,    46,   100,    13,    71,    72,   102,
      10,   114,    69,    69,    69,    11,    17,    92,    56,    98,
      99,   115,    61,    75,    94,    94,    94,    94,    94,    56,
     116,   103,   114,   101,   115,    13,   114,    85,    87,   115,
      86,   115,    86,    69,    91,    93,    94,    71,    92,    68,
      70,    70,    70,    70,    70,    70,    27,    39,    40,    71,
      92,   103,    70,    71,    21,    22,    23,    24,    88,    70,
      71,    70,    57,    58,    59,    60,    61,    89,    90,    26,
      52,    53,    54,    68,    73,    74,    96,    99,    90,   115,
     100,   100,   100,   100,   100,   100,   114,    27,    27,   114,
      47,   108,    92,    87,    69,   115,    70,    71,    91,    90,
      94,    97,    62,    63,    65,    75,    28,   114,   114,    16,
      48,   111,   108,    58,    90,    90,    90,    90,    90,    91,
      28,    28,    94,    95,    41,    42,    43,    44,    45,   109,
     110,    15,   104,   111,    70,    91,    91,    71,    69,    69,
      69,    69,    69,    96,    26,    16,    49,   112,   104,    94,
      75,    94,    94,    94,    94,    94,    97,   109,    94,   105,
     106,    58,   112,    70,    70,    70,    70,    70,    70,    96,
       8,    14,   107,    71,    97,   106
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    76,    77,    77,    77,    77,    78,    78,    78,    78,
      78,    79,    79,    79,    79,    80,    80,    80,    81,    82,
      82,    82,    82,    82,    83,    83,    83,    83,    83,    83,
      84,    84,    85,    85,    86,    86,    87,    88,    88,    88,
      88,    89,    89,    90,    90,    90,    90,    90,    91,    92,
      92,    93,    93,    94,    94,    95,    95,    96,    96,    96,
      96,    96,    96,    97,    97,    98,    98,    99,    99,    99,
      99,    99,   100,   100,   101,   101,   101,   101,   101,   101,
     101,   102,   102,   102,   103,   103,   103,   103,   103,   103,
     104,   104,   105,   105,   106,   107,   107,   107,   108,   108,
     109,   109,   109,   109,   109,   109,   110,   110,   111,   111,
     112,   112,   113,   113,   114,   114,   115,   116
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     4,     2,     4,     6,
       3,     2,     6,     6,     7,     4,     5,     9,    10,     4,
       1,     1,     1,     3,     1,     3,     2,     1,     4,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     3,     0,
       2,     1,     3,     3,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     5,     5,
       5,     5,     2,     0,     2,     5,     5,     5,     5,     5,
       5,     1,     1,     3,     1,     3,     3,     5,     6,     6,
       3,     0,     1,     3,     2,     1,     1,     0,     3,     0,
       4,     4,     4,     4,     4,     4,     3,     5,     2,     0,
       2,     0,     1,     1,     1,     2,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* start: stmt ';'  */
#line 70 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = (yyvsp[-1].sv_node);
        YYACCEPT;
    }
#line 1763 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 3: /* start: HELP  */
#line 75 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
#line 1772 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 4: /* start: EXIT  */
#line 80 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1781 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 5: /* start: T_EOF  */
#line 85 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1790 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 11: /* txnStmt: TXN_BEGIN  */
#line 101 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnBegin>();
    }
#line 1798 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 12: /* txnStmt: TXN_COMMIT  */
#line 105 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnCommit>();
    }
#line 1806 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 13: /* txnStmt: TXN_ABORT  */
#line 109 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnAbort>();
    }
#line 1814 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 14: /* txnStmt: TXN_ROLLBACK  */
#line 113 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnRollback>();
    }
#line 1822 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 15: /* dbStmt: SHOW TABLES  */
#line 120 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowTables>();
    }
#line 1830 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 16: /* dbStmt: SHOW INDEX FROM tbName  */
#line 124 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowIndex>((yyvsp[0].sv_str));
    }
#line 1838 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 17: /* dbStmt: CREATE STATIC_CHECKPOINT  */
#line 128 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateCheckpoint>();
    }
#line 1846 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 18: /* setStmt: SET set_knob_type '=' VALUE_BOOL  */
#line 135 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<SetStmt>((yyvsp[-2].sv_setKnobType), (yyvsp[0].sv_bool));
    }
#line 1854 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 19: /* ddl: CREATE TABLE tbName '(' fieldList ')'  */
#line 142 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateTable>((yyvsp[-3].sv_str), (yyvsp[-1].sv_fields));
    }
#line 1862 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 20: /* ddl: DROP TABLE tbName  */
#line 146 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropTable>((yyvsp[0].sv_str));
    }
#line 1870 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 21: /* ddl: DESC tbName  */
#line 150 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DescTable>((yyvsp[0].sv_str));
    }
#line 1878 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 22: /* ddl: CREATE INDEX tbName '(' colNameList ')'  */
#line 154 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1886 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 23: /* ddl: DROP INDEX tbName '(' colNameList ')'  */
#line 158 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1894 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 24: /* dml: INSERT INTO tbName VALUES '(' valueList ')'  */
#line 165 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<InsertStmt>((yyvsp[-4].sv_str), (yyvsp[-1].sv_vals));
    }
#line 1902 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 25: /* dml: DELETE FROM tbName optWhereClause  */
#line 169 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DeleteStmt>((yyvsp[-1].sv_str), (yyvsp[0].sv_conds));
    }
#line 1910 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 26: /* dml: UPDATE tbName SET setClauses optWhereClause  */
#line 173 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<UpdateStmt>((yyvsp[-3].sv_str), (yyvsp[-1].sv_set_clauses), (yyvsp[0].sv_conds));
    }
#line 1918 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 27: /* dml: SELECT select_list FROM tableList optWhereClause group_by_clause having_clauses opt_order_clause opt_limit_clause  */
#line 177 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        // $$ = std::static_pointer_cast<Expr>(std::make_shared<SelectStmt>($2, $4, $5, current_joins, $6, $7, $8, $9));
        (yyval.sv_node) = std::make_shared<SelectStmt>((yyvsp[-7].sv_col_extra_infos), (yyvsp[-5].sv_strs), (yyvsp[-4].sv_conds), current_joins, (yyvsp[-3].sv_cols), (yyvsp[-2].sv_havings), (yyvsp[-1].sv_orderby), (yyvsp[0].sv_limit));
        current_joins.clear(); //清空，以便下一个语句使用
    }
#line 1928 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 28: /* dml: EXPLAIN SELECT select_list FROM tableList optWhereClause group_by_clause having_clauses opt_order_clause opt_limit_clause  */
#line 183 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        auto select_stmt = std::make_shared<SelectStmt>((yyvsp[-7].sv_col_extra_infos), (yyvsp[-5].sv_strs), (yyvsp[-4].sv_conds), current_joins, (yyvsp[-3].sv_cols), (yyvsp[-2].sv_havings), (yyvsp[-1].sv_orderby), (yyvsp[0].sv_limit));
        current_joins.clear(); //清空，以便下一个语句使用
        select_stmt->explain = true;
        (yyval.sv_node) = select_stmt;
    }
#line 1939 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 29: /* dml: LOAD file_path INTO tbName  */
#line 190 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<LoadStmt>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 1947 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 30: /* file_path: VALUE_STRING  */
#line 196 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    { (yyval.sv_str) = (yyvsp[0].sv_str); }
#line 1953 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 31: /* file_path: VALUE_PATH  */
#line 198 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    { (yyval.sv_str) = (yyvsp[0].sv_str); }
#line 1959 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 32: /* fieldList: field  */
#line 202 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_fields) = std::vector<std::shared_ptr<Field>>{(yyvsp[0].sv_field)};
    }
#line 1967 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 33: /* fieldList: fieldList ',' field  */
#line 206 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_fields).push_back((yyvsp[0].sv_field));
    }
#line 1975 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 34: /* colNameList: colName  */
#line 213 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 1983 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 35: /* colNameList: colNameList ',' colName  */
#line 217 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 1991 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 36: /* field: colName type  */
#line 224 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_field) = std::make_shared<ColDef>((yyvsp[-1].sv_str), (yyvsp[0].sv_type_len));
    }
#line 1999 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 37: /* type: INT  */
#line 231 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
#line 2007 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 38: /* type: CHAR '(' VALUE_INT ')'  */
#line 235 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_STRING, (yyvsp[-1].sv_int));
    }
#line 2015 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 39: /* type: FLOAT  */
#line 239 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
#line 2023 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 40: /* type: DATE  */
#line 243 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_DATE, 19);
    }
#line 2031 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 41: /* valueList: value  */
#line 250 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_vals) = std::vector<std::shared_ptr<Value>>{(yyvsp[0].sv_val)};
    }
#line 2039 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 42: /* valueList: valueList ',' value  */
#line 254 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_vals).push_back((yyvsp[0].sv_val));
    }
#line 2047 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 43: /* value: VALUE_INT  */
#line 261 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<IntLit>((yyvsp[0].sv_int));
    }
#line 2055 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 44: /* value: VALUE_FLOAT  */
#line 265 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>((yyvsp[0].sv_float));
    }
#line 2063 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 45: /* value: VALUE_STRING  */
#line 269 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<StringLit>((yyvsp[0].sv_str));
    }
#line 2071 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 46: /* value: VALUE_BOOL  */
#line 273 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<BoolLit>((yyvsp[0].sv_bool));
    }
#line 2079 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 47: /* value: VALUE_DATE  */
#line 277 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<DateLit>((yyvsp[0].sv_date));
    }
#line 2087 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 48: /* condition: col op expr  */
#line 284 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_col), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 2095 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 49: /* optWhereClause: %empty  */
#line 290 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2101 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 50: /* optWhereClause: WHERE whereClause  */
#line 292 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_conds) = (yyvsp[0].sv_conds);
    }
#line 2109 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 51: /* whereClause: condition  */
#line 299 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 2117 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 52: /* whereClause: whereClause AND condition  */
#line 303 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 2125 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 53: /* col: tbName '.' colName  */
#line 310 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 2133 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 54: /* col: colName  */
#line 314 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[0].sv_str));
    }
#line 2141 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 55: /* colList: col  */
#line 321 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::vector<std::shared_ptr<Col>>{(yyvsp[0].sv_col)};
    }
#line 2149 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 56: /* colList: colList ',' col  */
#line 325 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cols).push_back((yyvsp[0].sv_col));
    }
#line 2157 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 57: /* op: '='  */
#line 332 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_EQ;
    }
#line 2165 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 58: /* op: '<'  */
#line 336 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LT;
    }
#line 2173 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 59: /* op: '>'  */
#line 340 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GT;
    }
#line 2181 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 60: /* op: NEQ  */
#line 344 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_NE;
    }
#line 2189 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 61: /* op: LEQ  */
#line 348 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LE;
    }
#line 2197 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 62: /* op: GEQ  */
#line 352 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GE;
    }
#line 2205 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 63: /* expr: value  */
#line 359 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_val));
    }
#line 2213 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 64: /* expr: col  */
#line 363 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_col));
    }
#line 2221 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 65: /* setClauses: setClause  */
#line 370 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses) = std::vector<std::shared_ptr<SetClause>>{(yyvsp[0].sv_set_clause)};
    }
#line 2229 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 66: /* setClauses: setClauses ',' setClause  */
#line 374 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses).push_back((yyvsp[0].sv_set_clause));
    }
#line 2237 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 67: /* setClause: colName '=' value  */
#line 381 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_val));
    }
#line 2245 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 68: /* setClause: colName '=' colName OP_PLUS value  */
#line 385 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '+', (yyvsp[0].sv_val));
    }
#line 2253 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 69: /* setClause: colName '=' colName OP_MINUS value  */
#line 389 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '-', (yyvsp[0].sv_val));
    }
#line 2261 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 70: /* setClause: colName '=' colName '*' value  */
#line 393 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '*', (yyvsp[0].sv_val));
    }
#line 2269 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 71: /* setClause: colName '=' colName OP_DIVIDE value  */
#line 397 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '/', (yyvsp[0].sv_val));
    }
#line 2277 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 72: /* asClause: AS alias  */
#line 404 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_str) = std::move((yyvsp[0].sv_str));
    }
#line 2285 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 73: /* asClause: %empty  */
#line 408 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_str) = "";
    }
#line 2293 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 74: /* select_item: col asClause  */
#line 415 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-1].sv_col)), NO_AGG, std::move((yyvsp[0].sv_str)));
    }
#line 2301 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 75: /* select_item: COUNT '(' '*' ')' asClause  */
#line 419 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::make_shared<Col>("", ""), AGG_COUNT, std::move((yyvsp[0].sv_str)));
    }
#line 2309 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 76: /* select_item: COUNT '(' col ')' asClause  */
#line 423 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_COUNT, std::move((yyvsp[0].sv_str)));
    }
#line 2317 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 77: /* select_item: MAX '(' col ')' asClause  */
#line 427 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_MAX, std::move((yyvsp[0].sv_str)));
    }
#line 2325 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 78: /* select_item: MIN '(' col ')' asClause  */
#line 431 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_MIN, std::move((yyvsp[0].sv_str)));
    }
#line 2333 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 79: /* select_item: SUM '(' col ')' asClause  */
#line 435 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_SUM, std::move((yyvsp[0].sv_str)));
    }
#line 2341 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 80: /* select_item: AVG '(' col ')' asClause  */
#line 439 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_AVG, std::move((yyvsp[0].sv_str)));
    }
#line 2349 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 81: /* select_list: '*'  */
#line 453 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_infos) = {};
    }
#line 2357 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 82: /* select_list: select_item  */
#line 457 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_infos).emplace_back(std::move((yyvsp[0].sv_col_extra_info)));
    }
#line 2365 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 83: /* select_list: select_list ',' select_item  */
#line 461 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_infos).emplace_back(std::move((yyvsp[0].sv_col_extra_info)));
    }
#line 2373 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 84: /* tableList: tbName  */
#line 468 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 2381 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 85: /* tableList: tableList ',' tbName  */
#line 472 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2389 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 86: /* tableList: tableList JOIN tbName  */
#line 476 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2397 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 87: /* tableList: tableList JOIN tbName ON condition  */
#line 480 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = (yyvsp[-4].sv_strs);
        (yyval.sv_strs).push_back((yyvsp[-2].sv_str));
        
        // 创建JOIN表达式
        std::string right_tab = (yyvsp[-2].sv_str);
        std::string left_tab = (yyvsp[-4].sv_strs)[(yyvsp[-4].sv_strs).size() - 1];
        
        // 创建JOIN条件对象
        auto join_expr = std::make_shared<JoinExpr>(
            left_tab,
            right_tab,
            std::vector<std::shared_ptr<ast::BinaryExpr>>{(yyvsp[0].sv_cond)},
            INNER_JOIN
        );
        
        // 存储JOIN表达式，将在创建SelectStmt时使用
        current_joins.push_back(join_expr);
    }
#line 2421 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 88: /* tableList: tableList SEMI JOIN tbName ON condition  */
#line 500 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = (yyvsp[-5].sv_strs);
        (yyval.sv_strs).push_back((yyvsp[-2].sv_str));
        
        //创建SEMI JOIN表达式
        std::string right_tab = (yyvsp[-2].sv_str);
        std::string left_tab = (yyvsp[-5].sv_strs)[(yyvsp[-5].sv_strs).size() - 1];
        auto join_expr = std::make_shared<JoinExpr>(
            left_tab,
            right_tab,
            std::vector<std::shared_ptr<ast::BinaryExpr>>{(yyvsp[0].sv_cond)},
            SEMI_JOIN 
        );
        current_joins.push_back(join_expr);
    }
#line 2441 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 89: /* tableList: tableList ANTI JOIN tbName ON condition  */
#line 516 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = (yyvsp[-5].sv_strs);
        (yyval.sv_strs).push_back((yyvsp[-2].sv_str));
        
        //创建ANTI JOIN表达式
        std::string right_tab = (yyvsp[-2].sv_str);
        std::string left_tab = (yyvsp[-5].sv_strs)[(yyvsp[-5].sv_strs).size() - 1];
        auto join_expr = std::make_shared<JoinExpr>(
            left_tab,
            right_tab,
            std::vector<std::shared_ptr<ast::BinaryExpr>>{(yyvsp[0].sv_cond)},
            ANTI_JOIN 
        );
        current_joins.push_back(join_expr);
    }
#line 2461 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 90: /* opt_order_clause: ORDER BY order_items  */
#line 535 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = (yyvsp[0].sv_orderby); 
    }
#line 2469 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 91: /* opt_order_clause: %empty  */
#line 538 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2475 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 92: /* order_items: order_item  */
#line 543 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_orderby) = std::make_shared<OrderBy>();
        (yyval.sv_orderby)->order_items.push_back((yyvsp[0].sv_order_item));
    }
#line 2484 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 93: /* order_items: order_items ',' order_item  */
#line 548 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyvsp[-2].sv_orderby)->order_items.push_back((yyvsp[0].sv_order_item));
        (yyval.sv_orderby) = (yyvsp[-2].sv_orderby);
    }
#line 2493 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 94: /* order_item: col opt_asc_desc  */
#line 556 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    { 
        (yyval.sv_order_item) = std::make_shared<OrderByItem>((yyvsp[-1].sv_col), (yyvsp[0].sv_orderby_dir));
    }
#line 2501 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 95: /* opt_asc_desc: ASC  */
#line 569 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_ASC;     }
#line 2507 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 96: /* opt_asc_desc: DESC  */
#line 570 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_DESC;    }
#line 2513 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 97: /* opt_asc_desc: %empty  */
#line 571 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
            { (yyval.sv_orderby_dir) = OrderBy_DEFAULT; }
#line 2519 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 98: /* group_by_clause: GROUP BY colList  */
#line 576 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::move((yyvsp[0].sv_cols));
    }
#line 2527 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 99: /* group_by_clause: %empty  */
#line 580 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        /* ignore */
    }
#line 2535 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 100: /* agg_expr: COUNT '(' '*' ')'  */
#line 587 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::make_shared<Col>("", ""), AGG_COUNT);
    }
#line 2543 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 101: /* agg_expr: COUNT '(' col ')'  */
#line 591 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_COUNT);
    }
#line 2551 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 102: /* agg_expr: MAX '(' col ')'  */
#line 595 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_MAX);
    }
#line 2559 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 103: /* agg_expr: MIN '(' col ')'  */
#line 599 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_MIN);
    }
#line 2567 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 104: /* agg_expr: SUM '(' col ')'  */
#line 603 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_SUM);
    }
#line 2575 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 105: /* agg_expr: AVG '(' col ')'  */
#line 607 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_AVG);
    }
#line 2583 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 106: /* having_clause: agg_expr op expr  */
#line 614 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_havings).emplace_back(std::make_shared<HavingExpr>((yyvsp[-2].sv_agg_expr), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr)));
    }
#line 2591 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 107: /* having_clause: having_clause AND agg_expr op expr  */
#line 618 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_havings).emplace_back(std::make_shared<HavingExpr>((yyvsp[-2].sv_agg_expr), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr)));
    }
#line 2599 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 108: /* having_clauses: HAVING having_clause  */
#line 625 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_havings) = std::move((yyvsp[0].sv_havings));
    }
#line 2607 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 109: /* having_clauses: %empty  */
#line 629 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        /* ignore */
    }
#line 2615 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 110: /* opt_limit_clause: LIMIT VALUE_INT  */
#line 636 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_limit) = std::make_shared<LimitExpr>(std::move((yyvsp[0].sv_int)));
    }
#line 2623 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 111: /* opt_limit_clause: %empty  */
#line 640 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        /* ignore */
    }
#line 2631 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 112: /* set_knob_type: ENABLE_NESTLOOP  */
#line 646 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                    { (yyval.sv_setKnobType) = EnableNestLoop; }
#line 2637 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 113: /* set_knob_type: ENABLE_SORTMERGE  */
#line 647 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                         { (yyval.sv_setKnobType) = EnableSortMerge; }
#line 2643 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 114: /* tbName: IDENTIFIER  */
#line 652 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[0].sv_str);
    }
#line 2651 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 115: /* tbName: IDENTIFIER IDENTIFIER  */
#line 656 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        //格式: "表名 别名"
        (yyval.sv_str) = (yyvsp[-1].sv_str) + " " + (yyvsp[0].sv_str);
    }
#line 2660 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;


#line 2664 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
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
                      yytoken, &yylval, &yylloc);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 665 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"

