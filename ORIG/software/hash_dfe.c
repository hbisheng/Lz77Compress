/***************************************************************************
*          Lempel, Ziv, Storer, and Szymanski Encoding and Decoding
*
*   File    : hash.c
*   Purpose : Implement hash table optimized matching of uncoded strings
*             for LZSS algorithm.
*   Author  : Michael Dipperstein
*   Date    : February 21, 2004
*
****************************************************************************
*
* Hash: Hash table optimized matching routines used by LZSS
*       Encoding/Decoding Routine
* Copyright (C) 2004 - 2007, 2014 by
* Michael Dipperstein (mdipper@alumni.engr.ucsb.edu)
*
* This file is part of the lzss library.
*
* The lzss library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The lzss library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include "lzlocal.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/
typedef enum
{
    SRC_SLIDING_WINDOW, /* use slidingWindow */
    SRC_LOOKAHEAD       /* use uncodedLookahead */
} hash_src_t;

/***************************************************************************
*                                CONSTANTS
***************************************************************************/
#define NULL_INDEX      (WINDOW_SIZE + 1)

#define HASH_SIZE       (WINDOW_SIZE >> 2)  /* size of hash table */

#define COMPARE_CNT     8
/***************************************************************************
*                            GLOBAL VARIABLES
***************************************************************************/
/* cyclic buffer sliding window of already read characters */
extern unsigned char slidingWindow[];
extern unsigned char uncodedLookahead[];

unsigned int hashTable[COMPARE_CNT][HASH_SIZE];          /* list head for each hash key */

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : HashKey
*   Description: This function generates a hash key for a (MAX_UNCODED + 1)
*                long string located either in the sliding window or the
*                uncoded lookahead.  The key generation algorithm is
*                supposed to be based on the algorithm used by gzip.  As
*                reported in K. Sadakane, H. Imai. "Improving the Speed of
*                LZ77 Compression by Hashing and Suffix Sorting". IEICE
*                Trans. Fundamentals, Vol. E83-A, No. 12 (December 2000)
*   Parameters : offset - offset into either the uncoded lookahead or the
*                         sliding window.
*                hashSource - indicate whether offset is an offset into the
*                             sliding window uncoded lookahead buffer.
*   Effects    : NONE
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
static unsigned int HashKey(const unsigned int offset,
    const hash_src_t hashSource)
{
    unsigned int i;
    unsigned int hashKey;

    hashKey = 0;

    if (SRC_LOOKAHEAD == hashSource)
    {
        /* string is in the lookahead buffer */
        for (i = 0; i < (MAX_UNCODED + 1); i++)
        {
            hashKey = (hashKey << 5) ^
                uncodedLookahead[Wrap((offset + i), MAX_CODED)];
            hashKey %= HASH_SIZE;
        }
    }
    else
    {
        /* string is in the sliding window */
        for (i = 0; i < (MAX_UNCODED + 1); i++)
        {
            hashKey = (hashKey << 5) ^
                slidingWindow[Wrap((offset + i), WINDOW_SIZE)];
            hashKey %= HASH_SIZE;
        }
    }

    return hashKey;
}

/****************************************************************************
*   Function   : InitializeSearchStructures
*   Description: This function initializes structures used to speed up the
*                process of mathcing uncoded strings to strings in the
*                sliding window.  For hashed searches, this means that a
*                hash table pointing to linked lists is initialized.
*   Parameters : None
*   Effects    : The hash table and next array are initialized.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
*
*   NOTE: This function assumes that the sliding window is initially filled
*         with all identical characters.
****************************************************************************/
int InitializeSearchStructures()
{
    unsigned int i;
    unsigned int j;
    for(i = 0; i < COMPARE_CNT; i++)
    {   
        for(j = 0; j < HASH_SIZE; j++)
        {
            hashTable[i][j] = 0;
        }
    }
    return 0;
}

/****************************************************************************
*   Function   : FindMatch
*   Description: This function will search through the slidingWindow
*                dictionary for the longest sequence matching the MAX_CODED
*                long string stored in uncodedLookahead.
*   Parameters : uncodedHead - head of uncoded lookahead buffer
*   Effects    : NONE
*   Returned   : The sliding window index where the match starts and the
*                length of the match.  If there is no match a length of
*                zero will be returned.
****************************************************************************/
encoded_string_t FindMatch(const unsigned int windowHead,
    const unsigned int uncodedHead)
{
    encoded_string_t matchData;
    unsigned int i;
    unsigned int j;
    unsigned int cnt;

    (void)windowHead;       /* prevents unused variable warning */
    matchData.length = 0;
    matchData.offset = 0;

    /* use hash to find the start of the list that we need to check */
    j = 0;
    for(cnt = 0; cnt < COMPARE_CNT; cnt++)
    {
        i = hashTable[cnt][HashKey(uncodedHead, SRC_LOOKAHEAD)];
        if (slidingWindow[i] == uncodedLookahead[uncodedHead])
        {
            /* we matched one how many more match? */
            j = 1;

            while(slidingWindow[Wrap((i + j), WINDOW_SIZE)] ==
                uncodedLookahead[Wrap((uncodedHead + j), MAX_CODED)])
            {
                if (j >= MAX_CODED)
                {
                    break;
                }
                j++;
            }

            if (j > matchData.length)
            {
                matchData.length = j;
                matchData.offset = i;
            }
        }

        if (j >= MAX_CODED)
        {
            matchData.length = MAX_CODED;
            break;
        }   
    }

    return matchData;
}

/****************************************************************************
*   Function   : AddString
*   Description: This function adds the (MAX_UNCODED + 1) long string
*                starting at slidingWindow[charIndex] to the hash table's
*                linked list associated with its hash key.
*   Parameters : charIndex - sliding window index of the string to be
*                            added to the linked list.
*   Effects    : The string starting at slidingWindow[charIndex] is appended
*                to the end of the appropriate linked list.
*   Returned   : NONE
****************************************************************************/
static void AddString(const unsigned int charIndex)
{
    unsigned int hashKey;
    hashKey = HashKey(charIndex, SRC_SLIDING_WINDOW);
    hashTable[charIndex % 8][hashKey] = charIndex;
    
}

/****************************************************************************
*   Function   : ReplaceChar
*   Description: This function replaces the character stored in
*                slidingWindow[charIndex] with the one specified by
*                replacement.  The hash table entries effected by the
*                replacement are also corrected.
*   Parameters : charIndex - sliding window index of the character to be
*                            removed from the linked list.
*   Effects    : slidingWindow[charIndex] is replaced by replacement.  Old
*                hash entries for strings containing slidingWindow[charIndex]
*                are removed and new ones are added.
*   Returned   : 0 for success, -1 for failure.  errno will be set in the
*                event of a failure.
****************************************************************************/


int ReplaceChar(const unsigned int charIndex, const unsigned char replacement)
{
    unsigned int firstIndex;

    if (charIndex < MAX_UNCODED)
    {
        firstIndex = (WINDOW_SIZE + charIndex) - MAX_UNCODED;
    }
    else
    {
        firstIndex = charIndex - MAX_UNCODED;
    }

    slidingWindow[charIndex] = replacement;
    AddString( firstIndex );

    return 0;
}

