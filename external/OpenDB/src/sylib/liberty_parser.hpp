/* A Bison parser, made by GNU Bison 3.4.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_LIBERTY_PARSER_HOME_AHMED_GIT_PERSONAL_THE_OPENROAD_PROJECT_SCALE_LAB_PHYKNIGHT_EXTERNAL_OPENDB_SRC_SYLIB_LIBERTY_PARSER_HPP_INCLUDED
# define YY_LIBERTY_PARSER_HOME_AHMED_GIT_PERSONAL_THE_OPENROAD_PROJECT_SCALE_LAB_PHYKNIGHT_EXTERNAL_OPENDB_SRC_SYLIB_LIBERTY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int liberty_parser_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    COMMA = 258,
    SEMI = 259,
    LPAR = 260,
    RPAR = 261,
    LCURLY = 262,
    RCURLY = 263,
    COLON = 264,
    KW_DEFINE = 265,
    KW_DEFINE_GROUP = 266,
    KW_TRUE = 267,
    KW_FALSE = 268,
    NUM = 269,
    STRING = 270,
    IDENT = 271
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 39 "/home/ahmed/Git/Personal/The-OpenROAD-Project/scale-lab/PhyKnight/external/OpenDB/src/sylib/liberty_parser.y"

	char *str;
	double num;
	liberty_attribute_value *val;
		

#line 81 "/home/ahmed/Git/Personal/The-OpenROAD-Project/scale-lab/PhyKnight/external/OpenDB/src/sylib/liberty_parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE liberty_parser_lval;

int liberty_parser_parse (void);

#endif /* !YY_LIBERTY_PARSER_HOME_AHMED_GIT_PERSONAL_THE_OPENROAD_PROJECT_SCALE_LAB_PHYKNIGHT_EXTERNAL_OPENDB_SRC_SYLIB_LIBERTY_PARSER_HPP_INCLUDED  */
