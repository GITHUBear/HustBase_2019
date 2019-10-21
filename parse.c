#include "str.h"
#include "yacc_sql.tab.h"

selects * get_selects();
Condition * get_Condition();
RelAttr * get_RelAttr();
Value * get_Value();
union sqls * get_sqls();
sqlstr * get_sqlstr();
RC parse(char* st,sqlstr* sqln);


int i;
//allocate struct of condition
Condition *get_Condition(){
	Condition *condition;
	condition=(Condition *)malloc(sizeof(Condition));
	condition->lhsAttr = *get_RelAttr();
	condition->rhsAttr = *get_RelAttr();
	condition->lhsValue = *get_Value();
	condition->rhsValue = *get_Value();
	condition->bLhsIsAttr = -1;
	condition->bRhsIsAttr = -1;
	return condition;
}
//allocate struct of relattr
RelAttr *get_RelAttr(){
	RelAttr * relAttr;
	relAttr = (RelAttr*)malloc(sizeof(RelAttr));
	relAttr->relName = (char *)malloc(sizeof(char) * MAX_REL_NAME);
	memset(relAttr->relName,0,sizeof(char)* MAX_REL_NAME);
	relAttr->attrName = (char *)malloc(sizeof(char) * MAX_ATTR_NAME);
	memset(relAttr->attrName,0,sizeof(char)* MAX_ATTR_NAME);
	return relAttr;
}
//allocate struct of value,the default type is int
Value * get_Value(){
	Value * value = (Value *)malloc(sizeof(Value));
	value->data = (void *)malloc(sizeof(char)*MAX_DATA);
	memset(value->data,0,sizeof(char) * MAX_DATA);
	value->type = ints;
	return value;
}
//allocate struct of select
selects *get_selects(){
	selects *se;
	se=(selects *)malloc(sizeof(selects));
	(se)->nSelAttrs=0;
	for(i=0;i<MAX_NUM;i++){
		(se)->selAttrs[i]=get_RelAttr();
	}
	(se)->nRelations=0;
	for(i=0;i<MAX_NUM;i++){
		(se)->relations[i]=(char*)malloc(sizeof(char));
		memset(se->relations[i],0,sizeof(char));
	}
	(se)->nConditions=0;
	for(i=0;i<MAX_NUM;i++){
		(se)->conditions[i]=*get_Condition();
	}
	return se;
}
//allocate union of sqls
union sqls *get_sqls(){
	union sqls *sql;
	sql=(union sqls *)malloc(sizeof(union sqls));
	(sql)->sel=*get_selects();
	(sql)->errors = (char *)malloc(sizeof(char)*MAX_ERROR_MESSAGE);
	memset(sql->errors,0,sizeof(char) * MAX_ERROR_MESSAGE);
	return sql;
}
sqlstr *get_sqlstr(){
	sqlstr *sq;
	sq=(sqlstr *)malloc(sizeof(sqlstr));
	(sq)->flag=0;
	(sq)->sstr=*get_sqls();
	return sq;
}

extern int hust_parse(sqlstr *sqls);
extern 

RC parse(char* st,sqlstr* sqln){
	
	yy_switch_to_buffer(yy_scan_string(st)); //sql语句进入缓存

	
	hust_parse(sqln);				//解析得到SQL语句，存在语句相应的结构体中 并返回
	/*判断flag的值，非零值返回CORRECT，为0时说明解析过程中出现语法错误，返回SQL_SYNTAX*/
	if(sqln->flag==0)
	return SQL_SYNTAX;
	else
	return SUCCESS;

}