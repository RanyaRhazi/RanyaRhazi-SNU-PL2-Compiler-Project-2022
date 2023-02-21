//--------------------------------------------------------------------------------------------------
/// @brief SnuPL parser
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/07 Bernhard Egger adapted to SnuPL/0
/// 2014/11/04 Bernhard Egger maintain unary '+' signs in the AST
/// 2016/04/01 Bernhard Egger adapted to SnuPL/1 (this is not a joke)
/// 2019/09/15 Bernhard Egger added support for constant expressions
/// 2020/07/31 Bernhard Egger adapted to SnuPL/2
/// 2020/09/27 Bernhard Egger assignment 2: parser for SnuPL/-1
///
/// @section license_section License
/// Copyright (c) 2012-2022, Computer Systems and Platforms Laboratory, SNU
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without modification, are permitted
/// provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice, this list of condi-
///   tions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice, this list of condi-
///   tions and the following disclaimer in the documentation and/or other materials provided with
///   the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
/// IMPLIED WARRANTIES,  INCLUDING, BUT NOT LIMITED TO,  THE IMPLIED WARRANTIES OF MERCHANTABILITY
/// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
/// CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
/// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//--------------------------------------------------------------------------------------------------

#include <limits.h>
#include <cassert>
#include <errno.h>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <exception>
#include <string.h>

#include "parser.h"
#include "symtab.h"
using namespace std;

//--------------------------------------------------------------------------------------------------
// EBNF of SnuPL/-1
//   module        =  statSequence "."
//   digit         =  "0".."9".
//   letter        =  "a".."z".
//   factOp        =  "*" | "/".
//   termOp        =  "+" | "-".
//   relOp         =  "=" | "#".
//   factor        =  digit | "(" expression ")".
//   term          =  factor { factOp factor }.
//   simpleexpr    =  term { termOp term }.
//   expression    =  simpleexpr [ relOp simpleexpr ].
//   assignment    =  letter ":=" expression.
//   statement     =  assignment.
//   statSequence  =  [ statement { ";" statement } ].
//   whitespace    =  { " " | \n }+.

// Global Symbol Table
CSymtab GST();

//--------------------------------------------------------------------------------------------------
// CParser
//
CParser::CParser(CScanner *scanner)
{
  _scanner = scanner;
  _module = NULL;
}

CAstNode* CParser::Parse(void)
{
  _abort = false;

  if (_module != NULL) { delete _module; _module = NULL; }

  try {
    if (_scanner != NULL) _module = module();
  } catch (...) {
    _module = NULL;
  }

  return _module;
}

const CToken* CParser::GetErrorToken(void) const
{
  if (_abort) return &_error_token;
  else return NULL;
}

string CParser::GetErrorMessage(void) const
{
  if (_abort) return _message;
  else return "";
}

void CParser::SetError(CToken t, const string message)
{
  _error_token = t;
  _message = message;
  string throwmessage = message;
  throwmessage.append(" [at ");
  throwmessage.append(to_string(_scanner->GetLineNumber()));
  throwmessage.append(":");
  throwmessage.append(to_string(_scanner->GetCharPosition()));
  throwmessage.append("]");
  _abort = true;
  throw message;
}

bool CParser::Consume(EToken type, CToken *token)
{
  if (_abort) return false;

  CToken t = _scanner->Get();

  if (t.GetType() != type) {
    SetError(t, "expected '" + CToken::Name(type) + "', got '" +
             t.GetName() + "'");
  }

  if (token != NULL) *token = t;

  return t.GetType() == type;
}

void CParser::InitSymbolTable(CSymtab *st)
{
  // TODO (phase 2)
  // add predefined functions here
  //_module->SetSymbolTable(st);
}

CAstModule* CParser::module(void)
{
  //
  // module ::= "module" ident ";" { constDeclaration | varDeclaration | subroutineDecl } [ "begin" statSequence ] "end" ident ".".
  //
  CToken t0;
  Consume(tModule, NULL);
  Consume(tIdent, &t0);
  CAstModule *m = new CAstModule(t0, t0.GetValue());
  Consume(tSemicolon, NULL);
  // TODO From here
  CAstStatement *statseq = NULL;

  InitSymbolTable(m->GetSymbolTable());

  statseq = statSequence(m);
  Consume(tDot);

  m->SetStatementSequence(statseq);

  return m;
}

CAstStatement* CParser::statSequence(CAstScope *s)
{
  //
  // statSequence ::= [ statement { ";" statement } ].
  // statement ::= assignment.
  //
  // FIRST(statSequence) = { tLetter }
  // FOLLOW(statSequence) = { tDot }
  //
  // FIRST(statement) = { tLetter }
  // FOLLOW(statement) = { tSemicolon, tDot }
  //

  // The linking of statement sequences is a bit akward here because
  // we implement statSequence as a loop and not recursively.
  // We keep a 'head' that points to the first statement and is finally
  // returned at the end of the function. Head can be NULL if no statement
  // is present. 
  // In the loop, we track the end of the linked list using 'tail' and
  // attach new statements to that tail.
  CAstStatement *head = NULL;

  if (_scanner->Peek().GetType() != tDot) {
    CAstStatement *tail = NULL;

    do {
      CAstStatement *st = NULL;

      switch (_scanner->Peek().GetType()) {
        // statement ::= assignment
        case tLetter:
          st = assignment(s);
          break;

        default:
          SetError(_scanner->Peek(), "statement expected.");
          break;
      }

      assert(st != NULL);
      if (head == NULL) head = st;
      else tail->SetNext(st);
      tail = st;

      if (_scanner->Peek().GetType() == tDot) break;

      Consume(tSemicolon);
    } while (!_abort);
  }

  return head;
}

CAstStatAssign* CParser::assignment(CAstScope *s)
{
  //
  // assignment ::= letter ":=" expression.
  // to 
  CToken t;

  CAstDesignator *lhs = qualident(s);
  Consume(tAssign, &t);

  CAstExpression *rhs = expression(s);

  return new CAstStatAssign(t, lhs, rhs);
}

CAstExpression* CParser::expression(CAstScope* s)
{
  //
  // expression ::= simpleexpr [ relOp simpleexpr ].
  //
  CToken t;
  EOperation relop;
  CAstExpression *left = NULL, *right = NULL;

  left = simpleexpr(s);

  if (_scanner->Peek().GetType() == tRelOp) {
    Consume(tRelOp, &t);
    right = simpleexpr(s);

    if (t.GetValue() == "=")       relop = opEqual;
    else if (t.GetValue() == "#")  relop = opNotEqual;
    else if (t.GetValue() == "<=") relop = opLessEqual;
    else if (t.GetValue() == ">=") relop = opBiggerEqual;
    else if (t.GetValue() == "<")  relop = opLessThan;
    else if (t.GetValue() == ">")  relop = opBiggerThan;
    else SetError(t, "invalid relation.");

    return new CAstBinaryOp(t, relop, left, right);
  } else {
    return left;
  }
}

CAstExpression* CParser::simpleexpr(CAstScope *s)
{
  //
  // simpleexpr ::= ["+"|"-"] term { termOp term }.
  // 
  CToken t;

  CAstExpression *n = NULL;
  if(_scanner->Peek().GetType() == tPlusMinus){
    Consume(tPlusMinus, &t);
    n = new CAstUnaryOp(t, t.GetValue() == "+" ? opPos : opNeg, term(s));
  }
  else n = term(s);

  while (_scanner->Peek().GetType() == tPlusMinus || _scanner->Peek().GetType() == tLogicOR) {
    
    CAstExpression *l = n, *r;

    Consume(_scanner->Peek().GetType(), &t);

    r = term(s);

    EOperation op;
    if(t.GetValue() == "||") op = opOr;
    else if(t.GetValue() == "+") op = opAdd;
    else if(t.GetValue() == "-") op = opSub;
    else SetError(t, "Encountered non-termOp (||, +, -) token while parsing simpleexpr");

    n = new CAstBinaryOp(t, op, l, r);
  }


  return n;
}

CAstExpression* CParser::term(CAstScope *s)
{
  //
  // term ::= factor { ("*"|"/") factor }.
  //
  CAstExpression *n = NULL;

  n = factor(s);

  EToken tt = _scanner->Peek().GetType();

  while (tt == tMulDiv) {
    CToken t;
    CAstExpression *l = n, *r;

    Consume(tMulDiv, &t);

    r = factor(s);

    n = new CAstBinaryOp(t, t.GetValue() == "*" ? opMul : opDiv, l, r);

    tt = _scanner->Peek().GetType();
  }

  return n;
}

CAstFunctionCall* CParser::subroutinecall(CAstScope *s, CToken prev){
  //
  // subroutineCall  ::= ident "(" [ expression {"," expression} ] ")"
  //
  CToken t;
  CAstExpression* ni;
  string errormsg;


  // consume identifier if not provided
  if(prev == CToken()) Consume(tIdent, &t);
  else t = prev;
  CSymtab* ST = s->GetSymbolTable();

  //const CSymProc* sym = ST->FindSymbol(t.GetValue()); // This is CSymbol*, but how to get CSymProc* from FindSymbol? Is there something else? (TODO later)
  const CSymProc* sym = new CSymProc(t.GetValue(), CTypeManager::Get()->GetInteger()); // Type is temporary, this has to be redone at some point!
  CAstFunctionCall* n = new CAstFunctionCall(t, sym);
  
  Consume(tLBrak, NULL);

  while(true){
    ni = expression(s);
    n->AddArg(ni);
    if(_scanner->Peek().GetType() == tComma) Consume(tComma, NULL);
    else break;
  }

  Consume(tRBrak, NULL);


}

CAstExpression* CParser::factor(CAstScope *s)
{
  //
  // factor ::= qualident | number | boolean | char | string | "(" expression ")" | subroutineCall | "!" factor.
  //
  // as of -1: FIRST(factor) = { tDigit, tLBrak }
  //

  CToken t;
  CAstExpression *n = NULL;
  EToken tokentype = _scanner->Peek().GetType();
  

  if(tokentype == tIdent){
    Consume(tIdent, &t);

    if(_scanner->Peek().GetType() == tLBrak){
      n = subroutinecall(s, t);
    }
    // qualident
    else{
      n = qualident(s, t);
    }
  }
  else if (tokentype == tNumber){
    n = number();
  }
  else if (tokentype == tTrue || tokentype == tFalse){
    n = boolean();
  }
  else if (tokentype == tCharConst){
    Consume(tCharConst, &t);
    n = new CAstStringConstant(t, t.GetValue(), s);
  }
  else if (tokentype == tStringConst){
    Consume(tStringConst, &t);
    n = new CAstStringConstant(t, t.GetValue(), s);
  }
  else if (tokentype == tLBrak){
    Consume(tLBrak, NULL);
    n = expression(s);
    n->SetParenthesized(true);
    Consume(tRBrak, NULL);
  }
  else if (tokentype == tLogicNOT){
    CAstExpression *n2;
    Consume(tLogicNOT, NULL);
    n2 = expression(s);
    n = new CAstUnaryOp(t, opNot, n2);
  }
  // switch (_scanner->Peek().GetType()) {
  //   // factor ::= qualident - starts with ident
  //   case tIdent:
  //     n = number();
  //     break;

  //   // factor ::= "(" expression ")"
  //   case tLBrak:
  //     Consume(tLBrak);
  //     n = expression(s);
  //     Consume(tRBrak);
  //     break;

  //   default:
  //     SetError(_scanner->Peek(), "factor expected.");
  //     break;
  // }

  return n;
}

CAstDesignator* CParser::identifier(CAstScope *s)
{
  //
  // identifier
  //

  CToken t;
  CSymtab *st = s->GetSymbolTable();

  Consume(tIdent, &t);

  // check if symbol exists in (local) symbol table
  const CSymbol *sym = st->FindSymbol(t.GetValue(), sLocal);

  if (sym == NULL) {
    // if not, create one and add it to the symbol table
    CSymbol *nsym = s->CreateVar(t.GetValue(), CTypeManager::Get()->GetInteger());
    st->AddSymbol(nsym);

    sym = nsym;
  }

  return new CAstDesignator(t, sym);
}

CAstDesignator* CParser::qualident(CAstScope *s, CToken prev){
  //
  // qualident ::= ident { "[" simpleexpr "]" }
  // ident may have been consumed already - that ident part is in prev token.
  //
  CToken t;
  CSymtab *st = s->GetSymbolTable();

  // consume if ident is in the token stream, if already consumed it is provided as an argument
  if (prev == CToken()) Consume(tIdent, &t);
  else t = prev;

  const CSymbol *sym = st->FindSymbol(t.GetValue(), sLocal);
    if (sym == NULL) {
    // if not, create one and add it to the symbol table
    CSymbol *nsym = s->CreateVar(t.GetValue(), CTypeManager::Get()->GetInteger());
    st->AddSymbol(nsym);

    sym = nsym;
    }

  // if array. Otherwise, same as identifier
  if(_scanner->Peek().GetType() == tLBrakSQ){

    CAstArrayDesignator *arrayn = new CAstArrayDesignator(t, sym);
    CAstExpression *idxexp = NULL; // expression for index

    while(_scanner->Peek().GetType() == tLBrakSQ){
      Consume(tLBrakSQ, NULL);
      
      idxexp = simpleexpr(s);
      arrayn->AddIndex(idxexp);

      if(!Consume(tRBrakSQ, &t)) SetError(t, "no closing square bracket");
    }
    return arrayn;
  }

  return new CAstDesignator(t, sym);
}

CAstConstant* CParser::number(void)
{
  //
  // number ::= integer or longint
  //

  CToken t;

  Consume(tNumber, &t);
  errno = 0;
  long long v = strtoll(t.GetValue().c_str(), NULL, 10);
  if (errno != 0) SetError(t, "invalid number");
  
  if(t.GetValue().back() == 'L') return new CAstConstant(t, CTypeManager::Get()->GetLongint(), v); // longint if last character is 'L'
  else return new CAstConstant(t, CTypeManager::Get()->GetInteger(), v);
}
CAstConstant* CParser::boolean(void)
{
  //
  // boolean: true or false
  //

  CToken t = _scanner->Peek(); // check the next token, type must be 'tTrue' or 'tFalse'

  long long v;

  if(t.GetType() == tTrue) {
    Consume(tTrue, &t);
    v = 1;
  }
  else if (t.GetType() == tFalse) {
    Consume(tFalse, &t);
    v = 0;
  }
  else SetError(t, "invalid boolean.");
  
  return new CAstConstant(t, CTypeManager::Get()->GetBool(),  v);
}

CAstProcedure* CParser::subroutinedecl(CAstScope* s){
  // subroutineDecl    = (procedureDecl | functionDecl) ( "extern" | subroutineBody ident ) ";".
  // procedureDecl     = "procedure" ident [ formalParam ] ";".
  // functionDecl      = "function" ident [ formalParam ] ":" type ";".
  // formalParam       = "(" [ varDeclSequence ] ")".
  // subroutineBody    = constDeclaration varDeclaration "begin" statSequence "end".
  // varDeclSequence   = varDecl { ";" varDecl }.
  // varDecl           = ident { "," ident } ":" type.

  CToken t = _scanner->Peek(); // peeked token
  CToken t1 = _scanner->Peek(); // first token (function or procedure)
  CToken itoken; // identifier for function/procedure name (retained to check whether it matches identifier at the end)
  CSymProc* symprocedure;
  CType* returntype;
  CAstProcedure* proc_scopenode;
  // procedureDecl
  if(t1.GetType() == tProcedure){
    Consume(tProcedure, NULL);
  }
  // functionDecl
  else{
    Consume(tFunction, NULL);
  }

  // in both cases, consume identifier and create CSymProc symbol and CAstProcedure node
  // from this point onwards, the scope is proc_scopenode, not s.
  Consume(tIdent, &itoken);
  symprocedure = new CSymProc(itoken.GetValue(), CTypeManager::Get()->GetNull());
  proc_scopenode = new CAstProcedure(t, itoken.GetValue(), s, symprocedure);
  
  // if formalParam exists, get parameters from varDeclSequence
  t = _scanner->Peek();
  if(t.GetType() == tLBrak){
    Consume(tLBrak, NULL);
    if(t.GetType() == tRBrak) Consume(tRBrak, NULL); // no varDeclSequence, just brackets
    else {
      // continue parsing variable declarations as long as there is a semicolon
      while(true){
        vardecl(proc_scopenode, symprocedure);
        
        t = _scanner->Peek();
        if(t.GetType() != tSemicolon) break;
        else Consume(tSemicolon, NULL);
      }

      Consume(tRBrak, NULL);
    }
  }

  // if procedure, only need to consume semicolon
  if(t1.GetType() == tProcedure){
    Consume(tSemicolon, NULL);
  }
  // if function, ":" type ";"
  else{
    Consume(tColon, NULL);
    symprocedure->SetDataType(type(proc_scopenode)->GetType());
    Consume(tSemicolon, NULL);
  }



  if(_scanner->Peek().GetType() == tExtern){
    symprocedure->SetExternal(true);
  }
  else{
    constdeclaration(proc_scopenode);
    vardeclaration(proc_scopenode);
    CAstStatement* statementseq = statSequence(proc_scopenode);
    Consume(tBegin, NULL);
    proc_scopenode->SetStatementSequence(statementseq);
    Consume(tEnd, NULL);

    Consume(tIdent, &t);
    if(t.GetValue() != itoken.GetValue()) SetError(t, "Identifier mismatch in subroutine declaration");
  }

  Consume(tSemicolon, NULL);

  return proc_scopenode;
}

const CType* CParser::vardecl(CAstScope* s, CSymProc* p){
  // add variables declared to the symbol table of scope s
  // varDecl           = ident { "," ident } ":" type
  
  CToken t, idt; // t is peeking token, idt is identifier
  const CType* ctype;
  bool typeknown = false;

  Consume(tIdent, &idt);
  if(_scanner->Peek().GetType() == tComma){
    // what is remaining after removing [ident ","] is still a vardecl as there is at least one more ident remaining
    Consume(tComma, NULL);
    ctype = vardecl(s);
    typeknown = true;
  }
  // if there is no comma and type is not known, colon then type is followed. Return that type to calling instance, as the type will have to be used in all of iterations.
  if(!typeknown){
    t = _scanner->Peek();
    if(t.GetType() == tColon) Consume(tColon, NULL);
    // get type
    else if (t.GetType() == tBoolean || t.GetType() == tChar || t.GetType() == tInteger || t.GetType() == tLongInt){
      CAstType* tmp = type(s);
      ctype = tmp->GetType();
      typeknown = true;
      delete tmp; // AST node of type is unnecessary (maybe? this may have to be modified later)
    }
  }
  
  if(!typeknown) SetError(idt, "type is not known after variable declaration");
  // after the type is known, create symbol with corresponding type
      CSymbol *nsym = s->CreateVar(idt.GetValue(), ctype);
      s->GetSymbolTable()->AddSymbol(nsym);
      if(p){
        CSymParam *symparam = new CSymParam(p->GetNParams(), idt.GetValue(), ctype);
        p->AddParam(symparam);
      }
      return ctype;
  
}

CAstType* CParser::type(CAstScope* s){
  // for the time being, does not consider array types (only consumes tokens)
  // type              = basetype | type "[" [ simpleexpr ] "]".
  CToken t, base;
  const CType* tp;
  base = _scanner->Peek();
  if(base.GetType() == tBoolean) tp = CTypeManager::Get()->GetBool();
  else if(base.GetType() == tChar) tp = CTypeManager::Get()->GetChar();
  else if(base.GetType() == tInteger) tp = CTypeManager::Get()->GetInteger();
  else if (base.GetType() == tLongInt) tp = CTypeManager::Get()->GetLongint();


  Consume(base.GetType(), &t);
  // array
  while(_scanner->Peek().GetType() == tLBrakSQ){
    Consume(tLBrakSQ, NULL);
    simpleexpr(s);
    Consume(tLBrakSQ, NULL);
  }

  CAstType* n = new CAstType(base, tp);
}

