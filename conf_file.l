%option prefix="conf_"
%option noyywrap
%option never-interactive
%option reentrant
%option bison-bridge

%{

#include <string.h>
#include "../config_helpers.h"
#include "../conf_file.h"
#include "conf_file.parser.h"

#define YYSTYPE CONF_STYPE

%}

INT         [0-9]+
COMMENT     #.*$
WS          [\v\r\n\t ]+

%x STR

%%

"\"" { BEGIN(STR); }

<STR>[^\"]+ { 
  yylval->str = strndup(yytext, yyleng);
  return TOKEN_STRING;
}

<STR>"\"" { BEGIN(INITIAL); }

{INT} {
  yylval->val = atoi(yytext);
  return TOKEN_VALUE;
}

"locations" {
  yylval->param = confLocations;
  return TOKEN_PARAM;
}

"api-key" {
  yylval->param = confApiKey;
  return TOKEN_PARAM;
}

"refresh-rate" {
  yylval->param = confRefreshRate;
  return TOKEN_PARAM;
}

"smtp-server" {
  yylval->param = confSmtpServer;
  return TOKEN_PARAM;
}

"smtp-port" {
  yylval->param = confSmtpPort;
  return TOKEN_PARAM;
}

"smtp-user" {
  yylval->param = confSmtpUser;
  return TOKEN_PARAM;
}

"smtp-pwd" {
  yylval->param = confSmtpPwd;
  return TOKEN_PARAM;
}

"smtp-sender" {
  yylval->param = confSmtpSender;
  return TOKEN_PARAM;
}

"smtp-sender-name" {
  yylval->param = confSmtpSenderName;
  return TOKEN_PARAM;
}

"smtp-recipient" {
  yylval->param = confSmtpRecipient;
  return TOKEN_PARAM;
}

"smtp-tls" {
  yylval->param = confSmtpTLS;
  return TOKEN_PARAM;
}

"=" { return '='; }

";" { return ';'; }

{COMMENT} { }

{WS} { }

%%
