#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"

int main(int argc, char *argv[]) {
    // TODO: Implement this function
    return 0;
}

void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    // TODO: Implement this function
    if(playerId >= '0' && playerId <= '9'){
        return true;
    }

    return false;
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        perror("文件打开失败");
        return false;
    }
    char line[1024];
    labyrinth->cols = 0;
    labyrinth->rows = 0;
    while(fgets(line, sizeof(line), file) != NULL){
        int col = 0;
        for(int i = 0; line[i] != '\n' && line[i] != '\r' && line[i] != '\0'; i++){
            labyrinth->map[labyrinth->rows][i] = line[i];
            col++;
        }
        if(col > labyrinth->cols){
            labyrinth->cols = col;
        }

        labyrinth->rows++;
    }

    return labyrinth->rows > 0;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    // TODO: Implement this function
    Position pos = {-1, -1};
    int n = labyrinth->rows;
    int m = labyrinth->cols;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            if(labyrinth->map[i][j] == playerId){
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }
    return pos;
}

Position findFirstEmptySpace(Labyrinth *labyrinth) {
    // TODO: Implement this function
    Position pos = {-1, -1};
    int n = labyrinth->rows;
    int m = labyrinth->cols;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            if(labyrinth->map[i][j] == '.'){
                pos.row = i, pos.col = j;
                return pos;
            }
        }
    }
    return pos;
}

bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    // TODO: Implement this function
    int n = labyrinth->rows;
    int m = labyrinth->cols;
    // out boundary
    if(row < 0 || row >= n || col < 0 || col >= m){
        return false;
    }   
    // only . is moveable
    if(labyrinth->map[row][col] == '.'){
        return true;
    }

    return false;
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    // TODO: Implement this function
    return false;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    return false;
}

// Check if all empty spaces are connected using DFS
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    // TODO: Implement this function
}

bool isConnected(Labyrinth *labyrinth) {
    // TODO: Implement this function
    return false;
}
