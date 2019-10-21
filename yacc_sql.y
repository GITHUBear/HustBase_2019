/*
file: yacc_sql.y
content:语法分析
author:Emily Zhang
*/
%{

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

%}
//标识tokens
%token SEMICOLON CREATE DROP TABLE INDEX SELECT INSERT DELETE UPDATE
       LBRACE RBRACE COMMA INT_T STRING_T FLOAT_T HELP EXIT DOT //QUOTE
       INTO VALUES  FROM WHERE AND  SET	ON 
       EQ LT GT LE GE NE

%union {
    struct _Attr *attr;
	  struct _Condition *condition1;
  	struct _Value *value1;
    char *string;
  //  char *ssss;
    int number;
    float floats;

}

%token <number> NUMBER
%token <floats> FLOAT 
%token <string> ID
%token <string> SSS
%token <string>  STAR
%token <string> STRING_V
//非终结符

%type <number> type;
%type <condition1> condition;
%type <value1> value;
%type <number> number;

%%

commands:		//commands or sqls. parser starts here.
    /* empty */
    | commands command
    ;

command:
    	 select  
        | insert
	| update
	| delete
	| create_table
	| drop_table
	| create_index	
	| drop_index
	| help
	| exit
    ;

/*match to the flag of sqls 
		0--error;
		1--select;
		2--insert;
		3--update;
		4--delete;
		5--create table;
		6--drop table;
		7--create index;
		8--drop index;
		9--help;
		10--exit;
*/

exit:			
    EXIT SEMICOLON {
        ssql->flag=10;//"exit";
    };

help:
    HELP SEMICOLON {
        ssql->flag=9;//"help";
    };
drop_table:		/*drop table 语句的语法解析树*/
    DROP TABLE ID SEMICOLON {
        ssql->flag=6;//"drop_table";
        ssql->sstr.drt.relName=$3;
    };
create_index:		/*create idnex 语句的语法解析树*/
    CREATE INDEX ID ON ID LBRACE ID RBRACE SEMICOLON {
																							        ssql->flag=7;//"create_index";
																							        ssql->sstr.crei.indexName=$3;
																							        ssql->sstr.crei.relName=$5;
																							        ssql->sstr.crei.attrName=$7;
																							    }
    ;

drop_index:			/*drop index 语句的语法解析树*/
    DROP INDEX ID  SEMICOLON {
											        ssql->flag=8;//"drop_index";
											        ssql->sstr.dri.indexName=$3;
											    }
    ;
create_table:		/*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE SEMICOLON {
	
																																		ssql->flag=5;//"create_table";
																																		ssql->sstr.cret.relName=$3;
																																		ssql->sstr.cret.attrCount=valueleng;
																																		//临时变量清零	
																																		valueleng=0;
																																		
																																	    }
    ;
attr_def_list:
    /* empty */
    | COMMA attr_def attr_def_list {    }
    ;
    
attr_def:
    ID_get type LBRACE number RBRACE {
																ssql->sstr.cret.attributes[valueleng].attrName=(char*)malloc(sizeof(char));
																strcpy(ssql->sstr.cret.attributes[valueleng].attrName,get_id); 
																ssql->sstr.cret.attributes[valueleng].attrType=$2;  
																ssql->sstr.cret.attributes[valueleng].attrLength=$4;
																valueleng++;
															    }
    |ID_get type{
															ssql->sstr.cret.attributes[valueleng].attrName=(char*)malloc(sizeof(char));
															strcpy(ssql->sstr.cret.attributes[valueleng].attrName,get_id); 
															ssql->sstr.cret.attributes[valueleng].attrType=$2;  
															ssql->sstr.cret.attributes[valueleng].attrLength=4;
															valueleng++;
														    }
    ;
number:
		NUMBER {$$ = $1;}
		;
type:
	INT_T { $$=1; }
       | STRING_T { $$=0; }
       | FLOAT_T { $$=2; }
       ;
ID_get:
	ID {
				 temp=$1; 
				for(i=0;i<MAX_NUM;i++){
				get_id[i]=*(temp+i);
				}
			}
	;

	
insert:				/*insert   语句的语法解析树*/
    INSERT INTO ID VALUES LBRACE value value_list RBRACE SEMICOLON {
 																																valueT[valueleng++] = *$6;
																																
																																ssql->flag=2;//"insert";
																																ssql->sstr.ins.relName=$3;	
																																ssql->sstr.ins.nValues=valueleng;
																																for(i=0;i<valueleng;i++){
																																ssql->sstr.ins.values[i] = valueT[i];
																												}
																																//临时变量清零	
																																valueleng=0;
																															//	free(valueT);
	
    }

value_list:
    /* empty */
    | COMMA value value_list  { 
  													 valueT[valueleng++] = *$2;
	}
    ;

value:
    NUMBER{	
					$$ = (Value*)malloc(sizeof(Value));
					$$->data=(int*)malloc((sizeof(int)));
					$$->type =1; 					                      // "ints";
					*((int*)($$->data)) = $1;
					}
    |FLOAT{								
					$$=(Value*)malloc(sizeof(Value));
					$$->data=malloc(MAX_NUM*(sizeof(char)));
					$$->type =2; 					                     	//"floats"
					*((float*)($$->data)) = $1;
					}
    |SSS {																		
					$$=(Value*)malloc(sizeof(Value));				
					$$->data=malloc(MAX_NUM*(sizeof(char)));		
    			$$->type = 0;					// "chars";						
    			$1 = substr($1,1,strlen($1)-2);
    			sprintf((char*)($$->data), "%s", $1); 			
    		
					
    			 }																				
   								
    ;
    
delete:		/*  delete 语句的语法解析树*/
    DELETE FROM ID where SEMICOLON {
	
	ssql->flag=4;//"delete";
	ssql->sstr.del.relName=$3;
	for(i=0;i<whereleng;i++){
	ssql->sstr.del.conditions[i]=wherecondi[i];
	}
	ssql->sstr.del.nConditions=whereleng;
	whereleng=0;	
    }
    ;
update:			/*  update 语句的语法解析树*/
    UPDATE ID SET ID EQ value where SEMICOLON{
	
											ssql->flag=3;//"update";
											ssql->sstr.upd.relName=$2;
											ssql->sstr.upd.value=*$6;
											ssql->sstr.upd.attrName=$4;
											for(i=0;i<whereleng;i++){
											ssql->sstr.upd.conditions[i]=wherecondi[i];
											}
											ssql->sstr.upd.nConditions=whereleng;
											whereleng=0;
									
										    }
    ;
select:				/*  select 语句的语法解析树*/
    SELECT select_attr FROM ID rel_list where SEMICOLON {
      
																												ssql->sstr.sel.relations[fromleng++]=$4;
																											
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
   																										 }
    ;

select_attr:
    STAR { ssql->sstr.sel.selAttrs[selectleng]->attrName="*";	
    			ssql->sstr.sel.selAttrs[selectleng++]->relName=NULL;
    	}
    | ID attr_list {
										ssql->sstr.sel.selAttrs[selectleng]->attrName=$1;
										ssql->sstr.sel.selAttrs[selectleng++]->relName=NULL;
  									}
  	| ID DOT ID attr_list{
  						ssql->sstr.sel.selAttrs[selectleng]->attrName=$3;
							ssql->sstr.sel.selAttrs[selectleng++]->relName=$1;
  			}
    ;
attr_list:
    /* empty */
    | COMMA ID attr_list {
     	  ssql->sstr.sel.selAttrs[selectleng]->relName = NULL;
        ssql->sstr.sel.selAttrs[selectleng++]->attrName=$2;
    }
    | COMMA ID DOT ID attr_list{
  						ssql->sstr.sel.selAttrs[selectleng]->attrName=$4;
							ssql->sstr.sel.selAttrs[selectleng++]->relName=$2;
  			}
  	;

rel_list:
    /* empty */
    | COMMA ID rel_list {	
									        ssql->sstr.sel.relations[fromleng++]=$2;
									    	}
    ;
where:
    /* empty */ 
    | WHERE condition condition_list {	
																		wherecondi[whereleng++]=*$2;
																	    }
    ;
condition_list:
    /* empty */
    | AND condition condition_list{
																		wherecondi[whereleng++]=*$2;
													   			}
    ;
condition:
    ID comOp value {
									$$ = ( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 1;
									$$->lhsAttr.relName = NULL;
									$$->lhsAttr.attrName = $1;
									$$->op = compOpT;
									$$->bRhsIsAttr = 0;
									$$->rhsAttr.relName = NULL;
									$$->rhsAttr.attrName = NULL;
									$$->rhsValue = *$3;

								    }
		|value comOp value {
									$$ = ( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 0;
									$$->lhsAttr.relName=NULL;
									$$->lhsAttr.attrName=NULL;
									$$->lhsValue = *$1;
									$$->op = compOpT;
									$$->bRhsIsAttr = 0;
									$$->rhsAttr.relName = NULL;
									$$->rhsAttr.attrName = NULL;
									$$->rhsValue = *$3;

								    }
		|ID comOp ID {
									$$=( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 1;
									$$->lhsAttr.relName=NULL;
									$$->lhsAttr.attrName=$1;
									$$->op=compOpT;
									$$->bRhsIsAttr = 1;
									$$->rhsAttr.relName=NULL;
									$$->rhsAttr.attrName=$3;

								    }
    |value comOp ID{
									$$=( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 0;
									$$->lhsAttr.relName=NULL;
									$$->lhsAttr.attrName=NULL;
									$$->lhsValue = *$1;
									$$->op=compOpT;
									
									$$->bRhsIsAttr = 1;
									$$->rhsAttr.relName=NULL;
									$$->rhsAttr.attrName=$3;
								
								    }
    |ID DOT ID comOp value{
									$$=( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 1;
									$$->lhsAttr.relName=$1;
									$$->lhsAttr.attrName=$3;
									$$->op=compOpT;
									$$->bRhsIsAttr = 0;   //属性值
									$$->rhsAttr.relName=NULL;
									$$->rhsAttr.attrName=NULL;
									$$->rhsValue=*$5;			
							
    											}
    |value comOp ID DOT ID{
									$$=( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 0;//属性值
									$$->lhsAttr.relName=NULL;
									$$->lhsAttr.attrName=NULL;
									$$->lhsValue = *$1;
									$$->op=compOpT;
									$$->bRhsIsAttr = 1;//属性
									$$->rhsAttr.relName = $3;
									$$->rhsAttr.attrName = $5;
									
    						}
    |ID DOT ID comOp ID DOT ID{
									$$=( Condition *)malloc(sizeof( Condition));
									$$->bLhsIsAttr = 1;		//属性
									$$->lhsAttr.relName=$1;
									$$->lhsAttr.attrName=$3;
									$$->op=compOpT;
									$$->bRhsIsAttr = 1;		//属性
									$$->rhsAttr.relName=$5;
									$$->rhsAttr.attrName=$7;								
    											}
    ;

comOp:
  	  EQ { compOpT = 0; }
    | LT { compOpT = 3; }
    | GT { compOpT = 5; }
    | LE { compOpT = 1; }
    | GE { compOpT = 4; }
    | NE { compOpT = 2; }
    ;

%%
//_____________________________________________________________________
int hust_parse( sqlstr *sqls){
	ssql=sqls;
	return yyparse();
}
