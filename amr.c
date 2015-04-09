#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


//prototype
int matchCode(int argc, char* argv[], char* name); 
//hi

struct TimeSlot{
    char date[11];
    char start_time[6];
    char end_time[6];
    int start_time_int;
    int end_time_int;
    char type[10];
    int rescheduled;
    char people[30];
};



int main(int argc, char* argv[]){
    int userNum = argc-1;
    int fd[userNum][2], fd2[userNum][2];
    int pid;
    int i;
    char* representName;
    int childCode;
    char buf[100];
    FILE *infile, *outfile;

    outfile = fopen("schd.txt", "w");
    if (outfile == NULL) {
    printf("Error in opening output file\n");
    exit(1);
    }


    if(userNum <3 || userNum > 10){
        printf("ERROR: user number should be in 3 - 10, now %d users inputted\n", userNum);
        exit(1);
    }
    
    
    /////////create pipe for communication later
    for(i=0;i<userNum;i++){
        //pipe that parent write to child
        if (pipe(fd[i]) < 0) {
            printf("Pipe creation error\n");
            exit(1);
        }
        //pipe that child write to parent
        if (pipe(fd2[i]) < 0) {
            printf("Pipe creation error\n");
            exit(1);
        }
    }
    //////////fork child for each user
    for(i=0;i<userNum;i++){
        pid = fork();
        if (pid == 0){//child no need fork
            childCode = i;
            representName = argv[i+1];
            break;
        }
    }
    
    
    
    
    
    
    if(pid < 0){
        printf("error fork");
        exit(1);
    }else if (pid ==0){
        //###############Child part##################
        
        //close the irrelevant pipe
        for(i=0;i<userNum;i++){
            if (i==childCode){
                close(fd[i][1]);//child dont write to fd
                close(fd2[i][0]);//child dont read from fd2
                continue;
            }
            close(fd[i][0]);//close other childs pipe
            close(fd[i][1]);
            close(fd2[i][0]);
            close(fd2[i][1]);
        }
        
        //create timeSlot list
        
        struct TimeSlot acceptedTimeSlot[40];
        struct TimeSlot rejectedTimeSlot[999];
        int acceptedCount = 0;
        int rejectedCount = 0;


        


        //read the pipe from parent
        while(1){
            i = read(fd[childCode][0],buf,100);
            buf[i]=0;
            
            //printf("I am %s, I received message \"%s\"\n",representName,buf);
            
            //split the string
            int argIndex = 0;
            char argument[14][20];//command + user max 14 string;
            i=0;
            int j=0;
            int splitCount = 0;
            while(buf[i] != 0){
                argument[argIndex][j++] = buf[i++];//copy until end or space
                if (buf[i] == ' ' && splitCount < 5){//no need split name of people(one string contains few names)
                    argument[argIndex][j] = 0; //replace space with null byte
                    i++;
                    j=0;
                    argIndex++;
                    splitCount++;
                }
            }//end while
            argument[argIndex][j] = 0;//terminate the last string
            int argCount = argIndex+1;
            //end split the string
            

            //addStudy -adam xx xx xx xx
            //remove the minus character
            char temp[20];
            for(i=0;argument[1][i]!=0;i++){
                temp[i] = argument[1][i+1];
            }
            temp[i] = 0;
            strcpy(argument[1],temp);
            //end remove the minus character
            
            
            //printf("\n@@@@@@@@@@@@@@@@@@@argument[1] is %s after split\n\n", argument[1]);
            
            
            
            //do the task
            if(strcmp(argument[0], "printSchd")==0){

                if(pendingCount < 1){
                    printf("%s has no appointment\n",representName);
                    continue;
                }
                //printf("argument[1] is >>%s<<\n",argument[1]);
                if(strcmp(argument[1],"fcfs")==0){
                    //printf("argument[1] is fcfs\n");
                    for(i=0;i<pendingCount;i++){
                        //printf("In outer loop\n");
                        int okToInsert = 1;
                        for(j=0;j<acceptedCount;j++){
                            //printf("In inside loop\n");
                            //determine the availability

                            if (strcmp(pendingTimeSlot[i].date, pendingTimeSlot[j].date)==0 && //in the same day, not the case that
                            !(( pendingTimeSlot[i].start_time_int < acceptedTimeSlot[j].start_time_int && //pending happen before accepted
                            pendingTimeSlot[i].end_time_int <= acceptedTimeSlot[j].start_time_int) || //or
                            (pendingTimeSlot[i].start_time_int >= acceptedTimeSlot[j].end_time_int &&// after the accepted
                            pendingTimeSlot[i].end_time_int > acceptedTimeSlot[j].end_time_int))){//without collision
                                okToInsert = 0;
                                //printf("OK to insert = 0 in i=%d,j=%d\n",i,j);
                                break;
                            }
                        }

                        if(okToInsert){
                            //printf("OK to Insert\n");
                            acceptedTimeSlot[acceptedCount].start_time_int = pendingTimeSlot[i].start_time_int;
                            acceptedTimeSlot[acceptedCount].end_time_int = pendingTimeSlot[i].end_time_int;
                            strcpy(acceptedTimeSlot[acceptedCount].date, pendingTimeSlot[i].date);
                            strcpy(acceptedTimeSlot[acceptedCount].start_time,pendingTimeSlot[i].start_time);
                            strcpy(acceptedTimeSlot[acceptedCount].end_time, pendingTimeSlot[i].end_time);
                            strcpy(acceptedTimeSlot[acceptedCount].type,pendingTimeSlot[i].type);
                            strcpy(acceptedTimeSlot[acceptedCount].people,pendingTimeSlot[i].people);
/*
                            if (strcmp(acceptedTimeSlot[acceptedCount].type, "Project")==0)
                            {
                                sprintf(buf, "ok");
                                write(fd2[0][1],buf,100);

                            }
*/
                            acceptedCount++;
                        }
                        else{
                            //printf("reject\n");
                            rejectedTimeSlot[rejectedCount].start_time_int = pendingTimeSlot[i].start_time_int;
                            rejectedTimeSlot[rejectedCount].end_time_int = pendingTimeSlot[i].end_time_int;
                            strcpy(rejectedTimeSlot[rejectedCount].date, pendingTimeSlot[i].date);
                            strcpy(rejectedTimeSlot[rejectedCount].start_time,pendingTimeSlot[i].start_time);
                            strcpy(rejectedTimeSlot[rejectedCount].end_time, pendingTimeSlot[i].end_time);
                            strcpy(rejectedTimeSlot[rejectedCount].type,pendingTimeSlot[i].type);
                            strcpy(rejectedTimeSlot[rejectedCount].people,pendingTimeSlot[i].people);
                            /*
                            if (strcmp(rejectedTimeSlot[rejectedCount].type, "Project")==0)
                            {
                                sprintf(buf, "cancelPG %s %s %s %s  ",argument[2],argument[3],argument[4],argument[5]);
                                write(fd2[0][1],buf,100);

                            }
*/
                            rejectedCount++;

                        }




                    }





                }else if(strcmp(argument[1],"prio")==0){
                    //printf("argument[1] is priority\n");
                    for(i=0;i<pendingCount;i++){
                        //printf("In outer loop\n");
                        int okToInsert = 1;
                        int okToReplace = 0;
                        for(j=0;j<acceptedCount;j++){
                            //printf("In inside loop\n");
                            //determine the availability

                            if (strcmp(pendingTimeSlot[i].date, pendingTimeSlot[j].date)==0 && //in the same day, not the case that
                            !(( pendingTimeSlot[i].start_time_int < acceptedTimeSlot[j].start_time_int && //pending happen before accepted
                            pendingTimeSlot[i].end_time_int <= acceptedTimeSlot[j].start_time_int) || //or
                            (pendingTimeSlot[i].start_time_int >= acceptedTimeSlot[j].end_time_int &&// after the accepted
                            pendingTimeSlot[i].end_time_int > acceptedTimeSlot[j].end_time_int))){//without collision
                                okToInsert = 0;
                                //printf("OK to insert = 0 in i=%d,j=%d\n",i,j);
                                printf("type =%s\n",pendingTimeSlot[i].type);
                                if        (    (strcmp(pendingTimeSlot[i].type, "Assignment")==0)
                                        ||    (    strcmp(pendingTimeSlot[i].type,"Project")==0&&!(strcmp(acceptedTimeSlot[j].type,"Assignment")==0)    )
                                        ||    (    strcmp(pendingTimeSlot[i].type,"Study")==0&&strcmp(acceptedTimeSlot[j].type,"Gathering")==0        )        )
                                {
                                    okToReplace = 1;
                                    //printf("OK to replace = 1 in i=%d,j=%d\n",i,j);
                                    break;
                                }
                                break;
                            }

                        }

                        if(okToInsert){
                            //printf("OK to Insert\n");
                            acceptedTimeSlot[acceptedCount].start_time_int = pendingTimeSlot[i].start_time_int;
                            acceptedTimeSlot[acceptedCount].end_time_int = pendingTimeSlot[i].end_time_int;
                            strcpy(acceptedTimeSlot[acceptedCount].date, pendingTimeSlot[i].date);
                            strcpy(acceptedTimeSlot[acceptedCount].start_time,pendingTimeSlot[i].start_time);
                            strcpy(acceptedTimeSlot[acceptedCount].end_time, pendingTimeSlot[i].end_time);
                            strcpy(acceptedTimeSlot[acceptedCount].type,pendingTimeSlot[i].type);
                            strcpy(acceptedTimeSlot[acceptedCount].people,pendingTimeSlot[i].people);
                            acceptedCount++;
                        }
                        else if(okToReplace){
                            //printf("OK to Replace\n");

                            rejectedTimeSlot[rejectedCount].start_time_int = acceptedTimeSlot[acceptedCount-1].start_time_int;
                            rejectedTimeSlot[rejectedCount].end_time_int = acceptedTimeSlot[acceptedCount-1].end_time_int;
                            strcpy(rejectedTimeSlot[rejectedCount].date, acceptedTimeSlot[acceptedCount-1].date);
                            strcpy(rejectedTimeSlot[rejectedCount].start_time,acceptedTimeSlot[acceptedCount-1].start_time);
                            strcpy(rejectedTimeSlot[rejectedCount].end_time, acceptedTimeSlot[acceptedCount-1].end_time);
                            strcpy(rejectedTimeSlot[rejectedCount].type,acceptedTimeSlot[acceptedCount-1].type);
                            strcpy(rejectedTimeSlot[rejectedCount].people,acceptedTimeSlot[acceptedCount-1].people);
                            rejectedCount++;

                            acceptedTimeSlot[acceptedCount-1].start_time_int = pendingTimeSlot[i].start_time_int;
                            acceptedTimeSlot[acceptedCount-1].end_time_int = pendingTimeSlot[i].end_time_int;
                            strcpy(acceptedTimeSlot[acceptedCount-1].date, pendingTimeSlot[i].date);
                            strcpy(acceptedTimeSlot[acceptedCount-1].start_time,pendingTimeSlot[i].start_time);
                            strcpy(acceptedTimeSlot[acceptedCount-1].end_time, pendingTimeSlot[i].end_time);
                            strcpy(acceptedTimeSlot[acceptedCount-1].type,pendingTimeSlot[i].type);
                            strcpy(acceptedTimeSlot[acceptedCount-1].people,pendingTimeSlot[i].people);

                        }
                        else{
                            //printf("reject\n");
                            rejectedTimeSlot[rejectedCount].start_time_int = pendingTimeSlot[i].start_time_int;
                            rejectedTimeSlot[rejectedCount].end_time_int = pendingTimeSlot[i].end_time_int;
                            strcpy(rejectedTimeSlot[rejectedCount].date, pendingTimeSlot[i].date);
                            strcpy(rejectedTimeSlot[rejectedCount].start_time,pendingTimeSlot[i].start_time);
                            strcpy(rejectedTimeSlot[rejectedCount].end_time, pendingTimeSlot[i].end_time);
                            strcpy(rejectedTimeSlot[rejectedCount].type,pendingTimeSlot[i].type);
                            strcpy(rejectedTimeSlot[rejectedCount].people,pendingTimeSlot[i].people);
                            rejectedCount++;
                        }
                    }

                }else if(strcmp(argument[1],"opti")==0){

                }

                
                printf("***Appointment Schedule --ACCEPTED***\n");
                printf("%s, you have %d appointments.\n", representName, acceptedCount);
                printf("  Date\tStart\tEnd\tType\tRescheduled\tPeople\n");
                
                fprintf(outfile, "***Appointment Schedule ??ACCEPTED***\n");
                fprintf(outfile, "%s, you have %d appointments.\n", representName, acceptedCount);
                fprintf(outfile, "  Date\tStart\tEnd\tType\tRescheduled\tPeople\n");

                for(i=0;i<acceptedCount;i++){
                    printf("%s\t%s\t%s\t%s\t%s\t%s\n",acceptedTimeSlot[i].date,acceptedTimeSlot[i].start_time,acceptedTimeSlot[i].end_time,acceptedTimeSlot[i].type,"No", acceptedTimeSlot[i].people);
                    fprintf(outfile, "%s\t%s\t%s\t%s\t%s\t%s\n",acceptedTimeSlot[i].date,acceptedTimeSlot[i].start_time,acceptedTimeSlot[i].end_time,acceptedTimeSlot[i].type,"No", acceptedTimeSlot[i].people);
                }
                
                printf("\n---------------------------------------------------------\n");
                fprintf(outfile, "\n---------------------------------------------------------\n");

                printf("***Appointment Schedule --REJECTED***\n");
                printf("%s, you have %d appointments rejected.\n", representName, rejectedCount);
                printf("  Date\tStart\tEnd\tType\tRescheduled\tPeople\n");

                fprintf(outfile, "***Appointment Schedule ??REJECTED***\n");
                fprintf(outfile, "%s, you have %d appointments rejected.\n", representName, rejectedCount);
                fprintf(outfile, "  Date\tStart\tEnd\tType\tRescheduled\tPeople\n");

                for(i=0;i<rejectedCount;i++){
                    printf("%s\t%s\t%s\t%s\t%s\t%s\n",rejectedTimeSlot[i].date,rejectedTimeSlot[i].start_time,rejectedTimeSlot[i].end_time,rejectedTimeSlot[i].type,"No", rejectedTimeSlot[i].people);
                    fprintf(outfile, "%s\t%s\t%s\t%s\t%s\t%s\n",rejectedTimeSlot[i].date,rejectedTimeSlot[i].start_time,rejectedTimeSlot[i].end_time,rejectedTimeSlot[i].type,"No", rejectedTimeSlot[i].people);
                }

                printf("\n---------------------------------------------------------\n");
                fprintf(outfile, "\n---------------------------------------------------------\n");
                
                printf("Performance:\n");
                printf("Total Number of Appointment Assigned: %d\n", acceptedCount);
                printf("Total Number of Appointment Rejected: %d\n", rejectedCount);
                printf("Total Number of Appointment pendingCount: %d\n", pendingCount);
                float u = (acceptedCount/pendingCount)*100;
                printf("Utilization of Time Slot: %.2f %\n", u);
                
                fprintf(outfile, "Performance:\n");
                fprintf(outfile, "Total Number of Appointment Assigned: %d\n", acceptedCount);
                fprintf(outfile, "Total Number of Appointment Rejected: %d\n", rejectedCount);
                fprintf(outfile, "Utilization of Time Slot: %.2f %\n", u);
                
                
                printf("\n---------------------------------------------------------\n");
                fprintf(outfile, "\n---------------------------------------------------------\n");
                

                fclose(outfile);
                printf("####  Done printSchd ####\n");
            }else if(strcmp(argument[0], "endProgram")==0){
                //close the pipe
                close(fd[childCode][0]);
                close(fd2[childCode][1]);
                printf("Child %s exit\n",representName);
                exit(0);
            }else if(strcmp(argument[0], "cancelPG")==0){
                printf("child cancel received\n");
                for(j=0;j<acceptedCount;j++){
                    if((strcmp(acceptedTimeSlot[j].date, argument[1])==0)&&(strcmp(acceptedTimeSlot[j].start_time,argument[2])==0))
                    {
                        printf("%d found\n",j);
                        printf("now replace\n");
                        rejectedTimeSlot[rejectedCount].start_time_int = acceptedTimeSlot[j].start_time_int;
                        rejectedTimeSlot[rejectedCount].end_time_int = acceptedTimeSlot[j].end_time_int;
                        strcpy(rejectedTimeSlot[rejectedCount].date, acceptedTimeSlot[j].date);
                        strcpy(rejectedTimeSlot[rejectedCount].start_time,acceptedTimeSlot[j].start_time);
                        strcpy(rejectedTimeSlot[rejectedCount].end_time, acceptedTimeSlot[j].end_time);
                        strcpy(rejectedTimeSlot[rejectedCount].type,acceptedTimeSlot[j].type);
                        strcpy(rejectedTimeSlot[rejectedCount].people,acceptedTimeSlot[j].people);
                        rejectedCount++;
                    }
                    else printf("%d not found\n",j);
                    printf("end loop\n");

                }
                printf("end cancel\n");


            }else{

                //printf("\n\n\n\nInside else\n");
                char type[10]; //e.g cut "addStudy" to "Study"
                j=0;
                for(i=3;argument[0][i]!=0;i++){
                    type[j++] = argument[0][i];
                }
                type[j] = 0;

                ///////////convert string of start time to int
                char temp[5];
                temp[0] = argument[3][0];
                temp[1] = argument[3][1];
                if (argument[3][3] == '3'){//skip the colon 18:00 -> 1800
                    temp[2] = '5';    // use 1950 to represent 19:30, for better calculation
                }else{
                    temp[2] = '0';
                }
                temp[3] = argument[3][4];
                temp[4] = 0;

                int end_time = atoi(temp) + (atof(argument[4]) * 100);
                //printf("temp is %s\n start_time_int is :%d\n atof is %f\n",temp,atoi(temp),(atof(argument[4]) * 100));
                char tempString[5];
                sprintf(tempString, "%d", end_time);
                tempString[4]=0;
                char end_time_string[6];
                end_time_string[0] = tempString[0];
                end_time_string[1] = tempString[1];
                end_time_string[2] = ':';
                if (tempString[2] == '5'){
                    end_time_string[3] = '3';
                }else{
                    end_time_string[3] = '0';
                }
                end_time_string[4] = tempString[3];
                end_time_string[5] = 0;
                ///////////end convert string of start time to int

                //printf("address of timeSlot[%d].date = %p\n",pendingCount,&argument[2]); <<always same address
                pendingTimeSlot[pendingCount].start_time_int = atoi(temp);
                pendingTimeSlot[pendingCount].end_time_int = end_time;
                strcpy(pendingTimeSlot[pendingCount].date, argument[2]);
                strcpy(pendingTimeSlot[pendingCount].start_time,argument[3]);
                strcpy(pendingTimeSlot[pendingCount].end_time, end_time_string);
                strcpy(pendingTimeSlot[pendingCount].type,type);
                if(argCount<6){//no accompany people
                    strcpy(pendingTimeSlot[pendingCount].people ,"---");
                }else{
                    strcpy(pendingTimeSlot[pendingCount].people , argument[5]);
                }
                pendingCount++;
                
                printf("#####  Pending %s  #####\n",argument[0]);
            }
        }
    }else{
        //#############Parent part####################
        
        //close the irrelevant pipe
        for(i=0;i<userNum;i++){
                close(fd2[i][1]);//parent dont write to fd2
                close(fd[i][0]);//parent dont read from fd
        }
        
        
        
        printf("################  Welcome To Appointment Manager  ################\n");
        printf("methods you can use: \n");
        printf("addStudy, addAssignment, addProject, addGathering, addBatch, printSchd, endProgram\n");
        
		
		struct TimeSlot pendingTimeSlot[999]; //= malloc(999* sizeof (struct TimeSlot));
		 int pendingCount = 0;
		
        //keep reading input command until read "endProgram"
        while(1){
            printf("Please Enter Appointment: \n");
            //read a line
            i = read(STDIN_FILENO, buf, 100);
            buf[--i] = 0; //replace '\n' with null terminator
            
            
            //split the input
            int argIndex = 0;
            char argument[14][20];
            i=0;
            int j=0;
            while(buf[i] != 0){
                argument[argIndex][j++] = buf[i++];//copy until end or space
                if (buf[i] == ' '){
                    argument[argIndex][j] = 0; //replace space with null byte
                    i++;
                    j=0;
                    argIndex++;
                }
            }//end while
            argument[argIndex][j] = 0;//terminate the last string
            int argCount = argIndex+1;
            
            

            //printf("!!!!!!!!!!!!!!!Now I split the string:!!!!!!!!!!!!!!!!!!!!!\n");
            //printf("argIndex = %d\n" , argIndex);
            //for(i=0;i<argCount;i++){
            //    printf("%s\t",argument[i]);
            //}
            //printf("\n");
            
            
            
            //addStudy -adam xx xx xx xx
            //remove the minus character
            
            char temp[20];
            for(i=0;argument[1][i]!=0;i++){
                temp[i] = argument[1][i+1];
            }
            temp[i] = 0;
            strcpy(argument[1],temp);
            

            //////////determind command
            
			
            if(strcmp(argument[0], "addStudy")==0 || strcmp(argument[0], "addAssignment")==0 || strcmp(argument[0], "addProject")==0) || strcmp(argument[0], "addGathering")==0{
			    char type[10]; //e.g cut "addStudy" to "Study"
                j=0;
                for(i=3;argument[0][i]!=0;i++){
                    type[j++] = argument[0][i];
                }
                type[j] = 0;
			
                ///////////convert string of start time to int
                char temp[5];
                temp[0] = argument[3][0];
                temp[1] = argument[3][1];
                if (argument[3][3] == '3'){//skip the colon 18:00 -> 1800
                    temp[2] = '5';    // use 1950 to represent 19:30, for better calculation
                }else{
                    temp[2] = '0';
                }
                temp[3] = argument[3][4];
                temp[4] = 0;

                int end_time = atoi(temp) + (atof(argument[4]) * 100);
                //printf("temp is %s\n start_time_int is :%d\n atof is %f\n",temp,atoi(temp),(atof(argument[4]) * 100));
                char tempString[5];
                sprintf(tempString, "%d", end_time);
                tempString[4]=0;
                char end_time_string[6];
                end_time_string[0] = tempString[0];
                end_time_string[1] = tempString[1];
                end_time_string[2] = ':';
                if (tempString[2] == '5'){
                    end_time_string[3] = '3';
                }else{
                    end_time_string[3] = '0';
                }
                end_time_string[4] = tempString[3];
                end_time_string[5] = 0;
                ///////////end convert string of start time to int
			
			
			
			
			
                //find corresponding pipe
                //int code = matchCode(argc,argv,argument[1]);
                //write(fd[code][1],buf,100);
				//addStudy adam 2015-01-01 18:00 2.0
				//0          1         2       3   4
				
				char people[30];
				if (argCount > 5)
					for(i=5;i<argCount;i++){
						
					}
				
				
				
				
				
				pendingTimeSlot[pendingCount].start_time_int = atoi(temp);
                           pendingTimeSlot[pendingCount].end_time_int = end_time;
                           strcpy( pendingTimeSlot[pendingCount].date, argument[1];
                           strcpy( pendingTimeSlot[pendingCount].start_time,argument[3]);
                           strcpy( pendingTimeSlot[pendingCount].end_time, end_time_string;
                           strcpy( pendingTimeSlot[pendingCount].type,type);
                           strcpy( pendingTimeSlot[pendingCount].people,"---");
				
				
				pendingCount++;
				
				
            }/*else if(strcmp(argument[0], "addGathering")==0){
                int code = matchCode(argc,argv,argument[1]);
                write(fd[code][1],buf,100);
                
                //tell the tagged people to also add it to schedule
                for(i=5;i<argCount;i++){
                    int code = matchCode(argc,argv,argument[i]);
                    sprintf(buf, "addGathering -%s %s %s %s %s",argument[i],argument[2],argument[3],argument[4],argument[1]);
                    write(fd[code][1],buf,100);
                }
            }*/else if(strcmp(argument[0], "addBatch")==0){

                FILE* infile = fopen(argument[1], "r");
                 if (infile == NULL) {
                    printf("Error in opening %s, plz make sure it exists\n", argument[1]);
                    continue;
                 }

                while (fscanf(infile, "%[^\n]\n", &buf) != EOF)
                {
                    int argIndex = 0;
                    char argument[14][20];
                    i=0;
                    int j=0;
                    int splitCount = 0;
                    while(buf[i] != 0){
                        argument[argIndex][j++] = buf[i++];//copy until end or space
                        if (buf[i] == ' ' && splitCount < 5){//no need split name of people(one string contains few names)
                            argument[argIndex][j] = 0; //replace space with null byte
                            i++;
                            j=0;
                            argIndex++;
                            splitCount++;
                        }
                    }//end while
                    argument[argIndex][j] = 0;//terminate the last string
                    int argCount = argIndex+1;

                    char temp[20];
                    for(i=0;argument[1][i]!=0;i++){
                        temp[i] = argument[1][i+1];
                    }
                    temp[i] = 0;
                    strcpy(argument[1],temp);


                        
                    int code = matchCode(argc,argv,argument[1]);
                    write(fd[code][1],buf,100);
                }
                 fclose(infile);
            //end addBatch
            }else if(strcmp(argument[0], "printSchd")==0){
                for(i=0;i<userNum;i++){
                    write(fd[i][1],buf,100);
                    sleep(1);
                /*    i = read(fd2[i][0],buf,100);
                    buf[i]=0;
                    printf("p buf: %s\n",buf);
                    
                    if (!(strcmp(buf, "ok")==0)){

                        int argIndex = 0;
                        char argument[14][20];
                        i=0;
                        int j=0;
                        while(buf[i] != 0){
                            argument[argIndex][j++] = buf[i++];//copy until end or space
                            if (buf[i] == ' '){
                                argument[argIndex][j] = 0; //replace space with null byte
                                i++;
                                j=0;
                                argIndex++;
                            }
                        }//end while
                        argument[argIndex][j] = 0;//terminate the last string
                        int argCount = argIndex+1;


                        for(i=4;i<argCount;i++){
                        int code = matchCode(argc,argv,argument[i]);
                        sprintf(buf, "cancelPG  %s %s",argument[1],argument[2]);
                        write(fd[code][1],buf,100);

                    } */
                    }
                    
                    
                
                    /*for(j=0;j<userNum;j++){
                    i = read(fd2[j][0],buf,100);
                    buf[i]=0;
                    printf("p buf: %s\n",buf);
                    }*/


            }else if(strcmp(argument[0], "endProgram")==0){
                //tell child to finish
                strcpy(buf, "endProgram");
                for(i=0;i<userNum;i++){
                    write(fd[i][1],buf,100);
                }
            
                //close the pipe first
                for(i=0;i<userNum;i++){
                    close(fd2[i][0]);
                    close(fd[i][1]);
            }
                
                //wait to get back child
                for(i=0;i<userNum;i++)
                    wait(NULL);
                printf("Parent exit\n");
                exit(0);
            }
            else{
                printf("######  Unknown command %s  ######\n", argument[0]);
            }
            
            sleep(1);//allow time for child to response
        }
    }
}

int matchCode(int argc, char* argv[], char* name){
    int i;
    for(i=1;i<argc;i++){
        if (strcmp(argv[i],name) == 0){
            return i-1;
        }
    }
    return -1;
}



