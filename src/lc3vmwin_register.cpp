#include "lc3vmwin_register.hpp"
#include <assert.h>

LC3VMRegisterWindow::LC3VMRegisterWindow(int regNum, const void* regFile, const char** regNames, int regSize, const WindowConfig& config)
{
    /* Gracefully disable the window if regNum = 0 */
    if (regNum <= 0 || regSize <= 0)
    {
        fprintf(stderr, "Function: %s gracely shows you an error message: regNum and regSize should be natural numbers. \n", __func__);
        regFile = nullptr;
        regNames = nullptr;
        regSize = 0;
        regNum = 0;
    }
    else
    {
        if (regNum > sizeof(regFile) / regSize)
        {
            fprintf(stderr, "Function: %s gracely shows you an error message: regNum should be equal or smaller than # of elements in regFile. \n", __func__);
            regNum = sizeof(regFile) / regSize;
        }
        if (sizeof(regFile) % regSize != 0)
        {
            fprintf(stderr, "Function: %s gracely shows you an error message: number of elements in regFile (%d) should divide regSize (%d). May lose some data at the end. \n", __func__, sizeof(regFile), regSize);
        }
        this->regNum = regNum;
        this->regFile = regFile;
        this->regNames = regNames;
        this->regSizeInBytes = regSize;
    }
}

LC3VMRegisterWindow::~LC3VMRegisterWindow()
{
    regFile = nullptr;
    regNames = nullptr;
}