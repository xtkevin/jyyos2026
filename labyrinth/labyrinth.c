#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"
#include <getopt.h>

// main: 程序入口 —— 程序本身无状态, 每次运行对应一次"玩家操作"。
// 整体流程: 解析参数 -> 校验参数组合 -> 加载并检查地图 -> 执行操作 -> 返回退出码。
// 全程遵守 UNIX 约定: 成功返回 EXIT_SUCCESS(0), 任何非法输入返回 EXIT_FAILURE(1)。
int main(int argc, char *argv[]) {
    printUsage();
    // ---- 第 1 部分: 解析命令行参数 ----
    // 解析结果先存进这几个变量 (NULL 表示"命令行里没出现这个选项")
    const char *mapFile = NULL;    // --map / -m 的参数
    const char *playerArg = NULL;  // --player / -p 的参数 (注意: 是字符串, 例如 "1")
    const char *moveDir = NULL;    // --move 的参数
    bool hasVersion = false;       // 是否出现了 --version
    struct option long_options[] = {
        {"map", required_argument, NULL, 'm'},
        {"player", required_argument, NULL, 'p'},
        {"move", required_argument, NULL, 'o'},
        {"version", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    // TODO(1): 从 argv[1] 开始逐个识别选项, 把参数值填进上面的变量。
    //   * "-m" 与 "--map" 等价, "-p" 与 "--player" 等价, 选项顺序可以互换
    //   * 以下情况直接 return EXIT_FAILURE:
    //       - 选项缺少参数 (如 "--map" 恰好是最后一个参数)
    //       - 不认识的选项 (如 "--nonexist") 或多余的操作数 (如 "hello os world")
    //   写法二选一: 手写 for 循环 + strcmp, 或 <getopt.h> 的 getopt_long()
    //   (提示: 认出一个"带值选项"后, 下标要多跳一格, 别把参数值当成下一个选项)

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "m:p:o:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'm':
                mapFile = optarg;
                break;
            case 'p':
                playerArg = optarg;
                break;
            case 'o':
                moveDir = optarg;
                break;
            case 'v':
                // long option with flag
                hasVersion = true;
                break;
            default:
                return EXIT_FAILURE;
        }
    }
    // ---- 第 2 部分: 校验参数组合的合法性 ----
    // TODO(2): 按优先级依次判断, 命中哪种非法情况就 return EXIT_FAILURE:
    //   * 只有 --version 单独出现 -> printf("%s\n", VERSION_INFO); return EXIT_SUCCESS;
    //   * --version 与其他任何参数同时出现 -> 失败
    //   * mapFile == NULL 或 playerArg == NULL (缺少必需参数) -> 失败
    //   * 玩家 ID 非法 -> 失败
    //     (提示: playerArg 是字符串, 需先确认 strlen(playerArg) == 1,
    //      再用 isValidPlayer(playerArg[0]); 所以 "--player 12" 也是非法的)
    if(optind < argc){
        return EXIT_FAILURE;
    }

    if(hasVersion && (mapFile != NULL || playerArg != NULL || moveDir != NULL)){
        return EXIT_FAILURE;
    }
    if(hasVersion && mapFile == NULL && playerArg == NULL && moveDir == NULL){
        printf("%s\n", VERSION_INFO);
        return EXIT_SUCCESS;
    }

    if(mapFile == NULL || playerArg == NULL){
        return EXIT_FAILURE;
    }

    if(strlen(playerArg) != 1 || !isValidPlayer(playerArg[0])){
        return EXIT_FAILURE;
    }

    // ---- 第 3 部分: 加载地图并做全局检查 ----
    Labyrinth labyrinth;
    // TODO(3):
    //   * loadMap(&labyrinth, mapFile) 返回 false (文件不存在/格式错/地图过大) -> 失败
    //   * isConnected(&labyrinth) 返回 false (空地不连通) -> 失败
    if(!loadMap(&labyrinth, mapFile)){
        printf("Failed to load map\n");
        return EXIT_FAILURE;
    }
    if(!isConnected(&labyrinth)){
        printf("Map is not connected\n");
        return EXIT_FAILURE;
    }
    
    printMap(&labyrinth);

    // ---- 第 4 部分: 执行本次"玩家操作" ----
    if (moveDir == NULL) {
        // TODO(4a): 不带 --move: 把地图按行原样打印到 stdout
        //   (每行输出 cols 个字符再输出 '\n', 最后一行之后也要有 '\n')
        for(int i = 0; i < labyrinth.rows; i++){
            for(int j = 0; j < labyrinth.cols; j++){
                putchar(labyrinth.map[i][j]);
            }
            putchar('\n');
        }
        return EXIT_SUCCESS;
    } else {
        // TODO(4b): 带 --move: 调用 movePlayer(&labyrinth, playerArg[0], moveDir)
        //   * 返回 true  -> 先 saveMap 写回 mapFile, 再 return EXIT_SUCCESS
        //   * 返回 false (撞墙/撞玩家/方向非法) -> 不能改写地图文件, return EXIT_FAILURE
        if(movePlayer(&labyrinth, playerArg[0], moveDir)){
            if(!saveMap(&labyrinth, mapFile)){
                printf("Failed to save map\n");
                return EXIT_FAILURE;
            }
            printMap(&labyrinth);
            return EXIT_SUCCESS;
        } else {
            printf("Invalid move\n");
            return EXIT_FAILURE;
        }
    }

    // 正常流程在 TODO 里就 return 了; 走到这里说明参数组合没被处理过
    return EXIT_FAILURE;
}
bool printMap(Labyrinth *labyrinth) {
    for(int i = 0; i < labyrinth->rows; i++){
        for(int j = 0; j < labyrinth->cols; j++){
            putchar(labyrinth->map[i][j]);
        }
        putchar('\n');
    }
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

static bool isValidChar(char x){
    if( (x >= '0' && x <= '9') || x == '#' || x == '.'){
        return true;
    }
    
    return false;
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    FILE *file = fopen(filename, "r");
    if(file == NULL){
        return false;
    }
    char line[1024];
    labyrinth->cols = 0;
    labyrinth->rows = 0;
    while(fgets(line, sizeof(line), file) != NULL){
        int col = 0;
        if(labyrinth->rows >= MAX_ROWS){
            fclose(file);
            return false;
        }
        for(int i = 0; line[i] != '\n' && line[i] != '\r' && line[i] != '\0'; i++){
            if(!isValidChar(line[i])){
                fclose(file);
                return false;
            }
            col++;
            if(col > MAX_COLS){
                fclose(file);
                return false;
            }
            labyrinth->map[labyrinth->rows][i] = line[i];
            
        }
        if(labyrinth->cols == 0){
            labyrinth->cols = col;
        }else{
            if(labyrinth->cols != col){
                fclose(file);
                return false;
            }
        }
        labyrinth->rows++;
    }

    fclose(file);

    return labyrinth->rows > 0 && labyrinth->rows <= MAX_ROWS;
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
    Position pos = findPlayer(labyrinth, playerId);
    if(pos.row == -1 || pos.col == -1){
        Position emptyPos = findFirstEmptySpace(labyrinth);
        if(emptyPos.row == -1 || emptyPos.col == -1){
            return false;
        }else{
            labyrinth->map[emptyPos.row][emptyPos.col] = playerId;
            pos.row = emptyPos.row;
            pos.col = emptyPos.col;
        }
    }
    int newRow = pos.row;
    int newCol = pos.col;
    if(strcmp(direction, "up") == 0){
        newRow--;
    }else if(strcmp(direction, "down") == 0){
        newRow++;
    }else if(strcmp(direction, "left") == 0){
        newCol--;
    }else if(strcmp(direction, "right") == 0){
        newCol++;
    }else{
        return false;
    }
    if(isEmptySpace(labyrinth, newRow, newCol)){
        labyrinth->map[pos.row][pos.col] = '.';
        labyrinth->map[newRow][newCol] = playerId;
        return true;
    }
    
    
    return false;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    FILE *file = fopen(filename, "w");
    if(file == NULL){
        return false;
    }

    for(int i = 0; i < labyrinth->rows; i++){
        for(int j = 0; j < labyrinth->cols; j++){
            fputc(labyrinth->map[i][j], file);
        }
        fputc('\n', file);
    }

    fclose(file);
    return true;
}

// dfs: 从 (row, col) 出发做深度优先遍历 (即"泛洪填充", 相当于画图软件的油漆桶),
// 把所有与起点连通的可走格子都标记到 visited 数组里。
// 可走 = '.' 或玩家数字 ('0'~'9'); '#' 是墙, 不可走。
//
// 注意: 虽然返回值是 void, 但 visited 是"输出参数":
// 数组形参在 C 里会被编译器自动调整成指针 (等价于 bool (*visited)[MAX_COLS]),
// 所以 dfs 内部对 visited[row][col] 的每一次赋值, 调用者 isConnected 都能看到。
// dfs 不负责"回答问题", 只负责"涂色"; 答案要在它返回之后, 由调用者读 visited 得到。
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    // TODO: 依次处理:
    //   1. 越界 / visited 里已标记 / 是墙 '#' -> 直接 return
    //   2. 否则标记 visited[row][col] = true
    //   3. 对 (row-1,col) (row+1,col) (row,col-1) (row,col+1) 四个方向递归调用 dfs
    visited[row][col] = true;
    if(row > 0 && !visited[row - 1][col] && (isEmptySpace(labyrinth, row - 1, col) || isValidPlayer(labyrinth->map[row - 1][col]))){
        dfs(labyrinth, row - 1, col, visited);
    }
    if(row < labyrinth->rows - 1 && !visited[row + 1][col] && (isEmptySpace(labyrinth, row + 1, col) || isValidPlayer(labyrinth->map[row + 1][col]))){
        dfs(labyrinth, row + 1, col, visited);
    }
    if(col > 0 && !visited[row][col - 1] && (isEmptySpace(labyrinth, row, col - 1) || isValidPlayer(labyrinth->map[row][col - 1]))){
        dfs(labyrinth, row, col - 1, visited);
    }
    if(col < labyrinth->cols - 1 && !visited[row][col + 1] && (isEmptySpace(labyrinth, row, col + 1) || isValidPlayer(labyrinth->map[row][col + 1]))){
        dfs(labyrinth, row, col + 1, visited);
    }
}

// isConnected: 检查迷宫里所有可走格子是否互相连通 (题目要求: 玩家也算空地)。
// 思路 (洪水填充):
//   1. 在栈上声明 bool visited[MAX_ROWS][MAX_COLS] 并清零
//      (用 = {false} 初始化或 memset 都可以)
//   2. 扫描全图, 找到任意一个可走格子作为起点
//   3. 从起点调用一次 dfs -- 它返回后, visited 里为 true 的格子
//      恰好就是"从起点出发能走到的全部格子"
//   4. 再扫一遍全图: 只要发现某个可走格子没有被标记 -> 不连通, return false
//   5. 所有可走格子都被标记过 -> return true
bool isConnected(Labyrinth *labyrinth) {
    // TODO: 按上面的注释实现
    for(int i = 0; i < labyrinth->rows; i++){
        for(int j = 0; j < labyrinth->cols; j++){
            if(isEmptySpace(labyrinth, i, j) || isValidPlayer(labyrinth->map[i][j])){
                bool visited[MAX_ROWS][MAX_COLS] = {false};
                dfs(labyrinth, i, j, visited);
                for(int x = 0; x < labyrinth->rows; x++){
                    for(int y = 0; y < labyrinth->cols; y++){
                        if((isEmptySpace(labyrinth, x, y) || isValidPlayer(labyrinth->map[x][y])) && !visited[x][y]){
                            return false;
                        }
                    }
                }
                return true;
            }
        }
    }

    return false;
}
