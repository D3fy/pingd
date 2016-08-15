%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include "config.h"

_CONFIG_T *config;

int  yylex(void);
void yyerror(char *str, ...);
int  yyval;
int  yyparse();

%}


%union
{
	double  d;
	char   *string;
	int     i;
}

%token <d> DECIMAL;
%token <i> INT;
%token <string> STRING;
%token <string> LOG_FACILITY;
%token <string> LOG_LEVEL;
%token <string> ADDRESS;

%token PIDFILE;
%token USER;
%token GROUP;
%token LOG;
%token HOSTS;
%token LOGLEVEL;
%token FACILITY;

%%

configuration:
	| configuration config
	| configuration LOG optional_eol '{' log_section '}'
	;

config:
	  PIDFILE STRING { config->pidfile = $2; }
	| USER    STRING { config->user    = $2; }
	| GROUP   STRING { config->group   = $2; }
	;

log_section:
	| log_section log_statement
	;

log_statement:
	  LOGLEVEL LOG_LEVEL    { config->log.level    = $2; }
	| FACILITY LOG_FACILITY { config->log.facility = $2; }
	;

optional_eol:
	| optional_eol '\n'
	;

%%


void yyerror(char *str, ...)
{
	fprintf(stderr, "error: %s\n", str);
	extern int yylineno;
	fprintf (stderr, "configuration file line: %d\n", yylineno);
}

int yywrap()
{
	return 1;
}

int parse_config_file(_CONFIG_T *config, const char *path)
{
	// parse the configuration file and store the results in the structure referenced
	// error messages are output to stderr
	// Returns: 0 for success, otherwise non-zero if an error occurred
	//
	extern FILE *yyin;
	extern int yylineno;

	config = malloc(sizeof(_CONFIG_T));
	memset(config, 0, sizeof(_CONFIG_T));

	yyin = fopen (path, "r");
	if (yyin == NULL) {
		fprintf (stderr, "can't open configuration file %s: %s\n", path, strerror(errno));
		return 1;
	}

	yylineno = 1;
	if (yyparse()) {
		fclose(yyin);
		return 1;
	} else {
		fclose(yyin);
		return 0;
	}
}
