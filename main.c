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


// #define DEBUG_PRINT


int debug_printf(char *format, ...)
{
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
}



struct Array {
    struct ArrayFuncTable *funcTable;
    unsigned int length;
    size_t size;
    void *privateData;
};

struct ArrayFuncTable {
    void (*AddElement)(struct Array * array, size_t size, void *element);
    void (*CopyArray)(struct Array * dest, struct Array * src);
    void ** (*GetArray)(struct Array * array);
};

struct ArrayPrivateData {
    void **data;
};

struct Array* MakeArray(size_t size)
{
    struct Array *ret = malloc(sizeof(struct Array));
    ret->size = size;
    ret->length = 0;
    ret->privateData = malloc(sizeof(struct ArrayPrivateData));
    return ret;
}

void ArrayAddElement (struct Array * array, size_t size, void *element) {
    struct ArrayPrivateData * innerArray = (struct ArrayPrivateData *) array->privateData;
    if(array->length <= 0) {
        //if this is the first element
        array->length = 0;
        array->size = size;
    }
    void **newArray = malloc(size * (array->length + 1));
    if (array->length > 0) {
        //if we have elements copy into new array and free the old data
        memcpy(newArray, innerArray->data, array->length * size);
        free(innerArray->data);
    }
    memcpy(newArray + size/sizeof(void*) * array->length, element, size);
    array->length += 1;
    innerArray->data = newArray;
}

void ArrayCopyArray (struct Array * dest, struct Array * src) {
    if(src->length > 0 && src->size) {
        struct ArrayPrivateData * srcInnerArray = (struct ArrayPrivateData *) src->privateData;
        struct ArrayPrivateData * destInnerArray = (struct ArrayPrivateData *) dest->privateData;
        if(dest->length > 0) {
            //if we have old data in the dest array
            free(destInnerArray->data);
        }
        debug_printf("Copy meme: %zu * %d\n", src->size, src->length);
        destInnerArray->data = malloc(src->size * src->length);
        memcpy(destInnerArray->data, srcInnerArray->data, src->size * src->length);
        dest->length = src->length;
        dest->size = src->size;
    }
}


# pragma mark Doubles array

struct ArrayDoublePrivateData {
    double *data;
};

void ArrayDoubleAddElement (struct Array * array, double element) {
    ArrayAddElement(array, sizeof(double), &element);
}

void ArrayDoubleCopyArray (struct Array * dest, struct Array * src) {
    ArrayCopyArray(dest, src);
}

double * ArrayDoubleGetArray (struct Array * array) {
    struct ArrayDoublePrivateData * innerArray = (struct ArrayDoublePrivateData *) array->privateData;
    return innerArray->data;
}

struct ArrayDoubleFuncTable {
    void (*AddElement)(struct Array * array, double element);
    void (*CopyArray)(struct Array * dest, struct Array * src);
    double * (*GetArray)(struct Array * array);
} arrayDoubleFuncTable = {
    ArrayDoubleAddElement,
    ArrayDoubleCopyArray,
    ArrayDoubleGetArray
};


struct Array * MakeArrayDouble () {
    struct Array * obj = malloc(sizeof(*obj));
    obj->length = 0;
    struct ArrayDoublePrivateData * rdata = malloc (sizeof(struct ArrayDoublePrivateData));
    obj->funcTable = (struct ArrayFuncTable*) &arrayDoubleFuncTable;
    obj->privateData = rdata;
    return obj;
};

void FreeArrayDouble (struct Array * arr) {
    free(((struct ArrayDoublePrivateData *)arr->privateData)->data);
    free(arr);
};

#pragma mark Unit array

struct Unit {
    unsigned short unit;
    struct Array *array;
};

struct Unit * MakeUnit (unsigned short punit) {
    struct Unit *unit = malloc(sizeof(struct Unit));
    unit->array = MakeArrayDouble();
    unit->unit = punit;
    return unit;
}

void FreeUnit(struct Unit * unit) {
    FreeArrayDouble(unit->array);
    //free(unit);
}

struct UnitsArrayPrivateData {
    struct Unit *data;
};

void UnitsArrayAddElement (struct Array * array, struct Unit element) {
    ArrayAddElement(array, sizeof(element), &element);
};

struct Unit * UnitsGetUnitEx (struct Array * array, unsigned short unit, unsigned short index) {
    struct UnitsArrayPrivateData * innerArray = (struct UnitsArrayPrivateData *) array->privateData;
    for(size_t i = 0; i < array->length; i++) {
        debug_printf("unnt: %d ? %d\n", innerArray->data[i].unit, unit);
        if(i == index || innerArray->data[i].unit == unit) {
            return &innerArray->data[i];
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

int UnitsHasUnit (struct Array * array, unsigned short unit) {
    return (UnitsGetUnit(array, unit) != NULL);
};


struct Unit * UnitsArrayGetArray (struct Array * array) {
    struct UnitsArrayPrivateData * innerArray = (struct UnitsArrayPrivateData *) array->privateData;
    return innerArray->data;
}

struct UnitsArrayFuncTable {
    int (*UHasUnit)(struct Array * units, unsigned short unit);
    struct Unit * (*UGetUnit)(struct Array * units, unsigned short unit);
    struct Unit * (*UGetUnitAtIndex)(struct Array * units, unsigned short index);
    void (*UAddElement)(struct Array * array, struct Unit element);
    void (*AddElement)(struct Array * array, size_t size, void *element);
    struct Unit * (*GetArray)(struct Array * array);
} unitsArrayFuncTable = {
    UnitsHasUnit,
    UnitsGetUnit,
    UnitsGetUnitAtIndex,
    UnitsArrayAddElement,
    ArrayAddElement,
    UnitsArrayGetArray
};


struct Array * MakeUnitsArray() {
    struct Array * obj = malloc(sizeof(*obj));
    obj->length = 0;
    struct UnitsArrayPrivateData * rdata = malloc (sizeof(* rdata));
    obj->funcTable = (struct ArrayFuncTable*) &unitsArrayFuncTable;
    obj->privateData = rdata;
    
    return obj;
};

void FreeUnitsArray(struct Array * arr) {
    for(size_t i = 0; i < arr->length; i++) {
        FreeUnit(&((struct UnitsArrayPrivateData*)arr->privateData)->data[i]);
    }
    free((struct UnitsArrayPrivateData*)arr->privateData);
    //free(((struct UnitsArrayPrivateData*)arr->privateData)->data);
    free(arr);
}

#pragma mark Print utils

void printArrayOfDoubles(struct Array * array) {
    double * arr = ((struct ArrayDoubleFuncTable*)array->funcTable)->GetArray(array);
    //struct ArrayPrivateData * innerArray = (struct ArrayPrivateData *) array->privateData;

    printf("Print array: [");
    if(array->length > 0) {
        int j;
        for(j = 0; j < array->length - 1; j++) {
            //printf("%f,", *(double*)innerArray->data[j]);
            printf("%.2f, ", arr[j]);
        }
        printf("%.2f", arr[j]);
    }
    printf("]\n");
}

void printArrayOfUnits(struct Array * array) {
    if(array->length > 0) {
        printf("Array of units (has length %d): \n", array->length);
        //struct Unit ** arr = ((struct UnitsArrayFuncTable*)array->funcTable)->GetArray(array);
        for(size_t i = 0; i < array->length; i++) {
            struct Unit * unit = ((struct UnitsArrayFuncTable*)array->funcTable)->UGetUnitAtIndex(array, i);
            if(unit != NULL) {
                double * arr = ((struct ArrayDoubleFuncTable*)unit->array->funcTable)->GetArray(unit->array);
                double sum = 0;
                printf("Unit: %d (%d) => [", unit->unit, unit->array->length);
                size_t j;
                for(j = 0; j < unit->array->length - 1; j++) {
                    printf("%.2f, ", arr[j]);
                    sum += arr[j];
                }
                printf("%.2f", arr[j]);
                sum += arr[j];
                printf("] (%.3f)\n", sum);
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
    debug_printf("-Current: %f\n", data);
    struct Unit * unit;
    int currentUnit = (int)round(data * 100) % 100, newUnit;
    if(currentUnit == 0) {
        //we found the solution
        struct Array * sol = MakeArrayDouble();
        ((struct ArrayDoubleFuncTable*)sol->funcTable)->AddElement(sol, data);
        //solution = malloc(sizeof(struct Array *));
        *solution = sol;
        return;
    }
    debug_printf("--Unit: %d\n", currentUnit);
    int unitsLength = units->length;
    debug_printf("--Units available: %d\n", unitsLength);
    for(int i = 0; i < unitsLength; i++) {
        unit = ((struct UnitsArrayFuncTable*)units->funcTable)->UGetUnitAtIndex(units, i);
        if(unit != NULL) {
            newUnit = (unit->unit + currentUnit) % 100;
            debug_printf("---Unit: %d creates new unit %d\n", unit->unit, newUnit);
            struct Unit * newUnitObj = ((struct UnitsArrayFuncTable*)units->funcTable)->UGetUnit(units, newUnit);
            if(newUnitObj == NULL) {
                debug_printf("----New unit does not exist\n");
                struct Unit * unitToAdd = MakeUnit(newUnit);
                if(unit->array) {
                    ((struct ArrayDoubleFuncTable*)unitToAdd->array->funcTable)->CopyArray(unitToAdd->array, unit->array);
                }
                ((struct ArrayDoubleFuncTable*)unitToAdd->array->funcTable)->AddElement(unitToAdd->array, data);
                //printArrayOfDoubles(unitToAdd->array);
                ((struct UnitsArrayFuncTable*)units->funcTable)->UAddElement(units, *unitToAdd);
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
    unit = ((struct UnitsArrayFuncTable*)units->funcTable)->UGetUnit(units, currentUnit);
    if(unit == NULL) {
        struct Unit * unitToAdd = MakeUnit(currentUnit);
        ((struct ArrayDoubleFuncTable*)unitToAdd->array->funcTable)->AddElement(unitToAdd->array, data);
        ((struct UnitsArrayFuncTable*)units->funcTable)->UAddElement(units, *unitToAdd);
        debug_printf("--Creates new unit %d\n", unitToAdd->unit);
    }
    debug_printf("-Finished: %f\n", data);
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

#pragma mark testing

void testArrayDouble()
{
    size_t len = 10;
    double *array = malloc(sizeof(double) * len);
    array[0] = 2.3f;
    array[1] = 2.9f;
    array[5] = 1.2f;
    array[6] = 4.3f;
    
    struct Array *arr = MakeArrayDouble();
    ((struct ArrayDoubleFuncTable*)arr->funcTable)->AddElement(arr, array[0]);
    ((struct ArrayDoubleFuncTable*)arr->funcTable)->AddElement(arr, array[1]);
    ((struct ArrayDoubleFuncTable*)arr->funcTable)->AddElement(arr, array[6]);
    for(int count = 0; count < 30000; count++) {
       ((struct ArrayDoubleFuncTable*)arr->funcTable)->AddElement(arr, rand()/100);
    }
    printArrayOfDoubles(arr);
    free(((struct ArrayDoublePrivateData*)arr->privateData)->data);
    free(arr->privateData);
    free(arr);
    free(array);
}

struct test {
    char str[20];
    double d;
    int i;
    char *strp;
};

struct test getRandomTestStruct()
{
    struct test s;
    sprintf(s.str, "Str: %0.10d", rand());
    s.d = rand() / 1000;
    s.i = rand();
    s.strp = malloc(sizeof(char) * 20);
    sprintf(s.strp, "Dstr: %0.10d", rand());
    return s;
}

void printTestStruct(struct test t)
{
    debug_printf("Struct: str: '%s', d: %f, i %d, dstr: '%s'\n", t.str, t.d, t.i, t.strp);
}

void printArrayOfStructTest(struct Array* arr)
{
    for(int i = 0; i < arr->length; i++)
    {
        struct test t = ((struct test*)((struct ArrayPrivateData *)arr->privateData)->data)[i];
        printTestStruct(t);
    }
}

void freeStructTest(struct test *t)
{
    free(t->strp);
}

void freeArrayOfStructTestDynamicData(struct Array* arr)
{
    for(int i = 0; i < arr->length; i++)
    {
        struct test t = ((struct test*)((struct ArrayPrivateData *)arr->privateData)->data)[i];
        freeStructTest(&t);
    }
}

void freeArrayOfStructTest(struct Array* arr)
{
    
    free(((struct ArrayPrivateData *)arr->privateData)->data);
    free(arr->privateData);
}

void testArrayStruct()
{
    size_t len = 10;
    size_t size = sizeof(struct test);
    struct Array *arr = MakeArray(size);
    int count;
    for(count = 0; count < len; count++) {
        struct test t = getRandomTestStruct();
        //((struct ArrayFuncTable*)arr->funcTable)->AddElement(arr, &t);
        ArrayAddElement(arr, arr->size, &t);
    }
    printArrayOfStructTest(arr);
    //test copy array
    
    struct Array *test = MakeArray(size);
    ArrayCopyArray(test, arr);
    printArrayOfStructTest(test);
    //only once free the dynamic data as it is referenced also from the array copyed
    freeArrayOfStructTestDynamicData(arr);
    freeArrayOfStructTest(arr);
    free(arr);
    freeArrayOfStructTest(test);
    free(test);
}

#pragma mark Main

int main(int argc, const char * argv[])
{
    testArrayDouble();
    testArrayStruct();
    
    int charsToRead = 10;
    size_t readChars;
    size_t size = sizeof(char) * charsToRead;
    char *readData;

    readData = (char *)malloc(size);
    FILE *f = fopen("./data.txt", "r");
    int n = 0;
    char * leftOverData = "";
    
    struct Array * solution = NULL;
    struct Array * units = MakeUnitsArray();
    
    while(f && !feof(f)) {
        readChars = fread(readData, sizeof(char), charsToRead, f);
        if(readChars > 0) {
            debug_printf("Read data: chars: %lu, data: %s, left: %s\n", readChars, readData, leftOverData);
            processDataAndReturnLeftOver(readData, &leftOverData, &n, &solution, units);
            if(solution != NULL) {
                break;
            }
        }
    }
    fclose(f);
    
    free(readData);
    
    debug_printf("N: %d\n", n);
    printArrayOfUnits(units);
    
    if(solution != NULL) {
        printf("SOLUTION:\n");
        printArrayOfDoubles(solution);
    }
    
    debug_printf("End");
    
    FreeUnitsArray(units);
    
    return 0;
}

