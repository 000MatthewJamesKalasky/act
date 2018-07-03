/*************************************************************************
 *
 *  Copyright (c) 2011-2018 Rajit Manohar
 *  All Rights Reserved
 *
 **************************************************************************
 */
#include <act/types.h>
#include <act/inst.h>
#include <act/lang.h>
#include <string.h>
#include "misc.h"

/* XXX: free actbody */

void ActBody::Append (ActBody *b)
{
  ActBody *t, *u;
  if (!next) {
    next = b;
  }
  else {
    t = next;
    u = next->next;
    while (u) {
      t = u;
      u = u->next;
    }
    t->next = b;
  }
}

ActBody *ActBody::Tail ()
{
  ActBody *b = this;

  while (b->next) {
    b = b->next;
  }
  return b;
}

ActBody_Inst::ActBody_Inst (InstType *it, const char *_id)
{
  id = _id;
  t = it;
}

Type *ActBody_Inst::BaseType ()
{
  Assert (t, "NULL deref on ActBody_Inst's insttype");
  return t->BaseType();
}

/*------------------------------------------------------------------------*/

/*
 * ns = the current namespace. Namespaces get expanded in place, since
 * there's no notion of a parameterized namespace.
 * s = the *fresh*, new scope for expansion
 */
void ActBody_Inst::Expand (ActNamespace *ns, Scope *s)
{
  InstType *x, *it;

  /* typechecking should all pass, so there shouldn't be an issue
     beyond creating the actual object. For arrays, there may be
     duplicate dereference issues
  */

  /* 
     expand instance type!
  */
  it = t->Expand (ns, s);
  x = s->Lookup (id);

  if (x) {
    ValueIdx *vx;
    Array *old;

    vx = s->LookupVal (id);
    Assert (vx && vx->t == x, "What?");
    old = x->arrayInfo()->Clone();

#if 0
    fprintf (stderr, "Original: ");
    vx->t->Print (stderr);
    fprintf (stderr, "\n");
#endif

    /* sparse array */
    x->arrayInfo()->Merge (it->arrayInfo());

#if 0
    fprintf (stderr, "New: ");
    vx->t->Print (stderr);
    fprintf (stderr, "\n");
#endif
    
    if (vx->init) {
      /* it's been allocated; needs reallocation! */
      if (TypeFactory::isParamType (vx->t)) {
	bitset_t *b = bitset_new (old->size());
	/* realloc and copy */

	if (TypeFactory::isPIntType (vx->t->BaseType())) {
	  int *vals;
	  Arraystep *as;
	  
	  MALLOC (vals, int, old->size());
	  for (int i=0; i < old->size(); i++) {
	    if (s->issetPInt (vx->u.idx + i)) {
	      bitset_set (b, i);
	      vals[i] = s->getPInt (vx->u.idx + i);
	    }
	  }
	  s->DeallocPInt (vx->u.idx, old->size());
	  vx->u.idx = s->AllocPInt (x->arrayInfo()->size());

	  as = old->stepper();
	  while (!as->isend()) {
	    if (bitset_tst (b, as->index())) {
	      s->setPInt (vx->u.idx + as->index (x->arrayInfo()),
			  vals[as->index()]);
	    }
	    as->step();
	  }
	  delete as;
	  FREE (vals);
	}
	else if (TypeFactory::isPIntsType (vx->t->BaseType())) {
	  int *vals;
	  Arraystep *as;
	  
	  MALLOC (vals, int, old->size());
	  for (int i=0; i < old->size(); i++) {
	    if (s->issetPInts (vx->u.idx + i)) {
	      bitset_set (b, i);
	      vals[i] = s->getPInts (vx->u.idx + i);
	    }
	  }
	  s->DeallocPInts (vx->u.idx, old->size());
	  vx->u.idx = s->AllocPInts (x->arrayInfo()->size());

	  as = old->stepper();
	  while (!as->isend()) {
	    if (bitset_tst (b, as->index())) {
	      s->setPInts (vx->u.idx + as->index (x->arrayInfo()),
			  vals[as->index()]);
	    }
	    as->step();
	  }
	  delete as;
	  FREE (vals);
	}
	else if (TypeFactory::isPRealType (vx->t->BaseType())) {
	  double *vals;
	  Arraystep *as;
	  
	  MALLOC (vals, double, old->size());
	  for (int i=0; i < old->size(); i++) {
	    if (s->issetPReal (vx->u.idx + i)) {
	      bitset_set (b, i);
	      vals[i] = s->getPReal (vx->u.idx + i);
	    }
	  }
	  s->DeallocPReal (vx->u.idx, old->size());
	  vx->u.idx = s->AllocPReal (x->arrayInfo()->size());

	  as = old->stepper();
	  while (!as->isend()) {
	    if (bitset_tst (b, as->index())) {
	      s->setPReal (vx->u.idx + as->index (x->arrayInfo()),
			   vals[as->index()]);
	    }
	    as->step();
	  }
	  delete as;
	  FREE (vals);
	}
	else if (TypeFactory::isPBoolType (vx->t->BaseType())) {
	  int *vals;
	  Arraystep *as;
	  
	  MALLOC (vals, int, old->size());
	  for (int i=0; i < old->size(); i++) {
	    if (s->issetPBool (vx->u.idx + i)) {
	      bitset_set (b, i);
	      vals[i] = s->getPBool (vx->u.idx + i);
	    }
	  }
	  s->DeallocPBool (vx->u.idx, old->size());
	  vx->u.idx = s->AllocPBool (x->arrayInfo()->size());

	  as = old->stepper();
	  while (!as->isend()) {
	    if (bitset_tst (b, as->index())) {
	      s->setPBool (vx->u.idx + as->index (x->arrayInfo()),
			   vals[as->index()]);
	    }
	    as->step();
	  }
	  delete as;
	  FREE (vals);
	}
	else {
	  Assert (0, "no ptype arrays");
	}
        bitset_free (b);
      }
      else {
	/* XXX: HERE: WORK ON THIS ONCE CONNECTIONS ARE DONE */
	warning ("Sparse array: Fix this please");
      }
    }
    else {
      /* nothing needed here, since this was not allocated at all */
    }

    delete old;
#if 0
    act_error_ctxt (stderr);
    warning ("Sparse array--FIXME, skipping right now!\n");
#endif    
  }
  else {
    Assert (s->Add (id, it), "Should succeed; what happened?!");
  }
}


void ActBody_Assertion::Expand (ActNamespace *ns, Scope *s)
{
  Expr *ex;

  ex = expr_expand (e, ns, s);
  if (ex->type != E_TRUE && ex->type != E_FALSE) {
    act_error_ctxt (stderr);
    fprintf (stderr, "Expression: ");
    print_expr (stderr, e);
    fprintf (stderr, "\n");
    fatal_error ("Not a Boolean constant!");
  }
  if (ex->type == E_FALSE) {
    act_error_ctxt (stderr);
    fprintf (stderr, "*** Assertion failed ***\n");
    fprintf (stderr, " assertion: ");
    print_expr (stderr, e);
    fprintf (stderr, "\n");
    if (msg) {
      char *s = Strdup (msg+1);
      s[strlen(s)-1] = '\0';
      fprintf (stderr, "   message: %s\n", s);
    }
    fatal_error ("Aborted execution on failed assertion");
  }
}

static int offset (act_connection **a, act_connection *c)
{
  int i;
  i = 0;
  while (1) {
    if (a[i] == c) return i;
    i++;
    if (i > 10000) return -1;
  }
  return -1;
}

static void print_id (act_connection *c)
{
  list_t *stk = list_new ();
  ValueIdx *vx;

  while (c) {
    stack_push (stk, c);
    if (c->vx) {
      c = c->parent;
    }
    else if (c->parent->vx) {
      c = c->parent->parent;
    }
    else {
      Assert (c->parent->parent->vx, "What?");
      c = c->parent->parent->parent;
    }
    
  }
  
  while (!stack_isempty (stk)) {
    c = (act_connection *) stack_pop (stk);
    if (c->vx) {
      vx = c->vx;
      printf ("%s", vx->u.obj.name);
    }
    else if (c->parent->vx) {
      vx = c->parent->vx;
      if (vx->t->arrayInfo()) {
	Array *tmp;
	tmp = vx->t->arrayInfo()->unOffset (offset (c->parent->a, c));
	printf ("%s", vx->u.obj.name);
	tmp->Print (stdout);
	delete tmp;
      }
      else {
	UserDef *ux;
	ux = dynamic_cast<UserDef *> (vx->t->BaseType());
	Assert (ux, "what?");
	printf ("%s.%s", vx->u.obj.name, ux->getPortName (offset (c->parent->a, c)));
      }
    }
    else {
      vx = c->parent->parent->vx;
      Assert (vx, "What?");
      
      Array *tmp;
      tmp = vx->t->arrayInfo()->unOffset (offset (c->parent->parent->a, c->parent));
      UserDef *ux;
      ux = dynamic_cast<UserDef *> (vx->t->BaseType());
      Assert (ux, "what?");

      printf ("%s", vx->u.obj.name);
      tmp->Print (stdout);
      printf (".%s", ux->getPortName (offset (c->parent->a, c)));

      delete tmp;
    }
    if (vx->global) {
      printf ("(g)");
    }
    if (!stack_isempty (stk)) {
      printf (".");
    }
  }
  list_free (stk);
}

#if 0
static void print_id (act_connection *c)
{
  printf ("<rev: ");

  while (c) {
    if (c->vx) {
      printf (".");
      if (c->vx->global) {
	char *s = c->vx->t->getNamespace()->Name();
	printf ("%s%s(g)", s, c->vx->u.obj.name);
	FREE (s);
      }
      else {
	UserDef *ux;
	printf ("{t:");
	ux = c->vx->t->getUserDef();
	if (ux) {
	  printf ("#%s#", ux->getName());
	}
	c->vx->t->Print (stdout);
	printf ("}%s", c->vx->u.obj.name);
      }
    }
    else {
      InstType *it;
      if (c->parent->vx) {
	/* one level: either x[] or x.y */
	it = c->parent->vx->t;
	if (it->arrayInfo()) {
	  printf ("[i:%d]", offset (c->parent->a, c));
	}
	else {
	  UserDef *ux = dynamic_cast<UserDef *>(it->BaseType());
	  Assert (ux, "What?");
	  printf (".%s", ux->getPortName (offset (c->parent->a, c)));
	}
      }
      else if (c->parent->parent->vx) {
	UserDef *ux;
	it = c->parent->parent->vx->t;
	/* x[].y */
	Assert (it->arrayInfo(), "What?");
	ux = dynamic_cast<UserDef *>(it->BaseType());
	Assert (ux, "What?");
	printf ("[i:%d]", offset (c->parent->parent->a, c->parent));
	printf (".%s", ux->getPortName(offset (c->parent->a, c)));
      }
      else {
	Assert (0, "What?");
      }
    }
    c = c->parent;
  }
  printf (">");
}
#endif

static void dump_conn (act_connection *c)
{
  act_connection *tmp, *root;

  root = c;
  while (root->up) root = root->up;

  tmp = c;

  printf ("conn: ");
  do {
    print_id (tmp);
    if (tmp == root) {
      printf ("*");
    }
    printf (" , ");
    tmp = tmp->next;
  } while (tmp != c);
  printf("\n");
}



static void mk_connection (UserDef *ux, const char *s1, act_connection *c1,
			   const char *s2, act_connection *c2)
{
  int p1, p2;
  act_connection *tmp;
  ValueIdx *vx1, *vx2;

#if 0
  printf ("connect: %s and %s\n", s1, s2);
  
  dump_conn (c1);
  dump_conn (c2);
#endif
  vx1 = (c1->vx ? c1->vx : (c1->parent->vx ? c1->parent->vx : c1->parent->parent->vx));
  Assert (vx1, "What");
  vx2 = (c2->vx ? c2->vx : (c2->parent->vx? c2->parent->vx : c2->parent->parent->vx));
  Assert (vx2, "Hmm...");

  if (vx1->global || vx2->global) {
    if (vx2->global && !vx1->global) {
      tmp = c1;
      c1 = c2;
      c2 = tmp;
      p1 = -1;
      p2 = -1;
    }
    else if (vx1->global && !vx2->global) {
      p1 = -1;
      p2 = -1;
    }
    else {
      p1 = 0;
      p2 = 0;
    }
  }
  else {
    if (ux) {
      /* user defined type */
      p1 = ux->FindPort (s1);
      p2 = ux->FindPort (s2);
      if (p1 > 0 || p2 > 0) {
	if (p2 > 0 && (p1 == 0 || (p2 < p1))) {
	  tmp = c1;
	  c1 = c2;
	  c2 = tmp;
	}
      }
    }
  }
  if (!ux || (p1 == 0 && p2 == 0)) {
    p1 = strlen (s1);
    p2 = strlen (s2);
    if (p2 > p1) {	
      tmp = c1;
      c1 = c2;
      c2 = tmp;
    }
    else if (p1 == p2) {
      p1 = strcmp (s1, s2);
      if (p1 > 0) {
	tmp = c1;
	c1 = c2;
	c2 = tmp;
      }
    }
  }
  
  /* c1 is the root, not c2 */
  while (c2->up) {
    c2 = c2->up;
  }
  c2->up = c1;

  act_connection *t1, *t2;

  /* merge c1, c2 connection ring */
  t1 = c1->next;
  t2 = c2->next;
  c1->next = t2;
  c2->next = t1;
}

void ActBody_Conn::Expand (ActNamespace *ns, Scope *s)
{
  Expr *e;
  AExpr *alhs, *arhs;
  ActId *ex;
  InstType *tlhs, *trhs;

  switch (type) {
  case 0:
    /*--  basic --*/

#if 0
    fprintf (stderr, "Conn: ");
    u.basic.lhs->Print (stderr);
    fprintf (stderr, " = ");
    u.basic.rhs->Print (stderr);
    fprintf (stderr, "\n");
#endif
    
    /* lhs */
    ex = u.basic.lhs->Expand (ns, s);
    Assert (ex, "What?");
    e = ex->Eval (ns, s, 1 /* it is an lval */);
    Assert (e, "What?");
    Assert (e->type == E_VAR, "Hmm...");
    tlhs = (InstType *)e->u.e.r;

    /* rhs */
    arhs = u.basic.rhs->Expand (ns, s);
    trhs = arhs->getInstType (s, 1);

    if (!type_connectivity_check (tlhs, trhs)) {
      act_error_ctxt (stderr);
      fprintf (stderr, "Connection: ");
      ex->Print (stderr);
      fprintf (stderr, " = ");
      arhs->Print (stderr);
      fprintf (stderr, "\n  LHS: ");
      tlhs->Print (stderr);
      fprintf (stderr, "\n  RHS: ");
      trhs->Print (stderr);
      fprintf (stderr, "\n");
      fatal_error ("Type-checking failed on connection");
    }

    if (TypeFactory::isParamType (tlhs)) {
      /* a parameter assignment */
      if (TypeFactory::isPTypeType (tlhs->BaseType())) {
	/* ptype assignment */
	AExprstep *astep = arhs->stepper();

	s->BindParam ((ActId *)e->u.e.l, astep->getPType());
	
	astep->step();
	Assert (astep->isend(), "What?");
	delete astep;
      }
      else {
	/* any other parameter assignment */
	s->BindParam ((ActId *)e->u.e.l, arhs);
      }
    }
    else {
      ActId *id = (ActId *)e->u.e.l;
      act_connection *lcx;
      act_connection *rcx;
      AExprstep *rhsstep = arhs->stepper();
      int done_conn;
      
      done_conn = 0;
      if (!id->arrayInfo() || id->arrayInfo()->isDeref()) {
	/* this is a direct connection */
	/* check for special case for rhs */
	if (rhsstep->isSimpleID()) {
	  int ridx;
	  ActId *rid;
	  int rsize;
	  
	  done_conn = 1;
	  
	  lcx = id->Canonical (s);
	  rhsstep->getID (&rid, &ridx, &rsize);
	  rcx = rid->Canonical (s);
	  if (ridx == -1) {
	    mk_connection (s->getUserDef(),
			   id->getName(), lcx,
			   rid->getName(), rcx);
	  }
	  else {
	    Assert (trhs->arrayInfo(), "What?");
	    Assert (rsize == trhs->arrayInfo()->size(), "What?");
	    if (!rcx->a) {
	      MALLOC (rcx->a, act_connection *, rsize);
	      for (int i=0; i < rsize; i++) {
		rcx->a[i] = NULL;
	      }
	    }
	    Assert (0 <= ridx && ridx < rsize, "What?");
	    if (!rcx->a[ridx]) {
	      NEW (rcx->a[ridx], act_connection);
	      rcx->a[ridx]->vx = NULL;
	      rcx->a[ridx]->parent = rcx;
	      rcx->a[ridx]->up = NULL;
	      rcx->a[ridx]->next = rcx->a[ridx];
	      rcx->a[ridx]->a = NULL;
	    }
	    rcx = act_mk_id_canonical (rcx->a[ridx]);
	    
	    mk_connection (s->getUserDef(),
			   id->getName(), lcx,
			   rid->getName(), rcx);
	  }
	}
      }
      if (!done_conn) {
	Arraystep *lhsstep = tlhs->arrayInfo()->stepper (id->arrayInfo());
	/* element by element array connection */

	while (!lhsstep->isend()) {
	  int lidx, lsize;
	  ActId *lid;
	  int ridx, rsize;
	  ActId *rid;
	  act_connection *lx, *rx;

	  lid = id;
	  lidx = lhsstep->index();
	  lsize = lhsstep->typesize();

	  rhsstep->getID (&rid, &ridx, &rsize);

	  lx = lid->Canonical (s);
	  rx = rid->Canonical (s);

	  if (lidx != -1) {
	    if (!lx->a) {
	      MALLOC (lx->a, act_connection *, lsize);
	      for (int i=0; i < lsize; i++) {
		lx->a[i] = NULL;
	      }
	    }
	    if (!lx->a[lidx]) {
	      NEW (lx->a[lidx], act_connection);
	      lx->a[lidx]->vx = NULL;
	      lx->a[lidx]->parent = lx;
	      lx->a[lidx]->up = NULL;
	      lx->a[lidx]->next = lx->a[lidx];
	      lx->a[lidx]->a = NULL;
	    }
	    lx = lx->a[lidx];
	  }
	  if (ridx != -1) {
	    if (!rx->a) {
	      MALLOC (rx->a, act_connection *, rsize);
	      for (int i=0; i < rsize; i++) {
		rx->a[i] = NULL;
	      }
	    }
	    if (!rx->a[ridx]) {
	      NEW (rx->a[ridx], act_connection);
	      rx->a[ridx]->vx = NULL;
	      rx->a[ridx]->parent = rx;
	      rx->a[ridx]->up = NULL;
	      rx->a[ridx]->next = rx->a[ridx];
	      rx->a[ridx]->a = NULL;
	    }
	    rx = rx->a[ridx];
	  }

	  mk_connection (s->getUserDef(),
			 lid->getName(), lx,
			 rid->getName(), rx);
	  
	  lhsstep->step();
	  rhsstep->step();
	}
	Assert (rhsstep->isend(), "What?");
	delete lhsstep;
      }
      delete rhsstep;
    }

    if (e) { FREE (e); }
    delete tlhs;
    delete trhs;
    delete arhs;

    break;
  case 1:
    /* aexpr */

#if 0
    fprintf (stderr, "Conn2: ");
    u.general.lhs->Print (stderr);
    fprintf (stderr, " = ");
    u.general.rhs->Print (stderr);
    fprintf (stderr, "\n");
#endif

    /* lhs */
    alhs = u.general.lhs->Expand (ns, s, 1); /* an lval */
    tlhs = alhs->getInstType (s, 1);

    /* rhs */
    arhs = u.basic.rhs->Expand (ns, s);
    trhs = arhs->getInstType (s, 1);

    if (!type_connectivity_check (tlhs, trhs)) {
      act_error_ctxt (stderr);
      fprintf (stderr, "Connection: ");
      ex->Print (stderr);
      fprintf (stderr, " = ");
      arhs->Print (stderr);
      fprintf (stderr, "\n  LHS: ");
      tlhs->Print (stderr);
      fprintf (stderr, "\n  RHS: ");
      trhs->Print (stderr);
      fprintf (stderr, "\n");
      fatal_error ("Type-checking failed on connection");
    }

    if (TypeFactory::isParamType (tlhs)) {
      /* a parameter assignment */

      if (TypeFactory::isPTypeType (tlhs->BaseType())) {
	/* ptype assignment */
	AExprstep *astep = arhs->stepper();

	s->BindParam (alhs->toid(), astep->getPType());
	
	astep->step();
	Assert (astep->isend(), "What?");
	delete astep;
      }
      else {
	ActId *lhsid;
	int lhsidx;
	AExprstep *aes = alhs->stepper();
	AExprstep *bes = arhs->stepper();
	/* any other parameter assignment */
	int ii = 0;
	while (!aes->isend()) {

	  aes->getID (&lhsid, &lhsidx, NULL);
	  if (lhsidx == -1) {
	    /* it's a pure ID */
	    s->BindParam (lhsid, bes);
	  }
	  else {
	    s->BindParam (lhsid, bes, lhsidx);
	  }
	  aes->step();
	  bes->step();
	}
	Assert (bes->isend(), "Should have been caught earlier!");
	delete aes;
	delete bes;
      }
    }
    else {
      AExprstep *aes, *bes;
      
      ActId *lid, *rid;
      int lidx, ridx;
      int lsize, rsize;
      act_connection *lx, *rx;
      
      /* 
	 check for direct connection id = id.
	 otherwise, we step
      */
      aes = alhs->stepper();
      bes = arhs->stepper();

      if (aes->isSimpleID() && bes->isSimpleID()) {
	/* a simple ID is either foo without array specifier,
	   or foo[complete deref] (i.e. not a subrange)
	*/

	aes->getID (&lid, &lidx, &lsize);
	bes->getID (&rid, &ridx, &rsize);

	lx = lid->Canonical (s);
	rx = rid->Canonical (s);

	mk_connection (s->getUserDef(), lid->getName(), lx,
		       rid->getName(), rx);
      }
      else {
	while (!aes->isend()) {
	  aes->getID (&lid, &lidx, &lsize);
	  bes->getID (&rid, &ridx, &rsize);

	  lx = lid->Canonical (s);
	  rx = rid->Canonical (s);

	  if (lidx != -1) {
	    if (!lx->a) {
	      MALLOC (lx->a, act_connection *, lsize);
	      for (int i=0; i < lsize; i++) {
		lx->a[i] = NULL;
	      }
	    }
	    if (!lx->a[lidx]) {
	      NEW (lx->a[lidx], act_connection);
	      lx->a[lidx]->vx = NULL;
	      lx->a[lidx]->parent = lx;
	      lx->a[lidx]->up = NULL;
	      lx->a[lidx]->next = lx->a[lidx];
	      lx->a[lidx]->a = NULL;
	    }
	    lx = lx->a[lidx];
	  }
	  if (ridx != -1) {
	    if (!rx->a) {
	      MALLOC (rx->a, act_connection *, rsize);
	      for (int i=0; i < rsize; i++) {
		rx->a[i] = NULL;
	      }
	    }
	    if (!rx->a[ridx]) {
	      NEW (rx->a[ridx], act_connection);
	      rx->a[ridx]->vx = NULL;
	      rx->a[ridx]->parent = rx;
	      rx->a[ridx]->up = NULL;
	      rx->a[ridx]->next = rx->a[ridx];
	      rx->a[ridx]->a = NULL;
	    }
	    rx = rx->a[ridx];
	  }

	  mk_connection (s->getUserDef(),
			 lid->getName(), lx,
			 rid->getName(), rx);
	  
	  aes->step();
	  bes->step();
	}
	Assert (bes->isend(), "What?");
      }
      delete aes;
      delete bes;
    }

    delete tlhs;
    delete trhs;
    delete arhs;
    delete alhs;
    
    break;
  default:
    fatal_error ("Should not be here");
    break;
  }
}

void ActBody_Loop::Expand (ActNamespace *ns, Scope *s)
{
  int ilo, ihi;
  ValueIdx *vx;
  Expr *e;
  ActBody *bi;
  
  Assert (t == ActBody_Loop::SEMI, "What loop is this?");

  Assert (s->Add (id, TypeFactory::Factory()->NewPInt()), "Should have been caught earlier");
  vx = s->LookupVal (id);
  vx->init = 1;
  vx->u.idx = s->AllocPInt();

  if (lo) {
    e = expr_expand (lo, ns, s);
    if (!expr_is_a_const (e)) {
      act_error_ctxt (stderr);
      print_expr (stderr, lo);
      fprintf (stderr, "\n");
      fatal_error ("Isn't a constant expression");
    }
    Assert (e->type == E_INT, "Should have been caught earlier");
    ilo = e->u.v;
    FREE (e);
  }

  e = expr_expand (hi, ns, s);
  if (!expr_is_a_const (e)) {
    act_error_ctxt (stderr);
    print_expr (stderr, hi);
    fprintf (stderr, "\n");
    fatal_error ("Isn't a constant expression");
  }
  Assert (e->type == E_INT, "Should have been caught earlier");
  ihi = e->u.v;
  FREE (e);

  if (!lo) {
    ilo = 0;
    ihi--;
  }

  for (; ilo <= ihi; ilo++) {
    s->setPInt (vx->u.idx, ilo);
    b->Expandlist (ns, s);
  }

  s->Del (id);
}

void ActBody_Select::Expand (ActNamespace *ns, Scope *s)
{
  ActBody_Select_gc *igc;
  ActBody *bi;
  Expr *guard;
  
  for (igc = gc; igc; igc = igc->next) {
    if (!igc->g) {
      /* else */
      igc->s->Expandlist (ns, s);
      return;
    }
    guard = expr_expand (igc->g, ns, s);
    if (!expr_is_a_const (guard)) {
      act_error_ctxt (stderr);
      print_expr (stderr, igc->g);
      fprintf (stderr, "\n");
      fatal_error ("Not a constant expression");
    }
    Assert (guard->type == E_TRUE || guard->type == E_FALSE,
	    "Should have been caught earlier");
    if (guard->type == E_TRUE) {
      igc->s->Expandlist (ns, s);
      return;
    }
  }
  /* all guards false, skip it */
  act_error_ctxt (stderr);
  warning ("All guards in selection are false.");
}

void ActBody_Lang::Expand (ActNamespace *ns, Scope *s)
{
  switch (t) {
  case ActBody_Lang::LANG_PRS:
    prs_expand ((act_prs *)lang, ns, s);
    break;

  case ActBody_Lang::LANG_CHP:
  case ActBody_Lang::LANG_HSE:
    chp_expand ((act_chp *)lang, ns, s);
    break;
  default:
    fatal_error ("Unknown language");
    break;
  }
}

void ActBody_Namespace::Expand (ActNamespace *_ns, Scope *s)
{
  /* expand the top-level of the namespace that was imported */
  ns->Expand ();
}


void ActBody::Expandlist (ActNamespace *ns, Scope *s)
{
  ActBody *b;

  for (b = this; b; b = b->Next()) {
    b->Expand (ns, s);
  }
}