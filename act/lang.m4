/*************************************************************************
 *
 *  Copyright (c) 2011 Rajit Manohar
 *  All Rights Reserved
 *
 **************************************************************************
 */

/* 
   Language bodies
*/

language_body[ActBody *]: lang_chp 
| lang_hse 
| lang_prs 
{{X:
    return $1;
}}
| lang_spec 
| lang_size ;

supply_spec: "<" bool_expr_id [ "," bool_expr_id ]
                 [ "|" bool_expr_id "," bool_expr_id ] 
             ">"
{{X:
    ActRet *r;

    $0->supply.vdd = $2;
    if (!OPT_EMPTY ($3)) {
      r = OPT_VALUE ($3);
      $A(r->type == R_ID);
      $0->supply.gnd = r->u.id;
      FREE (r);
    }
    else {
      $0->supply.gnd = NULL;
    }
    OPT_FREE ($3);

    if (!OPT_EMPTY ($4)) {
      r = OPT_VALUE ($4);
      $A(r->type == R_ID);
      $0->supply.psc = r->u.id;
      FREE (r);
      r = OPT_VALUE2 ($4);
      $A(r->type == R_ID);
      $0->supply.nsc = r->u.id;
      FREE (r);
    }
    else {
      $0->supply.psc = NULL;
      $0->supply.nsc = NULL;
    }
    OPT_FREE ($4);
    return NULL;
}}
;

lang_chp: "chp" [ supply_spec ] "{" [ chp_body ] "}"
{{X:
    $0->supply.vdd = NULL;
    $0->supply.gnd = NULL;
    $0->supply.psc = NULL;
    $0->supply.nsc = NULL;
    OPT_FREE ($2);
    return NULL;
}}
;

lang_hse: "hse" [ supply_spec ] "{" [ hse_body ] "}" 
{{X:
    $0->supply.vdd = NULL;
    $0->supply.gnd = NULL;
    $0->supply.psc = NULL;
    $0->supply.nsc = NULL;
    OPT_FREE ($2);
    return NULL;
}}
;

lang_prs[ActBody *]: "prs" [ supply_spec ] "{" [ prs_body ] "}" 
{{X:
    ActBody *b;
    act_prs *p;

    b = NULL;
    p = NULL;
    if (!OPT_EMPTY ($4)) {
      ActRet *r;

      r = OPT_VALUE ($4);
      $A(r->type == R_PRS_LANG);
      NEW (p, act_prs);
      p->p = r->u.prs;
      FREE (r);
      p->vdd = $0->supply.vdd;
      p->gnd = $0->supply.gnd;
      p->nsc = $0->supply.nsc;
      p->psc = $0->supply.psc;
    }
    if (p) {
      b = new ActBody_Lang (p);
    }
    $0->supply.vdd = NULL;
    $0->supply.gnd = NULL;
    $0->supply.psc = NULL;
    $0->supply.nsc = NULL;
    OPT_FREE ($2);
    return b;
}}
;

lang_spec[ActBody *]: "spec" "{" [ spec_body ] "}"
;

chp_body: { chp_comma_list ";" }*
;

chp_comma_list: { chp_body_item "," }*
;

chp_body_item: base_stmt
| select_stmt
| loop_stmt
;

base_stmt: send_stmt
    | recv_stmt
    | assign_stmt
    | "skip" 
    | "(" chp_body ")"
    | ID "(" { chp_log_item "," }* ")" /* log */
;


chp_log_item: expr_id
|  STRING
;

send_stmt: expr_id "!" send_data
;

send_data: w_expr 
| "(" { w_expr "," }* ")" 
;

recv_stmt: expr_id "?" recv_id
;

recv_id: expr_id
| "(" { expr_id "," }** ")" 
;

assign_stmt: expr_id ":=" w_expr
| expr_id dir
;

select_stmt: "[" { guarded_cmd "[]" }* "]"
| "[" wbool_expr "]" 
;


guarded_cmd: wbool_expr "->" chp_body 
| "else" "->" chp_body
;

loop_stmt: "*[" chp_body "]"
| "*[" { guarded_cmd "[]" }* "]"
;

hse_body[ActBody *]: { hse_body_item ";" }*
;

hse_body_item: { hse_assign_stmt "," }* 
| hse_loop_stmt
| hse_select_stmt
| "skip"
| "(" hse_body ")"
;

hse_assign_stmt: expr_id dir 
;

hse_select_stmt: "[" { hse_guarded_cmd "[]" }* "]"
| "[" wbool_expr "]" 
;

hse_guarded_cmd: wbool_expr "->" hse_body
| "else" "->" hse_body
;

hse_loop_stmt: "*[" hse_body "]"
| "*[" { hse_guarded_cmd "[]" }* "]"
;

prs_body[act_prs_lang_t *]: [ attr_list ] 
{{X:
    $0->line = $l;
    $0->column = $c;
    $0->file = $n;
}}
single_prs prs_body
{{X:
    $2->next=  $3;
    return $2;
}}
| [ attr_list ] 
{{X:
    $0->line = $l;
    $0->column = $c;
    $0->file = $n;
}}
single_prs
{{X:
    return $2;
}}
;

attr_list[act_attr_t *]: "[" { ID "=" w_expr ";" }** "]"
{{X: 
    listitem_t *li;
    act_attr_t *a, *ret, *prev;
    ActRet *r;

    a = ret = prev = NULL;
    for (li = list_first ($2); li; li = list_next (li)) {
      r = (ActRet *)list_value (li);
      $A(r);
      $A(r->type == R_STRING);
      if (!ret) {
	NEW (ret, act_attr_t);
	a = ret;
      }
      else {
	NEW (a, act_attr_t);
      }
      a->next = NULL;
      a->attr = r->u.str;
      FREE (r);
      if (prev) {
	prev->next = a;
      }
      prev = a;
	
      li = list_next (li);
      $A(li);
      r = (ActRet *)list_value (li);
      $A(r);
      $A(r->type == R_EXPR);
      a->e = r->u.exp;
      FREE (r);
    }
    list_free ($2);
    return ret;
}}
;

single_prs[act_prs_lang_t *]: EXTERN[prs_expr] arrow bool_expr_id dir
{{X:
    act_prs_lang_t *p;

    NEW (p, act_prs_lang_t);
    p->next = NULL;
    p->type = ACT_PRS_RULE;
    p->u.one.attr = NULL;
    p->u.one.e = (act_prs_expr_t *) $1;
    p->u.one.arrow_type = $2;
    p->u.one.id = $3;
    p->u.one.dir = $4;
    p->u.one.label = 0;
    return p;
}}
| EXTERN[prs_expr] arrow "@" ID dir
{{X:
    act_prs_lang_t *p;

    NEW (p, act_prs_lang_t);
    p->next = NULL;
    p->type = ACT_PRS_RULE;
    p->u.one.attr = NULL;
    p->u.one.e = (act_prs_expr_t *) $1;
    p->u.one.arrow_type = $2;
    p->u.one.id = (ActId *)$4;
    p->u.one.dir = $5;
    p->u.one.label = 1;
    return p;
}}
| ID [ tree_subckt_spec ]
{{X:
    ActRet *r;
    act_prs_lang_t *p;

    if (!OPT_EMPTY ($2)) {
      r = OPT_VALUE ($2);
    }
    else {
      r = NULL;
    }
    if (strcmp ($1, "tree") == 0) {
      if ($0->in_tree) {
	$E("tree { } directive in prs cannot be nested");
      }
      $0->in_tree++;
      if (r && (r->type != R_EXPR)) {
	$E("tree < > parameter must be an expression");
      }
    }
    else if (strcmp ($1, "subckt") == 0) {
      if ($0->in_subckt) {
	$E("subckt { } directive in prs cannot be nested");
      }
      $0->in_subckt++;
      if (r && (r->type != R_STRING)) {
	$E("subckt < > parameter must be a string");
      }
    }
    else {
      $E("Unknown type of body within prs { }: ``%s''", $1);
    }
}}
 "{" prs_body "}"
{{X:
    ActRet *r;
    act_prs_lang_t *p;

    if (OPT_EMPTY ($2)) {
      r = NULL;
    }
    else {
      r = OPT_VALUE ($2);
    }
    OPT_FREE ($2);

    NEW (p, act_prs_lang_t);
    p->next = NULL;
    p->u.l.p = $4;

    if (strcmp ($1, "tree") == 0) {
      p->type = ACT_PRS_TREE;
      if (r) {
	$A(r->type == R_EXPR);
	p->u.l.lo = r->u.exp;
      }
      else {
	p->u.l.lo = NULL;
      }
      p->u.l.hi = NULL;
      p->u.l.id = NULL;
      $0->in_tree--;
    }
    else if (strcmp ($1, "subckt") == 0) {
      p->type = ACT_PRS_SUBCKT;
      p->u.l.lo = NULL;
      p->u.l.hi = NULL;
      if (r) {
	$A(r->type == R_STRING);
	p->u.l.id = r->u.str;
      }
      else {
	p->u.l.id = NULL;
      }
      $0->in_subckt--;
    }
    if (r) { FREE (r); }
    return p;
}}
| "(" [":"] ID 
{{X:
    if ($0->scope->Lookup ($3)) {
      $E("Identifier `%s' already defined in current scope", $3);
    }
    $0->scope->Add ($3, $0->tf->NewPInt ());
    OPT_FREE ($2);
}}
":" !noreal wint_expr [ ".." wint_expr ] ":" prs_body ")"
{{X:
    act_prs_lang_t *p;
    $0->scope->Del ($3);

    NEW (p, act_prs_lang_t);
    p->type = ACT_PRS_LOOP;
    p->next = NULL;
    p->u.l.id = $3;
    p->u.l.lo = $5;
    
    if (OPT_EMPTY ($6)) {
      p->u.l.hi = $5;
      p->u.l.lo = NULL;
    }
    else {
      ActRet *r;
      r = OPT_VALUE ($6);
      $A(r->type == R_EXPR);
      p->u.l.hi = r->u.exp;
      FREE (r);
    }
    OPT_FREE ($6);
    p->u.l.p = $8;
    return p;
}}
/* gate, source, drain */
| "passn" size_spec "(" bool_expr_id "," bool_expr_id "," bool_expr_id ")"
{{X:
    act_prs_lang_t *p;

    NEW (p, act_prs_lang_t);
    p->type = ACT_PRS_GATE;
    p->next = NULL;
    p->u.p.sz = $2;
    p->u.p.g = $4;
    p->u.p.s = $6;
    p->u.p.d = $8;
    p->u.p._g = NULL;

    return p;
}}
| "passp" size_spec "(" bool_expr_id "," bool_expr_id "," bool_expr_id ")"
{{X:
    act_prs_lang_t *p;

    NEW (p, act_prs_lang_t);
    p->type = ACT_PRS_GATE;
    p->next = NULL;
    p->u.p.sz = $2;
    p->u.p._g = $4;
    p->u.p.s = $6;
    p->u.p.d = $8;
    p->u.p.g = NULL;

    return p;
}}
/* n first, then p */
| "transgate" size_spec "(" bool_expr_id "," bool_expr_id "," bool_expr_id "," bool_expr_id ")"
{{X:
    act_prs_lang_t *p;

    NEW (p, act_prs_lang_t);
    p->type = ACT_PRS_GATE;
    p->next = NULL;
    p->u.p.sz = $2;
    p->u.p.g = $4;
    p->u.p._g = $6;
    p->u.p.s = $8;
    p->u.p.d = $10;

    return p;
}}
;

arrow[int]: "->" {{X: return 0; }}
| "=>" {{X: return 1; }}
| "#>" {{X: return 2; }}
;

dir[int]: "+"  {{X: return 1; }}
| "-" {{X: return 0; }}
;

tree_subckt_spec: "<" wint_expr ">"
{{X:
    ActRet *r;
    NEW (r, ActRet);
    r->type = R_EXPR;
    r->u.exp = $2;
    return r;
}}
| "<" STRING ">"
{{X:
    ActRet *r;
    NEW (r, ActRet);
    r->type = R_STRING;
    r->u.str = $2;
    return r;
}}
;

/*
  CONSISTENCY: Check _process_id in prs.c
*/
bool_expr_id[ActId *]: expr_id
{{X:
    int t;
    t = act_type_var ($0->scope, $1);
    if (t == T_ERR) 
    if (t != T_BOOL) {
      $e("Identifier ``");
      $1->Print ($f, NULL);
      fprintf ($f, "'' is not of type bool");
      exit (1);
    }
    return $1;
}}
;

/*
  CONSISTENCY: MAKE SURE THIS IS CONSISTENT WITH prs.c
  XXX: main change: this one allows real expressions, whereas
      prs.c is integer expressions.

      < width , length, flavor : somethingelse >

*/
size_spec[act_size_spec_t *]: "<" wnumber_expr [ "," wnumber_expr ] [ "," ID [ ":" INT ] ] ">"
{{X:
    act_size_spec_t *s;
    
    NEW (s, act_size_spec_t);
    s->w = $2;
    s->l = NULL;
    s->flavor = ACT_FET_STD;
    s->subflavor = -1;
    
    if (!OPT_EMPTY ($3)) {
      ActRet *r;

      r = OPT_VALUE ($3);
      $A(r->type == R_EXPR);
      s->l = r->u.exp;
      FREE (r);
    }
    OPT_FREE ($3);

    if (!OPT_EMPTY ($4)) {
      ActRet *r;

      r = OPT_VALUE ($4);
      $A(r->type == R_STRING);

      s->flavor = act_fet_string_to_value (r->u.str);
      if (s->flavor == ACT_FET_END) {
	$E("Unknown transistor flavor ``%s''", r->u.str);
      }
      FREE (r);
      
      r = OPT_VALUE2 ($4);
      $A(r->type == R_LIST);
      
      if (!OPT_EMPTY (r->u.l)) {
	ActRet *r2;

	r2 = OPT_VALUE (r->u.l);
	$A(r2->type == R_INT);
	s->subflavor = r2->u.ival;
	FREE (r2);
      }
      list_free (r->u.l);
      FREE (r);
    }
    return s;
}}
|  /* empty */
;
  
spec_body: spec_body_item spec_body 
| spec_body_item 
;

spec_body_item: ID "(" { bool_expr_id "," }* ")" 
| "$(" wbool_expr ")"
;

/*
  Sizing body: specify drive strength for rules
*/
lang_size: "size" "{" [ size_body ] "}"
;

strength_directive: bool_expr_id dir "->" wint_expr
;

size_body: { strength_directive ";" }*
;