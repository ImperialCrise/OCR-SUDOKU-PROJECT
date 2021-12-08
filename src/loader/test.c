#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "../loader/loader.h"
#include "../grid_detection2/grid_detection.h"
#include "../buildgrid/buildgrid.h"
#include "../buildgrid/SDL_rotozoom.h"
#include "../solver/solver.h"

#define PATH_TMP0 "./datas/tmp/original.bmp"
#define PATH_TMP1 "./datas/tmp/greyscale.bmp"
#define PATH_TMP2 "./datas/tmp/sauvola.bmp"
#define PATH_TMP3 "./datas/tmp/floodfill.bmp"
#define PATH_TMP4 "./datas/tmp/grid.bmp"
#define PATH_TMP5 "./datas/tmp/rotation.bmp"
#define PATH_TMP6 "./datas/tmp/cutting.bmp"
#define PATH_TMP7 "./datas/tmp/cutting_rotated.bmp"
#define PATH_TMP8 "./datas/tmp/ocr.bmp"
#define PATH_TMP9 "./datas/tmp/solved.bmp"
#define PATH_TMP_GRILLE "./datas/tmp/grille"
#define M_PI 3.14159265358979323846

Bool file_exists (char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

typedef struct OCR
{
	int add;
	int added_loaded;
	int page_actuel;
	int rotationAngle;

	GtkScale* scaler;
	GtkWindow* window;
	GtkImage* imager;

	GtkButton* load_button;
	GtkButton* reset_button;
	GtkFileChooser* file_button;

	GtkButton* left_rotate_button;
	GtkButton* right_rotate_button;

	GtkButton* original_button;
	GtkButton* greyscale_button;
	GtkButton* sovola_button;
	GtkButton* floodfill_button;
	GtkButton* grid_button;
	GtkButton* autorotation_button;
	GtkButton* cutting_button;
	GtkButton* ocr_button;
	GtkButton* solving_button;

	GdkPixbuf* original_pixbuf;
	GdkPixbuf* greyscale_pixbuf;
	GdkPixbuf* sovola_pixbuf;
	GdkPixbuf* floodfill_pixbuf;
	GdkPixbuf* grid_pixbuf;
	GdkPixbuf* autorotation_pixbuf;
	GdkPixbuf* cutting_pixbuf;
	GdkPixbuf* ocr_pixbuf;
	GdkPixbuf* solving_pixbuf;
} OCR;

void set_active(GtkButton* button, gboolean b){
	gtk_widget_set_sensitive(GTK_WIDGET(button), b);
}

void set_active_all(OCR* ocr, gboolean blue)
{
	set_active(ocr->reset_button, blue);
	set_active(ocr->load_button, !blue);
	set_active(ocr->original_button, blue);

	set_active(ocr->greyscale_button, blue);
	set_active(ocr->sovola_button, blue);
	set_active(ocr->floodfill_button, blue);
	set_active(ocr->grid_button, blue);
	set_active(ocr->autorotation_button, blue);
	set_active(ocr->cutting_button, blue);
	set_active(ocr->ocr_button, blue);
	set_active(ocr->solving_button, blue);

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);
}

void reset(OCR* ocr){
	set_active_all(ocr, FALSE);
	gtk_image_set_from_file(ocr->imager, NULL);
	set_active(ocr->load_button, FALSE);
	ocr->added_loaded = 0;
	ocr->rotationAngle = 0;
}

void refresh(OCR* ocr){
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->reset_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->original_button));

	gtk_widget_queue_draw(GTK_WIDGET(ocr->greyscale_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->sovola_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->floodfill_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->grid_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->autorotation_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->cutting_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->ocr_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->solving_button));
}

void on_reset(GtkButton *button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;

	reset(ocr);

	gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), TRUE);
	gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ocr->file_button));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->window));

	//RemovePrevImg
	if (file_exists(PATH_TMP1))
		remove(PATH_TMP1);
	
	if (file_exists(PATH_TMP2))
		remove(PATH_TMP2);
	
	if (file_exists(PATH_TMP3))
		remove(PATH_TMP3);
	
	if (file_exists(PATH_TMP4))
		remove(PATH_TMP4);
	
	if (file_exists(PATH_TMP5))
		remove(PATH_TMP5);
	
	if (file_exists(PATH_TMP6))
		remove(PATH_TMP6);

	if (file_exists(PATH_TMP7))
		remove(PATH_TMP7);

	if (file_exists(PATH_TMP8))
		remove(PATH_TMP8);

	if (file_exists(PATH_TMP9))
		remove(PATH_TMP9);

	DIR *d;
	struct dirent *dir;
	char buf[200]; 
	d = opendir(PATH_TMP_GRILLE);
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			realpath(dir->d_name, buf);
			remove(buf);
		}
		closedir(d);
	}
}

GdkPixbuf* load_pixbuf(GdkPixbuf* pix, char* filename){
	GError* err = NULL;
	pix = gdk_pixbuf_new_from_file_at_scale(filename,800, 500, TRUE,&err);

	if (err != NULL)
	{
		g_printerr("Error loading file: %s\n", err->message);
		g_clear_error(&err);
		return NULL;
	}

	return pix;
}

SDL_Surface* zoomimage(SDL_Surface* grille, SDL_Rect* position){
	SDL_Surface* grille2 = SDL_CreateRGBSurface(0, position->w, position->h, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(grille, position, grille2, NULL) != 0)
	{
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}	

	SDL_FreeSurface(grille);
	return grille2;
}

void autorotate(SDL_Surface **image, SDL_Surface **grille, int* width, int* height, SDL_Rect* position, Uint32* white_color, coordonate* coord_11, coordonate* coord_12, coordonate*coord_21, coordonate* coord_22)
{
	if(shouldrotate(*grille))
	{
		double angle_rot = angularRotation(*grille);
		*grille = rotozoomSurface(*grille, angle_rot, 1, 0);
		*image = rotozoomSurface(*image, angle_rot, 1, 0);

		int w = (*grille)->w;
		int h = (*grille)->h;
		floodfill_black_plus1(*grille, w, h, 0, 0, white_color);
		floodfill_black_plus2(*grille, w, h, w-1, 0, white_color);
		floodfill_black_minus1(*grille, w, h, w-1, h-1, white_color);
		floodfill_black_minus2(*grille, w, h, 0, h-1, white_color);

		getcoord(*grille, w, h, 0, coord_11, coord_12, coord_21, coord_22);
	}

	position->w = coord_21->x - coord_11->x;
	position->h = position->w;
	position->x = coord_11->x;
	position->y = coord_11->y;

	*width = position->w + 1;
	*height = position->h + 1;

	*grille = zoomimage(*grille, position);
	*image = zoomimage(*image, position);
}

void load_ocr(SDL_Surface **grille, Uint32* white_color, char tab[9][10], TTF_Font* Sans)
{
	SDL_FreeSurface(*grille);
	*grille = SDL_CreateRGBSurface(0, 500, 500, 32, 0, 0, 0, 0);

	Uint32 pixel = SDL_MapRGB((*grille)->format, 246, 177, 100);
	SDL_Color color = {0,0,0,0}, bgcolor = {246,177,100,0};
	SDL_Surface *text_surface;

	SDL_Rect bloc = {0,0,500,500};
	SDL_Rect message = {26,15,0,0};

	SDL_FillRect(*grille, &bloc, *white_color);

	char c[2] = "";

	bloc.w = 55;
	bloc.h = 55;

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] != ' ')
			{
				c[0] = tab[i][j];
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				bloc.y = message.y - 9;
				bloc.x = message.x - 19;
				SDL_FillRect(*grille, &bloc, pixel);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 5 ? 1 : 0);
	}

	drawSudoku(*grille, 500, 500);
}

void solve_ocr(SDL_Surface **grille, char tab[9][10], TTF_Font* Sans)
{
	int sudoku[9][9];

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			sudoku[i][j] = tab[i][j] == ' ' ? 0 : tab[i][j] - '0';

	solveSudoku(sudoku, 0, 0);

	SDL_Color color = {0,0,0,0}, bgcolor = {0xff,0xff,0xff,0};
	SDL_Surface *text_surface;

	SDL_Rect message = {26,15,0,0};
	char c[2] = "";

	for (int i = 0; i < 9; ++i){
		message.x = 25;
		for (int j = 0; j < 9; ++j)
		{
			message.x += (j > 6 ? 2 : 0);
			if (tab[i][j] == ' ')
			{
				c[0] = sudoku[i][j] + '0';
				text_surface = TTF_RenderText_Shaded(Sans, c, color, bgcolor);
				SDL_BlitSurface(text_surface, NULL, *grille, &message);
				SDL_FreeSurface(text_surface);
			}
			message.x += 54 ;
		}	
		message.y += 54 + (i > 4 ? 1 : 0);
	}
}

void load_file(OCR* ocr, char* filename){

	SDL_Surface *image = IMG_Load(filename);
	
	int width = image->w;
	int height = image->h;

	SDL_Surface* grille = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};
	
	//GreyScale
	toGreyScale(image);
	SDL_SaveBMP(image, PATH_TMP1);
	ocr->greyscale_pixbuf = load_pixbuf(ocr->greyscale_pixbuf, PATH_TMP1);

	//Sauvola
	SauvolaThresholding(image);
	SDL_SaveBMP(image, PATH_TMP2);
	ocr->sovola_pixbuf = load_pixbuf(ocr->sovola_pixbuf, PATH_TMP2);

	//FloodFill
	color(image, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, ocr->added_loaded * 50);
	SDL_SaveBMP(image, PATH_TMP3);
	ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

	//Grid
	returngrid(image, grille, width, height, &colormax, &white_color);
	SDL_SaveBMP(grille, PATH_TMP4);
	ocr->grid_pixbuf = load_pixbuf(ocr->grid_pixbuf, PATH_TMP4);

	//Autorotation
	SDL_Rect position = {0,0,0,0};
	autorotate(&image, &grille, &width, &height, &position, &white_color, &coord_11, &coord_12, &coord_21, &coord_22);
	SDL_SaveBMP(grille, PATH_TMP5);
	SDL_FreeSurface(grille);
	ocr->autorotation_pixbuf = load_pixbuf(ocr->autorotation_pixbuf, PATH_TMP5);

	//Cutting
	grille = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_Rect bloc = {0,0,width,height}; SDL_FillRect(grille, &bloc, white_color);
	cutting(image, grille, &position, &white_color);
	grille = rotozoomSurfaceXY(grille, 0, (double)500/width, (double)500/(double)height, 0);
	drawSudoku(grille, grille->w, grille->h);
	SDL_SaveBMP(grille, PATH_TMP6);
	SDL_SaveBMP(grille, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	char tab[9][10];
	strcpy(tab[0], "7 89    2");
	strcpy(tab[1], "513  2  8");
	strcpy(tab[2], " 9231   7");
	strcpy(tab[3], " 5  3 9  ");
	strcpy(tab[4], "16  2  75");
	strcpy(tab[5], "  9 4  6 ");
	strcpy(tab[6], "9   8421 ");
	strcpy(tab[7], "2  6  749");
	strcpy(tab[8], "4    15 3");

	//OCR
	TTF_Font* Sans = TTF_OpenFont("./datas/font_sans.ttf", 24);
	load_ocr(&grille, &white_color, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP8);
	ocr->ocr_pixbuf = load_pixbuf(ocr->ocr_pixbuf, PATH_TMP8);

	//resolving
	solve_ocr(&grille, tab, Sans);
	SDL_SaveBMP(grille, PATH_TMP9);
	ocr->solving_pixbuf = load_pixbuf(ocr->solving_pixbuf, PATH_TMP9);

	TTF_CloseFont(Sans);
	SDL_FreeSurface(grille);
	SDL_FreeSurface(image);
}


void set_page(OCR *ocr, int page)
{
	ocr->page_actuel = page;
	set_active_all(ocr, TRUE);

	if (page == 1)
	{
		set_active(ocr->original_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	}
	else if (page == 2)
	{
		set_active(ocr->greyscale_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->greyscale_pixbuf);
	}else if (page == 3)
	{
		set_active(ocr->sovola_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->sovola_pixbuf);
	}else if (page == 4)
	{
		set_active(ocr->floodfill_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->floodfill_pixbuf);
	}else if (page == 5)
	{
		set_active(ocr->grid_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->grid_pixbuf);
	}else if (page == 6)
	{
		set_active(ocr->autorotation_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->autorotation_pixbuf);
	}else if (page == 7)
	{
		set_active(ocr->cutting_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
		set_active(ocr->left_rotate_button, TRUE);
		set_active(ocr->right_rotate_button, TRUE);
	}else if (page == 8)
	{
		set_active(ocr->ocr_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->ocr_pixbuf);
	}else if (page == 9)
	{
		set_active(ocr->solving_button, FALSE);
		gtk_image_set_from_pixbuf(ocr->imager, ocr->solving_pixbuf);
	}

	refresh(ocr);
}

void on_page_click1(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 1);}
void on_page_click2(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 2);}
void on_page_click3(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 3);}
void on_page_click4(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 4);}
void on_page_click5(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 5);}
void on_page_click6(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 6);}
void on_page_click7(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 7);}
void on_page_click8(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 8);}
void on_page_click9(GtkButton* button, gpointer user_data){(void) button; set_page((OCR*) user_data, 9);}


void on_left_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle -= 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}

void on_right_click(GtkButton* button, gpointer user_data)
{
	(void) button;
	OCR *ocr = user_data;
	ocr->rotationAngle += 90;

	SDL_Surface* image = IMG_Load(PATH_TMP6);
	image = rotation(image, ocr->rotationAngle);
	SDL_SaveBMP(image, PATH_TMP7);
	ocr->cutting_pixbuf = load_pixbuf(ocr->cutting_pixbuf, PATH_TMP7);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->cutting_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	SDL_FreeSurface(image);
}


void on_load_click(GtkButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	char* filename = PATH_TMP0;
	ocr->added_loaded = ocr->add;

	set_active(ocr->left_rotate_button, FALSE);
	set_active(ocr->right_rotate_button, FALSE);

	if (gtk_widget_get_sensitive(GTK_WIDGET(ocr->sovola_button)))
	{
		SDL_Surface *image = IMG_Load(PATH_TMP2);
		int width = image->w;
		int height = image->h;
		coordonate coord_11 = {-1, -1};
		Uint32 colormax = 0;

		color(image, width, height, &colormax, &coord_11, &coord_11, &coord_11, &coord_11, ocr->added_loaded * 50);
		SDL_SaveBMP(image, PATH_TMP3);
		ocr->floodfill_pixbuf = load_pixbuf(ocr->floodfill_pixbuf, PATH_TMP3);

		set_active(button, FALSE);
		set_page(ocr, ocr->page_actuel);
		SDL_FreeSurface(image);
	}
	else
	{
		load_file(ocr, filename);

		set_active(ocr->reset_button, TRUE);
		set_active(button, FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(ocr->file_button), FALSE);

		set_page(ocr, 1);
	}
}

void on_scaler(GtkRange *range, gpointer user_data)
{
	OCR *ocr = user_data;
	ocr->add = ((int) gtk_range_get_value(range));
	gtk_widget_queue_draw(GTK_WIDGET(ocr->scaler));

	if (ocr->added_loaded != 0)
	{
		set_active(ocr->load_button, ocr->add != ocr->added_loaded);
		gtk_widget_queue_draw(GTK_WIDGET(ocr->load_button));
	}
}

void on_file_choose(GtkFileChooserButton* button, gpointer user_data)
{
	OCR *ocr = user_data;
	SDL_Surface* image = IMG_Load(gtk_file_chooser_get_filename(ocr->file_button));
	SDL_SaveBMP(image, PATH_TMP0);

	ocr->original_pixbuf = load_pixbuf(ocr->original_pixbuf, PATH_TMP0);

	if (ocr->original_pixbuf != NULL)
	{
		set_active(ocr->load_button, TRUE);
		set_active(ocr->reset_button, FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);

	gtk_image_set_from_pixbuf(ocr->imager, ocr->original_pixbuf);
	gtk_widget_queue_draw(GTK_WIDGET(ocr->imager));

	//char str[100] = "OCR - Project S3 | ";
	//strcat(str, gtk_file_chooser_get_current_name(ocr->file_button));
	//gtk_window_set_title(ocr->window, str);
	SDL_FreeSurface(image);

}

int main(int argc, char *argv[])
{
	(void) argc; (void) argv;

	gtk_init(NULL,NULL);
	TTF_Init();

	GtkBuilder* builder = gtk_builder_new();
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "src/UI/main.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	OCR ocr =
	{
		.scaler = GTK_SCALE(gtk_builder_get_object(builder, "add_scale")),
		.window = GTK_WINDOW(gtk_builder_get_object(builder, "org.gtk.ocr")),
		.imager = GTK_IMAGE(gtk_builder_get_object(builder, "imager")),

		.load_button = GTK_BUTTON(gtk_builder_get_object(builder, "load_button")),
		.reset_button = GTK_BUTTON(gtk_builder_get_object(builder, "reset_button")),
		.file_button = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_button")),

		.original_button = GTK_BUTTON(gtk_builder_get_object(builder, "original")),
		.greyscale_button = GTK_BUTTON(gtk_builder_get_object(builder, "greyscale")),
		.sovola_button = GTK_BUTTON(gtk_builder_get_object(builder, "sovola")),
		.floodfill_button = GTK_BUTTON(gtk_builder_get_object(builder, "floodfill")),
		.grid_button = GTK_BUTTON(gtk_builder_get_object(builder, "grid")),
		.autorotation_button = GTK_BUTTON(gtk_builder_get_object(builder, "autorotation")),
		.cutting_button = GTK_BUTTON(gtk_builder_get_object(builder, "cutting")),
		.ocr_button = GTK_BUTTON(gtk_builder_get_object(builder, "ocr")),
		.solving_button = GTK_BUTTON(gtk_builder_get_object(builder, "solving")),

		.left_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button")),
		.right_rotate_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button")),
	};

	gtk_scale_set_value_pos(ocr.scaler, GTK_POS_BOTTOM);
	gtk_range_set_range(GTK_RANGE(ocr.scaler), 1, 10);
	ocr.add = 1;
	
	reset(&ocr);
	gtk_window_set_title(ocr.window, "OCR - Project S3");
	
	g_signal_connect(ocr.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(GTK_RANGE(ocr.scaler), "value-changed", G_CALLBACK(on_scaler), &ocr);
	g_signal_connect(ocr.load_button, "clicked", G_CALLBACK(on_load_click), &ocr);
	g_signal_connect(ocr.reset_button, "clicked", G_CALLBACK(on_reset), &ocr);
	g_signal_connect(ocr.file_button, "file-set", G_CALLBACK(on_file_choose), &ocr);

	g_signal_connect(ocr.left_rotate_button, "clicked", G_CALLBACK(on_left_click), &ocr);
	g_signal_connect(ocr.right_rotate_button, "clicked", G_CALLBACK(on_right_click), &ocr);

	g_signal_connect(ocr.original_button, "clicked", G_CALLBACK(on_page_click1), &ocr);
	g_signal_connect(ocr.greyscale_button, "clicked", G_CALLBACK(on_page_click2), &ocr);
	g_signal_connect(ocr.sovola_button, "clicked", G_CALLBACK(on_page_click3), &ocr);
	g_signal_connect(ocr.floodfill_button, "clicked", G_CALLBACK(on_page_click4), &ocr);
	g_signal_connect(ocr.grid_button, "clicked", G_CALLBACK(on_page_click5), &ocr);
	g_signal_connect(ocr.autorotation_button, "clicked", G_CALLBACK(on_page_click6), &ocr);
	g_signal_connect(ocr.cutting_button, "clicked", G_CALLBACK(on_page_click7), &ocr);
	g_signal_connect(ocr.ocr_button, "clicked", G_CALLBACK(on_page_click8), &ocr);
	g_signal_connect(ocr.solving_button, "clicked", G_CALLBACK(on_page_click9), &ocr);
	
	gtk_main();

	TTF_Quit();
	SDL_Quit();

	return 0;
}
