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
  YYSYMBOL_COUNT = 40,                     /* COUNT  */
  YYSYMBOL_MAX = 41,                       /* MAX  */
  YYSYMBOL_MIN = 42,                       /* MIN  */
  YYSYMBOL_SUM = 43,                       /* SUM  */
  YYSYMBOL_AVG = 44,                       /* AVG  */
  YYSYMBOL_AS = 45,                        /* AS  */
  YYSYMBOL_GROUP = 46,                     /* GROUP  */
  YYSYMBOL_HAVING = 47,                    /* HAVING  */
  YYSYMBOL_LIMIT = 48,                     /* LIMIT  */
  YYSYMBOL_STATIC_CHECKPOINT = 49,         /* STATIC_CHECKPOINT  */
  YYSYMBOL_LEQ = 50,                       /* LEQ  */
  YYSYMBOL_NEQ = 51,                       /* NEQ  */
  YYSYMBOL_GEQ = 52,                       /* GEQ  */
  YYSYMBOL_T_EOF = 53,                     /* T_EOF  */
  YYSYMBOL_IDENTIFIER = 54,                /* IDENTIFIER  */
  YYSYMBOL_VALUE_STRING = 55,              /* VALUE_STRING  */
  YYSYMBOL_VALUE_INT = 56,                 /* VALUE_INT  */
  YYSYMBOL_VALUE_FLOAT = 57,               /* VALUE_FLOAT  */
  YYSYMBOL_VALUE_DATE = 58,                /* VALUE_DATE  */
  YYSYMBOL_VALUE_BOOL = 59,                /* VALUE_BOOL  */
  YYSYMBOL_OP_PLUS = 60,                   /* OP_PLUS  */
  YYSYMBOL_OP_MINUS = 61,                  /* OP_MINUS  */
  YYSYMBOL_OP_TIMES = 62,                  /* OP_TIMES  */
  YYSYMBOL_OP_DIVIDE = 63,                 /* OP_DIVIDE  */
  YYSYMBOL_64_ = 64,                       /* ';'  */
  YYSYMBOL_65_ = 65,                       /* '='  */
  YYSYMBOL_66_ = 66,                       /* '('  */
  YYSYMBOL_67_ = 67,                       /* ')'  */
  YYSYMBOL_68_ = 68,                       /* ','  */
  YYSYMBOL_69_ = 69,                       /* '.'  */
  YYSYMBOL_70_ = 70,                       /* '<'  */
  YYSYMBOL_71_ = 71,                       /* '>'  */
  YYSYMBOL_72_ = 72,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 73,                  /* $accept  */
  YYSYMBOL_start = 74,                     /* start  */
  YYSYMBOL_stmt = 75,                      /* stmt  */
  YYSYMBOL_txnStmt = 76,                   /* txnStmt  */
  YYSYMBOL_dbStmt = 77,                    /* dbStmt  */
  YYSYMBOL_setStmt = 78,                   /* setStmt  */
  YYSYMBOL_ddl = 79,                       /* ddl  */
  YYSYMBOL_dml = 80,                       /* dml  */
  YYSYMBOL_fieldList = 81,                 /* fieldList  */
  YYSYMBOL_colNameList = 82,               /* colNameList  */
  YYSYMBOL_field = 83,                     /* field  */
  YYSYMBOL_type = 84,                      /* type  */
  YYSYMBOL_valueList = 85,                 /* valueList  */
  YYSYMBOL_value = 86,                     /* value  */
  YYSYMBOL_condition = 87,                 /* condition  */
  YYSYMBOL_optWhereClause = 88,            /* optWhereClause  */
  YYSYMBOL_whereClause = 89,               /* whereClause  */
  YYSYMBOL_col = 90,                       /* col  */
  YYSYMBOL_colList = 91,                   /* colList  */
  YYSYMBOL_op = 92,                        /* op  */
  YYSYMBOL_expr = 93,                      /* expr  */
  YYSYMBOL_setClauses = 94,                /* setClauses  */
  YYSYMBOL_setClause = 95,                 /* setClause  */
  YYSYMBOL_asClause = 96,                  /* asClause  */
  YYSYMBOL_select_item = 97,               /* select_item  */
  YYSYMBOL_select_list = 98,               /* select_list  */
  YYSYMBOL_tableList = 99,                 /* tableList  */
  YYSYMBOL_opt_order_clause = 100,         /* opt_order_clause  */
  YYSYMBOL_order_clause = 101,             /* order_clause  */
  YYSYMBOL_opt_asc_desc = 102,             /* opt_asc_desc  */
  YYSYMBOL_group_by_clause = 103,          /* group_by_clause  */
  YYSYMBOL_agg_expr = 104,                 /* agg_expr  */
  YYSYMBOL_having_clause = 105,            /* having_clause  */
  YYSYMBOL_having_clauses = 106,           /* having_clauses  */
  YYSYMBOL_opt_limit_clause = 107,         /* opt_limit_clause  */
  YYSYMBOL_set_knob_type = 108,            /* set_knob_type  */
  YYSYMBOL_tbName = 109,                   /* tbName  */
  YYSYMBOL_colName = 110,                  /* colName  */
  YYSYMBOL_alias = 111                     /* alias  */
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
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   241

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  111
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  242

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   318


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
      66,    67,    72,     2,    68,     2,    69,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    64,
      70,    65,    71,     2,     2,     2,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    66,    66,    71,    76,    81,    89,    90,    91,    92,
      93,    97,   101,   105,   109,   116,   120,   124,   131,   138,
     142,   146,   150,   154,   161,   165,   169,   173,   179,   189,
     193,   200,   204,   211,   218,   222,   226,   230,   237,   241,
     248,   252,   256,   260,   264,   271,   278,   279,   286,   290,
     297,   301,   308,   312,   319,   323,   327,   331,   335,   339,
     346,   350,   357,   361,   368,   372,   376,   380,   384,   391,
     396,   402,   406,   410,   414,   418,   422,   426,   440,   444,
     448,   455,   459,   463,   467,   487,   506,   510,   514,   521,
     522,   523,   527,   532,   538,   542,   546,   550,   554,   558,
     565,   569,   576,   581,   587,   592,   598,   599,   603,   607,
     614,   616
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
  "ENABLE_NESTLOOP", "ENABLE_SORTMERGE", "EXPLAIN", "SEMI", "COUNT", "MAX",
  "MIN", "SUM", "AVG", "AS", "GROUP", "HAVING", "LIMIT",
  "STATIC_CHECKPOINT", "LEQ", "NEQ", "GEQ", "T_EOF", "IDENTIFIER",
  "VALUE_STRING", "VALUE_INT", "VALUE_FLOAT", "VALUE_DATE", "VALUE_BOOL",
  "OP_PLUS", "OP_MINUS", "OP_TIMES", "OP_DIVIDE", "';'", "'='", "'('",
  "')'", "','", "'.'", "'<'", "'>'", "'*'", "$accept", "start", "stmt",
  "txnStmt", "dbStmt", "setStmt", "ddl", "dml", "fieldList", "colNameList",
  "field", "type", "valueList", "value", "condition", "optWhereClause",
  "whereClause", "col", "colList", "op", "expr", "setClauses", "setClause",
  "asClause", "select_item", "select_list", "tableList",
  "opt_order_clause", "order_clause", "opt_asc_desc", "group_by_clause",
  "agg_expr", "having_clause", "having_clauses", "opt_limit_clause",
  "set_knob_type", "tbName", "colName", "alias", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-202)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-109)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      80,    19,    -4,    24,   -41,    16,    29,   -41,    38,    62,
    -202,  -202,  -202,  -202,  -202,  -202,    18,  -202,    46,   -24,
    -202,  -202,  -202,  -202,  -202,  -202,    42,   -41,   -41,  -202,
     -41,   -41,    11,  -202,   -41,   -41,    51,  -202,  -202,     7,
      30,    35,    49,    54,    69,   -19,  -202,    92,  -202,    -7,
      70,  -202,    62,  -202,  -202,   -41,   103,   104,  -202,   111,
    -202,   167,    63,   125,   121,   -25,   127,   127,   127,   127,
     128,  -202,   -41,    84,   125,    -6,  -202,   125,   125,   125,
     117,   127,  -202,  -202,    -1,  -202,   119,  -202,   118,   120,
     122,   123,   124,   126,  -202,  -202,    -5,  -202,  -202,  -202,
     -41,    23,  -202,   108,    27,  -202,    40,   109,  -202,   160,
      71,   125,  -202,    89,    92,    92,    92,    92,    92,    92,
     -41,   161,   -41,   146,    -5,  -202,   125,  -202,   129,  -202,
    -202,  -202,  -202,   125,  -202,  -202,  -202,  -202,  -202,  -202,
      88,  -202,   127,  -202,  -202,  -202,  -202,  -202,  -202,    95,
    -202,  -202,    21,  -202,  -202,  -202,  -202,  -202,  -202,   166,
     -41,  -202,   180,   157,   146,  -202,   150,  -202,  -202,   109,
    -202,  -202,  -202,  -202,   109,   109,   109,   109,   127,   179,
     127,   132,   193,   157,   142,  -202,  -202,  -202,  -202,  -202,
    -202,   127,  -202,   143,   144,   147,   148,   149,   151,    71,
     186,   200,   170,   193,  -202,  -202,   127,    14,   127,   127,
     127,   127,    95,   132,   127,   163,  -202,   170,  -202,   153,
     154,   155,   156,   158,   159,  -202,    71,    25,  -202,  -202,
    -202,  -202,  -202,  -202,  -202,  -202,  -202,    95,  -202,  -202,
    -202,  -202
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     3,    11,    12,    13,    14,     0,     5,     0,     0,
       9,     6,    10,     7,     8,    15,     0,     0,     0,    17,
       0,     0,   108,    21,     0,     0,     0,   106,   107,     0,
       0,     0,     0,     0,     0,   110,    78,    70,    79,     0,
       0,    51,     0,     1,     2,     0,     0,     0,    20,     0,
     109,     0,    46,     0,     0,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,    16,     0,     0,     0,
       0,     0,    25,   110,    46,    62,     0,    18,     0,     0,
       0,     0,     0,     0,   111,    69,    46,    81,    80,    50,
       0,     0,    29,     0,     0,    31,     0,     0,    48,    47,
       0,     0,    26,     0,    70,    70,    70,    70,    70,    70,
       0,     0,     0,    93,    46,    19,     0,    34,     0,    36,
      37,    33,    22,     0,    23,    42,    40,    41,    44,    43,
       0,    38,     0,    58,    57,    59,    54,    55,    56,     0,
      63,    64,     0,    72,    73,    74,    75,    76,    77,    83,
       0,    82,     0,   103,    93,    30,     0,    32,    24,     0,
      49,    60,    61,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    87,   103,     0,    39,    65,    66,    68,    67,
      84,     0,    52,    92,     0,     0,     0,     0,     0,     0,
     102,     0,   105,    87,    35,    85,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,   105,    53,     0,
       0,     0,     0,     0,     0,   100,     0,    91,    86,   104,
      28,    94,    95,    96,    97,    98,    99,     0,    90,    89,
      88,   101
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -202,  -202,  -202,  -202,  -202,  -202,  -202,  -202,  -202,   145,
     101,  -202,  -202,   -98,  -137,   -76,  -202,    -9,  -202,  -189,
    -201,  -202,   130,    44,   162,   177,   131,    31,  -202,  -202,
      66,    20,  -202,    53,    15,  -202,    -3,   -60,  -202
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    18,    19,    20,    21,    22,    23,    24,   101,   104,
     102,   131,   140,   171,   108,    82,   109,   110,   193,   149,
     173,    84,    85,    71,    48,    49,    96,   202,   228,   240,
     163,   199,   200,   182,   216,    39,    50,    51,    95
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      47,    33,    27,    86,    36,   170,    72,   100,   112,   141,
     212,   225,    81,    32,    99,   151,    81,   103,   105,   105,
     123,    28,   120,    25,    56,    57,    34,    58,    59,    45,
      30,    61,    62,   238,   121,    60,   241,   237,    52,   239,
      54,   190,    35,    47,    26,    29,    53,    88,   164,    31,
    -108,    86,    76,   152,   205,    55,    89,    90,    91,    92,
      93,    73,    73,   122,    47,    60,   103,   111,    45,    97,
      63,   185,    64,   167,    37,    38,   186,   187,   188,   189,
      81,   174,   175,     1,   176,     2,   219,     3,     4,     5,
     125,   126,     6,   177,   132,   133,    65,    97,     7,     8,
       9,    66,    40,    41,    42,    43,    44,   134,   133,    10,
      11,    12,    13,    14,    15,    67,    45,   159,    16,   161,
      68,   143,   144,   145,    40,    41,    42,    43,    44,   127,
     128,   129,   130,    17,    46,    69,   146,    70,    45,    74,
     172,   147,   148,    83,   135,   136,   137,   138,   139,    45,
     135,   136,   137,   138,   139,   168,   169,   179,   153,   154,
     155,   156,   157,   158,   135,   136,   137,   138,   139,    77,
      78,   192,   194,   195,   196,   197,   198,    79,    80,    83,
      87,    45,    94,   107,   113,   114,   142,   115,   160,   116,
     117,   118,   162,   119,   178,   166,   180,   218,   220,   221,
     222,   223,   224,   172,   181,   227,   184,   191,   201,   204,
     207,   206,   213,   208,   209,   210,   214,   211,   215,   229,
     231,   232,   233,   234,   106,   235,   236,   165,   172,    75,
     183,   124,   230,   226,   217,    98,   203,     0,     0,     0,
       0,   150
};

static const yytype_int16 yycheck[] =
{
       9,     4,     6,    63,     7,   142,    13,    13,    84,   107,
     199,   212,    17,    54,    74,   113,    17,    77,    78,    79,
      96,    25,    27,     4,    27,    28,    10,    30,    31,    54,
       6,    34,    35,     8,    39,    54,   237,   226,    20,    14,
      64,   178,    13,    52,    25,    49,     0,    72,   124,    25,
      69,   111,    55,   113,   191,    13,    65,    66,    67,    68,
      69,    68,    68,    68,    73,    54,   126,    68,    54,    72,
      19,   169,    65,   133,    36,    37,   174,   175,   176,   177,
      17,    60,    61,     3,    63,     5,    72,     7,     8,     9,
      67,    68,    12,    72,    67,    68,    66,   100,    18,    19,
      20,    66,    40,    41,    42,    43,    44,    67,    68,    29,
      30,    31,    32,    33,    34,    66,    54,   120,    38,   122,
      66,    50,    51,    52,    40,    41,    42,    43,    44,    21,
      22,    23,    24,    53,    72,    66,    65,    45,    54,    69,
     149,    70,    71,    54,    55,    56,    57,    58,    59,    54,
      55,    56,    57,    58,    59,    67,    68,   160,   114,   115,
     116,   117,   118,   119,    55,    56,    57,    58,    59,    66,
      66,   180,    40,    41,    42,    43,    44,    66,    11,    54,
      59,    54,    54,    66,    65,    67,    26,    67,    27,    67,
      67,    67,    46,    67,    28,    66,    16,   206,   207,   208,
     209,   210,   211,   212,    47,   214,    56,    28,    15,    67,
      66,    68,    26,    66,    66,    66,    16,    66,    48,    56,
      67,    67,    67,    67,    79,    67,    67,   126,   237,    52,
     164,   100,   217,   213,   203,    73,   183,    -1,    -1,    -1,
      -1,   111
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     7,     8,     9,    12,    18,    19,    20,
      29,    30,    31,    32,    33,    34,    38,    53,    74,    75,
      76,    77,    78,    79,    80,     4,    25,     6,    25,    49,
       6,    25,    54,   109,    10,    13,   109,    36,    37,   108,
      40,    41,    42,    43,    44,    54,    72,    90,    97,    98,
     109,   110,    20,     0,    64,    13,   109,   109,   109,   109,
      54,   109,   109,    19,    65,    66,    66,    66,    66,    66,
      45,    96,    13,    68,    69,    98,   109,    66,    66,    66,
      11,    17,    88,    54,    94,    95,   110,    59,    72,    90,
      90,    90,    90,    90,    54,   111,    99,   109,    97,   110,
      13,    81,    83,   110,    82,   110,    82,    66,    87,    89,
      90,    68,    88,    65,    67,    67,    67,    67,    67,    67,
      27,    39,    68,    88,    99,    67,    68,    21,    22,    23,
      24,    84,    67,    68,    67,    55,    56,    57,    58,    59,
      85,    86,    26,    50,    51,    52,    65,    70,    71,    92,
      95,    86,   110,    96,    96,    96,    96,    96,    96,   109,
      27,   109,    46,   103,    88,    83,    66,   110,    67,    68,
      87,    86,    90,    93,    60,    61,    63,    72,    28,   109,
      16,    47,   106,   103,    56,    86,    86,    86,    86,    86,
      87,    28,    90,    91,    40,    41,    42,    43,    44,   104,
     105,    15,   100,   106,    67,    87,    68,    66,    66,    66,
      66,    66,    92,    26,    16,    48,   107,   100,    90,    72,
      90,    90,    90,    90,    90,    93,   104,    90,   101,    56,
     107,    67,    67,    67,    67,    67,    67,    92,     8,    14,
     102,    93
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    73,    74,    74,    74,    74,    75,    75,    75,    75,
      75,    76,    76,    76,    76,    77,    77,    77,    78,    79,
      79,    79,    79,    79,    80,    80,    80,    80,    80,    81,
      81,    82,    82,    83,    84,    84,    84,    84,    85,    85,
      86,    86,    86,    86,    86,    87,    88,    88,    89,    89,
      90,    90,    91,    91,    92,    92,    92,    92,    92,    92,
      93,    93,    94,    94,    95,    95,    95,    95,    95,    96,
      96,    97,    97,    97,    97,    97,    97,    97,    98,    98,
      98,    99,    99,    99,    99,    99,   100,   100,   101,   102,
     102,   102,   103,   103,   104,   104,   104,   104,   104,   104,
     105,   105,   106,   106,   107,   107,   108,   108,   109,   109,
     110,   111
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     4,     2,     4,     6,
       3,     2,     6,     6,     7,     4,     5,     9,    10,     1,
       3,     1,     3,     2,     1,     4,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     3,     0,     2,     1,     3,
       3,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     5,     5,     5,     5,     2,
       0,     2,     5,     5,     5,     5,     5,     5,     1,     1,
       3,     1,     3,     3,     5,     6,     3,     0,     2,     1,
       1,     0,     3,     0,     4,     4,     4,     4,     4,     4,
       3,     5,     2,     0,     2,     0,     1,     1,     1,     2,
       1,     1
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
#line 67 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = (yyvsp[-1].sv_node);
        YYACCEPT;
    }
#line 1749 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 3: /* start: HELP  */
#line 72 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
#line 1758 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 4: /* start: EXIT  */
#line 77 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1767 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 5: /* start: T_EOF  */
#line 82 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1776 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 11: /* txnStmt: TXN_BEGIN  */
#line 98 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnBegin>();
    }
#line 1784 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 12: /* txnStmt: TXN_COMMIT  */
#line 102 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnCommit>();
    }
#line 1792 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 13: /* txnStmt: TXN_ABORT  */
#line 106 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnAbort>();
    }
#line 1800 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 14: /* txnStmt: TXN_ROLLBACK  */
#line 110 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnRollback>();
    }
#line 1808 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 15: /* dbStmt: SHOW TABLES  */
#line 117 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowTables>();
    }
#line 1816 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 16: /* dbStmt: SHOW INDEX FROM tbName  */
#line 121 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowIndex>((yyvsp[0].sv_str));
    }
#line 1824 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 17: /* dbStmt: CREATE STATIC_CHECKPOINT  */
#line 125 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateCheckpoint>();
    }
#line 1832 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 18: /* setStmt: SET set_knob_type '=' VALUE_BOOL  */
#line 132 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<SetStmt>((yyvsp[-2].sv_setKnobType), (yyvsp[0].sv_bool));
    }
#line 1840 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 19: /* ddl: CREATE TABLE tbName '(' fieldList ')'  */
#line 139 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateTable>((yyvsp[-3].sv_str), (yyvsp[-1].sv_fields));
    }
#line 1848 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 20: /* ddl: DROP TABLE tbName  */
#line 143 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropTable>((yyvsp[0].sv_str));
    }
#line 1856 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 21: /* ddl: DESC tbName  */
#line 147 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DescTable>((yyvsp[0].sv_str));
    }
#line 1864 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 22: /* ddl: CREATE INDEX tbName '(' colNameList ')'  */
#line 151 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1872 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 23: /* ddl: DROP INDEX tbName '(' colNameList ')'  */
#line 155 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1880 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 24: /* dml: INSERT INTO tbName VALUES '(' valueList ')'  */
#line 162 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<InsertStmt>((yyvsp[-4].sv_str), (yyvsp[-1].sv_vals));
    }
#line 1888 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 25: /* dml: DELETE FROM tbName optWhereClause  */
#line 166 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DeleteStmt>((yyvsp[-1].sv_str), (yyvsp[0].sv_conds));
    }
#line 1896 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 26: /* dml: UPDATE tbName SET setClauses optWhereClause  */
#line 170 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<UpdateStmt>((yyvsp[-3].sv_str), (yyvsp[-1].sv_set_clauses), (yyvsp[0].sv_conds));
    }
#line 1904 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 27: /* dml: SELECT select_list FROM tableList optWhereClause group_by_clause having_clauses opt_order_clause opt_limit_clause  */
#line 174 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        // $$ = std::static_pointer_cast<Expr>(std::make_shared<SelectStmt>($2, $4, $5, current_joins, $6, $7, $8, $9));
        (yyval.sv_node) = std::make_shared<SelectStmt>((yyvsp[-7].sv_col_extra_infos), (yyvsp[-5].sv_strs), (yyvsp[-4].sv_conds), current_joins, (yyvsp[-3].sv_cols), (yyvsp[-2].sv_havings), (yyvsp[-1].sv_orderby), (yyvsp[0].sv_limit));
        current_joins.clear(); //清空，以便下一个语句使用
    }
#line 1914 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 28: /* dml: EXPLAIN SELECT select_list FROM tableList optWhereClause group_by_clause having_clauses opt_order_clause opt_limit_clause  */
#line 180 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        auto select_stmt = std::make_shared<SelectStmt>((yyvsp[-7].sv_col_extra_infos), (yyvsp[-5].sv_strs), (yyvsp[-4].sv_conds), current_joins, (yyvsp[-3].sv_cols), (yyvsp[-2].sv_havings), (yyvsp[-1].sv_orderby), (yyvsp[0].sv_limit));
        current_joins.clear(); //清空，以便下一个语句使用
        select_stmt->explain = true;
        (yyval.sv_node) = select_stmt;
    }
#line 1925 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 29: /* fieldList: field  */
#line 190 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_fields) = std::vector<std::shared_ptr<Field>>{(yyvsp[0].sv_field)};
    }
#line 1933 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 30: /* fieldList: fieldList ',' field  */
#line 194 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_fields).push_back((yyvsp[0].sv_field));
    }
#line 1941 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 31: /* colNameList: colName  */
#line 201 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 1949 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 32: /* colNameList: colNameList ',' colName  */
#line 205 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 1957 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 33: /* field: colName type  */
#line 212 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_field) = std::make_shared<ColDef>((yyvsp[-1].sv_str), (yyvsp[0].sv_type_len));
    }
#line 1965 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 34: /* type: INT  */
#line 219 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
#line 1973 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 35: /* type: CHAR '(' VALUE_INT ')'  */
#line 223 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_STRING, (yyvsp[-1].sv_int));
    }
#line 1981 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 36: /* type: FLOAT  */
#line 227 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
#line 1989 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 37: /* type: DATE  */
#line 231 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_DATE, 19);
    }
#line 1997 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 38: /* valueList: value  */
#line 238 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_vals) = std::vector<std::shared_ptr<Value>>{(yyvsp[0].sv_val)};
    }
#line 2005 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 39: /* valueList: valueList ',' value  */
#line 242 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_vals).push_back((yyvsp[0].sv_val));
    }
#line 2013 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 40: /* value: VALUE_INT  */
#line 249 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<IntLit>((yyvsp[0].sv_int));
    }
#line 2021 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 41: /* value: VALUE_FLOAT  */
#line 253 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>((yyvsp[0].sv_float));
    }
#line 2029 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 42: /* value: VALUE_STRING  */
#line 257 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<StringLit>((yyvsp[0].sv_str));
    }
#line 2037 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 43: /* value: VALUE_BOOL  */
#line 261 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<BoolLit>((yyvsp[0].sv_bool));
    }
#line 2045 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 44: /* value: VALUE_DATE  */
#line 265 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<DateLit>((yyvsp[0].sv_date));
    }
#line 2053 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 45: /* condition: col op expr  */
#line 272 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_col), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 2061 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 46: /* optWhereClause: %empty  */
#line 278 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2067 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 47: /* optWhereClause: WHERE whereClause  */
#line 280 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_conds) = (yyvsp[0].sv_conds);
    }
#line 2075 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 48: /* whereClause: condition  */
#line 287 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 2083 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 49: /* whereClause: whereClause AND condition  */
#line 291 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 2091 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 50: /* col: tbName '.' colName  */
#line 298 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 2099 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 51: /* col: colName  */
#line 302 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[0].sv_str));
    }
#line 2107 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 52: /* colList: col  */
#line 309 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::vector<std::shared_ptr<Col>>{(yyvsp[0].sv_col)};
    }
#line 2115 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 53: /* colList: colList ',' col  */
#line 313 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cols).push_back((yyvsp[0].sv_col));
    }
#line 2123 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 54: /* op: '='  */
#line 320 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_EQ;
    }
#line 2131 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 55: /* op: '<'  */
#line 324 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LT;
    }
#line 2139 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 56: /* op: '>'  */
#line 328 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GT;
    }
#line 2147 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 57: /* op: NEQ  */
#line 332 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_NE;
    }
#line 2155 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 58: /* op: LEQ  */
#line 336 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LE;
    }
#line 2163 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 59: /* op: GEQ  */
#line 340 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GE;
    }
#line 2171 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 60: /* expr: value  */
#line 347 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_val));
    }
#line 2179 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 61: /* expr: col  */
#line 351 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_col));
    }
#line 2187 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 62: /* setClauses: setClause  */
#line 358 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses) = std::vector<std::shared_ptr<SetClause>>{(yyvsp[0].sv_set_clause)};
    }
#line 2195 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 63: /* setClauses: setClauses ',' setClause  */
#line 362 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses).push_back((yyvsp[0].sv_set_clause));
    }
#line 2203 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 64: /* setClause: colName '=' value  */
#line 369 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_val));
    }
#line 2211 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 65: /* setClause: colName '=' colName OP_PLUS value  */
#line 373 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '+', (yyvsp[0].sv_val));
    }
#line 2219 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 66: /* setClause: colName '=' colName OP_MINUS value  */
#line 377 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '-', (yyvsp[0].sv_val));
    }
#line 2227 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 67: /* setClause: colName '=' colName '*' value  */
#line 381 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '*', (yyvsp[0].sv_val));
    }
#line 2235 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 68: /* setClause: colName '=' colName OP_DIVIDE value  */
#line 385 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), '/', (yyvsp[0].sv_val));
    }
#line 2243 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 69: /* asClause: AS alias  */
#line 392 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_str) = std::move((yyvsp[0].sv_str));
    }
#line 2251 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 70: /* asClause: %empty  */
#line 396 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_str) = "";
    }
#line 2259 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 71: /* select_item: col asClause  */
#line 403 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-1].sv_col)), NO_AGG, std::move((yyvsp[0].sv_str)));
    }
#line 2267 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 72: /* select_item: COUNT '(' '*' ')' asClause  */
#line 407 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::make_shared<Col>("", ""), AGG_COUNT, std::move((yyvsp[0].sv_str)));
    }
#line 2275 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 73: /* select_item: COUNT '(' col ')' asClause  */
#line 411 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_COUNT, std::move((yyvsp[0].sv_str)));
    }
#line 2283 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 74: /* select_item: MAX '(' col ')' asClause  */
#line 415 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_MAX, std::move((yyvsp[0].sv_str)));
    }
#line 2291 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 75: /* select_item: MIN '(' col ')' asClause  */
#line 419 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_MIN, std::move((yyvsp[0].sv_str)));
    }
#line 2299 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 76: /* select_item: SUM '(' col ')' asClause  */
#line 423 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_SUM, std::move((yyvsp[0].sv_str)));
    }
#line 2307 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 77: /* select_item: AVG '(' col ')' asClause  */
#line 427 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_info) = std::make_shared<ColExtraInfo>(std::move((yyvsp[-2].sv_col)), AGG_AVG, std::move((yyvsp[0].sv_str)));
    }
#line 2315 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 78: /* select_list: '*'  */
#line 441 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_infos) = {};
    }
#line 2323 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 79: /* select_list: select_item  */
#line 445 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_infos).emplace_back(std::move((yyvsp[0].sv_col_extra_info)));
    }
#line 2331 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 80: /* select_list: select_list ',' select_item  */
#line 449 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_col_extra_infos).emplace_back(std::move((yyvsp[0].sv_col_extra_info)));
    }
#line 2339 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 81: /* tableList: tbName  */
#line 456 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 2347 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 82: /* tableList: tableList ',' tbName  */
#line 460 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2355 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 83: /* tableList: tableList JOIN tbName  */
#line 464 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2363 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 84: /* tableList: tableList JOIN tbName ON condition  */
#line 468 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
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
#line 2387 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 85: /* tableList: tableList SEMI JOIN tbName ON condition  */
#line 488 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
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
#line 2407 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 86: /* opt_order_clause: ORDER BY order_clause  */
#line 507 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = (yyvsp[0].sv_orderby); 
    }
#line 2415 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 87: /* opt_order_clause: %empty  */
#line 510 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2421 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 88: /* order_clause: col opt_asc_desc  */
#line 515 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = std::make_shared<OrderBy>((yyvsp[-1].sv_col), (yyvsp[0].sv_orderby_dir));
    }
#line 2429 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 89: /* opt_asc_desc: ASC  */
#line 521 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_ASC;     }
#line 2435 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 90: /* opt_asc_desc: DESC  */
#line 522 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_DESC;    }
#line 2441 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 91: /* opt_asc_desc: %empty  */
#line 523 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
            { (yyval.sv_orderby_dir) = OrderBy_DEFAULT; }
#line 2447 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 92: /* group_by_clause: GROUP BY colList  */
#line 528 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::move((yyvsp[0].sv_cols));
    }
#line 2455 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 93: /* group_by_clause: %empty  */
#line 532 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        /* ignore */
    }
#line 2463 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 94: /* agg_expr: COUNT '(' '*' ')'  */
#line 539 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::make_shared<Col>("", ""), AGG_COUNT);
    }
#line 2471 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 95: /* agg_expr: COUNT '(' col ')'  */
#line 543 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_COUNT);
    }
#line 2479 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 96: /* agg_expr: MAX '(' col ')'  */
#line 547 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_MAX);
    }
#line 2487 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 97: /* agg_expr: MIN '(' col ')'  */
#line 551 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_MIN);
    }
#line 2495 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 98: /* agg_expr: SUM '(' col ')'  */
#line 555 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_SUM);
    }
#line 2503 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 99: /* agg_expr: AVG '(' col ')'  */
#line 559 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_agg_expr) = std::make_shared<AggExpr>(std::move((yyvsp[-1].sv_col)), AGG_AVG);
    }
#line 2511 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 100: /* having_clause: agg_expr op expr  */
#line 566 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_havings).emplace_back(std::make_shared<HavingExpr>((yyvsp[-2].sv_agg_expr), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr)));
    }
#line 2519 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 101: /* having_clause: having_clause AND agg_expr op expr  */
#line 570 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_havings).emplace_back(std::make_shared<HavingExpr>((yyvsp[-2].sv_agg_expr), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr)));
    }
#line 2527 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 102: /* having_clauses: HAVING having_clause  */
#line 577 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_havings) = std::move((yyvsp[0].sv_havings));
    }
#line 2535 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 103: /* having_clauses: %empty  */
#line 581 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        /* ignore */
    }
#line 2543 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 104: /* opt_limit_clause: LIMIT VALUE_INT  */
#line 588 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_limit) = std::make_shared<LimitExpr>(std::move((yyvsp[0].sv_int)));
    }
#line 2551 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 105: /* opt_limit_clause: %empty  */
#line 592 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        /* ignore */
    }
#line 2559 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 106: /* set_knob_type: ENABLE_NESTLOOP  */
#line 598 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                    { (yyval.sv_setKnobType) = EnableNestLoop; }
#line 2565 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 107: /* set_knob_type: ENABLE_SORTMERGE  */
#line 599 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
                         { (yyval.sv_setKnobType) = EnableSortMerge; }
#line 2571 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 108: /* tbName: IDENTIFIER  */
#line 604 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        (yyval.sv_str) = (yyvsp[0].sv_str);
    }
#line 2579 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;

  case 109: /* tbName: IDENTIFIER IDENTIFIER  */
#line 608 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"
    {
        //格式: "表名 别名"
        (yyval.sv_str) = (yyvsp[-1].sv_str) + " " + (yyvsp[0].sv_str);
    }
#line 2588 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"
    break;


#line 2592 "/home/wangyiming/db2025-yoursql/src/parser/yacc.tab.cpp"

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

#line 617 "/home/wangyiming/db2025-yoursql/src/parser/yacc.y"

