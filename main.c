#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_STUDENTS 100
#define SUBJECTS 5
#define FILE_NAME "students.dat"

const char *subjectNames[SUBJECTS] = {
    "English",
    "Mathematics",
    "Computer",
    "Hindi",
    "Environmental Studies"
};

typedef struct {
    int rollNo;
    char name[60];
    int marks[SUBJECTS];
    int total;
    float percentage;
    char grade[4];
    int passed;
} Student;

Student students[MAX_STUDENTS];
int studentCount = 0;

/* Read a complete line and discard excess characters. */
void readLine(char text[], int size) {
    if (fgets(text, size, stdin) == NULL) {
        printf("\nInput ended. Exiting program.\n");
        exit(0);
    }

    if (strchr(text, '\n') == NULL) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Discard excess input. */
        }
    } else {
        text[strcspn(text, "\n")] = '\0';
    }
}

/* Read an integer within an allowed range. */
int readInt(const char *prompt, int min, int max) {
    char input[100];
    char *end;
    long value;

    while (1) {
        printf("%s", prompt);
        readLine(input, sizeof(input));

        errno = 0;
        value = strtol(input, &end, 10);

        if (input[0] != '\0' &&
            end != input &&
            *end == '\0' &&
            errno != ERANGE &&
            value >= min && value <= max) {
            return (int)value;
        }

        printf("Invalid input! Enter a number from %d to %d.\n",
               min, max);
    }
}

/* Find a student's position using the roll number. */
int findStudent(int rollNo) {
    int i;

    for (i = 0; i < studentCount; i++) {
        if (students[i].rollNo == rollNo) {
            return i;
        }
    }

    return -1;
}

/* Calculate total, percentage, pass/fail and grade. */
void calculateResult(Student *s) {
    int i;
    s->total = 0;
    s->passed = 1;

    for (i = 0; i < SUBJECTS; i++) {
        s->total += s->marks[i];

        if (s->marks[i] < 33) {
            s->passed = 0;
        }
    }

    s->percentage = (s->total * 100.0f) /
                    (SUBJECTS * 100.0f);

    if (!s->passed) {
        strcpy(s->grade, "F");
    } else if (s->percentage >= 90) {
        strcpy(s->grade, "A+");
    } else if (s->percentage >= 80) {
        strcpy(s->grade, "A");
    } else if (s->percentage >= 70) {
        strcpy(s->grade, "B");
    } else if (s->percentage >= 60) {
        strcpy(s->grade, "C");
    } else if (s->percentage >= 50) {
        strcpy(s->grade, "D");
    } else {
        strcpy(s->grade, "E");
    }
}

/* Save all student records to a binary file. */
int saveRecords(void) {
    FILE *fp = fopen(FILE_NAME, "wb");

    if (fp == NULL) {
        perror("Unable to open file for saving");
        return 0;
    }

    if (fwrite(&studentCount, sizeof(studentCount), 1, fp) != 1) {
        printf("Error saving record count.\n");
        fclose(fp);
        return 0;
    }

    if (studentCount > 0 &&
        fwrite(students, sizeof(Student), studentCount, fp)
            != (size_t)studentCount) {
        printf("Error saving student records.\n");
        fclose(fp);
        return 0;
    }

    if (fclose(fp) != 0) {
        printf("Error closing the data file.\n");
        return 0;
    }

    return 1;
}

/* Load saved records when the program starts. */
void loadRecords(void) {
    FILE *fp = fopen(FILE_NAME, "rb");
    int savedCount;
    size_t recordsRead;

    if (fp == NULL) {
        return; /* First run: no saved file yet. */
    }

    if (fread(&savedCount, sizeof(savedCount), 1, fp) != 1 ||
        savedCount < 0 || savedCount > MAX_STUDENTS) {
        printf("Warning: Invalid data file. Starting empty.\n");
        fclose(fp);
        return;
    }

    recordsRead = fread(students, sizeof(Student),
                        savedCount, fp);

    if (recordsRead != (size_t)savedCount) {
        printf("Warning: Incomplete data file. Starting empty.\n");
        studentCount = 0;
        fclose(fp);
        return;
    }

    studentCount = savedCount;
    fclose(fp);
}

/* Add a new student's marks. */
void addStudent(void) {
    Student s;
    int i;

    if (studentCount >= MAX_STUDENTS) {
        printf("\nStudent limit reached!\n");
        return;
    }

    printf("\n--- Add Student ---\n");

    s.rollNo = readInt("Enter roll number (1-999999): ",
                       1, 999999);

    if (findStudent(s.rollNo) != -1) {
        printf("This roll number already exists!\n");
        return;
    }

    do {
        printf("Enter student name: ");
        readLine(s.name, sizeof(s.name));

        if (s.name[0] == '\0') {
            printf("Name cannot be empty.\n");
        }
    } while (s.name[0] == '\0');

    for (i = 0; i < SUBJECTS; i++) {
        char prompt[100];

        snprintf(prompt, sizeof(prompt),
                 "Enter marks in %s (0-100): ",
                 subjectNames[i]);

        s.marks[i] = readInt(prompt, 0, 100);
    }

    calculateResult(&s);

    students[studentCount] = s;
    studentCount++;

    if (saveRecords()) {
        printf("\nStudent record saved successfully!\n");
    } else {
        /* Roll back the new in-memory record if saving fails. */
        studentCount--;
        printf("Record was not saved. Please try again.\n");
        return;
    }

    printf("Total: %d/%d\n", s.total, SUBJECTS * 100);
    printf("Percentage: %.2f%%\n", s.percentage);
    printf("Grade: %s\n", s.grade);
    printf("Status: %s\n", s.passed ? "PASS" : "FAIL");
}

/* Display a complete marksheet. */
void displayStudent(const Student *s) {
    int i;

    printf("\n========================================\n");
    printf("          STUDENT MARKSHEET\n");
    printf("========================================\n");
    printf("Roll Number : %d\n", s->rollNo);
    printf("Name        : %s\n", s->name);
    printf("----------------------------------------\n");

    for (i = 0; i < SUBJECTS; i++) {
        printf("%-25s %3d/100\n",
               subjectNames[i], s->marks[i]);
    }

    printf("----------------------------------------\n");
    printf("Total       : %d/%d\n",
           s->total, SUBJECTS * 100);
    printf("Percentage  : %.2f%%\n", s->percentage);
    printf("Grade       : %s\n", s->grade);
    printf("Result      : %s\n",
           s->passed ? "PASS" : "FAIL");
    printf("========================================\n");
}

/* Display all stored results. */
void displayAll(void) {
    int i;

    if (studentCount == 0) {
        printf("\nNo student records found.\n");
        return;
    }

    printf("\n%-8s %-20s %8s %10s %-8s %-8s\n",
           "Roll No", "Name", "Total",
           "Percent", "Grade", "Result");

    printf("--------------------------------------------------------------\n");

    for (i = 0; i < studentCount; i++) {
        printf("%-8d %-20.20s %3d/%-4d %8.2f%% %-8s %-8s\n",
               students[i].rollNo,
               students[i].name,
               students[i].total,
               SUBJECTS * 100,
               students[i].percentage,
               students[i].grade,
               students[i].passed ? "PASS" : "FAIL");
    }
}

/* Search for a student and show the marksheet. */
void searchStudent(void) {
    int rollNo;
    int index;

    if (studentCount == 0) {
        printf("\nNo student records available.\n");
        return;
    }

    rollNo = readInt("Enter roll number to search: ",
                     1, 999999);

    index = findStudent(rollNo);

    if (index == -1) {
        printf("Student with roll number %d not found.\n",
               rollNo);
    } else {
        displayStudent(&students[index]);
    }
}

/* Show the class result summary. */
void classSummary(void) {
    int i, passed = 0, failed = 0;
    int highestIndex = 0, lowestIndex = 0;
    float totalPercentage = 0.0f;

    if (studentCount == 0) {
        printf("\nNo records available for summary.\n");
        return;
    }

    for (i = 0; i < studentCount; i++) {
        if (students[i].passed) {
            passed++;
        } else {
            failed++;
        }

        totalPercentage += students[i].percentage;

        if (students[i].percentage >
            students[highestIndex].percentage) {
            highestIndex = i;
        }

        if (students[i].percentage <
            students[lowestIndex].percentage) {
            lowestIndex = i;
        }
    }

    printf("\n======= CLASS RESULT SUMMARY =======\n");
    printf("Total students : %d\n", studentCount);
    printf("Passed         : %d\n", passed);
    printf("Failed         : %d\n", failed);
    printf("Class average  : %.2f%%\n",
           totalPercentage / studentCount);

    printf("Highest        : %s (%.2f%%)\n",
           students[highestIndex].name,
           students[highestIndex].percentage);

    printf("Lowest         : %s (%.2f%%)\n",
           students[lowestIndex].name,
           students[lowestIndex].percentage);

    printf("====================================\n");
}

int main(void) {
    int choice;

    loadRecords();

    printf("====================================\n");
    printf("  EXAM MARKS AUDIT & RESULT ANALYZER\n");
    printf("====================================\n");
    printf("Loaded records: %d\n", studentCount);

    do {
        printf("\n1. Add Student Result\n");
        printf("2. Display All Results\n");
        printf("3. Search Student by Roll Number\n");
        printf("4. Class Result Summary\n");
        printf("5. Exit\n");

        choice = readInt("Enter your choice (1-5): ", 1, 5);

        switch (choice) {
            case 1:
                addStudent();
                break;

            case 2:
                displayAll();
                break;

            case 3:
                searchStudent();
                break;

            case 4:
                classSummary();
                break;

            case 5:
                printf("\nThank you for using the analyzer!\n");
                break;
        }
    } while (choice != 5);

    return 0;
}