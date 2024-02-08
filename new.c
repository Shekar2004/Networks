#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// void deletemails(int a[],char *filepath)
// {
//     char buf[100];
//     FILE *curr=fopen("mymailbox.txt","r");
//     FILE *new=fopen("newfile.txt","w");
//     int count=0,state=0;
//     while(fgets(buf,100,curr)!=NULL)
//     {
//         if(strncmp(buf,"From:",5)==0)
//         {
//             count++;
//             if(a[count]==1)state=1;
//             else state=0;
//         }
//         if(state==1);
//         else fprintf(new,"%s",buf);
//     }fclose(curr);fclose(new);
//     rename("newfile.txt","wenfile.txt");
//     remove("mymailbox.txt");
// }

void deletemails(int a[],char *username)
{
    char buf[100];
    char file1[120];
    char file2[120];
    snprintf(file1,sizeof(file1),"./%s/mymailbox",username);
    snprintf(file2,sizeof(file2),"./%s/newfile",username);
    FILE *curr=fopen(file1,"r");
    FILE *new=fopen(file2,"w");
    int count=0,state=0;
    while(fgets(buf,100,curr)!=NULL)
    {
        if(strncmp(buf,"From:",5)==0)
        {
            count++;
            if(a[count-1]==1)state=1;
            else state=0;
        }
        if(state==1);
        else fprintf(new,"%s",buf);
    }fclose(curr);fclose(new);
    remove(file1);
    rename(file2,file1);
}

int main()
{
    int a[6]={0,1,0,0,1,0};
    deletemails(a,"user1");
}