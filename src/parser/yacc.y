%{
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
%}

// request a pure (reentrant) parser
%define api.pure full
// enable location in error handler
%locations
// enable verbose syntax error message
%define parse.error verbose

// keywords
%token SHOW TABLES CREATE TABLE DROP DESC INSERT INTO VALUES DELETE FROM ASC ORDER BY
WHERE UPDATE SET SELECT INT CHAR FLOAT DATE INDEX AND JOIN ON EXIT HELP TXN_BEGIN TXN_COMMIT 
TXN_ABORT TXN_ROLLBACK ORDER_BY ENABLE_NESTLOOP ENABLE_SORTMERGE EXPLAIN SEMI
COUNT MAX MIN SUM AVG AS GROUP HAVING LIMIT STATIC_CHECKPOINT
// non-keywords
%token LEQ NEQ GEQ T_EOF

// type-specific tokens
%token <sv_str> IDENTIFIER VALUE_STRING
%token <sv_int> VALUE_INT
%token <sv_float> VALUE_FLOAT
%token <sv_date> VALUE_DATE
%token <sv_bool> VALUE_BOOL
%token <sv_char> OP_PLUS OP_MINUS OP_TIMES OP_DIVIDE

// specify types for non-terminal symbol
%type <sv_node> stmt dbStmt ddl dml txnStmt setStmt
%type <sv_field> field
%type <sv_fields> fieldList
%type <sv_type_len> type
%type <sv_comp_op> op
%type <sv_expr> expr
%type <sv_val> value
%type <sv_vals> valueList
%type <sv_str> tbName colName alias asClause
%type <sv_strs> tableList colNameList
%type <sv_col> col
%type <sv_cols> colList selector group_by_clause
%type <sv_col_extra_infos> select_list
%type <sv_havings> having_clause having_clauses
%type <sv_set_clause> setClause
%type <sv_set_clauses> setClauses
%type <sv_cond> condition
%type <sv_conds> whereClause optWhereClause
%type <sv_orderby>  order_clause opt_order_clause
%type <sv_orderby_dir> opt_asc_desc
%type <sv_setKnobType> set_knob_type
%type <sv_col_extra_info> select_item
%type <sv_agg_expr> agg_expr
%type <sv_limit> opt_limit_clause
%%
start:
        stmt ';'
    {
        parse_tree = $1;
        YYACCEPT;
    }
    |   HELP
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
    |   EXIT
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
    |   T_EOF
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
    ;

stmt:
        dbStmt
    |   ddl
    |   dml
    |   txnStmt
    |   setStmt
    ;

txnStmt:
        TXN_BEGIN
    {
        $$ = std::make_shared<TxnBegin>();
    }
    |   TXN_COMMIT
    {
        $$ = std::make_shared<TxnCommit>();
    }
    |   TXN_ABORT
    {
        $$ = std::make_shared<TxnAbort>();
    }
    | TXN_ROLLBACK
    {
        $$ = std::make_shared<TxnRollback>();
    }
    ;

dbStmt:
        SHOW TABLES
    {
        $$ = std::make_shared<ShowTables>();
    }
    |   SHOW INDEX FROM tbName
    {
        $$ = std::make_shared<ShowIndex>($4);
    }
    |   CREATE STATIC_CHECKPOINT
    {
        $$ = std::make_shared<CreateCheckpoint>();
    }
    ;

setStmt:
        SET set_knob_type '=' VALUE_BOOL
    {
        $$ = std::make_shared<SetStmt>($2, $4);
    }
    ;

ddl:
        CREATE TABLE tbName '(' fieldList ')'
    {
        $$ = std::make_shared<CreateTable>($3, $5);
    }
    |   DROP TABLE tbName
    {
        $$ = std::make_shared<DropTable>($3);
    }
    |   DESC tbName
    {
        $$ = std::make_shared<DescTable>($2);
    }
    |   CREATE INDEX tbName '(' colNameList ')'
    {
        $$ = std::make_shared<CreateIndex>($3, $5);
    }
    |   DROP INDEX tbName '(' colNameList ')'
    {
        $$ = std::make_shared<DropIndex>($3, $5);
    }
    ;

dml:
        INSERT INTO tbName VALUES '(' valueList ')'
    {
        $$ = std::make_shared<InsertStmt>($3, $6);
    }
    |   DELETE FROM tbName optWhereClause
    {
        $$ = std::make_shared<DeleteStmt>($3, $4);
    }
    |   UPDATE tbName SET setClauses optWhereClause
    {
        $$ = std::make_shared<UpdateStmt>($2, $4, $5);
    }
    |   SELECT select_list FROM tableList optWhereClause group_by_clause having_clauses opt_order_clause opt_limit_clause
    {
        // $$ = std::static_pointer_cast<Expr>(std::make_shared<SelectStmt>($2, $4, $5, current_joins, $6, $7, $8, $9));
        $$ = std::make_shared<SelectStmt>($2, $4, $5, current_joins, $6, $7, $8, $9);
        current_joins.clear(); //清空，以便下一个语句使用
    }
    |   EXPLAIN SELECT select_list FROM tableList optWhereClause group_by_clause having_clauses opt_order_clause opt_limit_clause // TODO selector
    {
        auto select_stmt = std::make_shared<SelectStmt>($3, $5, $6, current_joins, $7, $8, $9, $10);
        current_joins.clear(); //清空，以便下一个语句使用
        select_stmt->explain = true;
        $$ = select_stmt;
    }
    ;

fieldList:
        field
    {
        $$ = std::vector<std::shared_ptr<Field>>{$1};
    }
    |   fieldList ',' field
    {
        $$.push_back($3);
    }
    ;

colNameList:
        colName
    {
        $$ = std::vector<std::string>{$1};
    }
    | colNameList ',' colName
    {
        $$.push_back($3);
    }
    ;

field:
        colName type
    {
        $$ = std::make_shared<ColDef>($1, $2);
    }
    ;

type:
        INT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
    |   CHAR '(' VALUE_INT ')'
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_STRING, $3);
    }
    |   FLOAT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
    |   DATE
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_DATE, 19);
    }
    ;

valueList:
        value
    {
        $$ = std::vector<std::shared_ptr<Value>>{$1};
    }
    |   valueList ',' value
    {
        $$.push_back($3);
    }
    ;

value:
        VALUE_INT
    {
        $$ = std::make_shared<IntLit>($1);
    }
    |   VALUE_FLOAT
    {
        $$ = std::make_shared<FloatLit>($1);
    }
    |   VALUE_STRING
    {
        $$ = std::make_shared<StringLit>($1);
    }
    |   VALUE_BOOL
    {
        $$ = std::make_shared<BoolLit>($1);
    }
    |   VALUE_DATE
    {
        $$ = std::make_shared<DateLit>($1);
    }
    ;

condition:
        col op expr
    {
        $$ = std::make_shared<BinaryExpr>($1, $2, $3);
    }
    ;

optWhereClause:
        /* epsilon */ { /* ignore*/ }
    |   WHERE whereClause
    {
        $$ = $2;
    }
    ;

whereClause:
        condition 
    {
        $$ = std::vector<std::shared_ptr<BinaryExpr>>{$1};
    }
    |   whereClause AND condition
    {
        $$.push_back($3);
    }
    ;

col:
        tbName '.' colName
    {
        $$ = std::make_shared<Col>($1, $3);
    }
    |   colName
    {
        $$ = std::make_shared<Col>("", $1);
    }
    ;

colList:
        col
    {
        $$ = std::vector<std::shared_ptr<Col>>{$1};
    }
    |   colList ',' col
    {
        $$.push_back($3);
    }
    ;

op:
        '='
    {
        $$ = SV_OP_EQ;
    }
    |   '<'
    {
        $$ = SV_OP_LT;
    }
    |   '>'
    {
        $$ = SV_OP_GT;
    }
    |   NEQ
    {
        $$ = SV_OP_NE;
    }
    |   LEQ
    {
        $$ = SV_OP_LE;
    }
    |   GEQ
    {
        $$ = SV_OP_GE;
    }
    ;

expr:
        value
    {
        $$ = std::static_pointer_cast<Expr>($1);
    }
    |   col
    {
        $$ = std::static_pointer_cast<Expr>($1);
    }
    ;

setClauses:
        setClause
    {
        $$ = std::vector<std::shared_ptr<SetClause>>{$1};
    }
    |   setClauses ',' setClause
    {
        $$.push_back($3);
    }
    ;

setClause:
        colName '=' value
    {
        $$ = std::make_shared<SetClause>($1, $3);
    }
    |   colName '=' colName OP_PLUS value
    {
        $$ = std::make_shared<SetClause>($1, $3, '+', $5);
    }
    |   colName '=' colName OP_MINUS value
    {
        $$ = std::make_shared<SetClause>($1, $3, '-', $5);
    }
    |   colName '=' colName '*' value
    {
        $$ = std::make_shared<SetClause>($1, $3, '*', $5);
    }
    |   colName '=' colName OP_DIVIDE value
    {
        $$ = std::make_shared<SetClause>($1, $3, '/', $5);
    }
    ;

asClause:
        AS alias
    {
        $$ = std::move($2);
    }
    |
    {
        $$ = "";
    }
    ;

select_item:
        col asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::move($1), NO_AGG, std::move($2));
    }
    |   COUNT '(' '*' ')' asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::make_shared<Col>("", ""), AGG_COUNT, std::move($5));
    }
    |   COUNT '(' col ')' asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::move($3), AGG_COUNT, std::move($5));
    }
    |   MAX '(' col ')' asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::move($3), AGG_MAX, std::move($5));
    }
    |   MIN '(' col ')' asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::move($3), AGG_MIN, std::move($5));
    }
    |   SUM '(' col ')' asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::move($3), AGG_SUM, std::move($5));
    }
    |   AVG '(' col ')' asClause
    {
        $$ = std::make_shared<ColExtraInfo>(std::move($3), AGG_AVG, std::move($5));
    }
    ;

selector:
        '*'
    {
        $$ = {};
    }
    |   colList
    ;
select_list:
        '*'
    {
        $$ = {};
    }
    |   select_item
    {
        $$.emplace_back(std::move($1));
    }
    |   select_list ',' select_item
    {
        $$.emplace_back(std::move($3));
    }
    ;

tableList:
        tbName
    {
        $$ = std::vector<std::string>{$1};
    }
    |   tableList ',' tbName
    {
        $$.push_back($3);
    }
    |   tableList JOIN tbName
    {
        $$.push_back($3);
    }
    | tableList JOIN tbName ON condition
    {
        $$ = $1;
        $$.push_back($3);
        
        // 创建JOIN表达式
        std::string right_tab = $3;
        std::string left_tab = $1[$1.size() - 1];
        
        // 创建JOIN条件对象
        auto join_expr = std::make_shared<JoinExpr>(
            left_tab,
            right_tab,
            std::vector<std::shared_ptr<ast::BinaryExpr>>{$5},
            INNER_JOIN
        );
        
        // 存储JOIN表达式，将在创建SelectStmt时使用
        current_joins.push_back(join_expr);
    }
    | tableList SEMI JOIN tbName ON condition
    {
        $$ = $1;
        $$.push_back($4);
        
        //创建SEMI JOIN表达式
        std::string right_tab = $4;
        std::string left_tab = $1[$1.size() - 1];
        auto join_expr = std::make_shared<JoinExpr>(
            left_tab,
            right_tab,
            std::vector<std::shared_ptr<ast::BinaryExpr>>{$6},
            SEMI_JOIN 
        );
        current_joins.push_back(join_expr);
    }
    ;

opt_order_clause:
    ORDER BY order_clause      
    { 
        $$ = $3; 
    }
    |   /* epsilon */ { /* ignore*/ }
    ;

order_clause:
      col  opt_asc_desc 
    { 
        $$ = std::make_shared<OrderBy>($1, $2);
    }
    ;   

opt_asc_desc:
    ASC          { $$ = OrderBy_ASC;     }
    |  DESC      { $$ = OrderBy_DESC;    }
    |       { $$ = OrderBy_DEFAULT; }
    ;    

group_by_clause:
        GROUP BY colList
    {
        $$ = std::move($3);
    }
    |   /* epsilon */
    {
        /* ignore */
    }
    ;

agg_expr:
        COUNT '(' '*' ')'
    {
        $$ = std::make_shared<AggExpr>(std::make_shared<Col>("", ""), AGG_COUNT);
    }
    |   COUNT '(' col ')'
    {
        $$ = std::make_shared<AggExpr>(std::move($3), AGG_COUNT);
    }
    |   MAX '(' col ')'
    {
        $$ = std::make_shared<AggExpr>(std::move($3), AGG_MAX);
    }
    |   MIN '(' col ')'
    {
        $$ = std::make_shared<AggExpr>(std::move($3), AGG_MIN);
    }
    |   SUM '(' col ')'
    {
        $$ = std::make_shared<AggExpr>(std::move($3), AGG_SUM);
    }
    |   AVG '(' col ')'
    {
        $$ = std::make_shared<AggExpr>(std::move($3), AGG_AVG);
    }
    ;

having_clause:
        agg_expr op expr
    {
        $$.emplace_back(std::make_shared<HavingExpr>($1, $2, $3));
    }
    |   having_clause AND agg_expr op expr
    {
        $$.emplace_back(std::make_shared<HavingExpr>($3, $4, $5));
    }
    ;

having_clauses:
        HAVING having_clause
    {
        $$ = std::move($2);
    }
    |   /* epsilon */
    {
        /* ignore */
    }
    ;

opt_limit_clause:
        LIMIT VALUE_INT
    {
        $$ = std::make_shared<LimitExpr>(std::move($2));
    }
    |   /* epsilon */
    {
        /* ignore */
    }
    ;

set_knob_type:
    ENABLE_NESTLOOP { $$ = EnableNestLoop; }
    |   ENABLE_SORTMERGE { $$ = EnableSortMerge; }
    ;

tbName: 
      IDENTIFIER
    {
        $$ = $1;
    }
    | IDENTIFIER IDENTIFIER
    {
        //格式: "表名 别名"
        $$ = $1 + " " + $2;
    }
    ;

colName: IDENTIFIER;

alias: IDENTIFIER;
%%
