// #ifndef CSV_PARSER_H_
// #define CSV_PARSER_H_

// #include "stdio.h"
// #include "string.h"
// #include "unistd.h"

// void ReadCSV(char* path)
// {
//     printf("%s\n",path);
//     char str_tmp[1024];
//     FILE *pFile = NULL;
//     int cnt;
//     char *p;
//     char *line;

//     pFile = fopen(path, "r" );
//     if( pFile == NULL ) {
//         printf("open error\n");
//         char path[1024];
//         getcwd(path, sizeof(path));
//         printf("%s\n", path);
//         return;
//     }

//     while( !feof( pFile ) ){
//         fgets( str_tmp, 1024, pFile );

//         cnt=0;
//         line = strtok(str_tmp, "\n");
        
//         while(line != NULL) {
//             p = strtok(line, ",");
//             while (p != NULL) {
//                 cnt++;
//                 printf( "[%s] ", p );
//                 p = strtok(NULL, ",");
//             }
//             line = strtok(NULL, "\n");
//         }
//         printf(" : %d\n", cnt);
//     }       
//     fclose( pFile );

// }


// #endif // CSV_PARSER_H_

