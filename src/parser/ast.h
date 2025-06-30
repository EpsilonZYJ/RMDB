/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */
#pragma once

#include <vector>
#include <string>
#include <memory>

enum JoinType {
    INNER_JOIN, LEFT_JOIN, RIGHT_JOIN, FULL_JOIN,SEMI_JOIN
};

enum AggType {
    AGG_COUNT,
    AGG_MAX,
    AGG_MIN,
    AGG_SUM,
    AGG_AVG,  
    NO_AGG  // 没有聚合函数
};

namespace ast {

enum SvType {
    SV_TYPE_INT, SV_TYPE_FLOAT, SV_TYPE_STRING, SV_TYPE_BOOL, SV_TYPE_DATE
};

enum SvCompOp {
    SV_OP_EQ, SV_OP_NE, SV_OP_LT, SV_OP_GT, SV_OP_LE, SV_OP_GE
};

enum OrderByDir {
    OrderBy_DEFAULT,
    OrderBy_ASC,
    OrderBy_DESC
};

enum SetKnobType {
    EnableNestLoop, EnableSortMerge
};

// Base class for tree nodes
struct TreeNode {
    virtual ~TreeNode() = default;  // enable polymorphism
};

struct Help : public TreeNode {
};

struct ShowTables : public TreeNode {
};

struct TxnBegin : public TreeNode {
};

struct TxnCommit : public TreeNode {
};

struct TxnAbort : public TreeNode {
};

struct TxnRollback : public TreeNode {
};

struct TypeLen : public TreeNode {
    SvType type;
    int len;

    TypeLen(SvType type_, int len_) : type(type_), len(len_) {}
};

struct Field : public TreeNode {
};

struct ColDef : public Field {
    std::string col_name;
    std::shared_ptr<TypeLen> type_len;

    ColDef(std::string col_name_, std::shared_ptr<TypeLen> type_len_) :
            col_name(std::move(col_name_)), type_len(std::move(type_len_)) {}
};

struct CreateTable : public TreeNode {
    std::string tab_name;
    std::vector<std::shared_ptr<Field>> fields;

    CreateTable(std::string tab_name_, std::vector<std::shared_ptr<Field>> fields_) :
            tab_name(std::move(tab_name_)), fields(std::move(fields_)) {}
};

struct DropTable : public TreeNode {
    std::string tab_name;

    DropTable(std::string tab_name_) : tab_name(std::move(tab_name_)) {}
};

struct DescTable : public TreeNode {
    std::string tab_name;

    DescTable(std::string tab_name_) : tab_name(std::move(tab_name_)) {}
};

struct CreateIndex : public TreeNode {
    std::string tab_name;
    std::vector<std::string> col_names;

    CreateIndex(std::string tab_name_, std::vector<std::string> col_names_) :
            tab_name(std::move(tab_name_)), col_names(std::move(col_names_)) {}
};

struct DropIndex : public TreeNode {
    std::string tab_name;
    std::vector<std::string> col_names;

    DropIndex(std::string tab_name_, std::vector<std::string> col_names_) :
            tab_name(std::move(tab_name_)), col_names(std::move(col_names_)) {}
};

struct ShowIndex : public TreeNode {
        std::string tab_name;

        explicit ShowIndex(std::string &tab_name_) : tab_name(std::move(tab_name_)) {
        }
    };

struct CreateCheckpoint : public TreeNode {
};

struct Expr : public TreeNode {
};

struct Value : public Expr {
};

struct IntLit : public Value {
    int val;

    IntLit(int val_) : val(val_) {}
};

struct FloatLit : public Value {
    float val;

    FloatLit(float val_) : val(val_) {}
};

struct StringLit : public Value {
    std::string val;

    StringLit(std::string val_) : val(std::move(val_)) {}
};

struct DateLit : public Value {
    std::string val; // YYYY-MM-DD格式的日期字符串

    DateLit(std::string val_) : val(std::move(val_)) {}
};

struct BoolLit : public Value {
    bool val;

    BoolLit(bool val_) : val(val_) {}
};

struct Col : public Expr {
    std::string tab_name;
    std::string col_name;

    Col(std::string tab_name_, std::string col_name_) :
            tab_name(std::move(tab_name_)), col_name(std::move(col_name_)) {}
};

// 定义更新表达式类型
enum UpdateExprType {
    SIMPLE_VALUE,  // 简单赋值
    COLUMN_EXPR    // 列表达式
};

struct SetClause : public TreeNode {
    std::string col_name;      // 要更新的列名
    std::shared_ptr<Value> val;// 当为简单赋值时的值
    UpdateExprType expr_type;  // 表达式类型
    std::string ref_col_name;  // 引用的列名
    char op;                   // 运算符：+, -, *, /
    
    // 简单赋值构造函数
    SetClause(std::string col_name_, std::shared_ptr<Value> val_) :
            col_name(std::move(col_name_)), val(std::move(val_)), 
            expr_type(SIMPLE_VALUE), ref_col_name(""), op(0) {}
    
    // 列表达式构造函数
    SetClause(std::string col_name_, std::string ref_col_name_, 
              char op_, std::shared_ptr<Value> val_) :
            col_name(std::move(col_name_)), val(std::move(val_)),
            expr_type(COLUMN_EXPR), ref_col_name(std::move(ref_col_name_)), op(op_) {}
};

/* WHERE条件语句的二元比较 */
struct BinaryExpr : public TreeNode {
    std::shared_ptr<Col> lhs;
    SvCompOp op;
    std::shared_ptr<Expr> rhs;

    BinaryExpr(std::shared_ptr<Col> lhs_, SvCompOp op_, std::shared_ptr<Expr> rhs_) :
            lhs(std::move(lhs_)), op(op_), rhs(std::move(rhs_)) {}
};

// ColExtraInfo存储包括别名以及聚合函数在内的更丰富的列信息
struct ColExtraInfo : public TreeNode {
    std::shared_ptr<Col> col;
    std::string alias; // 别名
    AggType type;

    ColExtraInfo(std::shared_ptr<Col> &&col_, AggType &&type_, std::string &&alias_ = "") : col(std::move(col_)),
        type(type_), alias(std::move(alias_)) {
    }
};

struct AggExpr : public TreeNode {
    std::shared_ptr<Col> col;
    AggType type;

    AggExpr(std::shared_ptr<Col> col_, AggType type_, std::string alias_ = "") :
            col(std::move(col_)), type(type_){}
};

struct OrderBy : public TreeNode
{
    std::shared_ptr<Col> cols;
    OrderByDir orderby_dir;
    OrderBy( std::shared_ptr<Col> cols_, OrderByDir orderby_dir_) :
       cols(std::move(cols_)), orderby_dir(std::move(orderby_dir_)) {}
};

struct InsertStmt : public TreeNode {
    std::string tab_name;
    std::vector<std::shared_ptr<Value>> vals;

    InsertStmt(std::string tab_name_, std::vector<std::shared_ptr<Value>> vals_) :
            tab_name(std::move(tab_name_)), vals(std::move(vals_)) {}
};

struct DeleteStmt : public TreeNode {
    std::string tab_name;
    std::vector<std::shared_ptr<BinaryExpr>> conds;

    DeleteStmt(std::string tab_name_, std::vector<std::shared_ptr<BinaryExpr>> conds_) :
            tab_name(std::move(tab_name_)), conds(std::move(conds_)) {}
};

struct UpdateStmt : public TreeNode {
    std::string tab_name;
    std::vector<std::shared_ptr<SetClause>> set_clauses;
    std::vector<std::shared_ptr<BinaryExpr>> conds;

    UpdateStmt(std::string tab_name_,
               std::vector<std::shared_ptr<SetClause>> set_clauses_,
               std::vector<std::shared_ptr<BinaryExpr>> conds_) :
            tab_name(std::move(tab_name_)), set_clauses(std::move(set_clauses_)), conds(std::move(conds_)) {}
};

struct JoinExpr : public TreeNode {
    std::string left;
    std::string right;
    std::vector<std::shared_ptr<BinaryExpr>> conds;
    JoinType type;

    JoinExpr(std::string left_, std::string right_,
               std::vector<std::shared_ptr<BinaryExpr>> conds_, JoinType type_=INNER_JOIN) :
            left(std::move(left_)), right(std::move(right_)), conds(std::move(conds_)), type(type_) {}
};

struct HavingExpr : public TreeNode {
    std::shared_ptr<AggExpr> lhs;
    SvCompOp op;
    std::shared_ptr<Expr> rhs;
    HavingExpr(std::shared_ptr<AggExpr> &lhs_, SvCompOp &op_, std::shared_ptr<Expr> &rhs_) : 
        lhs(std::move(lhs_)),
        op(op_),
        rhs(std::move(rhs_)) {}
};

struct LimitExpr : public TreeNode {
    int limit_num;
    explicit LimitExpr(int limit_num_) : limit_num(limit_num_) {}
};

struct SelectStmt : public TreeNode { // TODO extends Expr?
    std::vector<std::shared_ptr<ColExtraInfo>> cols;
    std::vector<std::string> tabs;
    std::vector<std::shared_ptr<BinaryExpr>> conds;
    std::vector<std::shared_ptr<JoinExpr>> jointree;
    std::vector<std::shared_ptr<Col> > group_bys;
    std::vector<std::shared_ptr<HavingExpr> > havings;

    
    bool has_sort;
    bool explain=false;
    std::shared_ptr<OrderBy> order;
    
    std::shared_ptr<LimitExpr> limit;


    SelectStmt(std::vector<std::shared_ptr<ColExtraInfo>> cols_,
               std::vector<std::string> tabs_,
               std::vector<std::shared_ptr<BinaryExpr>> conds_,
               std::vector<std::shared_ptr<JoinExpr>> joins_,
               std::vector<std::shared_ptr<Col> > &group_bys_, // 允许包含多个列
               std::vector<std::shared_ptr<HavingExpr> > &havings_,
               std::shared_ptr<OrderBy> order_,
               std::shared_ptr<LimitExpr> limit_) :
            cols(std::move(cols_)), 
            tabs(std::move(tabs_)), 
            conds(std::move(conds_)), 
            jointree(std::move(joins_)),
            group_bys(std::move(group_bys_)),
            havings(std::move(havings_)),
            order(std::move(order_)),
            limit(std::move(limit_)) {
                has_sort = (bool)order;
            }
};

// set enable_nestloop
struct SetStmt : public TreeNode {
    SetKnobType set_knob_type_;
    bool bool_val_;

    SetStmt(SetKnobType &type, bool bool_value) : 
        set_knob_type_(type), bool_val_(bool_value) { }
};


class ExplainStmt: public TreeNode {
    public:
        std::shared_ptr<SelectStmt> select;
        
        ExplainStmt(std::shared_ptr<SelectStmt> select_) : select(std::move(select_)) {}
        ~ExplainStmt() override = default;
    };

// Semantic value
struct SemValue {
    int sv_int;
    float sv_float;
    std::string sv_str;
    std::string sv_date; // 用于存储日期字符串，格式为YYYY-MM-DD
    bool sv_bool;
    char sv_char;  // 用于存储运算符
    OrderByDir sv_orderby_dir;
    std::vector<std::string> sv_strs;

    std::shared_ptr<TreeNode> sv_node;

    SvCompOp sv_comp_op;

    std::shared_ptr<TypeLen> sv_type_len;

    std::shared_ptr<Field> sv_field;
    std::vector<std::shared_ptr<Field>> sv_fields;

    std::shared_ptr<Expr> sv_expr;
    std::shared_ptr<AggExpr> sv_agg_expr;

    std::shared_ptr<Value> sv_val;
    std::vector<std::shared_ptr<Value>> sv_vals;

    std::shared_ptr<Col> sv_col;
    std::vector<std::shared_ptr<Col>> sv_cols;

    std::shared_ptr<ColExtraInfo> sv_col_extra_info;
    std::vector<std::shared_ptr<ColExtraInfo> > sv_col_extra_infos;

    std::shared_ptr<HavingExpr> sv_having;
    std::vector<std::shared_ptr<HavingExpr> > sv_havings;

    std::shared_ptr<SetClause> sv_set_clause;
    std::vector<std::shared_ptr<SetClause>> sv_set_clauses;

    std::shared_ptr<BinaryExpr> sv_cond;
    std::vector<std::shared_ptr<BinaryExpr>> sv_conds;

    std::shared_ptr<OrderBy> sv_orderby;

    std::shared_ptr<LimitExpr> sv_limit;

    SetKnobType sv_setKnobType;
};

extern std::shared_ptr<ast::TreeNode> parse_tree;

}

#define YYSTYPE ast::SemValue
