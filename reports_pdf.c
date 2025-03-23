#include <glib-2.0/glib.h>
#include <headers.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void hpdf_error_handler(HPDF_STATUS error_no,
						HPDF_STATUS detail_no,
						void *user_data) {
	printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
}

void draw_row_one_cell(Data_passer *data_passer,
					   enum Row_Type row_type,
					   const HPDF_BYTE *text) {
	Page_Layout *page_layout = data_passer->page_layout;
	
	guint list_length = g_slist_length (data_passer->pdf_pages);
	HPDF_Page *page = (HPDF_Page *)	g_slist_nth_data (data_passer->pdf_pages, list_length - 1);
	
	HPDF_Page_SetRGBFill(*page,
						 page_layout->shading[row_type].red,
						 page_layout->shading[row_type].green,
						 page_layout->shading[row_type].blue);
	HPDF_Page_SetRGBStroke(*page,
						   page_layout->borders.red,
						   page_layout->borders.green,
						   page_layout->borders.blue);
	HPDF_Page_SetLineWidth(*page, 0.5);
	HPDF_Page_Rectangle(*page,
						page_layout->left_margin,
						page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number,
						page_layout->table_width,
						page_layout->row_height * -1);
	HPDF_Page_FillStroke(*page);

	HPDF_Page_BeginText(*page);
	HPDF_Page_SetRGBFill(*page, 0, 0, 0);
	HPDF_Page_SetTextRenderingMode(*page, HPDF_FILL);
	HPDF_Page_SetFontAndSize(*page, *(data_passer->pdf_font), 15);
	HPDF_Page_TextRect(*page,
					   page_layout->left_margin + page_layout->cell_margin_left,
					   page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number - page_layout->cell_margin_top,
					   page_layout->left_margin + page_layout->table_width - page_layout->cell_margin_left,
					   page_layout->row_height - page_layout->row_height * data_passer->pdf_current_row_number + page_layout->row_height,
					   (char *)text,
					   HPDF_TALIGN_LEFT,
					   NULL);
	HPDF_Page_EndText(*page);
	data_passer->pdf_current_row_number++;
}

void draw_row_two_cells(Data_passer *data_passer,
						enum Row_Type row_type,
						const HPDF_BYTE *label,
						const HPDF_BYTE *amount) {

	guint list_length = g_slist_length (data_passer->pdf_pages);
	HPDF_Page *page = (HPDF_Page *)	g_slist_nth_data (data_passer->pdf_pages, list_length - 1);
							
	Page_Layout *page_layout = data_passer->page_layout;

	HPDF_Page_SetRGBFill(*page,
						 page_layout->shading[row_type].red,
						 page_layout->shading[row_type].green,
						 page_layout->shading[row_type].blue);

	/* Draw first cell */
	HPDF_Page_Rectangle(*page,
						page_layout->right_margin,
						page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number,
						page_layout->first_column_width,
						page_layout->row_height * -1);
	HPDF_Page_FillStroke(*page);

	/* Draw second cell */
	HPDF_Page_Rectangle(*page,
						page_layout->right_margin + page_layout->first_column_width,
						page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number,
						page_layout->second_column_width,
						page_layout->row_height * -1);
	HPDF_Page_FillStroke(*page);

	/* Write text */
	HPDF_Page_BeginText(*page);
	HPDF_Page_SetRGBFill(*page, 0, 0, 0);
	HPDF_Page_SetTextRenderingMode(*page, HPDF_FILL);
	HPDF_Page_SetFontAndSize(*page, *(data_passer->pdf_font), 15);
	int left = page_layout->left_margin + page_layout->cell_margin_left;
	if (row_type == BODY_INDENT) {
		left += page_layout->cell_indent;
	}
	HPDF_Page_TextRect(*page,
					   left,
					   page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number - page_layout->cell_margin_top,
					   page_layout->left_margin + page_layout->table_width - page_layout->cell_margin_left,
					   page_layout->row_height - page_layout->row_height * data_passer->pdf_current_row_number + page_layout->row_height,
					   (char *)label,
					   HPDF_TALIGN_LEFT,
					   NULL);
	HPDF_Page_TextRect(*page,
					   left,
					   page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number - page_layout->cell_margin_top,
					   page_layout->left_margin + page_layout->table_width - page_layout->cell_margin_left,
					   page_layout->row_height - page_layout->row_height * data_passer->pdf_current_row_number + page_layout->row_height,
					   (char *)amount,
					   HPDF_TALIGN_RIGHT,
					   NULL);
	HPDF_Page_EndText(*page);
	if ((row_type == SUBTOTAL) || (row_type == FOOTER)) {
		HPDF_Box bbox = HPDF_Font_GetBBox(*(data_passer->pdf_font));
		int height = ((bbox.top - bbox.bottom) * 15) / 1000;
		HPDF_TextWidth text_width = HPDF_Font_TextWidth(*(data_passer->pdf_font), amount, strlen((char *)amount));
		unsigned int underline_length = (text_width.width * 15) / 1000;
		int vertical = page_layout->table_top - page_layout->row_height * data_passer->pdf_current_row_number - height - page_layout->single_underline_offset;
		int left_edge = page_layout->left_margin + page_layout->table_width - page_layout->cell_margin_left - underline_length;
		HPDF_Page_MoveTo(*page, left_edge, vertical);
		HPDF_Page_LineTo(*page, left_edge + underline_length, vertical);
		HPDF_Page_Stroke(*page);
		if (row_type == FOOTER) {
			HPDF_Page_MoveTo(*page, left_edge, vertical - page_layout->double_underline_offset);
			HPDF_Page_LineTo(*page, left_edge + underline_length, vertical - page_layout->double_underline_offset);
			HPDF_Page_Stroke(*page);
		}
	}
	data_passer->pdf_current_row_number++;
}

/* void print_pdf_row(enum Row_Type row_type,
				   int row_number,
				   Page_Layout *page_layout,
				   HPDF_Page *page,
				   HPDF_Font *font,
				   HPDF_BYTE *label,
				   HPDF_BYTE *amount) {
	// Draw rectangle with shading 
	if (row_type == HEADING) {
		draw_row_one_cell(page, page_layout, font, row_type, row_number, label);
	} else {
		draw_row_two_cells(page, page_layout, font, row_type, row_number, label, amount);
	}
} */

Page_Layout *configure_pdf_layout() {
	Page_Layout *page_layout = g_malloc(sizeof(Page_Layout));
	page_layout->right_margin = 72;
	page_layout->left_margin = 72;
	page_layout->top_margin = 72;
	page_layout->bottom_margin = 72;
	page_layout->height = 792;
	page_layout->width = 612;

	page_layout->shading[TEXT_COLOR].red = 0;
	page_layout->shading[TEXT_COLOR].green = 0;
	page_layout->shading[TEXT_COLOR].blue = 0;
	page_layout->shading[HEADING].red = 207 / 256.0;
	page_layout->shading[HEADING].green = 226 / 256.0;
	page_layout->shading[HEADING].blue = 255 / 256.0;
	page_layout->shading[BODY].red = 256 / 256.0;
	page_layout->shading[BODY].green = 256 / 256.0;
	page_layout->shading[BODY].blue = 256 / 256.0;
	page_layout->shading[BODY_INDENT].red = 256 / 256.0;
	page_layout->shading[BODY_INDENT].green = 256 / 256.0;
	page_layout->shading[BODY_INDENT].blue = 256 / 256.0;
	page_layout->shading[SUBTOTAL].red = 256 / 256.0;
	page_layout->shading[SUBTOTAL].green = 256 / 256.0;
	page_layout->shading[SUBTOTAL].blue = 256 / 256.0;
	page_layout->shading[FOOTER].red = 209 / 256.0;
	page_layout->shading[FOOTER].green = 231 / 256.0;
	page_layout->shading[FOOTER].blue = 221 / 256.0;
	page_layout->borders.red = 33 / 256.0;
	page_layout->borders.green = 37 / 256.0;
	page_layout->borders.blue = 41 / 256.0;
	page_layout->cell_margin_left = 4;
	page_layout->cell_margin_top = 5;
	page_layout->cell_indent = 6;
	page_layout->row_height = 30;
	page_layout->table_top = 650;
	page_layout->text_vertical_offset = -7;
	page_layout->table_width = round(page_layout->width / 2.0);
	page_layout->first_column_percent = 0.70;
	page_layout->first_column_width = round(page_layout->table_width * page_layout->first_column_percent);
	page_layout->second_column_width = round(page_layout->table_width * (1.0 - page_layout->first_column_percent));

	page_layout->single_underline_offset = 3;
	page_layout->double_underline_offset = 2;

	return page_layout;
}

HPDF_Doc *instantiate_pdf() {
	HPDF_Doc *pdf = (HPDF_Doc *)g_malloc(sizeof(HPDF_Doc));
	*pdf = HPDF_New(hpdf_error_handler, NULL);
	if (!(*pdf)) {
		printf("error: cannot create PdfDoc object\n");
	}
	return pdf;
}

HPDF_Font *configure_pdf_font(HPDF_Doc *pdf) {
	HPDF_Font *font = (HPDF_Font *)g_malloc(sizeof(HPDF_Font));

	*font = HPDF_GetFont(*pdf, "Helvetica", NULL);
	if (!(*font)) {
		printf("error: cannot create a font object\n");
	}
	return font;
}

/* Cover page */

void create_pdf_title_page(Data_passer *data_passer,
						   char *start_date,
						   char *end_date) {
	Page_Layout *page_layout = data_passer->page_layout;
	HPDF_Page *title_page = g_malloc(sizeof(HPDF_Page));
	*title_page = HPDF_AddPage(*(data_passer->pdf));
	HPDF_Page_SetSize(*title_page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);

	char date_string[500];
	g_snprintf(date_string, 500, "For the period %s to %s", start_date, end_date);

	HPDF_Page_BeginText(*title_page);
	HPDF_Page_SetRGBFill(*title_page, 0, 0, 0);
	HPDF_Page_SetTextRenderingMode(*title_page, HPDF_FILL);
	HPDF_Page_SetFontAndSize(*title_page, *(data_passer->pdf_font), 25);
	HPDF_Page_TextRect(*title_page,
					   page_layout->left_margin,
					   page_layout->table_top,
					   page_layout->width - page_layout->right_margin,
					   300,
					   "Profit and Loss, Rentals",
					   HPDF_TALIGN_CENTER,
					   NULL);
	HPDF_Page_SetFontAndSize(*title_page, *(data_passer->pdf_font), 15);
	HPDF_Page_TextRect(*title_page,
					   page_layout->left_margin,
					   page_layout->table_top - 50,
					   page_layout->width - page_layout->right_margin,
					   300,
					   date_string,
					   HPDF_TALIGN_CENTER,
					   NULL);

	HPDF_Page_EndText(*title_page);
	data_passer->pdf_pages = g_slist_append(data_passer->pdf_pages, title_page);
}

void add_heading_to_pdf(Data_passer *data_passer,
						char *description) {
	HPDF_Page *page = g_malloc(sizeof(HPDF_Page));
	*page = HPDF_AddPage(*(data_passer->pdf));
	HPDF_Page_SetSize(*page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
	HPDF_Page_BeginText(*page);
	HPDF_Page_SetRGBFill(*page, 0, 0, 0);
	HPDF_Page_SetTextRenderingMode(*page, HPDF_FILL);
	HPDF_Page_SetFontAndSize(*page, *(data_passer->pdf_font), 15);
	HPDF_Page_TextOut(*page,
					  data_passer->page_layout->left_margin,
					  data_passer->page_layout->height - data_passer->page_layout->top_margin,
					  description);
	HPDF_Page_EndText(*page);
	data_passer->pdf_pages = g_slist_append(data_passer->pdf_pages, page);
	data_passer->pdf_current_row_number = 0;
}
/*
void add_property_to_pdf(Data_passer *data_passer,
						 Page_Layout *page_layout,
						 char *description) {
	print_row(HEADING, 0, page_layout, &page_1, data_passer->pdf_font, (HPDF_BYTE *)"Income", NULL);
	print_row(BODY_INDENT, 1, page_layout, &page_1, data_passer->pdf_font, (HPDF_BYTE *)"Rents", (HPDF_BYTE *)"19,300");
	print_row(SUBTOTAL, 2, &page_layout, &page_1, data_passer->pdf_font, (HPDF_BYTE *)"Total income", (HPDF_BYTE *)"19,300");
	print_row(FOOTER, 3, &page_layout, &page_1, data_passer->pdf_font, (HPDF_BYTE *)"Net income", (HPDF_BYTE *)"29,300");

	//HPDF_Outline outline[1];
	HPDF_Outline root = HPDF_CreateOutline(pdf, NULL,"PL Report",NULL);
	HPDF_Outline_SetOpened (root, HPDF_TRUE);

	outline[0] = HPDF_CreateOutline (pdf, root, "12202", NULL);
	HPDF_Destination dst = HPDF_Page_CreateDestination (page_1);
	HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight(page_1), 1);
	HPDF_Outline_SetDestination(outline[0], dst); 
}*/
