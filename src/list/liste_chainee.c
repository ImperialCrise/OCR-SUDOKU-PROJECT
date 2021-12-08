#include <stdio.h>
#include <stdlib.h>
#include "liste_chainee.h"
#include "../tools/tools.h"


List new_list(void){

	return NULL;
}

void print_list(List list){

	if (is_empty_list(list))
	{
		printf("La liste est vide. \n");
		return;
	}

	while(list!=NULL){
		printf("([%d],[%d])  ", list->value.x, list->value.y);
		list = list-> next;
	}
	printf("\n");
}

Bool is_empty_list(List list){
	if (list == NULL)
		return true;

	return false;
}


int list_length(List list){
	
	int size = 0;
	if (!is_empty_list(list))
		while(list != NULL)
		{
			size ++;
			list = list -> next; 
		}

	return size;
}

/**
 * ajoute un element a la fin
 * */
List push_back_list(List list, coordonate x){
	
	ListElement *element = malloc(sizeof(*element));

	if (element == NULL)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire" );
		exit(EXIT_FAILURE);
	}

	element->value = x;
	element->next = NULL;

	if(is_empty_list(list))
		return element;

	// variable temporaire qui permet d'eviter de deplacer le pointeur list sur elle meme. 
	//On garde le meme debut

	ListElement *temp;

	temp = list;

	while( temp-> next != NULL)
		temp = temp-> next; // typiquement ici

	temp->next = element; 

	free(element);
	return list;

}

/**
 * ajoute un element au debut
 * */
List push_front_list(List list, coordonate x){
	
	ListElement *element = malloc(sizeof(*element));

	if (element== NULL)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}

	element->value = x;

	if (is_empty_list(list))  
		element->next = NULL;
	else
		element->next = list;

	free(list);
	return element;
}

/**
 * supprime l'element a l'index index
 * */
List pop_list(List list, int index){
	if (is_empty_list(list) || list_length(list) < (index+1))
	{
		fprintf(stderr, "%s\n", "Erreur index non present dans la liste. \n" );
		exit(EXIT_FAILURE);
	}

	ListElement *temp = list;
	ListElement *before = list;

	for (int i = 0; i < index; i++)
	{
		before = temp;
		temp = temp->next;
	}

	//on connecte le precedent avec le suivant
	before->next = temp->next;
	free(temp);
	temp = NULL;

	return list;
}

/**
 * supprime le dernier element de la liste
 * */
List pop_back_list(List list){
	if (is_empty_list(list))
		return new_list();

	if(list->next == NULL)
	{
		free(list);

		list = NULL;
		return new_list();
	}

	ListElement *temp = list;
	ListElement *before = list;

	while(temp -> next != NULL)
	{
		before = temp;
		temp = temp->next;
	}

	before->next = NULL;
	free(temp);
	temp = NULL;

	return list;
}


/**
 * supprime l'element en debut de liste
 * */
List pop_front_list(List list){

	if (is_empty_list(list))
		return new_list();

	// on cree une liste qui recupere la liste sans le premier
	ListElement *element = malloc(sizeof(*element));

	if (element== NULL)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}

	element = list->next;

	//=======================================

	free(list);
	list = NULL;


	return element;

}

/**
 * concatene deux listes
 * */
List add_list(List list, List list2){
	if (is_empty_list(list))
	{
		if (is_empty_list(list2))
			return new_list();
		
		else
			return list2;
	}
	if (is_empty_list(list2))
		return list;

	ListElement *temp = list;

	while(temp -> next != NULL)
		temp = temp->next;


	temp->next = list2;
	return list;
	
}


List clear_list(List list){

	while(list != NULL)
		list = pop_front_list(list);
	
	return list;
}

List pop_list_value(List list, coordonate value){
	if (is_empty_list(list))
	{
		fprintf(stderr, "%s\n", "Erreur index non present dans la liste. \n" );
		exit(EXIT_FAILURE);
	}

	ListElement *temp = list;
	ListElement *before = list;

	for (int i = 0, len = list_length(list);
		i < len && is_coordonate_equal(&(temp -> value), &value) == false ; i++)
	{
		before = temp;
		temp = temp->next;
	}

	if (is_coordonate_equal(&(temp -> value), &value) == true)
	{
		//on connecte le precedent avec le suivant
		before->next = temp->next;
		//free(temp);
		temp = NULL;
	}


	return list;
}

int value_repetition(List list, coordonate value){

	ListElement *temp = list;
	int res = 0;

	while(temp != NULL)
	{
		if (is_coordonate_equal(&(temp->value), &value) == true)
			res ++;
		temp = temp->next;
	}
	
	return res;
}