#ifndef H_LISTE_CHAINEE
#define H_LISTE_CHAINEE
#include "../tools/tools.h"

/*definition d'une liste*/
typedef struct ListElement
{
    coordonate value;
    struct ListElement *next;
}ListElement, *List;


/*prototypes*/
List new_list(void);
void print_list(List list);
Bool is_empty_list(List list);
int list_length(List list);
List push_back_list(List list, coordonate value);
List push_front_list(List list, coordonate value);
List add_list(List list, List list2);

List pop_back_list(List list);
List pop_front_list(List list);
List pop_list(List list, int x);
List clear_list(List list);

List pop_list_value(List list, coordonate value);


int value_repetition(List list, coordonate value);


#endif
