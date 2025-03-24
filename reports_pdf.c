#include <glib-2.0/glib.h>
#include <headers.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/**
 * @file reports_pdf.c
 * @brief Contains functions for generating PDF output. Relies on the [Haru PDF library](https://github.com/libharu/libharu).
 *
 */

/**
 * Prints the error from a libharu exception.
 *
 * @param error_no Error number passed from the Haru library.
 * @param detail_no Error description passed from the Haru library.
 * @param user_data `NULL` in this case.
 */
void hpdf_error_handler(HPDF_STATUS error_no,
						HPDF_STATUS detail_no,
						void *user_data) {
	printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
}

/**
 * Draws a table row with one cell.
 *
 * @param data_passer Pointer to a Data_passer struct.
 * @param row_type Indicates type of row being drawn.
 * @param text Text printed in the cell.
 */
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

/**
 * Draws a table row with two cells.
 *
 * @param data_passer Pointer to a Data_passer struct.
 * @param row_type Indicates type of row being drawn.
 * @param label Text printed in the first cell.
 * @param amount Text printed in the second cell.
 */
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


/**
 * Instantiates a structure for configuring a PDF page's layout.
 *
 * @return Pointer to the structure.
 */
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
	page_layout->table_width = round(page_layout->width / 2.0);
	page_layout->first_column_percent = 0.70;
	page_layout->first_column_width = round(page_layout->table_width * page_layout->first_column_percent);
	page_layout->second_column_width = round(page_layout->table_width * (1.0 - page_layout->first_column_percent));

	page_layout->single_underline_offset = 3;
	page_layout->double_underline_offset = 2;

	return page_layout;
}

/**
 * Instantiates a PDF document.
 *
 * @return Pointer to the instantiated document.
 */
HPDF_Doc *instantiate_pdf() {
	HPDF_Doc *pdf = (HPDF_Doc *)g_malloc(sizeof(HPDF_Doc));
	*pdf = HPDF_New(hpdf_error_handler, NULL);
	if (!(*pdf)) {
		printf("error: cannot create PdfDoc object\n");
	}
	return pdf;
}

/**
 * Instantiates a font for a PDF document.
 *
 * @param pdf Pointer to an `HPDF_Doc`.
 * @return Pointer to the instantiated font.
 */
HPDF_Font *configure_pdf_font(HPDF_Doc *pdf) {
	HPDF_Font *font = (HPDF_Font *)g_malloc(sizeof(HPDF_Font));

	*font = HPDF_GetFont(*pdf, "Helvetica", NULL);
	if (!(*font)) {
		printf("error: cannot create a font object\n");
	}
	return font;
}

/**
 * Creates a title page for the  PDF document.
 *
 * @param data_passer Pointer to a Data_passer struct.
 * @param start_date String for the report's starting date.
 * @param end_date String for the report's ending date.
 */
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

	data_passer->pdf_outline_root = (HPDF_Outline *)g_malloc(sizeof(HPDF_Outline));

	*data_passer->pdf_outline_root = HPDF_CreateOutline(*data_passer->pdf, NULL,"P&L Report",NULL);
	HPDF_Outline_SetOpened (*data_passer->pdf_outline_root, HPDF_TRUE);
	
}

/**
 * Creates a new PDF page and adds the heading (the property address) to that page.
 * 
 * @param data_passer Pointer to a Data_passer struct.
 * @param description Text for the heading (the property address).
 */
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

	HPDF_Outline *outline = (HPDF_Outline *)g_malloc(sizeof(HPDF_Outline));
	*outline = HPDF_CreateOutline (*data_passer->pdf, *data_passer->pdf_outline_root, description, NULL);
	HPDF_Destination dst = HPDF_Page_CreateDestination (*page);
	HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight(*page), 1);
	HPDF_Outline_SetDestination(*outline, dst);
	data_passer->pdf_outline = g_slist_append(data_passer->pdf_outline, outline); 
}
