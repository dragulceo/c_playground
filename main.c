//
//  main.c
//  testpromat
//
//  Created by dragulceo on 09/06/14.
//  Copyright (c) 2014 Tavi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

//#define DEBUG_PRINT


int debug_printf(const char *format, ...) {
#ifdef DEBUG_PRINT
    va_list arg;
    int done;

    va_start (arg, format);
    done = vprintf (format, arg);
    va_end (arg);

    return done;
#else
    return 1;
#endif
};

struct Array {
    struct ArrayFuncTable *funcTable;
    unsigned int length;
    size_t size;
    void **data;
};

void ArrayAddElement (struct Array * array, void *element) {
    if(array->length <= 0) {
        //if this is the first element
        array->length = 0;
    }
    void **newArray = malloc(array->size * (array->length + 1));
    if (array->length > 0) {
        //if we have elements copy into new array and free the old data
        memcpy(newArray, array->data, array->length * array->size);
        free(array->data);
    }
    memcpy(newArray + array->size/sizeof(void*) * array->length, element, array->size);
    array->length += 1;
    array->data = newArray;
};

void ArrayCopyArray (struct Array * dest, struct Array * src) {
    if(src->length > 0 && src->size) {
        if(dest->length > 0) {
            //if we have old data in the dest array
            //free(destInnerArray->data);
        }
        dest->data = malloc(src->size * src->length);
        memcpy(dest->data, src->data, src->size * src->length);
        dest->length = src->length;
        dest->size = src->size;
    }
};

struct ArrayFuncTable {
    void (*AddElement)(struct Array * array, void *element);
    void (*CopyArray)(struct Array * dest, struct Array * src);
} arrayFuncTable = {
    ArrayAddElement,
    ArrayCopyArray
};

struct Array * MakeArrayDouble () {
    struct Array * obj = malloc(sizeof(struct Array));
    obj->funcTable = & arrayFuncTable;
    obj->size = sizeof(double);
    obj->length = 0;
    obj->data = NULL;
    return obj;
};

void FreeArrayDouble(struct Array * arr) {
    free(arr->data);
    free(arr);
};

struct Unit {
    unsigned short unit;
    struct Array *array;
};

struct Unit * MakeUnit () {
    struct Unit * unit = malloc(sizeof(struct Unit));
    unit->array = MakeArrayDouble();
    unit->unit = -1;
    return unit;
};

void FreeUnit(struct Unit * unit) {
    FreeArrayDouble(unit->array);
    //free(unit);
};

struct Unit * UnitsGetUnitEx (struct Array * array, unsigned short unit, unsigned short index) {
    for(size_t i = 0; i < array->length; i = i + 1) {
        unsigned long currentIndex = i * array->size / sizeof(void*);
        if(i == index) {
            return (struct Unit*)(array->data + currentIndex);
        }
        struct Unit* lUnit = ((struct Unit*)(array->data + currentIndex));
        if(lUnit != NULL && lUnit->unit == unit) {
            return lUnit;
        }
    }
    return NULL;
};

struct Unit * UnitsGetUnit (struct Array * array, unsigned short unit) {
    return UnitsGetUnitEx(array, unit, -1);
};

struct Unit * UnitsGetUnitAtIndex (struct Array * array, unsigned short index) {
    return UnitsGetUnitEx(array, -1, index);
};

struct Array * MakeUnitsArray () {
    struct Array * obj = malloc(sizeof(struct Array));
    obj->data = NULL;
    obj->funcTable = & arrayFuncTable;
    obj->size = sizeof(struct Unit);
    obj->length = 0;
    return obj;
};

void FreeUnitsArray(struct Array * arr) {
    for(int i = 0; i < arr->length; i++) {
        FreeUnit(UnitsGetUnitAtIndex(arr, i));
    }
    //free(((struct ArrayPrivateData *)arr->privateData)->data);
    free(arr->data);
    free(arr);
};

void printArrayOfDoublesEx(struct Array * array, int debug) {
    int (*fpprintf)(const char *, ...);
    if(debug) {
        fpprintf = &debug_printf;
    } else {
        fpprintf = &printf;
    }
    int j;
    for(j = 0; j < array->length - 1; j++) {
        fpprintf("%f, ", *(double*)(array->data + (j * array->size / sizeof(void*)) ));
    }
    fpprintf("%f", *(double*)(array->data + (j * array->size / sizeof(void*))) );
}

void printArrayOfDoubles(struct Array * array) {
    printArrayOfDoublesEx(array, 0);
}

void printArrayOfUnits(struct Array * array) {
    if(array->length > 0) {
        printf("Array: ");
        for(size_t i = 0; i < array->length; i++) {
            struct Unit * unit = UnitsGetUnitAtIndex(array, i);
            if(unit != NULL) {
                printf("Unit: %hu => ", unit->unit);
                printArrayOfDoubles(unit->array);
                printf("\n");
            }
        }
    }
}

# pragma mark Processing
/**
 Process double by creating new units by suming new unit to existing unit and appending if not existing
 params:
 double data - (in) new double
 struct Array ** solution - (out) pointer to solution
 struct Array * units - (in / out) pointer to array of units found
 */
void processData(double data, struct Array ** solution, struct Array * units) {
    debug_printf("Process: %f\n", data);
    struct Unit * unit;
    int currentUnit = (int)round(data * 100) % 100, newUnit;
    if(currentUnit == 0) {
        debug_printf("Solution found: %d with data %f\n", currentUnit, data);
        //we found the solution
        struct Array * sol = MakeArrayDouble();
        sol->funcTable->AddElement(sol, &data);
        *solution = sol;
        return;
    }
    int unitsLength = units->length;
    debug_printf("Create new units with %d units and data: %f\n", unitsLength, data);
    for(int i = 0; i < unitsLength; i++) {
        unit = UnitsGetUnitAtIndex(units, i);
        if(unit != NULL) {
            newUnit = (unit->unit + currentUnit) % 100;
            debug_printf("--Got newunit %d from unit at index with unit: %d %f\n", newUnit, unit->unit, data);
            struct Unit * newUnitObj = UnitsGetUnit(units, newUnit);
            if(newUnitObj == NULL) {
                debug_printf("--Create new unit with unit: %d\n", newUnit);
                struct Unit * unitToAdd = MakeUnit();
                unitToAdd->unit = newUnit;
                printArrayOfDoublesEx(unit->array, 1);
                debug_printf("\n");
                if(unit->array) {
                    unitToAdd->array->funcTable->CopyArray(unitToAdd->array, unit->array);
                }
                printArrayOfDoublesEx(unitToAdd->array, 1);
                debug_printf("\n");
                unitToAdd->array->funcTable->AddElement(unitToAdd->array, &data);
                printArrayOfDoublesEx(unitToAdd->array, 1);
                debug_printf("\n");
                units->funcTable->AddElement(units, unitToAdd);
                if(newUnit == 0) {
                    *solution = unitToAdd->array;
                    return;
                }
            }
            if(newUnit == 0) {
                *solution = unit->array;
                return;
            }
        }
    }
    unit = UnitsGetUnit(units, currentUnit);
    if(unit == NULL) {
        struct Unit * unitToAdd = MakeUnit();
        unitToAdd->unit = currentUnit;
        unitToAdd->array->funcTable->AddElement(unitToAdd->array, &data);
        units->funcTable->AddElement(units, unitToAdd);
    }
}

/**
 Process chunks of data read from the file and identify N, identify doubles
 This function can process unlimited doubles (we disregard the value of N because it's harder this way)
 
 params:
 char *newData - (in) chunk read from file
 char **leftOverData - (in, out) chumk left over from last processing
 n - (in, out) number of doubles (not used after reading)
 struct Array ** solution - (out) pointer to solution
 struct Array * units - (in / out) pointer to array of units found
 */
void processDataAndReturnLeftOver(char *newData, char** leftOverData, int *n, struct Array ** solution, struct Array * units) {
    
    size_t allDataSize = sizeof(char) * (strlen(newData) + strlen(*leftOverData) + 1);
    char *allData = (char *)malloc(allDataSize);
    strcpy(allData, *leftOverData);
    strcat(allData, newData);
    size_t allDataLength = strlen(allData);
    int prevI = 0;
    int i;
    
    for(i = 0; i < allDataLength; i++) {
        if(*n == 0 && (allData[i] == '\n' || allData[i] == '\r')) {
            //get N
            char *nStr = malloc(sizeof(char) * (i + 1));
            strncpy(nStr, allData, i);
            *n = atoi(nStr);
            free(nStr);
            prevI = i;
            break;
        }
        if(allData[i] == ' ' || allData[i] == '\n' || allData[i] == '\r') {
            //get a double and process it
            if(i > prevI) {
                char *nStr = malloc(sizeof(char) * (i - prevI + 1));
                strncpy(nStr, allData + prevI, i - prevI);
                nStr[i - prevI] = '\0';
                double d = atof(nStr);
                prevI = i;
                free(nStr);
                processData(d, solution, units);
                if(*solution != NULL) {
                    break;
                }
            }
            
        }
    }
    //printArrayOfUnits(units);
    *leftOverData = malloc(sizeof(char) * (allDataLength - prevI + 1));
    strcpy(*leftOverData, allData + prevI + 1);
    free(allData);
}

int main(int argc, const char * argv[])
{
    
    int charsToRead = 10;
    size_t readChars;
    size_t size = sizeof(char) * charsToRead;
    char *readData;
    
    readData = (char *)malloc(size);
    FILE *f = fopen("./data.txt", "r");
    int n = 0;
    char * leftOverData = malloc(sizeof(char));
    leftOverData[0] = '\0';
    
    struct Array * solution = NULL;
    struct Array * units = MakeUnitsArray();
    
    while(f && !feof(f)) {
        readChars = fread(readData, sizeof(char), charsToRead, f);
        if(readChars > 0) {
            processDataAndReturnLeftOver(readData, &leftOverData, &n, &solution, units);
            if(solution != NULL) {
                break;
            }
        }
    }
    fclose(f);
    free(leftOverData);
    free(readData);
    
    printArrayOfUnits(units);
    
    if(solution != NULL) {
        printf("SOLUTION:\n");
        printArrayOfDoubles(solution);
    }
    
    
    FreeUnitsArray(units);
    
    return 0;
}

