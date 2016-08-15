/*
 * Copyright 2015 Dan Molik <dan@danmolik.com>
 *
 * This file is part of Shitty Little Server
 *
 * Shitty Little Server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Shitty Little Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Shitty Little Server.  If not, see <http://www.gnu.org/licenses/>.
 */


%{
#include <signal.h>
#include "config.h"
#include "parse.h"
%}

%option yylineno
%option noinput nounput

%x LIST

LOG_TYPE     (?i:console|syslog|file)
LOG_LEVEL    (?i:warn|warning|error|notice|debug|info|crit|critical|emerg|emergency)
LOG_FACILITY (?i:kern|user|mail|daemon|auth|syslog|lpr|news|uucp|clock|12|13|14|authpriv|ftp|cron|local[0-7])
DECIMAL      ([0-9]+\.[0-9]+)
INT          ([0-9]+)
STRING       ([0-9A-Za-z_\/:\.]+)
ADDRESS      (((1?[0-9][0-9]?|2[0-4][0-9]|25[0-5])\.){3}(1?[0-9][0-9]?|2[0-4][0-9]|25[0-5]))

%%

<INITIAL>{
	/* options */
	pidfile  { return PIDFILE; }
	user     { return USER; }
	group    { return GROUP; }
	level    { return LOGLEVEL; }
	facility { return FACILITY; }
	log      { return LOG; }
	hosts    { return HOSTS; }
}

<LIST>{
	/* start the list macro */
	/* "{" {
	      BEGIN INITIAL;
	      return *yytext;
	    }
	*/
	"}" {
	      BEGIN INITIAL;
	      return *yytext;
	    }
	\n* /* suppress new lines */;
}

<INITIAL,LIST>{
	/* pull all the values from the list */
	{LOG_LEVEL} {
	              yylval.string = strdup(yytext);
	              return LOG_LEVEL;
	            }
	{LOG_FACILITY} {
	                 yylval.string = strdup(yytext);
	                 return LOG_FACILITY;
	               }
	{ADDRESS} {
	            yylval.string = strdup(yytext);
	            return ADDRESS;
	          }
	{DECIMAL} {
	            yylval.d = atof(yytext);
	            return DECIMAL;
	          }
	{INT}     {
	            yylval.i = atoi(yytext);
	            return INT;
	          }
	{STRING}  {
	            yylval.string = strdup(yytext);
	            return STRING;
	          }
	#.*       /* ignore comments */;
	[\n\r]*   /* ignore new lines */;
	[\t ]*    /* ignore whitespace */;
	\\\n      /* ignore linebreaks */;
	.|\n      { return *yytext; }
}

%%
