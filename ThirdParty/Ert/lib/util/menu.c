/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'menu.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/menu.h>
#include <ert/util/vector.h>

/**
   This file implements a simple character based menu system. The menu
   consists of a list of items, where each item has a description, a
   function to call, an argument to send to this function, and a set
   of keys which will invoke this function.


   Example:

   menu_type * menu = menu_alloc("Tittel paa menyen" , "qQ");  
   menu_add_item(menu , "Alternativ 1" , "1aA" , func1 , arg1);
   menu_add_item(menu , "Alternativ 2" , 2Bb" , func2 , arg2);
   menu_run(menu);
   menu_free(menu);

   In this case we will get a menu looking like this:


      Tittel paa menyen
      1: Alternativ 1
      2: Alternativ 2
      q: Quit
      ==> __

   Now typing '1' will invoke the function func1() with argument
   arg1. In addition the characters 'a' and 'A' can also be used to
   invoke the same function. Function func2(arg2) is invoked with '2',
   'b' or 'B'; finally the menu is exited with 'q' or 'Q' (the
   arguments to the alloc call). 


   Observe that (in the current implementation) only single characters
   are allowed as activator keys for the various items. I.e. 'save' to
   save is not an option'
*/





struct menu_item_struct {
  bool             separator;    /* If this is a separator - in that case all the following fields are moot. */ 
  bool             helptext;     /* If this is help text - Only label is present  */
  char           * key_set;      /* The characters which will activate this item , e.g. "sS" - must be a \0 terminated string */
  char           * label;        /* The label/description of this menu item */
  menu_func_type * func;         /* The function called when this item is activated. */
  void           * arg;          /* The argument passed to func. */
  int              label_length; /* The length of the label - zero for separators. */
  arg_free_ftype * free_arg;     /* Destructor for the argument - will typically be NULL */
  bool             enabled;
};




struct menu_struct {
  vector_type     * items;             /* Vector of menu_item_type instances */
  char            * quit_keys;         /* The keys which can be used to quit from this menu - typically "qQ" */
  char            * title;             /* The title of this menu */
  char            * quit_label;        /* The text printed on the back/quit item at the bottom. */
  char            * complete_key_set;  /* A string containing all the allowed characters - to validata input */
};



/**
   Free's the menu occupied by one menu item.
*/
static void menu_item_free(menu_item_type * item) {
  if (!item->separator) {
    free(item->label);
  }
  if (!item->separator && !item->helptext){
    free(item->key_set); 
  }


  if (item->free_arg != NULL)
    item->free_arg(item->arg);

  free(item);
} 

static void menu_item_free__(void * arg) {
  menu_item_free( (menu_item_type *) arg );
}



/**
   This function returns true if the first argument contains _any_ of the characters in
   the second argument. It is implemented by repeated calss to strchr();
*/

static bool __string_contains(const char * string , const char * char_set) {
  bool contains = false;
  int i = 0;
  do {
    if (strchr(string , char_set[i]) != NULL) 
      contains = true;
    i++;
  } while (!contains && i < strlen(char_set));
  return contains;
}



/**
   This function allocates an empty menu, with title 'title'. This
   particular menu will exit when one the keys in quit_keys is
   entered. */


menu_type * menu_alloc(const char * title , const char * quit_label , const char * quit_keys) {
  menu_type * menu = (menu_type*)util_malloc(sizeof * menu );
  
  menu->title     = util_alloc_sprintf_escape( title , 0 );
  menu->quit_keys = util_alloc_string_copy( quit_keys );
  menu->items     = vector_alloc_new();
  menu->complete_key_set = util_alloc_string_copy( quit_keys );
  menu->quit_label = util_alloc_string_copy( quit_label );
  
  return menu;
}

static menu_item_type * menu_item_alloc_empty() {
  menu_item_type * item = (menu_item_type*)util_malloc(sizeof * item );

  item->label   = NULL; 
  item->key_set = NULL; 
  item->func    = NULL;
  item->arg     = NULL;
  item->separator    = false;
  item->helptext     = false;
  item->label_length = 0;
  item->free_arg = NULL;
  item->enabled  = true;

  return item;
}


/**
   Low level function doing the actual append of a complete item.
*/
static void menu_append_item__(menu_type * menu ,  menu_item_type * item) {
  vector_append_owned_ref( menu->items , item , menu_item_free__);
}


void menu_set_title(menu_type * menu, const char * title) {
  menu->title = util_realloc_string_copy(menu->title, title);
}


void menu_item_disable( menu_item_type * item ) {
  item->enabled = false;
}

void menu_item_enable( menu_item_type * item ) {
  item->enabled = true;
}


char menu_item_get_key( const menu_item_type * item ) {
  if (item->enabled)
    return item->key_set[0];
  else
    return '-';
}


void menu_item_call( const menu_item_type * item ) {
  if (item->enabled)
    item->func(item->arg);
}



void menu_item_set_label(menu_item_type * item , const char * label) {
  item->label = util_realloc_string_copy(item->label , label);
  item->label_length = strlen(item->label);
}

/**
  Adds (appends) an item to the menu. 


  Obsereve that several keys can be used to invoke a particular
  function, however *only* the first key in the key_set is displayed
  when the menu is printed on stdout.
*/

menu_item_type * menu_add_item(menu_type * menu , const char * label , const char * key_set , menu_func_type * func, void * arg , arg_free_ftype * free_arg) {
  if (__string_contains(menu->complete_key_set , key_set)) 
    util_abort("%s:fatal error when building menu - key(s) in:%s already in use \n",__func__ , key_set);
  {
    menu_item_type * item = menu_item_alloc_empty();
    item->key_set = util_alloc_string_copy(key_set);
    item->func    = func;
    item->arg     = arg;
    item->separator = false;
    item->helptext = false;
    item->free_arg  = free_arg;
    menu_append_item__(menu , item);
    menu_item_set_label(item , label);
    menu->complete_key_set = util_strcat_realloc(menu->complete_key_set , key_set);
    return item;
  }
}




/**
   Will add a '-------------' line to the menu.
*/
void menu_add_separator(menu_type * menu) {
  menu_item_type * item = menu_item_alloc_empty();
  item->separator = true;
  menu_append_item__(menu , item);
}


/**
   Will print helptext to screen
*/

void menu_add_helptext(menu_type * menu, const char * label ) {
  menu_item_type * item = menu_item_alloc_empty();
  item->helptext = true;
  menu_append_item__(menu , item);
  menu_item_set_label(item , label);
}





/** level == 0 : top line
    level == 1 : separator line 
    level == 2 : bottom line
*/

static void __print_line(int l , int level) {                                                                
  int i;
  if (level == 0)
    fputc('/' , stdout);
  else if (level == 1)
    fputc('|' , stdout);
  else
    fputc('\\' , stdout);

  
  for (i=1; i < (l - 1); i++)
    fputc('-' , stdout);


  if (level == 0)
    fputc('\\' , stdout);
  else if (level == 1)
    fputc('|' , stdout);
  else
    fputc('/' , stdout);


  fputc('\n' , stdout);
}
  
static void __print_sep(int l) {
  int i;
  printf("| ");
  for (i=0; i < l; i++)
    fputc('-' , stdout);
  printf(" |\n");
}


static void __print_helptext(char * label, int l){
  bool end_reached = false;
  char * label_copy = util_alloc_string_copy( label );
  char * first_part = "Dummy3";
  char * second_part = "Dummy4";
  while(!end_reached){
    int i;
    if(strlen(label_copy) > l){
      util_binary_split_string_from_max_length(label_copy , " ", l , &first_part , &second_part);
      printf("|    %s",first_part);
      for (i=strlen(first_part); i < l; i++)
        fputc(' ' , stdout);
      printf("    |\n");
      label_copy = util_realloc_string_copy(label_copy, second_part);
    }
    else{
      printf("|    %s",label_copy);
      for (i=strlen(label_copy); i < l; i++)
        fputc(' ' , stdout);
      printf("    |\n");
      end_reached = true;
    }
  }
}




static void menu_display(const menu_type * menu) {
  int i;
  int length = strlen(menu->title);
  for (i = 0; i < vector_get_size(menu->items); i++) {
    const menu_item_type * item = (const menu_item_type*)vector_iget_const( menu->items , i);
    if(!item->helptext)
      length = util_int_max(length , item->label_length);
    if(item->helptext)
      length = util_int_max(length , 60); /* Hardcoded length for helptext*/
  }


  printf("\n");
  __print_line(length + 10 , 0);
  printf("| ");   
  util_fprintf_string(menu->title , length + 6 , center_pad , stdout);  printf(" |\n");
  __print_line(length + 10 , 1);
  for (i=0; i < vector_get_size(menu->items); i++) {
    const menu_item_type * item = (const menu_item_type*)vector_iget_const( menu->items , i);
    if (item->separator) 
      __print_sep(length + 6);
    else if (item->helptext)
      __print_helptext(item->label,length);
    else {
      printf("| %c: ", menu_item_get_key( item ));
      util_fprintf_string(item->label , length + 3 , right_pad , stdout);
      printf(" |\n");
    }
  }
  __print_sep(length + 6);
  printf("| %c: ",menu->quit_keys[0]);
  util_fprintf_string(menu->quit_label , length + 3 , right_pad , stdout);
  printf(" |\n");
  __print_line(length + 10 , 2);
  printf("\n");
}



/**
   Reads a string from stdin: If the string is longer than one single
   character it is discarded, if it is exactly one character long we
   check if it is in the menus set of available command characters,
   and return it *IF* it is a valid character. I.e. the return value
   from this function is *GUARANTEED* to correspond to a menu item.

   Observe that the function ends with a call to getchar() - this
   should remove the traling <RETURN> from the stdin input buffer.
*/

static int menu_read_cmd(const menu_type * menu) { 
  char cmd[256]; 
  
  do { 
    printf("==> ");
    fflush(stdout); fscanf(stdin , "%s" , cmd); /* We read a full string -
                                                   but we only consider it if it is exactly *ONE* character long. */
  } while ((strchr(menu->complete_key_set , cmd[0]) == NULL) || strlen(cmd) > 1);
  
  getchar(); /* Discards trailing <RETURN> from standard input buffer? */
  return cmd[0];
}




menu_item_type * menu_get_item(const menu_type * menu, char cmd) {
  int item_index = 0;
  menu_item_type * item = NULL;
  while (item_index < vector_get_size(menu->items)) {
    menu_item_type * current_item = (menu_item_type*)vector_iget(menu->items , item_index);
    if (!current_item->separator || !current_item->helptext) {
      if (strchr(current_item->key_set , cmd) != NULL) {
        item = current_item;
        break;
      }
      item_index++;
    }
  }

  if (item == NULL)
    util_abort("%s: could not locate item with key: %c \n",__func__ , cmd);
  return item;
}



void menu_run(const menu_type * menu) {
  while (1) {
    int cmd;
    

    menu_display(menu);
    cmd = menu_read_cmd(menu);
    if (strchr(menu->quit_keys , cmd) != NULL) /* We have recieved a quit command - leave the building. */
      break;

    /*
      OK - we start looking through all the available commands to see
      which this is. */
    {
      int item_index = 0;
      while (1) {
        const menu_item_type * item = (const menu_item_type*)vector_iget_const(menu->items , item_index);
        if (!item->separator) {
          if(!item->helptext) {
            if (strchr(item->key_set , cmd) != NULL) {
              /* Calling the function ... */
              menu_item_call( item );
              break;
            }
          }
        }
        item_index++;
      }
    }
  }
}



void menu_free(menu_type * menu) {
  free(menu->quit_keys);
  free(menu->title);
  free(menu->complete_key_set);
  free(menu->quit_label);
  vector_free(menu->items);
  free(menu);
}
