#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/**
 * @brief deal with the redirection symbol, including "<", ">", and ">>".
 * 
 * @param cmd the cmd from keyboard
 * @param len the lenth of cmd
 * @param filePath to save the filaPath extracted in cmd
 * @return int return -1 if error occurs, 0 if there is no redirectionm, 1~3 for different redirections
 */
enum{
    inputRe=1,
    outputRe,
    outputDoubleRe
};

int dealWithRedirection(char* cmd,int len,char* filePath){
    int i=0,k=0;
    int fd;
    int ret=0;
    int isDoubleOutput=0;
    int outputRedirectionIndex=-1,inputRedirectionIndex=-1;
    for(i=0;i<len;i++){
        if(outputRedirectionIndex==-1 && cmd[i]=='>') outputRedirectionIndex=i;
        if(inputRedirectionIndex==-1 && cmd[i]=='<') inputRedirectionIndex=i;
        if(outputRedirectionIndex!=-1 && inputRedirectionIndex!=-1){
            printf("A command including both '>' and '<' has not been supported yet\n");
            return -1;
        } 
    }
    if(outputRedirectionIndex!=-1){
        int fileBeginIndex=0;
        if(cmd[outputRedirectionIndex+1]=='>'){
            fileBeginIndex = outputRedirectionIndex+2;
            isDoubleOutput=1;
            ret=outputDoubleRe;
        }
        else{
            fileBeginIndex = outputRedirectionIndex+1;
            isDoubleOutput=0;
            ret=outputRe;
        }   
        for(int j=fileBeginIndex;j<len;j++){
            if(cmd[j]==' ')continue;
            else filePath[k++]=cmd[j];
        }
        filePath[k]='\0'; 
        strtok(cmd,">");//strtok is used to destroy the original cmd and save only the first half of cmd
        int n=strlen(cmd);
        if(cmd[n-1]==' ')cmd[n-1]=0;
    }

    if(inputRedirectionIndex!=-1){
        k=0;
        for(int j=inputRedirectionIndex+1;j<len;j++){
            if(cmd[j]==' ')continue;
            else filePath[k++]=cmd[j];
        }
        ret=inputRe;
        strtok(cmd,"<");
        int n=strlen(cmd);
        if(cmd[n-1]==' ')cmd[n-1]=0;
    }
    return ret;
}

int main(char argc,char** argv,char** envp){
    char orinBuffer[257];
    char bufferAfterRe[257];
    char **splitBuffer=(char**)malloc(11*sizeof(char*)); //10 arg, reserve an element for 0, meaning end
    char originPath[6][40]={"/usr/local/sbin/","/usr/sbin/","/sbin/","/usr/local/bin/","/usr/bin/","/bin/"};
    char finalPath[6][40];
    char filePath[100];
    char cwd[60];
    int flagRe=-1;
    for(int i=0;i<10;i++){
        splitBuffer[i]=(char*)malloc(100*sizeof(char));
    }
    printf("\twelcome to my sh system\n");
    printf("--------------processing loop---------------\n");
    while(1){
        getcwd(cwd,60);
        printf("mySH:%s$ ",cwd);
        for(int i=0;i<6;i++){
            strcpy(finalPath[i],originPath[i]);//reset path
        }
        fgets(orinBuffer,256,stdin);
        if(strcmp(orinBuffer,"\n")==0) continue;
        int n=strlen(orinBuffer);
        orinBuffer[n-1]=0;//delete '\n'
        strcpy(bufferAfterRe,orinBuffer);
        flagRe=dealWithRedirection(bufferAfterRe,n-1,filePath);
        if(flagRe<0)continue;
        char* s=strtok(bufferAfterRe," ");
        if(strcmp(s,"exit")==0) {
            for(int i=0;i<10;i++){
                free(splitBuffer[i]);
            }
            free(splitBuffer);
            printf("sh system is shut down\n");
            exit(0);
        }
        else if(strcmp(s,"cd")==0){
            s=strtok(NULL," ");
            if(s){
                chdir(s);
            }
            else{
                chdir("/home/liyu");
            }
        }
        else{
            int i=0;//number of arg - 1
            strcpy(splitBuffer[i++],s);
            while(s=strtok(NULL," ")){
                if(i>=10){
                    printf("number of arg should less than 11\n");
                    exit(0);
                }
                strcpy(splitBuffer[i++],s);
            }
            
            char* pNull;
            pNull=splitBuffer[i];
            *pNull=i;
            splitBuffer[i]=NULL;  
            //for(int j=0;j<i;j++) printf("argv %d:%s\n",j,splitBuffer[j]);
            for(int i=0;i<6;i++){
                strcat(finalPath[i],splitBuffer[0]);
            }
            int pid=fork();
            if(pid){//parent process
                int status=0;
                int ret=wait(&status);
                splitBuffer[*pNull]=pNull;//restore the address
                printf("child process end with exitCode %d\n",status);
            }
            else{//child process
                if(flagRe==outputDoubleRe){
                    int fd=open(filePath,O_APPEND | O_RDWR | O_CREAT,0666);//in this process,the fd is closed by process
                    if(fd<0){
                        printf("the input file %s does not exist\n",filePath);
                        exit(-1);
                    }
                    dup2(fd,1);
                }
                else if(flagRe==outputRe){
                    int fd=open(filePath,O_RDWR | O_CREAT,0666);
                    if(fd<0){
                        printf("the input file %s does not exist\n",filePath);
                        exit(-1);
                    }            
                    ftruncate(fd,0);//clear the contents in oringinal file. 
                                    //This is necessarry,but I don't known why the contents do not clear automatically
                                    //In Linux shell, the file always clears automatically
                    dup2(fd,1);
                }
                else if(flagRe==inputRe){
                    int fd=open(filePath,O_RDONLY);
                    if(fd<0) {
                        printf("the input file %s does not exist\n",filePath);
                        exit(-1);
                    }
                    dup2(fd,0);
                }
                else{
                    //there is no redirection
                }
                
                for(int i=0;i<6;i++){
                    //printf("now try %s:",finalPath[i]);
                    execve(finalPath[i],splitBuffer,envp);//try every path
                    //printf("fail\n");
                }
                printf("cannot find command in the default directory\n");
                return -1;
            }
        }   
    }
}