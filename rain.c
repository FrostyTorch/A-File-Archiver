// RAIN, A simple file archiver
// 3/04/2023 Syed Ahsan

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rain.h"
#include <stdint.h>
#include <string.h>

void create_dir_in_drop(char *pathname, FILE *output_stream);
void create_file_in_drop(char *pathname, char format, FILE *output_stream);
void get_perms_string(mode_t mode, char *buf);
// print the files & directories stored in drop_pathname (subset 0)
//
// if long_listing is non-zero then file/directory permissions, formats & sizes are also printed (subset 0)

void list_drop(char *drop_pathname, int long_listing) {
           
    FILE *fp = fopen(drop_pathname, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }
        
        if (long_listing == 0) {
	
	while (1) {

	// Read the magic number
        int magic = fgetc(fp);
        if (magic == EOF) {
            break;
        }
        if (magic != 0x63) {
            printf("Error: Invalid magic number.\n");
            break;
        }

        // Read the droplet format
        int format = fgetc(fp);
        if (format != '6' && format != '7' && format != '8') {
            printf("Error: Invalid droplet format.\n");
            break;
        }

        // Read the permissions
        char permissions[11];
	if (fread(permissions, sizeof(char), 10, fp) != 10) {
   	    printf("Error: Unexpected end of file.\n");
    	    break;
	}
	permissions[10] = '\0';
	            
        // Read the pathname length
        uint16_t pathname_length;
        if (fread(&pathname_length, sizeof(pathname_length), 1, fp) != 1) {
            printf("Error: Unexpected end of file.\n");
            break;
        }
	// Convert the pathname length string to an integer
	// Print the converted value
	
        // Read the pathname
        char *pathname = malloc(pathname_length + 1);
        if (fread(pathname, 1, pathname_length, fp) != pathname_length) {
            printf("Error: Unexpected end of file.\n");
            free(pathname);
            break;
        }
        pathname[pathname_length] = '\0';
        
        // Read the content length
        uint64_t content_length;
	uint8_t buffer[6];

	if (fread(buffer, sizeof(buffer), 1, fp) != 1) {
    	    printf("Error: Unexpected end of file.\n");
    	    free(pathname);
    	    break;
	}

	content_length = (uint64_t)buffer[0] |
                 ((uint64_t)buffer[1] << 8)  |
                 ((uint64_t)buffer[2] << 16) |
                 ((uint64_t)buffer[3] << 24) |
                 ((uint64_t)buffer[4] << 32) |
                 ((uint64_t)buffer[5] << 40);
              
        // Discard the content data
        for (uint64_t i = 0; i < content_length; i++) {
            if (fgetc(fp) == EOF) {
                printf("Error: Unexpected end of file.......\n");
                break;
            }
        }
       	 
	    printf("%2s\n", pathname);
        
        // Read and discard the hash
            fgetc(fp); 
	}  
	}
	
        // Read and parse each droplet in the file
        while(1) {
              
        // Read the magic number
        int magic = fgetc(fp);
        if (magic == EOF) {
            break;
        }
        if (magic != 0x63) {
            printf("Error: Invalid magic number.\n");
            break;
        }

        // Read the droplet format
        int format = fgetc(fp);
        if (format != '6' && format != '7' && format != '8') {
            printf("Error: Invalid droplet format.\n");
            break;
        }

        // Read the permissions
        char permissions[11];
	if (fread(permissions, sizeof(char), 10, fp) != 10) {
   	    printf("Error: Unexpected end of file.\n");
    	    break;
	}
	permissions[10] = '\0';
	            
        // Read the pathname length
        uint16_t pathname_length;
        if (fread(&pathname_length, sizeof(pathname_length), 1, fp) != 1) {
            printf("Error: Unexpected end of file.\n");
            break;
        }
	// Convert the pathname length string to an integer
	// Print the converted value
	
        // Read the pathname
        char *pathname = malloc(pathname_length + 1);
        if (fread(pathname, 1, pathname_length, fp) != pathname_length) {
            printf("Error: Unexpected end of file.\n");
            free(pathname);
            break;
        }
        pathname[pathname_length] = '\0';
        
        // Read the content length
        uint64_t content_length;
	uint8_t buffer[6];

	if (fread(buffer, sizeof(buffer), 1, fp) != 1) {
    	    printf("Error: Unexpected end of file.\n");
    	    free(pathname);
    	    break;
	}

	content_length = (uint64_t)buffer[0] |
                 ((uint64_t)buffer[1] << 8)  |
                 ((uint64_t)buffer[2] << 16) |
                 ((uint64_t)buffer[3] << 24) |
                 ((uint64_t)buffer[4] << 32) |
                 ((uint64_t)buffer[5] << 40);
              
        // Discard the content data, to move onto the next file
        for (uint64_t i = 0; i < content_length; i++) {
            if (fgetc(fp) == EOF) {
                printf("Error: Unexpected end of file.......\n");
                break;
            }
           
        }
        
	printf("%s  ", permissions);
        if (format >= '0' && format <= '9') {
    	    int digit = format - '0'; // convert from ASCII to integer value as the bytes in file are in ASCII.
    	    printf("%d", digit); 
	}
	    printf("%7lu  ", content_length);
	    printf("%2s\n", pathname);
        
        // Read and discard the hash
        
	    fgetc(fp);
    	}
        
    // Close the file
    
    fclose(fp);
}


// check the files & directories stored in drop_pathname (subset 1)
//
// prints the files & directories stored in drop_pathname with a message
// either, indicating the hash byte is correct, or
// indicating the hash byte is incorrect, what the incorrect value is and the correct value would be

void check_drop(char *drop_pathname) {
        
	FILE *fp = fopen(drop_pathname, "r");
            if (fp == NULL) {
            printf("Error opening file.\n");
            return;
        }
 	 	
 	// Read and parse each droplet in the file
        while(1) {
                   
        // Read the magic number
        int magic = fgetc(fp);
        if (magic == EOF) {
            break;
        }
        if (magic != 0x63) {
            fprintf(stderr, "error: incorrect first droplet byte: 0x%x should be 0x63\n", magic);
            break;
        }
		
        // Read the droplet format
        int format = fgetc(fp);
        if (format != '6' && format != '7' && format != '8') {
            
            printf("Error: Invalid droplet format. printed: %d\n", format);
            break;
        }

        // Read the permissions
        char permissions[11];
	if (fread(permissions, sizeof(char), 10, fp) != 10) {
   	    printf("Error: Unexpected end of file.\n");
    	    break;
	}
	permissions[10] = '\0';
	
        // Read the pathname length
        uint16_t pathname_length;
        if (fread(&pathname_length, sizeof(pathname_length), 1, fp) != 1) {
            printf("Error: Unexpected end of file.\n");
            break;
        }
	// Convert the pathname length string to an integer
	// Print the converted value
	
        // Read the pathname
        char *pathname = malloc(pathname_length + 1);
        if (fread(pathname, 1, pathname_length, fp) != pathname_length) {
            printf("Error: Unexpected end of file.\n");
            free(pathname);
            break;
        }
        pathname[pathname_length] = '\0';
        // Read the content length
        uint64_t content_length;
	uint8_t buffer[6];

	if (fread(buffer, sizeof(buffer), 1, fp) != 1) {
    	    printf("Error: Unexpected end of file.\n");
    	    free(pathname);
    	    break;
	}
                       
	content_length = (uint64_t)buffer[0] |
                 ((uint64_t)buffer[1] << 8)  |
                 ((uint64_t)buffer[2] << 16) |
                 ((uint64_t)buffer[3] << 24) |
                 ((uint64_t)buffer[4] << 32) |
                 ((uint64_t)buffer[5] << 40);
        FILE *output_file = fopen("output_file", "wb");
 	    if (output_file == NULL) {
            printf("Error opening file.\n");
            return;
        }      
        // Placing new content data into the output file
        for (uint64_t i = 0; i < content_length; i++) {
            int byte = fgetc(fp);
            if (byte == EOF) {
                printf("Error: Unexpected end of file.......\n");
                fclose(output_file);
                free(pathname);
                break;
            }
            fputc(byte, output_file);
        }          
        fclose(output_file);   
	uint8_t computed_hash = 0;
            	        
        // Going to the start of each file for this hash calculation
        fseek(fp, -(content_length+20+pathname_length), SEEK_CUR);
                
        for (uint8_t i = 0; i < (content_length+20+pathname_length); i++) {
            int byte = fgetc(fp);
          
            if (byte == EOF) {
                printf("Error: Unexpected end of file.\n");
                break;
	}		
	    computed_hash = droplet_hash(computed_hash, byte);	    
	}
	
        printf("%2s - ", pathname);    
       
        // Read and verify the hash byte
        uint8_t stored_hash = fgetc(fp);
        // Advancing byte, moving to the magic number of next file
       
        if (computed_hash != stored_hash) {
           printf("incorrect hash 0x%x should be 0x%x \n", computed_hash, stored_hash);
        } else {
           printf("correct hash\n");
          
        }
       	    
        }
    
     fclose(fp); 

}


// extract the files/directories stored in drop_pathname (subset 2 & 3)

void extract_drop(char *drop_pathname) {

        FILE *fp = fopen(drop_pathname, "r");
            if (fp == NULL) {
            printf("Error opening file.\n");
            return;
        }
 	 	
 	// Read and parse each droplet in the file
        while(1) {
                   
        // Read the magic number
        int magic = fgetc(fp);
        if (magic == EOF) {
            break;
        }
        if (magic != 0x63) {
            fprintf(stderr, "error: incorrect first droplet byte: 0x%x should be 0x63\n", magic);
            break;
        }
		
        // Read the droplet format
        int format = fgetc(fp);
        if (format != '6' && format != '7' && format != '8') {
            
            printf("Error: Invalid droplet format. printed: %d\n", format);
            break;
        }

        // Read the permissions
        char permissions[11];
	if (fread(permissions, sizeof(char), 10, fp) != 10) {
   	    printf("Error: Unexpected end of file.\n");
    	    break;
	}
	permissions[10] = '\0';
	// Changing it so chmod can read it.
	int mode = 0;
        for (int i = 0; i < 9; i++) {
            mode <<= 1;
            if (permissions[i+1] != '-') {
            mode |= 1;
            }
        }
	
        // Read the pathname length
        uint16_t pathname_length;
        if (fread(&pathname_length, sizeof(pathname_length), 1, fp) != 1) {
            printf("Error: Unexpected end of file.\n");
            break;
        }
	// Convert the pathname length string to an integer
	// Print the converted value
	
        // Read the pathname
        char *pathname = malloc(pathname_length + 1);
        if (fread(pathname, 1, pathname_length, fp) != pathname_length) {
            printf("Error: Unexpected end of file.\n");
            free(pathname);
            break;
        }
        pathname[pathname_length] = '\0';
        // Read the content length
        uint64_t content_length;
	uint8_t buffer[6];

	if (fread(buffer, sizeof(buffer), 1, fp) != 1) {
    	    printf("Error: Unexpected end of file.\n");
    	    free(pathname);
    	    break;
	}
                       
	content_length = (uint64_t)buffer[0] |
                 ((uint64_t)buffer[1] << 8)  |
                 ((uint64_t)buffer[2] << 16) |
                 ((uint64_t)buffer[3] << 24) |
                 ((uint64_t)buffer[4] << 32) |
                 ((uint64_t)buffer[5] << 40);
        FILE *output_file = fopen(pathname, "wb");
 	    if (output_file == NULL) {
            printf("Error opening file.\n");
            return;
        }      
        // Content loop
        for (uint64_t i = 0; i < content_length; i++) {
            int byte = fgetc(fp);
            if (byte == EOF) {
                printf("Error: Unexpected end of file.......\n");
                fclose(output_file);
                free(pathname);
                break;
            }
            fputc(byte, output_file);
        } 
        if (chmod(pathname, mode) == -1) {
            fprintf(stderr, "Error: Unable to change permission of file %s\n", drop_pathname);
        }
        fclose(output_file);
        printf("Extracting: %s\n", pathname);  
        fgetc(fp); 
        }
    
     fclose(fp); 

}
// create drop_pathname containing the files or directories specified in pathnames (subset 3)
//
// if append is zero drop_pathname should be over-written if it exists
// if append is non-zero droplets should be instead appended to drop_pathname if it exists
//
// format specifies the droplet format to use, it must be one DROPLET_FMT_6,DROPLET_FMT_7 or DROPLET_FMT_8

void create_drop(char *drop_pathname, int append, int format,
                int n_pathnames, char *pathnames[n_pathnames]) {

    char *mode = append ? "a" : "w";
    FILE *drop_file = fopen(drop_pathname, mode);
    if (drop_file == NULL) {
        perror("Failed to open drop file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_pathnames; i++) {
        struct stat path_stat;
        if (stat(pathnames[i], &path_stat) != 0) {
            perror("Failed to get file status");
            fclose(drop_file);
            exit(EXIT_FAILURE);
        }

        if (S_ISREG(path_stat.st_mode)) {
            create_file_in_drop(pathnames[i], format, drop_file);
        } else if (S_ISDIR(path_stat.st_mode)) {
            create_dir_in_drop(pathnames[i], drop_file);
        } else {
            fprintf(stderr, "%s is not a regular file or directory\n", pathnames[i]);
        }
    }

    fclose(drop_file);
}


// ADD YOUR EXTRA FUNCTIONS HERE

void create_dir_in_drop(char *pathname, FILE *output_stream)  {
         
    uint8_t hash = 0;
    printf("Adding: %s\n", pathname);

    // Write magic number
    fputc(0x63, output_stream);
    hash = droplet_hash(hash, 0x63);
    
    fputc(0x36, output_stream);
    hash = droplet_hash(hash, 0x63);
    // Write permissions
    struct stat s;
    if (stat(pathname, &s) != 0) {
        perror(pathname);
        exit(1);
    }
    char perms[11];
    get_perms_string(s.st_mode, perms);
    fwrite(perms, 1, 10, output_stream);
    for (int i = 0; i < 10; i++) {
        hash = droplet_hash(hash, perms[i]);
    }
    // Write pathname length
    int length = strlen(pathname);
        uint16_t pathlen = (uint16_t) length;
        fputc(pathlen & 0xFF, output_stream);
        fputc((pathlen >> 8) & 0xFF, output_stream);
        hash = droplet_hash(hash, pathlen & 0xFF);
        hash = droplet_hash(hash, (pathlen >> 8) & 0xFF);

    for (int i = 0; i < 6; i++) {
        fputc(0, output_stream);
        hash = droplet_hash(hash, 0);
    }
    // Write pathname
    fwrite(pathname, 1, length, output_stream);
    for (int i = 0; i < length; i++) {
        hash = droplet_hash(hash, pathname[i]);
    }
    // Write hash
    fputc(hash, output_stream);
}

void create_file_in_drop(char *pathname, char format, FILE *output_stream) {
    
    int h = 0;

    printf("Adding: %s\n", pathname);
    fputc(0x63, output_stream);

    h = droplet_hash(h, 0x63);
    struct stat s;

    if (stat(pathname, &s) != 0) {
        perror(pathname);
        exit(1);
    }
    // Writing file format byte.
    uint8_t format_byte;
    switch (format) {
        case '6':
            format_byte = 0x36;
            break;
        case '7':
            format_byte = 0x37;
            break;
        case '8':
            format_byte = 0x38;
            break;
        default:
            fprintf(stderr, "Unknown format: %c\n", format);
            exit(1);
    }

    fputc(format_byte, output_stream);
    h = droplet_hash(h, format_byte);
    // Writing permissions.
    char perms_str[10];
    get_perms_string(s.st_mode, perms_str);

    for (int i = 0; i < 10; i++) {
        fputc(perms_str[i], output_stream);
        h = droplet_hash(h, perms_str[i]);
    }
    // Writing pathname length.
    uint16_t length = strlen(pathname);
    for (int i = 0; i < 2; i++) {
        uint8_t namelen = (length >> ((1 - i) * 8)) & 0xff;
        fputc(namelen, output_stream);
        h = droplet_hash(h, namelen);
    }
    // Writing content length.
    for (int i = 0; i < 6; i++) {
        uint8_t size = (s.st_size >> ((5 - i) * 8)) & 0xff;
        fputc(size, output_stream);
        h = droplet_hash(h, size);
    }
    // Writing pathname.
    fwrite(pathname, 1, length, output_stream);
    for (int i = 0; i < length; i++) {
        h = droplet_hash(h, pathname[i]);
    }
    // Writing out contents.
    if (s.st_size > 0) {
        FILE *input_stream = fopen(pathname, "rb");
        char read[s.st_size];
        fread(read, 1, s.st_size, input_stream);
        fclose(input_stream);

        fwrite(read, 1, s.st_size, output_stream);
        for (int i = 0; i < s.st_size; i++) {
            h = droplet_hash(h, read[i]);
        }
    }
    // Writing out final hash.
    fputc(h, output_stream);
}

void get_perms_string(mode_t mode, char *buf) {
    buf[0] = (S_ISDIR(mode) ? 'd' : '-');
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    // buf[10] = '\0'; 
}
