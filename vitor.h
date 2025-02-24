int power(int b, int e) {
    int a = 1;
    while(e--)
        a *= b;
    return a;
}

void convertInt(char *formStr, int *formStrIndex, int value) {
    char aux[128];
    int size = 0;
    
    if(value == 0)
        aux[size++] = '0';
    while(value > 0) {
        aux[size++] = value % 10 + '0';
        value /= 10;
    }
    size--;
    while(size >= 0)
        formStr[(*formStrIndex)++] = aux[size--];
}

void convertFloat(char *formStr, int *formStrIndex, double value, int qtd) {
    char aux[128];
    int size = 0;
    int m = value;
    int i, dec;
    value = (value - m) * power(10, qtd);
    
    dec = value; 
    for(i = 0; i < qtd; i++) {
        aux[size++] = dec % 10 + '0';
        dec /= 10;
    }
    aux[size++] = '.';
    
    if(m == 0)
        aux[size++] = '0';
    while(m > 0) {
        aux[size++] = m % 10 + '0';
        m /= 10;
    }

    size--;
    while(size >= 0)
        formStr[(*formStrIndex)++] = aux[size--];
}

#if defined(_WIN64) 

void writeFile(const char *buffer, unsigned long long size, unsigned long long *bytesWritten) {
	asm(	
		"push %%rbp;"
		"mov %%rsp,%%rbp;"
   		"sub $0x20,%%rsp;"	
		
		"mov $0x0fffffff5, %%ecx;"
		"call GetStdHandle;"
		"mov %%rax, %%rcx;"
		
		"mov %0, %%rdx;"
		"mov %1, %%r8;"		
		"mov %2, %%r9;"
		"movq $0x0,0x20(%%rsp);"
		"call WriteFile;"
		
		"add $0x20,%%rsp;"
   		"pop %%rbp;"
		:
		:"r"(buffer), "r"(size), "r"(bytesWritten)
		:"rcx","rdx","rax", "r9", "r8"
	);
}

void formatStr(char *formStr, const char *ori, void *paramPtr) {
    int i, formStrIndex;
    
    formStrIndex = 0;
    for(i = 0; ori[i]; i++) {
        if(ori[i] != '%') 
            formStr[formStrIndex++] = ori[i];
        else {
            switch(ori[i + 1]) {
                case 'c':
                    formStr[formStrIndex++] = * (char *)paramPtr;
                break;
                case 'd':
                    convertInt(formStr, &formStrIndex, * (int *)paramPtr);
                break;
                case 'f':
                    convertFloat(formStr, &formStrIndex, * (double *)paramPtr,6);
                break;
            }
            paramPtr = paramPtr + 8;
            i++;
        }
        
    }
    formStr[formStrIndex] = 0;
    
}

void write(const char *buffer, ...) {
    int i;
	unsigned long long size;
	unsigned long long bytesWritten;
    char formStr[256];
    void *paramPtr = &buffer + 1;
    
    formatStr(formStr, buffer, paramPtr);
    
    size = 0;
    while(formStr[size])size++;
    
    writeFile(formStr,size,&bytesWritten);
    
}

#elif defined(_WIN32)


#define GetStdHandle(result, handle)                                    \
    __asm (                                                             \
        "pushl  %1\n\t"                                                 \
        "call   _GetStdHandle@4"                                        \
            : "=a" (result)                                             \
            : "g" (handle)                                              \
            : "ecx", "edx")                            

/*
void GetStdHandle(void *result, int handle) {
	__asm (                                                             
        "pushl  %1\n\t"                                                 
        "call   _GetStdHandle@4"                                        
            : "=a" (result)                                             
            : "g" (handle)                                              
            : "ecx", "edx");
}
*/

void WriteFile(int result, void* handle, const char* buf, int buf_size, unsigned long *written_bytes)  {
	asm (                                                  
        "pushl  $0\n\t"                                                 
        "pushl  %1\n\t"                                                 
        "pushl  %2\n\t"                                                 
        "pushl  %3\n\t"                                                 
        "pushl  %4\n\t"                                                 
        "call   _WriteFile@20"                                          
            : "=a" (result)                                             
            : "r" (written_bytes), "r" (buf_size), "r" (buf), "r" (handle) 
            : "memory", "ecx", "edx"
		);
}      

void formatStr(char *formStr, const char *ori, void *paramPtr) {
    int i, formStrIndex;
    
    formStrIndex = 0;
    for(i = 0; ori[i]; i++) {
        if(ori[i] != '%') 
            formStr[formStrIndex++] = ori[i];
        else {
            switch(ori[i + 1]) {
                case 'c':
                    formStr[formStrIndex++] = * (char *)paramPtr;
                break;
                case 'd':
                    convertInt(formStr, &formStrIndex, * (int *)paramPtr);
                break;
                case 'f':
                    convertFloat(formStr, &formStrIndex, * (double *)paramPtr,6);
                    paramPtr = paramPtr + 4;
                break;
            }
            paramPtr = paramPtr + 4;
            i++;
        }
        
    }
    formStr[formStrIndex] = 0;
    
}

void write(const char *buffer, ...) {
    int i;
	unsigned long long size;
	unsigned long long bytesWritten;
    char formStr[256];
    void *paramPtr = &buffer + 1;
    void* handle;
    int write_result;
    unsigned long written_bytes; 
    
    formatStr(formStr, buffer, paramPtr);
    
    size = 0;
    while(formStr[size])size++;
    
    GetStdHandle(handle, -11);
    if(handle != (void *) -1)  
        WriteFile(write_result, handle, formStr, size, &written_bytes);
    
}

#elif defined (__linux__)

__attribute__((ms_abi)) void formatStr(char *formStr, const char *ori, void *paramPtr) {
    int i, formStrIndex;
    
    formStrIndex = 0;
    for(i = 0; ori[i]; i++) {
        if(ori[i] != '%') 
            formStr[formStrIndex++] = ori[i];
        else {
            switch(ori[i + 1]) {
                case 'c':
                    formStr[formStrIndex++] = *(char *)paramPtr;
                break;
                case 'd':
                    convertInt(formStr, &formStrIndex, *(int *)paramPtr);
                break;
                case 'f':
                    convertFloat(formStr, &formStrIndex, *(double *)paramPtr,6);
                break;
            }
            
            i++;
            paramPtr = paramPtr + 8;
        }
        
    }
    formStr[formStrIndex] = 0;
    
}

__attribute__((ms_abi)) void write(const char *buffer, ...) {
    int i, size;
    static char formStr[256];
    void *paramPtr = &buffer + 1;
    
    formatStr(formStr, buffer, paramPtr);
    
    size = 0;
    while(formStr[size])size++;
    
    asm(
        "SYSCALL;"
        :
        :"a"(1), "D"(1), "d"(size), "S"(formStr)
    );
    
}

#endif
