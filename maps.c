/*========================================================================
 * maps - map utility functions
 *========================================================================*/
static const char maps_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/maps.c,v 1.4 1993-10-22 09:48:45 knowles Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <define.h>
#include <maps.h>

/*------------------------------------------------------------------------
 * draw_graticule - draw and (optionally) label grid of lat,lon lines
 *
 *	input : mapx - map definition
 *		move_pu - move pen up function
 *		draw_pd - draw pen down function
 *		label - print label function or NULL
 *
 *------------------------------------------------------------------------*/
void draw_graticule(mapx_class *mapx, void (*move_pu)(float lat, float lon),
		    void (*draw_pd)(float lat, float lon),
		    void (*label)(char *string, float lat, float lon))
{ float lat, lon, east, llon;
  char label_string[5];
  
  east = mapx->map_stradles_180 ? mapx->east+360 : mapx->east;
  
/*
 *	draw parallels
 */
  if (mapx->lat_interval > 0)
  { for (lat = mapx->south; lat <= mapx->north; lat += mapx->lat_interval)
    { move_pu(lat, (float)mapx->west);
      for (lon = mapx->west+1; lon < east; lon++)
	draw_pd(lat, lon);
      draw_pd(lat, east);
    }
  }

/*
 *	draw meridians
 */
  if (mapx->lon_interval > 0)
  { for (lon = mapx->west; lon <= east; lon += mapx->lon_interval)
    { move_pu((float)mapx->south, lon);
      for (lat = mapx->south+1; lat < mapx->north; lat++)
	draw_pd(lat, lon);
      draw_pd((float)mapx->north, lon);
    }
  }

/*
 *	label parallels
 */
  if (label != NULL)
  { if (mapx->lat_interval > 0)
    { lon = mapx->label_lon;
      for (lat = mapx->south; lat <= mapx->north; lat += mapx->lat_interval)
      { sprintf(label_string,"%3d%c", (int)fabs(lat), lat < 0 ? 'S' : 'N');
	label(label_string, lat, lon);
      }
    }

/*
 *	label meridians
 */
    if (mapx->lon_interval > 0)
    { lat = mapx->label_lat;
      for (lon = mapx->west; lon <= east; lon += mapx->lon_interval)
      { llon = lon < 180 ? lon : lon-360;
	sprintf(label_string,"%3d%c", (int)fabs(llon), llon < 0 ? 'W' : 'E');
	label(label_string, lat, lon);
      }
    }
  }
}

/*----------------------------------------------------------------------
 * arc_length_km - returns arc length (km) from lat1,lon1 to lat2,lon2
 *
 *	input : lat1,lon1, lat2,lon2 - in decimal degrees
 *
 *----------------------------------------------------------------------*/
float arc_length_km (float lat1, float lon1, float lat2, float lon2)
{ double beta;
  
  lat1 = radians(lat1);
  lon1 = radians(lon1);
  lat2 = radians(lat2);
  lon2 = radians(lon2);
  beta = acos( cos(lat1) * cos(lat2) * cos(lon1-lon2)
	      + sin(lat1) * sin(lat2) );
  return (float)(mapx_Re_km * beta);
}

/*----------------------------------------------------------------------
 * west_azimuth - azimuth west of north
 *
 *	input : lat1,lon1, lat2,lon2 (decimal degrees)
 *
 *	result: signed angle west of north from pt. 1 to 2 (decimal degrees)
 *
 *----------------------------------------------------------------------*/
float west_azimuth(float lat1, float lon1, float lat2, float lon2)
{ double phi1, phi2, dlam, sin_A, cos_A, A;

  phi1 = radians(lat1);
  phi2 = radians(lat2);
  dlam = radians(lon1 - lon2);

  sin_A = cos(phi2)*sin(dlam);
  cos_A = cos(phi1)*sin(phi2) - sin(phi1)*cos(phi2)*cos(dlam);
  A = atan2(sin_A, cos_A);

  return (float)degrees(A);
}

/*----------------------------------------------------------------------
 * bisect - find geographic point exactly halfway between two points
 *
 *	input : lat1,lon1, lat2,lon2 - end points (decimal degrees)
 *
 *	output: lat,lon - mid-point (decimal degrees)
 *
 *----------------------------------------------------------------------*/
void bisect(float lat1, float lon1, float lat2, float lon2, 
	    float *lat, float *lon)
{ double phi1,lam1, phi2, lam2, beta;
  double x1, y1, z1, x2, y2, z2, x, y, z, d;
  
  phi1 = radians(90.0 - lat1);
  lam1 = radians(lon1);
  phi2 = radians(90.0 - lat2);
  lam2 = radians(lon2);
  
/*
 *	convert to rectangular
 */
  x1 = sin(phi1) * cos(lam1);
  y1 = sin(phi1) * sin(lam1);
  z1 = cos(phi1);
  
  x2 = sin(phi2) * cos(lam2);
  y2 = sin(phi2) * sin(lam2);
  z2 = cos(phi2);
  
/*
 *	assume spherical earth
 *	normalized sum will bisect
 */
  x = x1 + x2;
  y = y1 + y2;
  z = z1 + z2;
  d = sqrt(x*x + y*y + z*z);
  
/*
 *	convert back to spherical
 */
  beta = acos(z/d);
  *lat = 90.0 - degrees(beta);
  *lon = degrees(acos(x/(d*sin(beta))));
  
}

/*------------------------------------------------------------------------
 * sscanf_lat_lon - scan lat lon from string buffer
 *
 *	input : readln - pointer to buffer
 *
 *	output: lat, lon - latitude, longitude in decimal degrees
 *
 *	result: 2 - success
 *		0 - error scanning string
 *
 *	format: dd mm N/S dd mm E/W => degrees minutes
 *		[+/-]dd.dd [N/S] [+/-]dd.dd [E/W] => decimal degrees
 *
 *------------------------------------------------------------------------*/
int sscanf_lat_lon(char *readln, float *lat, float *lon)
{ float dlat, dlon, mlat, mlon;
  char ns[2], ew[2];
  
  if (sscanf(readln,"%f %f %s %f %f %s", &dlat,&mlat,ns, &dlon,&mlon,ew) == 6)
  { *lat = (dlat + mlat/60.);
    if (*ns == 's' || *ns == 'S')
      *lat = -*lat;
    else if (*ns != 'n' && *ns != 'N')
      return 0;
    
    *lon = (dlon + mlon/60.);
    if (*ew == 'w' || *ew == 'W')
      *lon = -*lon;
    else if (*ew != 'e' && *ew != 'E')
      return 0;
    
    return 2;
  }
  else if (sscanf(readln,"%f %s %f %s", &dlat,ns, &dlon,ew) == 4)
  { *lat = dlat;
    if (*ns == 's' || *ns == 'S')
      *lat = -*lat;
    else if (*ns != 'n' && *ns != 'N')
      return 0;
    
    *lon = dlon;
    if (*ew == 'w' || *ew == 'W')
      *lon = -*lon;
    else if (*ew != 'e' && *ew != 'E')
      return 0;
    *lon = dlon * (*ew == 'e' || *ew == 'E' ? 1 : -1);
    
    return 2;
  }
  else if (sscanf(readln,"%f %f", &dlat, &dlon) == 2)
  { *lat = dlat;
    *lon = dlon;
    return 2;
  }
  else if (lat_lon_decode(readln, LAT_DESIGNATORS, &dlat)
	   && lat_lon_decode(readln, LON_DESIGNATORS, &dlon))
  { *lat = dlat;
    *lon = dlon;
    return 2;
  }
  else
    return 0;
}

/*------------------------------------------------------------------------
 * lat_lon_decode - decode lat or lon (decimal degrees) from buffer
 *
 *	input : readln - pointer to buffer
 *		designators - string of possible hemisphere designators
 *			for example "EWew" to extract a longitude
 *
 *	output: value - latitude or longitude in decimal degrees
 *
 *	result: 1 - success
 *		0 - error scanning string
 *
 *	format: dd.dd[optional white space]designator
 *		this function will attempt to extract the value from
 *		anywhere in the buffer up to the first newline
 *
 *------------------------------------------------------------------------*/
int lat_lon_decode(const char *readln, const char *designators, float *value)
{ const char *end, *pos;
  char hemi, number[80];
  int len;

  end = strchr(readln, '\n');
  if (NULL == end) end = readln + strlen(readln);

  pos = strpbrk(readln, designators);
  if (NULL == pos || pos > end) return 0;

  hemi = toupper(*pos);

  while (pos > readln && isspace(*--pos));

  while (pos > readln && NULL != strchr("0123456789.+-", *--pos));

  if (NULL == strchr("0123456789.+-", *pos)) ++pos;

  len = strspn(pos, "0123456789.+-");
  if (len <= 0) return 0;
  strncpy(number, pos, len);
  number[len] = '\0';

  if (sscanf(number, "%f", value) != 1) return 0;

  if ('W' == hemi || 'S' == hemi) *value = -(*value);

  return 1;
}

/*------------------------------------------------------------------------
 * search_path_fopen - search for file in colon separated list of directories
 *
 *	input : filename - name of file to try first
 *		pathvar - environment variable containing path
 *		mode - same as fopen modes ("r", "w", etc.)
 *
 *	output: filename - name of file successfully openned
 *
 *	result: file pointer of openned file or NULL on failure
 *
 *	note  : directories are searched in order
 *		if first attempt to open file fails then the directory
 *		information preceeding the filename is stripped before
 *		searching the directory path
 *
 *------------------------------------------------------------------------*/
FILE *search_path_fopen(char *filename, const char *pathvar, const char *mode)
{ const char *envpointer;
  char *basename = NULL, *directory, *pathvalue = NULL;
  FILE *fp = NULL;

/*
 *	try to open original name
 */
  fp = fopen(filename, mode);

/* 
 *	failing that, get path information
 */
  if (fp == NULL)
  { envpointer = getenv(pathvar);

/*
 *	strip off directory name
 */
    if (envpointer != NULL)
    { pathvalue = strdup(envpointer);
      basename = strrchr(filename,'/');
      basename = (basename != NULL) ? strdup(basename+1) : strdup(filename);
      if (basename == NULL) return NULL;

/*
 *	try each directory in turn
 */
      directory = strtok(pathvalue, ": ");
      while (directory != NULL)
      {	strcat(strcat(strcpy(filename, directory), "/"), basename);
	fp = fopen(filename, mode);
	if (fp != NULL) break;
	directory = strtok(NULL, ": ");
      }
    }
  }
  if (basename != NULL) free(basename);
  if (pathvalue != NULL) free(pathvalue);

  return fp;
}