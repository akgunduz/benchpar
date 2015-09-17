//
// Created by Haluk AKGUNDUZ on 13/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "function.h"

Function::Function() {

}

FuncList::FuncList()
{ 
        for (int i = 0; i < MAX_ARGUMENT; i++) {
                strcpy(kernelid[0], "");
        }
}

void FuncList::set(const char *id, fFuncs func, int argCount, const char *kernelid[], int argument[]) {
        this->id = id;
        if (argCount > MAX_ARGUMENT) {
            argCount = MAX_ARGUMENT;
        }

        this->argCount = argCount;

        for (int i = 0; i < argCount; i++) {
                
            if (kernelid != NULL) {
                strcpy(this->kernelid[i], kernelid[i]);
            }
            
            if (argument != NULL) {
                this->argument[i] = argument[i];
            } else {
                this->argument[i] = 0;
            }

        }

        this->f = func;
};