#include <stdio.h>
#include <stdlib.h>
#include "liste_chainee.h"
#include "../tools/tools.h"


int main()
{
    coordonate coord = {.x= 4, .y= 6};

    List list = new_list();
    
    printf("test 1\n");
    print_list(list); //=============

    list = push_back_list(list, coord);
    list = push_front_list(list, (coordonate){24, 60});
    list = push_front_list(list, coord);
    
    printf("test 2\n");
    print_list(list);   //=============

    pop_list_value(list, (coordonate){24,60});

    List list2 = new_list();
    list2 = push_front_list(list2, coord);
    list2 = push_front_list(list2, coord);

    printf("test 3\n");
    printf("il y a %d valeur ([%d],[%d]) dans la liste \n", value_repetition(list, coord), coord.x, coord.y );
    
    printf("test 4\n");
    print_list(list2);   //=============

    list = add_list(list, list2);

    printf("test 5\n");
    print_list(list);   //=============

    list = pop_back_list(list);
    list = pop_list(list, 2);

    printf("test 6\n");
    print_list(list);   //=============


    list = clear_list(list);
    
    printf("test 7\n");
    print_list(list);   //=============
    return 0;

}

