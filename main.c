#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>

void menu();
void getInformation(int*, int);
void getFolders();
char *formattedMemoryDigits(unsigned long long);
long int getSharedMemory(int);
char *getUser(int);
int getUserUID(int);
char *getExecutionTime(int);
char *formattedDate(char *, char *, char *);
void cleanName(char *);

long int getTotalCpuTime();
long int getWorkJiffies();

void waitMs(int);

typedef struct argsCpuThread {
    int index;
    int PID;
    float *cpuUsage;
} cpuT;

typedef struct {
    char name[100], state, *username, *executionTime,
        *virtMemory, *RSS, *SHR;
    long int priority, niceValue;
} procInfo;

float getPercentageOfMemUsage(long int);
void getCpuUsageOfProcess(cpuT *);
long int getProcessCPUTime(int);
procInfo *readStat(int, long int *);

int main(void) {
    // while (1) {
        // system("clear");
        printf("   PID  USER    PRI  NI  VIRT  RES  SHR  S   CPU%%  MEM%%   TIME+   Name\n");
        getFolders();
        // sleep(3);
    // }
    return 0;
}

void getFolders() {
    DIR *directoryProc;
    struct dirent *dp;
    char *filename;

    int *PIDs;
    int count = 0;

    directoryProc = opendir("/proc/");
    while ((dp = readdir(directoryProc)) != NULL) {
        filename = dp->d_name;
        if (atoi(filename) != 0) count++;
    }
    PIDs = malloc(count * sizeof(int));
    closedir(directoryProc);

    directoryProc = opendir("/proc/");
    int flag = 0;
    while ((dp = readdir(directoryProc)) != NULL) {
        int PID;
        filename = dp->d_name;
        if ((PID = atoi(filename)) != 0) {
            *(PIDs + flag) = PID;
            flag++;
        }
    }

    getInformation(PIDs, count);
    closedir(directoryProc);
    free(PIDs);
}

procInfo *readStat(int PID, long int *vmRSS) {
    char filename[50];
    sprintf(filename, "/proc/%d/stat", PID);
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        procInfo *empty = malloc(1);
        fprintf(stderr, "[!] - Houve um erro ao obter informações do processo PID: %d\n", PID);
        return empty;
    }
    long long unsigned int startTime;
    long int virtMem, sharedMem;
    procInfo *procI = malloc(sizeof(procInfo));
    fscanf(fp, "%*d %s %c %*d %*d %*d %*d %*d %*d %*d %*d %*d \
                %*d %*d %*d %*d %*d %ld %ld %*d %*d %llu %ld %ld",
                procI->name, &(procI->state), &(procI->priority),
                &(procI->niceValue), &startTime, &virtMem, vmRSS);
    
    sharedMem = getSharedMemory(PID);
    // procI->username = getUser(PID);
    procI->executionTime = getExecutionTime(startTime);
    procI->virtMemory = formattedMemoryDigits(virtMem);
    procI->RSS = formattedMemoryDigits(*vmRSS);
    procI->SHR = formattedMemoryDigits(sharedMem);
    cleanName(procI->name);
    fclose(fp);
    return procI;
}

void getInformation(int *PIDs, int PIDquantity) {
    float *savedCpuUse = malloc(PIDquantity * sizeof(float));
    pthread_t cpuThreads[PIDquantity];

    cpuT **cpuTArgsAddr = malloc(sizeof(cpuT *) * PIDquantity);
    for (int i = 0; i < PIDquantity; i++) {
        cpuT *cpuThreadArgs = (cpuT *) malloc(sizeof(cpuT));
        cpuTArgsAddr[i] = cpuThreadArgs;
        cpuThreadArgs->PID = *(PIDs + i);
        cpuThreadArgs->cpuUsage = savedCpuUse;
        cpuThreadArgs->index = i;
        pthread_create(&cpuThreads[i], NULL, (void *) getCpuUsageOfProcess, cpuThreadArgs);
    }

    for (int i = 0; i < PIDquantity; i++) {
        pthread_join(cpuThreads[i], NULL);
        free(*(cpuTArgsAddr+i));
    }
    free(cpuTArgsAddr);
    for (int i = 0; i < PIDquantity; i++) {
        long int vmRSS;
        int PID = *(PIDs+i);
        procInfo *procI = readStat(PID, &vmRSS);
        float memUse = getPercentageOfMemUsage(vmRSS);

        printf("%5ld   %s %4ld %3ld  %s  %3s   %s %2c %5.1f   %.1f    %s    %s\n", 
                PID, procI->username, procI->priority,
                procI->niceValue, procI->virtMemory, procI->RSS,
                procI->SHR, procI->state, *(savedCpuUse+i), memUse,
                procI->executionTime, procI->name);
        // free(procI->username);
        free(procI->executionTime);
        free(procI->virtMemory);
        free(procI->RSS);
        free(procI->SHR);
        free(procI);
    }
    free(savedCpuUse);
}

void waitMs(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


char *formattedMemoryDigits(unsigned long long bytes) {
    char *formattedDigits = malloc(20);
    unsigned long value;

    if (bytes >= 1099511627776ULL) {
        value = bytes / 1099511627776;
        sprintf(formattedDigits, "%luT", value);
    } else if (bytes >= 1073741824ULL) {
        value = bytes / 1073741824;
        sprintf(formattedDigits, "%luG", value);
    } else if (bytes >= 1048576ULL) {
        value = bytes / 1048576;
        sprintf(formattedDigits, "%luM", value);
    } else if (bytes >= 1024ULL) {
        value = bytes / 1024;
        sprintf(formattedDigits, "%luK", value);
    } else {
        sprintf(formattedDigits, "%lluB", bytes);
    }

    if (strlen(formattedDigits) == 2) {
        memmove(formattedDigits + 1, formattedDigits, 2);
        formattedDigits[0] = ' ';
        formattedDigits[3] = '\0';
    }

    return formattedDigits;
}

long int getSharedMemory(int PID) {
    char directory[50];
    sprintf(directory, "/proc/%d/smaps", PID);

    FILE *fp = fopen(directory, "r");
    if (fp == NULL) {
        fprintf(stderr, "Houve um erro ao obter o SHR do processo: %d\n", PID);
        return -1;
    }

    char fieldName[100];
    int totalSharedMemory = 0;
    while (fscanf(fp, "%s", fieldName) == 1) {
        if (strcmp(fieldName, "Shared_Clean:") == 0 || strcmp(fieldName, "Shared_Dirty:") == 0) {
            char fieldValue[100];
            fscanf(fp, "%s", fieldValue);
            totalSharedMemory += atoi(fieldValue);
        }
    }
    fclose(fp);
    return totalSharedMemory;
}

char *getUser(int PID) {
    int userUID = getUserUID(PID);
    printf("UserUID: %d\n", userUID);
    FILE *fp = fopen("/etc/passwd", "r");
    printf("fp is null? %d\n", fp == NULL);
    if (fp == NULL || userUID == -1) {
        char *err = malloc(4);
        char strErr[] = "[!]";
        memcpy(err, strErr, strlen(strErr));
        fprintf(stderr, "[!] - Houve um erro ao localizar o usuario do processo: %d\n", PID);
        return err;
    }

    while (!feof(fp)) {
        char *bufferUsername = malloc(50);
        char bufferUID[50];
        char line[1024];
        char *tok;
        
        fgets(line, sizeof(line), fp);
        tok = strtok(line, ":");

        int field = 0;
        while (tok != 0) {
            if (field == 0) strcpy(bufferUsername, tok);
            if (field == 2) strcpy(bufferUID, tok);
            tok = strtok(0, ":");
            field++;
        }

        if (atoi(bufferUID) == userUID) {
            size_t usernameLength = strlen(bufferUsername);

            if (usernameLength > 4) {
                strncpy(bufferUsername + 4, "..", 2);
                bufferUsername[6] = '\0';
            } else {
                for (size_t i = usernameLength; i < 4; i++) {
                    bufferUsername[i] = ' ';
                }
                bufferUsername[4] = ' ';
                bufferUsername[5] = ' ';
                bufferUsername[6] = '\0';
            }

            fclose(fp);
            return bufferUsername;
        }
    }
    fclose(fp);
}

int getUserUID(int PID) {
    char directory[50];
    sprintf(directory, "/proc/%d/status", PID);

    FILE *fp = fopen(directory, "r");
    if (fp == NULL) return -1;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        char fieldName[100];
        sscanf(line, "%s", fieldName);
        if (strcmp(fieldName, "Uid:") == 0) {
            int userUID;
            sscanf(line, "%*s %d", &userUID);
            return userUID;
        }
    }
    fclose(fp);
    return -1;
}

void getCpuUsageOfProcess(cpuT *input) {
    long matriz[2][2];

    matriz[0][0] = getTotalCpuTime();
    matriz[0][1] = getProcessCPUTime(input->PID);

    waitMs(100);

    matriz[1][0] = getTotalCpuTime();
    matriz[1][1] = getProcessCPUTime(input->PID);

    long deltaTotal = matriz[1][0] - matriz[0][0];
    long deltaProc = matriz[1][1] - matriz[0][1];

    float cpuUsage = ((float) deltaProc / (float) deltaTotal) * 100.0;
    *(input->cpuUsage + input->index) = cpuUsage;
    pthread_exit(NULL);
}

long int getProcessCPUTime(int PID) {
    char directory[50];
    sprintf(directory, "/proc/%d/stat", PID);

    FILE *fp = fopen(directory, "r");
    if (fp == NULL) {
        fprintf(stderr, "[!] - Erro ao obter uso de CPU de um dos processos\n");
        return 1;
    }

    long int utime, stime;
    fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u \
         %*lu %*lu %*lu %*lu %lu %lu", &utime, &stime);
    fclose(fp);
    return utime + stime;
}

long int getTotalCpuTime() {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro ao obter porcentagem de uso da CPU");
        return 1;
    }
    int user, nice, system, 
    idle, iowait, irq, softirq, steal;
   
    fscanf(fp, "%*s %d %d %d %d %d %d %d %d", 
       &user, &nice, &system, &idle, 
       &iowait, &irq, &softirq, &steal);
    
    return user + nice + system + idle +
        iowait + irq + softirq + steal;
}

long int getWorkJiffies() {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro ao obter porcentagem de uso da CPU");
        return 1;
    }

    int user, nice, system;
    fscanf(fp, "%*s %d %d %d", &user, &nice, &system);
    return user + nice + system;
}


float getPercentageOfMemUsage(long int vmRSS) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        fprintf(stderr, "[!] - Nao foi possivel obter o uso de memoria de um dos processos\n");
        return -1.0f;
    }
    long unsigned int totalMemory;

    fscanf(fp, "%*s %lu", &totalMemory);
    float memoryUse = ((float) vmRSS / (float) totalMemory) * 100;
    return memoryUse;
}

char *getExecutionTime(int startTime) {
    long int const CLK_TCK = sysconf(_SC_CLK_TCK);
    long unsigned int accumulatedSeconds = startTime / CLK_TCK;
    int hours = accumulatedSeconds / 3600;
    int minutes = (accumulatedSeconds % 3600) / 60;
    int seconds = seconds % 60;

    // Warning de possível overflow é esperado: os valores de hora, minuto e segundo
    // são sempre limitados a 2 dígitos, então o buffer é suficiente e seguro.
    char *date = malloc(9 * sizeof(char));
    sprintf(date, "%02d:%02d:%02d", hours, minutes, seconds);

    return date;
}

void cleanName(char *string) {
    char *newString;
    newString = malloc(sizeof(char) * strlen(string));
    memset(newString, '#', strlen(string));

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '(') continue;
        if (string[i] == ')') string[i] = '\0';

        for (int k = 0; string[i] != '\0'; k++) {
            if (newString[k] != '#') continue;
            newString[k] = string[i];
            break;
        }
    }

    for (int i = 0; i < strlen(newString); i++) if (newString[i] == '#') newString[i] = '\0';
    strcpy(string, newString);
    free(newString);
}