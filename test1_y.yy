%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern void exit(int);
%}



/* Definition des tokens du langage */
%token FUNCTION
%token TIME
%token INTEGER
%token ENDFILE
%token ENDLINE

%%
INPUT: ENDLINE INPUT {printf ("Once\n"); }
    | ENDFILE {printf ("Twice\n");}
 
%%

struct chained_list {
	int call;
	double time;
	char* name;
	struct chained_list* next;
};

struct chained_list* new_element (const int call, const double time, char* name, struct chained_list* list)
{
	struct chained_list* new_list = malloc (sizeof (struct chained_list));
	new_list->call = call;
	new_list->time = time;
	new_list->name = name;
	new_list->next = list;
	return new_list;
}

struct chained_list* del_list (struct chained_list* list)
{
	struct chained_list* ptr;
	
	ptr = list;
	while (ptr != NULL)
	{
		list = list->next;
		free (ptr);
		ptr = list;
	}

	return NULL;
}

double call_time (const int call, struct chained_list* list)
{
	double sum = 0.0;
	struct chained_list* ptr = list;
	
	while (ptr != NULL && ptr->call > call)
	{
		sum += ptr->time;
		ptr = ptr->next;
	}

	return sum;
}

struct chained_list* global_list = NULL;

int yyerror(const char *str)
{
        fprintf(stderr,"Ouch an error occurs in the parsing: %s\n",str);
        exit(0);
}

int yywrap()
{
   printf ("Fin de l'analyse syntaxique \n");
   return 1;
}

int main (int argc, char ** argv)
{
	yyparse ();

	return 0;
}
