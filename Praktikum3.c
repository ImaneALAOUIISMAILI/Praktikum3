#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SENSOR_ID 255

typedef struct {
    int warning_low;
    int warning_high;
    int alarm_low;
    int alarm_high;
} SensorThresholds;

typedef struct {
    int sensor_id;
    int measured_value;
} Measurement;

typedef struct {
    int alert_low_count;
    int alert_high_count;
    int alarm_low_count;
    int alarm_high_count;
} Frequency;

int lineNumber = 0;
int counter = 0;

// Funktion zum Vergleichen von zwei Integer-Werten für qsort
int compare_int(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int mainFilter(int values[], int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum / size;
}

int medianFilter(int values[], int size) {
    qsort(values, size, sizeof(int), compare_int);
    return values[size / 2];
}

void evaluate_process_data(const char *file_path, int filter_type) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        exit(EXIT_FAILURE);
    }

    Frequency sensor_frequency[MAX_SENSOR_ID + 1] = {0};
    Measurement last_three_values[MAX_SENSOR_ID + 1][3];
    // Initialisierung des Arrays mit -1
    for (int i = 0; i <= MAX_SENSOR_ID; i++) {
        for (int j = 0; j < 3; j++) {
            last_three_values[i][j].sensor_id = -1;
            last_three_values[i][j].measured_value = -1;
    }
}

    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *token = strtok(line, ";");
        int sensor_id, measured_value, warning_low, warning_high, alarm_low, alarm_high;

        sensor_id = atoi(token);
        token = strtok(NULL, ";");
        measured_value = atoi(token);
        token = strtok(NULL, ";");
        warning_low = atoi(token);
        token = strtok(NULL, ";");
        warning_high = atoi(token);
        token = strtok(NULL, ";");
        alarm_low = atoi(token);
        token = strtok(NULL, ";");
        alarm_high = atoi(token);

        // Anwenden des angegebenen Filters in R6.
        int index = sensor_id;
        last_three_values[index][0] = last_three_values[index][1];
        last_three_values[index][1] = last_three_values[index][2];
        last_three_values[index][2].sensor_id = sensor_id;
        last_three_values[index][2].measured_value = measured_value;

        int filtered_value;

        if (filter_type == 1) { // ungefiltert
            filtered_value = measured_value;
        } else if (filter_type == 2) { // Mittelwertfilter
            if (last_three_values[index][0].measured_value == -1 && last_three_values[index][1].measured_value == -1 ){
                filtered_value = (last_three_values[index][2].measured_value);
            }
            else if (last_three_values[index][0].measured_value == -1){
                int values[2] = {last_three_values[index][1].measured_value,
                last_three_values[index][2].measured_value};
                filtered_value = mainFilter(values, 2);
                //filtered_value = (last_three_values[index][1].measured_value +
                    //last_three_values[index][2].measured_value ) / 2;
            }
            else {
                int values[3] = {last_three_values[index][0].measured_value,
                last_three_values[index][1].measured_value,
                last_three_values[index][2].measured_value};
                filtered_value = mainFilter(values, 3);
            }

        } else if (filter_type == 3) { // Medianfilter
            if (last_three_values[index][0].measured_value == -1 && last_three_values[index][1].measured_value == -1 ){
            filtered_value = (last_three_values[index][2].measured_value);
            }
            else if (last_three_values[index][0].measured_value == -1){
            int values[2] = {last_three_values[index][1].measured_value,
            last_three_values[index][2].measured_value};
            filtered_value = medianFilter(values, 2);
            }
            else {
            int values[3] = {last_three_values[index][0].measured_value,
                             last_three_values[index][1].measured_value,
                             last_three_values[index][2].measured_value};
            filtered_value = medianFilter(values, 3);
            }
        }
        // printf("In Zeile %d der gefilterte Wert ist: %d\n", lineNumber, filtered_value);
        // last_three_values[index][2].measured_value = filtered_value;

        // Überprüfen der Voraussetzungen von R1
        if (filtered_value >= warning_high && filtered_value < alarm_high) {
            sensor_frequency[sensor_id].alert_high_count++;
        } else if (filtered_value >= alarm_high) {
            sensor_frequency[sensor_id].alarm_high_count++;
        } else if (filtered_value <= warning_low && filtered_value > alarm_low) {
            sensor_frequency[sensor_id].alert_low_count++;
        } else if (filtered_value <= alarm_low) {
            sensor_frequency[sensor_id].alarm_low_count++;
        } else {
            // printf("In Zeile %d keine Counterhöhung\n", lineNumber);
            counter++;
        }
        lineNumber++;
    }

    fclose(file);

    // Ausgabe der Ergebnisse
    for (int i = 0; i < MAX_SENSOR_ID; i++) {
        if (sensor_frequency[i].alert_low_count || sensor_frequency[i].alert_high_count ||
            sensor_frequency[i].alarm_low_count || sensor_frequency[i].alarm_high_count) {
            printf("SensorID %d: Warnung LOW %d, Warnung HIGH %d, Alarm LOW %d, Alarm HIGH %d\n",
                   i, sensor_frequency[i].alert_low_count, sensor_frequency[i].alert_high_count,
                   sensor_frequency[i].alarm_low_count, sensor_frequency[i].alarm_high_count);
        }
    }
    //printf("Es wurde %d-Mal keine Counterhöhung gefunden\n", counter);
}
int main() {
    const char *file_path = "processData.txt";
    int filter_type;
    printf("Bitte Filtertype wählen (1 für ungefiltert, 2 für Mittelwertfilter, 3 für Medianfilter): ");
    scanf("%d", &filter_type);
    if (filter_type < 1 || filter_type > 3) {
        printf("Ungültige Filteroption. Programm wird beendet.\n");
        return EXIT_FAILURE;
    }
    evaluate_process_data(file_path, filter_type);

    return 0;
}