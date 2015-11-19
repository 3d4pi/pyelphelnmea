/*
 * pyelphelnmea - Python module to encode NMEA sentences in Elphel format
 *
 * Copyright (c) 2015 FOXEL SA - http://foxel.ch
 * Please read <http://foxel.ch/license> for more information.
 *
 *
 * Author(s):
 *
 *      Nils Hamel <n.hamel@foxel.ch>
 *      Kevin Velickovic <k.velickovic@foxel.ch>
 *
 *
 * This file is part of the FOXEL project <http://foxel.ch>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Additional Terms:
 *
 *      You are required to preserve legal notices and author attributions in
 *      that material or in the Appropriate Legal Notices displayed by works
 *      containing it.
 *
 *      You are required to attribute the work as explained in the "Usage and
 *      Attribution" section of <http://foxel.ch/license>.
 */

# include "Python.h"
# include <stdio.h>
# include <string.h>

/* Define NMEA sentence model */
# define MODEL_RMC  "QBQBQBQQQQB"
# define MODEL_GGA  "QQBQBQQQQBQBBB"
# define MODEL_GSA  "BQQQQQQQQQQQQQQQQ"
# define MODEL_VTG  "QBQBQBQB"

/* Define NMEA sentence type */
# define IDENT_RMC  0
# define IDENT_GGA  1
# define IDENT_GSA  2
# define IDENT_VTG  3
# define IDENT_MAX  4
# define IDENT_FAI  63

/* Function to set a quartet */
void nmea_setquartet( unsigned char neQuartet, unsigned char * neRecord, int neOffset ) {

    /* Setting record quartet */
    neRecord[ neOffset >> 1 ] |= ( neQuartet % 16 ) << ( ( neOffset % 2 ) * 4 );

}

/* Function to encode NMEA sentences in Elphel format */
void nmea_encode( unsigned char * neSentence, unsigned char * neRecord ) {

    /* Sentence element models */
    char const * const neModel[IDENT_MAX] = {

        MODEL_RMC, MODEL_GGA, MODEL_GSA, MODEL_VTG

    };

    /* Sentence type */
    int neType = 0;

    /* Sentence length */
    int neLength = 0;

    /* Parsing variable */
    int neParse = 0;

    /* Model variable */
    int nePointer = 0;

    /* Offset variable */
    int neOffset = 0;

    /* Detect sentence type */
    if ( strstr( (const char *) neSentence, "$GPRMC," ) != NULL ) {

        /* Assign sentence type */
        neType = IDENT_RMC;

    } else if ( strstr( (const char *) neSentence, "$GPGGA," ) != NULL ) {

        /* Assign sentence type */
        neType = IDENT_GGA;

    } else if ( strstr( (const char *) neSentence, "$GPGSA," ) != NULL ) {

        /* Assign sentence type */
        neType = IDENT_GSA;

    } else if ( strstr( (const char *) neSentence, "$GPVTG," ) != NULL ) {

        /* Assign sentence type */
        neType = IDENT_VTG;

    } else {

        /* Assign sentence type */
        neType = IDENT_FAI;

    }

    /* Check sentence type */
    if ( neType < IDENT_MAX ) {

        /* Encode sentence type */
        nmea_setquartet( neType, neRecord, neOffset ++ );

        /* Avoid sentence header for encoding */
        neSentence += 7;

        /* Compute sentence string length */
        neLength = strlen( (const char *) neSentence );

        /* Parsing sentence string */
        for ( neParse = 0; neParse < neLength; neParse ++ ) {

            /* Detects sentence separator */
            if ( neSentence[neParse] == ',' ) {

                /* Check model */
                if ( neModel[neType][nePointer] == 'Q' ) {

                    /* Sending of of sequence */
                    nmea_setquartet( 0x0F, neRecord, neOffset ++ );

                } else {

                    /* Sending of of sequence */
                    nmea_setquartet( 0x0F, neRecord, neOffset ++ );
                    nmea_setquartet( 0x0F, neRecord, neOffset ++ );

                }

                /* Update model pointer */
                nePointer ++;

            } else {

                /* Check model */
                if ( neModel[neType][nePointer] == 'Q' ) {

                    /* Search for number */
                    if ( ( neSentence[neParse] >= 0x30 ) && ( neSentence[neParse] < 0x3A ) ) {

                        /* Insert symbol */
                        nmea_setquartet( neSentence[neParse] - 0x30, neRecord, neOffset ++ );

                    } else {

                        /* Insert symbol */
                        nmea_setquartet( neSentence[neParse] - 0x20, neRecord, neOffset ++ );

                    }

                } else {

                    /* Insert symbol */
                    nmea_setquartet( neSentence[neParse] % 16, neRecord, neOffset ++ );
                    nmea_setquartet( neSentence[neParse] >> 4, neRecord, neOffset ++ );

                }

            }

        }

    } else {

        /* Error */

    }

}

/* The module doc string */
PyDoc_STRVAR(pyelphelnmea__doc__,
"Python module to encode NMEA sentences in Elphel format");

PyDoc_STRVAR(nmea_encode__doc__,
"Function to encode NMEA sentences in Elphel format");

/* The wrapper to the underlying C function for nmea_encode */
static PyObject *
py_nmea_encode(PyObject *self, PyObject *args)
{
    /* Arguments containers */
    unsigned char * neSentence;
    int neSentence_length = 0;

    /* Try to parse arguments */
    if (!PyArg_ParseTuple(args, "s#:nmea_encode", &neSentence, &neSentence_length))
        return NULL;

    /* Create fake Elphel record */
    unsigned char neRecord[64] = {0};

    /* Encode sentence */
    nmea_encode( neSentence, neRecord + 8 );

    /* Result container */
    PyObject * result;

    /* Build result */
    result = Py_BuildValue("s#", neRecord, 64);

    /* Return result */
    return result;

}

/* Internal python methods bindings */
static PyMethodDef pyelphelnmea_methods[] = {
    {"nmea_encode",  py_nmea_encode, METH_VARARGS, nmea_encode__doc__},
    {NULL, NULL}      /* sentinel */
};

/* Internal python module initializer */
PyMODINIT_FUNC
initpyelphelnmea(void)
{

    /* Initialize module */
    Py_InitModule3("pyelphelnmea", pyelphelnmea_methods,
                   pyelphelnmea__doc__);
}
