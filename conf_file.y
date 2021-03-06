%{

#include <sys/types.h>

typedef void* yyscan_t;

#include "../config_helpers.h"
#include "../conf_file.h"
#include "conf_file.parser.h"
#include "conf_file.lexer.h"

static void conf_error(yyscan_t _scanner, PinotamsConfig *_cfg, char *_error)
{

}

%}

%union{
  ConfParam param;
  char *str;
  int val;
}

%define api.pure
%define api.prefix {conf_}
%parse-param {yyscan_t _scanner}
%lex-param {yyscan_t _scanner}
%parse-param {PinotamsConfig *_cfg}

%token<param> TOKEN_PARAM
%token<str> TOKEN_STRING
%token<val> TOKEN_VALUE
%start confFile

%%

confFile
: assignment
| confFile assignment
;

assignment
: TOKEN_PARAM '=' TOKEN_STRING ';' {
    switch($1)
    {
    case confLocations:
      addStrToVector(_cfg->locations, $3);
      free($3);
      break;
    case confFilter:
      addStrToVector(_cfg->filters, $3);
      free($3);
      break;
    case confApiKey:
      _cfg->apiKey = $3;
      break;
    case confSmtpServer:
      _cfg->smtpServer = $3;
      break;
    case confSmtpUser:
      _cfg->smtpUser = $3;
      break;
    case confSmtpPwd:
      _cfg->smtpPwd = $3;
      break;
    case confSmtpSender:
      _cfg->smtpSender = $3;
      break;
    case confSmtpSenderName:
      _cfg->smtpSenderName = $3;
      break;
    case confSmtpRecipient:
      addStrToVector(_cfg->smtpRecipients, $3);
      free($3);
      break;
    default:
      YYERROR;
    }
  }
| TOKEN_PARAM '=' TOKEN_VALUE ';' {
    switch($1)
    {
    case confRefreshRate:
      _cfg->refreshRate = $3 * 60;

      if (_cfg->refreshRate < 3600)
        _cfg->refreshRate = 3600;

      break;
    case confDebugLog:
      _cfg->debugLog = $3;
      break;
    case confSmtpPort:
      _cfg->smtpPort = $3;
      break;
    case confSmtpTLS:
      _cfg->smtpTLS = ($3 != 0);
      break;
    default:
      YYERROR;
    }
  }
| error ';'
;

%%
