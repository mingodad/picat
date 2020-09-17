//-------------------------------------------------------------------------------------------------------------
// QuineMcCluskey Algorithm
// =========================
//-------------------------------------------------------------------------------------------------------------
// English:
//-------------------------------------------------------------------------------------------------------------
// Description: Application to simplify boolean functions with Quine-McCluskey algorithm
// Date: 05/16/2012
// Author: Stefan Moebius (mail@stefanmoebius.de)
// Licence: Can be used freely (Public Domain)
//-------------------------------------------------------------------------------------------------------------
#define BP_TRUE 1
#define BP_FALSE 0
#define QC_MAXVARS 7
#define QC_MAX 2048

//Global fields:
int minterm[QC_MAX][QC_MAX];
int mask[QC_MAX][QC_MAX];  // mask of minterm
int used[QC_MAX][QC_MAX];  // minterm used
int result[QC_MAX];  // results
int primmask[QC_MAX];  // mask for prime implicants
int prim[QC_MAX];  // prime implicant
int wprim[QC_MAX];  // essential prime implicant (BP_TRUE/BP_FALSE)
int nwprim[QC_MAX];  // needed not essential prime implicant

//Count all set bits of the integer number
int popCount(unsigned x) {  // Taken from book "Hackers Delight"
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

//Calculate hamming weight/distance of two integer numbers
int hammingWeight(int v1, int v2) {
    return popCount(v1 ^ v2);
}

//Output upper part of term in console
void upperTerm(int bitfield, int mask, int num) {
    if (mask) {
        int z;
        for ( z = 0; z < num; z++) {
            if (mask & (1 << z)) {
                if (bitfield & (1 << z))
                    printf("_");
                else
                    printf(" ");
            }
        }
    }
}

//Output lower part of term in console
void lowerTerm(int mask, int num) {
    if (mask) {
        int z;
        for (z = 0; z < num; z++) {
            if (mask & (1 << z)) {
                printf("%c", 'z' - (num - 1) + z);
            }
        }
    }
}

//Output a term to console
void outputTerm(int bitfield, int mask, int num) {
    upperTerm(bitfield, mask, num);
    if (mask) printf("\n");
    lowerTerm(mask, num);
}

//Determines whether "value" contains "part"
int contains(value, mask, part, partmask) {
    if ((value & partmask) == (part & partmask)) {
        if ((mask & partmask) == partmask)
            return BP_TRUE;
    }
    return BP_FALSE;
}

int espresso_main() {
    int num = 5;  // Number of Variables
    int pos = 0;
    int cur = 0;
    int prim_count = 0;
    int x;


    init_qc();

    pos = (1 << num);  // 2 ^ num

    printf("pos=%d\n", pos);

    cur = 0;
    for ( x = 0; x < pos; x++) {
        //      if (x!=13 && x!=18 && x!=23){
        mask[cur][0] = ((1 << num)- 1);
        minterm[cur][0] = x;
        cur++;
        result[x] = 1;
        //      }
    }
    prim_count = qc(num, pos);
    output_res(num, prim_count);
}

int init_qc() {
    int x = 0;
    int y = 0;

    for (x = 0; x < QC_MAX; x++) {
        primmask[x] = 0;
        prim[x] = 0;
        wprim[x] = BP_FALSE;
        nwprim[x] = BP_FALSE;
        result[x] = BP_FALSE;
        nwprim[x] = BP_TRUE;  //unwesentliche Primimplikaten als benötigt markieren
        for (y = 0; y < QC_MAX; y++) {
            mask[x][y] = 0;
            minterm[x][y] = 0;
            used[x][y] = BP_FALSE;
        }
    }
}

int qc(int num, int pos) {
    int cur = 0;
    int reduction = 0;  //reduction step
    int maderedction = BP_FALSE;
    int prim_count = 0;
    int term = 0;
    int termmask = 0;
    int found = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    int count = 0;
    int lastprim = 0;
    int res = 0;  // actual result

    for (reduction = 0; reduction < QC_MAX; reduction++) {
        cur = 0;
        maderedction = BP_FALSE;
        for (y = 0; y < QC_MAX; y++) {
            for (x = 0; x < QC_MAX; x++) {
                if ((mask[x][reduction]) && (mask[y][reduction])) {
                    if (popCount(mask[x][reduction]) > 1) {  // Do not allow complete removal (problem if all terms are 1)
                        if ((hammingWeight(minterm[x][reduction] & mask[x][reduction], minterm[y][reduction] & mask[y][reduction]) == 1) && (mask[x][reduction] == mask[y][reduction])) {  // Simplification only possible if 1 bit differs
                            term = minterm[x][reduction];  // could be mintern x or y
                            //e.g.:
                            //1110
                            //1111
                            //Should result in mask of 1110
                            termmask = mask[x][reduction] ^ (minterm[x][reduction] ^ minterm[y][reduction]);
                            term &= termmask;

                            found = BP_FALSE;
                            for ( z = 0; z < cur; z++) {
                                if ((minterm[z][reduction+1] == term) && (mask[z][reduction+1] == termmask) ) {
                                    found = BP_TRUE;
                                }
                            }

                            if (found == BP_FALSE) {
                                minterm[cur][reduction+1] = term;
                                mask[cur][reduction+1] = termmask;
                                cur++;
                            }
                            used[x][reduction] = BP_TRUE;
                            used[y][reduction] = BP_TRUE;
                            maderedction = BP_TRUE;
                        }
                    }
                }
            }
        }
        if (maderedction == BP_FALSE)
            break;  //exit loop early (speed optimisation)
    }

    prim_count = 0;
    //printf("\nprime implicants:\n");
    for ( reduction = 0 ; reduction < QC_MAX; reduction++) {
        for ( x = 0 ; x < QC_MAX; x++) {
            //Determine all not used minterms
            if ((used[x][reduction] == BP_FALSE) && (mask[x][reduction]) ) {
                //Check if the same prime implicant is already in the list
                found = BP_FALSE;
                for ( z = 0; z < prim_count; z++) {
                    if (((prim[z] & primmask[z]) == (minterm[x][reduction] & mask[x][reduction])) && (primmask[z] == mask[x][reduction]) )
                        found = BP_TRUE;
                }
                if (found == BP_FALSE) {
                    //outputTerm(minterm[x][reduction], mask[x][reduction], num);
                    //printf("\n");
                    primmask[prim_count] = mask[x][reduction];
                    prim[prim_count] = minterm[x][reduction];
                    prim_count++;
                }
            }
        }
    }

    //find essential and not essential prime implicants
    //all alle prime implicants are set to "not essential" so far
    for (y = 0; y < pos; y++) {  //for all minterms
        count = 0;
        lastprim = 0;
        if (mask[y][0]) {
            for (x = 0; x < prim_count; x++ ) {  //for all prime implicants
                if (primmask[x]) {
                    // Check if the minterm contains prime implicant
                    if (contains(minterm[y][0], mask[y][0], prim[x], primmask[x])) {
                        count++;
                        lastprim = x;
                    }
                }
            }
            // If count = 1 then it is a essential prime implicant
            if (count == 1) {
                wprim[lastprim] = BP_TRUE;
            }
        }
    }

    // successively testing if it is possible to remove prime implicants from the rest matrix
    for ( z = 0; z < prim_count; z++) {
        if (primmask[z] ) {
            if ((wprim[z] == BP_FALSE)) {  // && (rwprim[z] == BP_TRUE))
                nwprim[z] = BP_FALSE;  // mark as "not essential"
                for ( y = 0; y < pos; y++) {  // for all possibilities
                    res = 0;
                    for ( x = 0; x < prim_count; x++) {
                        if ( (wprim[x] == BP_TRUE) || (nwprim[x] == BP_TRUE)) {  //essential prime implicant or marked as required
                            if ((y & primmask[x]) == (prim[x] & primmask[x])) {  //All bits must be 1
                                res = 1;
                                break;
                            }
                        }
                    }
                    //printf(" %d\t%d\n", result, result[y]);
                    if (res == result[y]) {  // compare calculated result with input value
                        //printf("not needed\n"); //prime implicant not required
                    }
                    else {
                        //printf("needed\n");
                        nwprim[z] = BP_TRUE;  //prime implicant required
                    }
                }
            }
        }
    }
    return prim_count;
}

int output_res(int num, int prim_count) {
    int count = 0;
    int x = 0;

    printf("\nResult:\n\n");
    // Output of essential and required prime implicants
    count = 0;
    for ( x = 0 ; x < prim_count; x++) {
        if (wprim[x] == BP_TRUE) {
            if (count > 0) printf("   ");
            upperTerm(prim[x], primmask[x], num);
            count++;
        }
        else if ((wprim[x] == BP_FALSE) && (nwprim[x] == BP_TRUE)) {
            if (count > 0) printf("   ");
            upperTerm(prim[x], primmask[x], num);
            count++;
        }
    }
    printf("\n");
    count = 0;
    for ( x = 0 ; x < prim_count; x++) {
        if (wprim[x] == BP_TRUE) {
            if (count > 0) printf(" + ");
            lowerTerm(primmask[x], num);
            count++;
        }
        else if ((wprim[x] == BP_FALSE) && (nwprim[x] == BP_TRUE)) {
            if (count > 0) printf(" + ");
            lowerTerm(primmask[x], num);
            count++;
        }
    }
    printf("\n");
}
