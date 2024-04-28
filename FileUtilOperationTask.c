// Include necessary headers
#define _XOPEN_SOURCE 500 // Defined a macro _XOPEN_SOURCE 500 for nftw() function.
#include <stdio.h> // Standard input-output functions
#include <stdlib.h> // Standard library functions
#include <string.h> // String manipulation functions
#include <sys/stat.h> // Functions for file status
#include <ftw.h> // File tree walk functions
#include <errno.h> // Include errno.h for errno and EISDIR
#include <limits.h> // Definitions of system limits
#include <zlib.h> // Functions for gzip compression

// Declaration for snprintf
int snprintf(char *s, size_t n, const char *format, ...); 
// s is a Pointer to the destination buffer 'dest_path' where the formatted string will be stored
// n is the maximum number of characters that can be written to the buffer pointed to by s, including the terminating null character.
// format is a pointer to a null-terminated string that specifies the format of the output
// ... indicates that snprintf can accept a variable number of arguments

// Declaration for strdup
char *strdup(const char *s);
// strdup takes a string (s) as input and creates a duplicate of that string, returning a pointer to the newly allocated memory containing the duplicated string. The caller is responsible for freeing the memory allocated by strdup when it's no longer needed.

// Global variables
const char *enteredFileName; // Stores the name of the file entered by the user
const char *rootDir; // Stores the root directory path
const char *storageDir; // Stores the storage directory path
const char *operation; // Stores the operation to be performed (copy or move)
const char *extension; // Stores the file extension to filter files
char *found_file = NULL; // Stores the path of the found file

// Function prototypes
// Function to check if a directory exists
int directory_exists(const char *path); 
// const char *path: This argument represents the path of the directory that needs to be checked for existence. It's a pointer to a null-terminated string (const char *) containing the path of the directory.

int search_and_process(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf); 
// Called during a file tree walk (nftw) operation.
// Searches for a specific file and performs operations like copy or move based on user input.
// const char *fpath: This argument represents the path of the current file or directory being visited during the file tree traversal. It's a pointer to a null-terminated string (const char *).
// const struct stat *sb: This argument represents a pointer to a structure containing information about the file specified by fpath. It includes details such as file type, size, permissions, etc. (const struct stat *).
// int typeflag: This argument indicates the type of the file specified by fpath. It can have different values to represent regular files, directories, symbolic links, etc.
// struct FTW *ftwbuf: This argument is a pointer to a structure containing information about the current file tree walk status. It includes details such as the depth of the traversal, base name, etc.
int search_and_create_tar(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf); // Callback function for searching and creating tar
// Also used with nftw function.
// Searches for files with a specific extension and creates a tar file containing those files.

void add_file_to_tar(const char *dir_path, const char *filename, gzFile tarfile); 
// Function to add a file to a tar archive
// const char *dir_path: This argument represents the path of the directory containing the file to be added to the tar archive. It's a pointer to a null-terminated string (const char *).
// const char *filename: This argument represents the name of the file to be added to the tar archive. It's a pointer to a null-terminated string (const char *).
// gzFile tarfile: This argument represents a handle to the gzip file (tar archive) where the file will be added. It's of type gzFile, which is a pointer to a gzFile_s structure representing a gzip file.
void copy_or_move_file(const char *src_path, const char *dest_path); 
// Function to copy or move a file
// const char *src_path: This argument represents the path of the source file to be copied or moved. It's a pointer to a null-terminated string (const char *).
// const char *dest_path: This argument represents the path of the destination where the source file will be copied or moved. It's a pointer to a null-terminated string (const char *).

//-----------------------------------------------------------------------------

// Function to check if a directory exists
int directory_exists(const char *path) { // Return Type of Int.
    struct stat st; // This line declares a structure st of type struct stat. The stat structure is used to hold information about a file, including its size, permissions, and type.
    // Check if the given path exists and is a directory
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        //stat(path, &st): It checks whether the *stat system call* succeeds in obtaining information about the file specified by the path and stores it in the st structure. The stat system call returns 0 on success and -1 on failure.
        // S_ISDIR(st.st_mode : This is a macro provided by the <sys/stat.h> header file. It checks whether the file type stored in st.st_mode indicates that the file is a directory. If st.st_mode represents a directory, S_ISDIR evaluates to true (non-zero), indicating that the given path points to a directory.
        return 1; // Return 1 if directory exists
    }
    return 0; // Return 0 if directory does not exist
}

// Function to search for a file so that we can copy/move it.
int search_and_process(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    // Called during a file tree walk (nftw) operation.
    // const char *fpath: This argument represents the path of the current file or directory being visited during the file tree traversal. It's a pointer to a null-terminated string (const char *).
    // const struct stat *sb: This argument represents a pointer to a structure containing information about the file specified by fpath. It includes details such as file type, size, permissions, etc. (const struct stat *).
    // int typeflag: This argument indicates the type of the file specified by fpath. It can have different values to represent regular files, directories, symbolic links, etc. It's an integer (int).
    // struct FTW *ftwbuf: This argument is a pointer to a structure containing information about the current file tree walk status. It includes details such as the depth of the traversal, base name, etc. (struct FTW *).

    // Check if the entry is a regular file and its name matches the entered file name
    if (typeflag == FTW_F && strcmp(fpath + ftwbuf->base, enteredFileName) == 0) {
        // typeflag == FTW_F: Checks if the entry represents a regular file (FTW_F is a macro indicating a regular file).
        // fpath is a pointer to a null-terminated string representing the full path of the current file being processed
        // ftwbuf->base is an offset representing the start position of the filename within the full path. In the expression ftwbuf->base, base is  a member of the struct FTW structure referenced by the pointer ftwbuf.
        // fpath + ftwbuf->base advances the pointer fpath by ftwbuf->base positions, effectively pointing it to the start of the filename portion within the full path. This expression ensures that we are comparing only the filename part of the path.
        found_file = strdup(fpath); //If the conditions are met, it duplicates the path of the fpath using the strdup function and stores it in the found_file variable.
        
        if (operation) {
            // Checks if an operation is specified (operation is not NULL or 0). If an operation is specified, it means the user wants to perform a copy or move operation on the found file.
            char dest_path[PATH_MAX]; // Buffer to store the destination path 
            snprintf(dest_path, sizeof(dest_path), "%s/%s", storageDir, enteredFileName); 
            // snprintf formats the destination path by combining the storageDir and enteredFileName into a single string, separated by a forward slash (/), and writes the formatted path to the dest_path buffer
            // dest_path: destination buffer where the formatted string will be written. It's a character array representing the path to the destination.
            // Check if the storage directory exists using "directory_exists" function.
            // sizeof(dest_path): This specifies the size of the buffer dest_path. It ensures that snprintf does not write more characters than the size of the buffer, preventing buffer overflow.
            // storageDir: This is the first additional argument corresponding to the first %s format specifier in the format string. It's a pointer to a null-terminated string (const char *) containing the path of the storage directory.
            //enteredFileName: This is the second additional argument corresponding to the second %s format specifier in the format string. It's a pointer to a null-terminated string (const char *) containing the name of the file entered by the user.
            if (!directory_exists(storageDir)) { // Checks if the storage directory exists using the directory_exists function. 
                printf("Search Successful: Invalid storageDir\n");
                return 1; // returns 1 to indicate failure
            }

            copy_or_move_file(found_file, dest_path); // Copy or move the file
        } else {
            printf("%s\n", found_file); // Print the absolute path of the found file
        }
        return 0; // Return 0 to continue searching
    }
    return 0; // Return 0 to continue searching
}

// Callback function to search for files with a specific extension and create tar
int search_and_create_tar(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    // const char *fpath: Represents the path of the current file or directory being visited by the nftw function.It is a pointer to a string (const char *) containing the full path of the file or directory.
    // const struct stat *sb: This argument is a pointer to a struct stat object. It holds information about the file attributes of the current file being visited, such as its size, permissions, timestamps, etc.
    // int typeflag:This argument indicates the type of file being visited. 
    // Check if the entry is a regular file and its name matches the specified extension
    // struct FTW *ftwbuf: It provides information about the state of the walk through the directory tree. The structure contains the following fields:base: The offset within the path at which the filename begins.level: The depth of the current file or directory in the directory tree.
    if (typeflag == FTW_F && strstr(fpath + ftwbuf->base, extension) != NULL) {
        // This part of the code searches for the substring specified by extension within the fpath starting from the position specified by ftwbuf->base.
        // fpath is a pointer to a string containing the full path of the current file.
        // ftwbuf->base is the offset within the path at which the *filename extension* begins. It indicates the starting point of the filename within the full path.
        // extension is a pointer to a string containing the file extension to search for.
        // != NULL: condition checks if the strstr() function successfully found the specified extension in the file path.
        
        printf("%s\n", fpath); // Print the path of the found file

        // Create tar file
        // This block of code prepares to create and open a gzip-compressed tar archive file (tarfile). It constructs the filename for the tar archive, attempts to open the file, and handles errors if the file opening operation fails.
        char tarfilename[PATH_MAX]; // Buffer to store the tar file name
        snprintf(tarfilename, sizeof(tarfilename), "%s/a1.tar", storageDir); // Create the tar file name
        // snprintf function combines the storageDir path with /a1.tar to create the filename for the tar archive, 
        //and stores the resulting string in the tarfilename buffer. 
        // tarfilename: It is a buffer.
        // sizeof(tarfilename): returns the size of the tarfilename buffer in bytes.
        // "%s/a1.tar": It specifies the format in which the output string will be constructed.
        // storageDir:  is a pointer to a string containing the directory where the tar archive will be stored.
        gzFile tarfile = gzopen(tarfilename, "wb"); // Open the tar file for writing in gzip format
        // gzopen is a function provided by the zlib library for opening gzip files.
        // tarfilename: This is a string representing the filename of the gzip file to be opened. It is the filename that was constructed earlier using snprintf.
        // "wb": This is a string specifying the mode in which the file will be opened.
        // gzFile : It is a type representing a gzip file pointer.
        // tarfile : This variable will hold the pointer to the opened gzip file.

        if (!tarfile) { // If the gzopen function fails to open the file, it returns NULL.
            perror("gzopen"); // Print an error message if opening the tar file fails
            return 1; // Return 1 to indicate failure
        }

        add_file_to_tar(fpath, fpath + ftwbuf->base, tarfile); // Add file to the tar file

        // Close the tar file
        gzclose(tarfile);
    }
    return 0; // Return 0 to continue searching
}

// Function to add a file to the tar file
void add_file_to_tar(const char *dir_path, const char *filename, gzFile tarfile) {
// this block of code adds a file to a tar archive. It constructs the full path of the file, opens the file for reading,
// reads its contents into a buffer, and then writes the contents of the buffer to the tar archive

    // const char *dir_path: This argument represents the path of the directory where the file to be added to the tar archive is located.
    // const char *filename:This argument represents the name of the file to be added to the tar archive.
    // gzFile tarfile: This argument represents the gzip file pointer to the tar archive.
    char filepath[PATH_MAX]; // This buffer will store the full path of the file to be added to the tar archive.
    snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, filename); 
    // Create the file path to be added to the tar archive
    // the snprintf function combines the dir_path (directory path) and filename to create the full path of the 
    //file to be added to the tar archive, and stores the resulting string in the filepath buffer.
    // It combines the directory path (dir_path) and the filename (filename) with a forward slash ("/").
    //filepath: The resulting above formatted string is stored in the filepath buffer. It is an array of characters (char[]) intended to hold the full path of the file.
    FILE *file = fopen(filepath, "rb"); // Open the file for reading in binary mode
    // This line opens the file specified by the filepath for reading in binary mode ("rb").
    if (!file) { // If opening the file fails, it returns NULL, indicating an error.
        // Skip directories
        if (errno == EISDIR) {
            // the code checks if the error is due to the entry being a directory (indicated by errno == EISDIR), and if so, it skips the directory entry and returns without processing.
            return; // Return if the entry is a directory
        }
        return; // Return if opening the file fails
    }

    char buffer[1024]; // Buffer to store data read from the file
    size_t bytes_read; // Variable to store the number of bytes read
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) { // Read data from the file
        if (gzwrite(tarfile, buffer, bytes_read) != bytes_read) { // Write data to the tar file
        // The gzwrite function writes the data from the buffer to the tar archive (tarfile).
            fprintf(stderr, "Error writing to tar file\n"); // Print an error message if writing to the tar file fails
            fclose(file); // Close the file
            return; // Return to indicate failure
        }
    }

    fclose(file); // Close the file
}

// Function to copy or move a file
void copy_or_move_file(const char *src_path, const char *dest_path) {
    if (strcmp(operation, "-cp") == 0) { // Check if the operation is copy
        FILE *src_file = fopen(src_path, "rb"); 
        //This assigns the pointer returned by fopen() to the variable src_file.
        //This declares a pointer variable named src_file of type FILE *
        // Open the source file for reading in binary mode
        if (!src_file) { // Check if opening the source file fails
            perror("fopen source file"); // Print an error message
            return; // Return to indicate failure
        }

        FILE *dest_file = fopen(dest_path, "wb"); // Open the destination file for writing in binary mode
        if (!dest_file) { // Check if opening the destination file fails
            perror("fopen destination file"); // Print an error message
            fclose(src_file); // Close the source file
            return; // Return to indicate failure
        }

        char buffer[1024]; // Buffer to store data read from the source file
        size_t bytes_read; // Variable to store the number of bytes read
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        // This block of code continuously reads data from the source file (src_file) in chunks and writes it to the destination file (dest_file) until the end of the source file is reached.
        // bytes_read variable will hold the number of bytes read by the fread() function
        // buffer: This is a pointer to the memory location where the data read from the file will be stored. It should be large enough to hold the data being read. In this case, buffer is typically an array of bytes.
        // 1: This argument specifies the size of each element to be read from the file. Since we're reading bytes, it's set to 1.

            if (fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) { // Write data to the destination file
            // fwrite writes data from the buffer to the file represented by dest_file.
            // buffer: Pointer to the memory location containing the data to be written to the file
            // 1: Specifies the size of each element to be written. In this case, it's set to 1 because fwrite() writes one byte at a time.
            // bytes_read: Specifies the number of elements to be written. It represents the number of bytes read from the source file in the current iteration of the loop.
            // dest_file: Pointer to the FILE structure representing the destination file.
            // If fwrite() successfully writes all the elements, it returns the same value as bytes_read. However, if it fails to write all the elements (due to disk full or other reasons), it will return a value less than bytes_read.
                perror("fwrite"); // Print an error message
                fclose(src_file); // Close the source file
                fclose(dest_file); // Close the destination file
                return; // Return to indicate failure
            }
        }

        fclose(src_file); // Close the source file
        fclose(dest_file); // Close the destination file

        printf("Search Successful\n"); // Print a success message
        printf("File copied to the storageDir\n"); // Print a message indicating the destination directory
    } else if (strcmp(operation, "-mv") == 0) { // Check if the operation is move
        if (rename(src_path, dest_path) != 0) { // Rename function changes the file path from the source file to dest path.
            perror("rename"); // Print an error message if renaming fails
            return; // Return to indicate failure
        }
        printf("Search Successful\n"); // Print a success message
        printf("File moved to the storageDir\n"); // Print a message indicating the destination directory
    }
}

// ---------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    // Main function accepts command-line arguments (argc and argv)
    // argc is the number of arguments passed to the program, and argv is an array of strings containing the arguments.

    // Check the number of arguments passed
    // Depending on the number of arguments, different parts of the program logic will be executed.

    if (argc == 3) {
        // Part 1: Search for a file in the specified directory subtree
        // If the number of arguments is 3, the program searches for a file in the specified directory subtree.

        // It retrieves the root directory path (rootDir) and the entered file name (enteredFileName) from the command-line arguments.
        rootDir = argv[1]; // Store the root directory path
        enteredFileName = argv[2]; // Store the entered file name

        // Check if the root directory exists
        if (!directory_exists(rootDir)) {
            fprintf(stderr, "Invalid rootDir\n"); // Print an error message
            return 1; // Return to indicate failure
        }

        // Search for the file using nftw() function
        // It searches for the file using the nftw function with the search_and_process callback.
        if (nftw(rootDir, search_and_process, 20, FTW_PHYS) == -1) {
            perror("nftw"); // Print an error message
            return 1; // Return to indicate failure
        }

        // Print a message if the file is not found
        if (found_file == NULL) {
            printf("Search Unsuccessful\n"); // Print a message indicating that the search was unsuccessful
        }
    } else if (argc == 5) {
        // If the number of arguments is 5, the program searches for a file and performs a copy or move operation based on options.

        // Part 2: Search for a file and copy/move it to another directory based on options
        rootDir = argv[1]; // Store the root directory path
        storageDir = argv[2]; // Store the storage directory path
        operation = argv[3]; // Store the operation
        enteredFileName = argv[4]; // Store the entered file name

        // Check if the root directory exists
        if (!directory_exists(rootDir)) {
            fprintf(stderr, "Invalid rootDir\n"); // Print an error message
            return 1; // Return to indicate failure
        }

        // Check if the operation is valid
        if (strcmp(operation, "-cp") != 0 && strcmp(operation, "-mv") != 0) {
            fprintf(stderr, "Invalid operation\n"); // Print an error message
            return 1; // Return to indicate failure
        }

        // Search for the file using nftw() function
        if (nftw(rootDir, search_and_process, 20, FTW_PHYS) == -1) {
            perror("nftw"); // Print an error message
            return 1; // Return to indicate failure
        }

        // Print a message if the file is not found
        if (found_file == NULL) {
            printf("Search Unsuccessful\n"); // Print a message indicating that the search was unsuccessful
        }
    } else if (argc == 4) {
        // If the number of arguments is 4, the program searches for files with a specific extension and creates a tar file.
        // It retrieves the root directory path, storage directory path, and file extension from the command-line arguments.
        // Part 3: Search for files with a specific extension and create a tar file
        rootDir = argv[1]; // Store the root directory path
        storageDir = argv[2]; // Store the storage directory path
        extension = argv[3]; // Store the file extension

        // Check if the root directory and storage directory exist
        if (!directory_exists(rootDir) || !directory_exists(storageDir)) {
            if (!directory_exists(rootDir)) {
                fprintf(stderr, "Invalid rootDir\n"); // Print an error message
            }
            if (!directory_exists(storageDir)) {
                printf("Search Successful: Invalid storageDir\n"); // Print a message indicating that the storage directory is invalid
            }
            return 1; // Return to indicate failure
        }

        // Search for files with the specified extension using nftw() function
        if (nftw(rootDir, search_and_create_tar, 20, FTW_PHYS) == -1) {
            perror("nftw"); // Print an error message
            return 1; // Return to indicate failure
        }
    } else {
        fprintf(stderr, "Invalid number of arguments\n"); // Print an error message
        return 1; // Return to indicate failure
    }

    return 0; // Return to indicate success
}
