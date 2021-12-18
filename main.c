#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(char argc,char** argv,char** envp){
    char buffer[257];
    char **splitBuffer=(char**)malloc(11*sizeof(char*)); //10 arg, reserve an element for 0, meaning end
    char originPath[6][40]={"/usr/local/sbin/","/usr/sbin/","/sbin/","/usr/local/bin/","/usr/bin/","/bin/"};
    char finalPath[6][40];
    for(int i=0;i<10;i++){
        splitBuffer[i]=(char*)malloc(100*sizeof(char));
    }
    //memset(splitBuffer,NULL,sizeof(splitBuffer));
    printf("welcome to my sh system\n");
    printf("------processing loop-------\n");
    while(1){
        //memset(splitBuffer,0,sizeof(splitBuffer));
        printf("mySH: # ");
        for(int i=0;i<6;i++){
            strcpy(finalPath[i],originPath[i]);//reset path
        }
        fgets(buffer,256,stdin);
        int n=strlen(buffer);
        buffer[n-1]=0;//delete '\n'
        char* s=strtok(buffer," ");
        if(strcmp(s,"exit")==0) {
            for(int i=0;i<10;i++){
                free(splitBuffer[i]);
            }
            free(splitBuffer);
            printf("sh system is shut down\n");
            exit(0);
        }
        else if(strcmp(s,"cd")==0){
            printf("cd command is not supported yet\nPlease insert another cmd\n");
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
            for(int j=0;j<i;j++) printf("argv %d:%s\n",j,splitBuffer[j]);
            for(int i=0;i<6;i++){
                strcat(finalPath[i],splitBuffer[0]);
            }
            int pid=fork();
            if(pid){
                int status=0;
                int ret=wait(&status);
                splitBuffer[*pNull]=pNull;//restore the address
                printf("child process end with exitCode %d\n",status);
            }
            else{
                for(int i=0;i<6;i++){
                    printf("now try %s:",finalPath[i]);
                    execve(finalPath[i],splitBuffer,envp);//try every path
                    //char *tmp[]={"ls",NULL};
                    //execve("/bin/ls",tmp,envp);
                    printf("fail\n");
                }
                printf("cannot find command in the default directory\n");
                return -1;
            }
        }
        
        
    }

         
}