/*======================================================================
 * grids.h - grid coordinate system parameters
 *
 *	26-Dec-1991 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 *======================================================================*/
#ifndef grids_h_
#define grids_h_

#include <mapx.h>

/* 
 * useful macros
 */

/*
 * grid parameters structure
 */
typedef struct {
	float map_origin_col, map_origin_row;
	float cols_per_map_unit, rows_per_map_unit;
	int cols, rows;
	FILE *gpd_file;
	char *gpd_filename;
	mapx_class *mapx;
} grid_class;

/*
 * function prototypes
 */
grid_class *init_grid(char grid_file_name[]);
void close_grid(grid_class *this);
int forward_grid(grid_class *this, float lat, float lon, float *r, float *s);
int inverse_grid(grid_class *this, float r, float s, float *lat, float *lon);

#endif